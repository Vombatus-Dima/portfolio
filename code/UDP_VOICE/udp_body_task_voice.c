/**
  ******************************************************************************
  * @file    udp_body_task_voice.c
  * @author  Trembach D.N.
  * @version V3.1.0
  * @date    18-11-2020
  * @brief   Файл контроля тела задачи UDP шлюза RS/ETH для голосовых потоков
  ******************************************************************************
  * @attention
  * 
  *
  *   
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "lwip/memp.h"
#include <stdio.h>
#include <string.h>
#include "stm32f4x7_eth.h"
#include "FreeRTOS.h"
#include "task.h"
#include "udp_voice.h"
#include "Core.h"    
#include "settings.h"
#include "loader_settings.h"
#include "printf_dbg.h"    
#include "board.h" 
#include "call_gate.h" 
#include "GenerationCRC16.h" 
#include "udp_spam_debug_gate.h" 

#define VoiceSpamGate ((voice_gate_t *)arg)
      
/**
  * @brief Эта функция для отработки внутренних команд задачи 
  * @param arg аргумент пользователя ((voice_gate_t *)arg)
  * @retval None
  */
static void UDP_Local_CMD_Processing(void *arg)
{
  uint16_t     size_box;
  dev_addr_t   dev_addr;
  /* Если очередь команд открыта */
  if (VoiceSpamGate->xQueueCMD != NULL)
  {
    /* Проверка очереди команд */
    if ( xQueueReceive( VoiceSpamGate->xQueueCMD, &(VoiceSpamGate->BufCMD), ( TickType_t ) 0 ) == pdTRUE )
    {
      /* Получена команда - запуск обработки */
      if ( ( VoiceSpamGate->BufCMD.CMD_ID == CMD_MASK_CODE_CH_RS_UPDATE ) || ( VoiceSpamGate->BufCMD.CMD_ID == CMD_MASK_CODE_CH_CODEC_UPDATE ) )
      {
        /* Обновление маски каналов RS                                                    */
        if ( VoiceSpamGate->BufCMD.CMD_ID == CMD_MASK_CODE_CH_RS_UPDATE )
        {
          VoiceSpamGate->table_spam->RSMaskChCoded = VoiceSpamGate->BufCMD.data_dword[0]; 
        }
        
        /* Обновление маски каналов кодека */
        if ( VoiceSpamGate->BufCMD.CMD_ID == CMD_MASK_CODE_CH_CODEC_UPDATE )
        {
          VoiceSpamGate->table_spam->CodecMaskChCoded = VoiceSpamGate->BufCMD.data_dword[0]; 
        }             
        
        /* Заполнение актуальной маски прослушки сжатых каналов */   
        VoiceSpamGate->table_spam->MaskChCoded = VoiceSpamGate->table_spam->CodecMaskChCoded | VoiceSpamGate->table_spam->RSMaskChCoded;    
        
        /* Проверка наличия появившихся каналов на прослушке */
        if ( ( VoiceSpamGate->BufCMD.data_dword[1] ) > 0)
        {
          /*================================================================================*/ 
          /* Функция формирования пакета запроса таблицы рассылки                           */
          size_box = ReqTableSpam( VoiceSpamGate->table_spam, (void*)&(VoiceSpamGate->buf_eth_rtr), REQ_CON_TIME_NO_GET, 0); 
          /*================================================================================*/  
          /* Рассылка по ETH  пакета всем слушателям канала о появлении нового слушателя    */
          while(GetAddrTable( VoiceSpamGate->table_spam, VoiceSpamGate->BufCMD.data_dword[1] , 0x00000000, TypeBitID(TYPE_PDH)|TypeBitID(TYPE_VRM)|TypeBitID(TYPE_PIETH)|TypeBitID(TYPE_GATE), TYPE_ADD_SPAM, 0 , &dev_addr )) 
          {
#ifdef UDP_UPDATE_LISTING_DEBUG  
            printf( "|<< SPAM_UP___CH 0x%.8x|%.3d|%.3d|%.3d|%.3d|%.5d|\r\n",
                   VoiceSpamGate->BufCMD.data_dword[1],
                   dev_addr.BtIPadr[0],
                   dev_addr.BtIPadr[1],
                   dev_addr.BtIPadr[2],
                   dev_addr.BtIPadr[3],
                   dev_addr.IPport);
#endif            
            /* Рассылка пакета всем слушателям появившегося канала  */
            UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_eth_rtr) , size_box , &(VoiceSpamGate->udp_soc), dev_addr.IPaddr , dev_addr.IPport );
          } 
        }
      }
      /*================================================================================*/           
#if ( ROUTER_SOFT_ENABLE == 1 )    
      /*================================================================================*/           
      /* Получена команда - запуск обработки */
      if ( VoiceSpamGate->BufCMD.CMD_ID == CMD_MASK_ENCODE_CH_UPDATE )
      {
        /* Заполнение актуальной маски прослушки не сжатых каналов */   
        VoiceSpamGate->table_spam->MaskChEnCoded = VoiceSpamGate->BufCMD.data_dword[0];
        
        /* Проверка наличия появившихся каналов на прослушке */
        if ( ( VoiceSpamGate->BufCMD.data_dword[1] ) > 0)
        {
          /*================================================================================*/  
          /*================================================================================*/  
          /* Рассылка по ETH  пакета всем слушателям канала о появлении нового слушателя    */
          while(GetAddrTable( VoiceSpamGate->table_spam, 0x00000000, VoiceSpamGate->BufCMD.data_dword[1], TypeBitID(TYPE_REC), 0xFF, 0 , &dev_addr )) 
          {  
#ifdef UDP_UPDATE_LISTING_DEBUG  
            printf( "|<< SPAM_UP_ENCH 0x%.8x|%.3d|%.3d|%.3d|%.3d|%.5d|\r\n",
                   VoiceSpamGate->BufCMD.data_dword[1],
                   dev_addr.BtIPadr[0],
                   dev_addr.BtIPadr[1],
                   dev_addr.BtIPadr[2],
                   dev_addr.BtIPadr[3],
                   dev_addr.IPport);
#endif  
            /* Рассылка пакета всем слушателям появившегося канала  */
            UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_eth_rtr) , size_box , &(VoiceSpamGate->udp_soc), dev_addr.IPaddr , dev_addr.IPport );
          } 
        }
      }          
      /*================================================================================*/   
#endif /* ROUTER_SOFT_ENABLE == 1 */ 
      /*================================================================================*/           
    }
  } 
}

/**
  * @brief Эта функция для анализа буфера несжатого звука и передачи пакетов в UDP 
  * @param arg аргумент пользователя ((voice_gate_t *)arg)
  * @retval None
  */
static void UDP_EncodeVoiceTransfer(void *arg)
{
#if ( ROUTER_SOFT_ENABLE == 1 ) 
  dev_addr_t   dev_addr; 
  /*================================================================================*/    
  /*======================= обслуживание не сжатых пакетов  ========================*/
  /*================================================================================*/ 
  /* Читаем и отправляем все пакеты что находятся в буфере */
  do
  {
    /* Получение указателя на пакета для чтения */
    VoiceSpamGate->pfifo_box = GetRDPointFIFO ( &(VoiceSpamGate->index_rd_box), VoiceSpamGate->mask_chanel, VoiceSpamGate->mask_index_port );
    /* Если указатель корректный */      
    if ( (VoiceSpamGate->pfifo_box) != NULL )
    { 
      /* Вещаем по всем выданным на запросу адресам */
      while(GetAddrTable( VoiceSpamGate->table_spam, 0x00000000 , BitChID(VoiceSpamGate->pfifo_box->data_box.ch_id), TypeBitID(TYPE_REC), 0xFF, 0, &dev_addr )) 
      {  
#ifdef UDP_TX_ENCODE_VOICE_DEBUG                                              
        printf( "|<< SEND_SFT_BOX 0x%.4x|%.3d|%.3d|%.3d|%.3d|%.5d|%.3d|\r\n",
               VoiceSpamGate->pfifo_box->data_box.src_phy_addr,
               dev_addr.BtIPadr[0],
               dev_addr.BtIPadr[1],
               dev_addr.BtIPadr[2],
               dev_addr.BtIPadr[3],
               dev_addr.IPport,
               VoiceSpamGate->pfifo_box->data_box.ch_id );
#endif    
        /* Рассылка пакета всем слушателям канала  */
        UDP_Send_Socket( (void*)(VoiceSpamGate->pfifo_box) , sizeof(sample_voice_box_t) , &(VoiceSpamGate->udp_soc), dev_addr.IPaddr , dev_addr.IPport );
        /* подсчет переданных пакетов */
        (VoiceSpamGate->cnt_tx_soft_box)++;  
      }
    }
  }while((VoiceSpamGate->pfifo_box) != NULL);   
  /*================================================================================*/  
#endif /* ROUTER_SOFT_ENABLE == 1 */  
}

/**
  * @brief Эта функция периодического запроса таблицы рассылки 
  * @param arg аргумент пользователя ((voice_gate_t *)arg)
  * @retval None
  */
static void UDP_ProcessingTableSpam(void *arg)
{
  uint16_t     size_box;
  
  /* Сервера рассылки запрашиваем по очереди */
  if ( VoiceSpamGate->table_spam->flag_req_table_spam > 0 )
  {
    /* Запрос основного сервера рассылки */
    VoiceSpamGate->table_spam->flag_req_table_spam = 0;
    
    /* Если адрес основного сервера не соответствует собственному - отправляем запрос */
    if (VoiceSpamGate->table_spam->sever_table_spam_main.IPaddr != VoiceSpamGate->table_spam->ReqIPaddr )
    {
    /* Функция формирования пакета запроса таблицы рассылки                           */
  if ( ( ( VoiceSpamGate->table_spam->TypeDev ) & (TYPE_ADD_TBL) ) != 0 )
    {
      /* Если это сервер рассылки - не запрашиваем таблицу */
        size_box = ReqTableSpam( VoiceSpamGate->table_spam, (void*)&(VoiceSpamGate->buf_eth_rtr), REQ_CON_SERVER_GET_TABL, 0 ); 
    }
    else
    {
      /* Если не сервер рассылки - запрашиваем таблицу */
        size_box = ReqTableSpam( VoiceSpamGate->table_spam, (void*)&(VoiceSpamGate->buf_eth_rtr), REQ_CON_TIME_GET_TABL, 0 ); 
    } 
      
#if ( UDP_SPAM_DEBUG == 1 )   
      if ( ( ( VoiceSpamGate->table_spam->TypeDev ) & (TYPE_ADD_TBL) ) != 0 )
      {                                                         
        printf( "|>> REQ_CON_SERVER_GET_TABL   |%.3d|%.3d|%.3d|%.3d|%.5d|\r\n",
               VoiceSpamGate->table_spam->sever_table_spam_main.BtIPadr[0],
               VoiceSpamGate->table_spam->sever_table_spam_main.BtIPadr[1],
               VoiceSpamGate->table_spam->sever_table_spam_main.BtIPadr[2],
               VoiceSpamGate->table_spam->sever_table_spam_main.BtIPadr[3],
               VoiceSpamGate->table_spam->sever_table_spam_main.IPport );
      }
      else
      {                                                           
        printf( "|>> REQ_CON_TIME_GET_TABL     |%.3d|%.3d|%.3d|%.3d|%.5d|\r\n",
               VoiceSpamGate->table_spam->sever_table_spam_main.BtIPadr[0],
               VoiceSpamGate->table_spam->sever_table_spam_main.BtIPadr[1],
               VoiceSpamGate->table_spam->sever_table_spam_main.BtIPadr[2],
               VoiceSpamGate->table_spam->sever_table_spam_main.BtIPadr[3],
               VoiceSpamGate->table_spam->sever_table_spam_main.IPport );
      }  
#endif 
      UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_eth_rtr) , size_box , &(VoiceSpamGate->udp_soc), VoiceSpamGate->table_spam->sever_table_spam_main.IPaddr ,VoiceSpamGate->table_spam->sever_table_spam_main.IPport);
    }
  }
  else
  {
    /* Запрос резервного сервера рассылки */  
    VoiceSpamGate->table_spam->flag_req_table_spam = 1;    
    
    /* Если адрес резервного сервера не соответствует собственному - отправляем запрос */        
    if (VoiceSpamGate->table_spam->sever_table_spam_rezerv.IPaddr != VoiceSpamGate->table_spam->ReqIPaddr )
    {
      /* Функция формирования пакета запроса таблицы рассылки                           */
      if ( ( ( VoiceSpamGate->table_spam->TypeDev ) & (TYPE_ADD_TBL) ) != 0 )
      {
        /* Если это сервер рассылки - не запрашиваем таблицу */
        size_box = ReqTableSpam( VoiceSpamGate->table_spam, (void*)&(VoiceSpamGate->buf_eth_rtr), REQ_CON_SERVER_GET_TABL, 0 ); 
      }
      else
      {
        /* Если не сервер рассылки - запрашиваем таблицу */
        size_box = ReqTableSpam( VoiceSpamGate->table_spam, (void*)&(VoiceSpamGate->buf_eth_rtr), REQ_CON_TIME_GET_TABL, 0 ); 
      }        
      
#if ( UDP_SPAM_DEBUG == 1 )  
      if ( ( ( VoiceSpamGate->table_spam->TypeDev ) & (TYPE_ADD_TBL) ) != 0 )
      {
        printf( "|>> REQ_CON_SERVER_GET_TABL   |%.3d|%.3d|%.3d|%.3d|%.5d|\r\n",
               VoiceSpamGate->table_spam->sever_table_spam_rezerv.BtIPadr[0],
               VoiceSpamGate->table_spam->sever_table_spam_rezerv.BtIPadr[1],
               VoiceSpamGate->table_spam->sever_table_spam_rezerv.BtIPadr[2],
               VoiceSpamGate->table_spam->sever_table_spam_rezerv.BtIPadr[3],
               VoiceSpamGate->table_spam->sever_table_spam_rezerv.IPport );
      }
      else
      {
        printf( "|>> REQ_CON_TIME_GET_TABL     |%.3d|%.3d|%.3d|%.3d|%.5d|\r\n",
               VoiceSpamGate->table_spam->sever_table_spam_rezerv.BtIPadr[0],
               VoiceSpamGate->table_spam->sever_table_spam_rezerv.BtIPadr[1],
               VoiceSpamGate->table_spam->sever_table_spam_rezerv.BtIPadr[2],
               VoiceSpamGate->table_spam->sever_table_spam_rezerv.BtIPadr[3],
               VoiceSpamGate->table_spam->sever_table_spam_rezerv.IPport );
      }  
#endif 
      UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_eth_rtr) , size_box , &(VoiceSpamGate->udp_soc), VoiceSpamGate->table_spam->sever_table_spam_rezerv.IPaddr ,VoiceSpamGate->table_spam->sever_table_spam_rezerv.IPport);
    }
}
}

#if ( TEST_REQ_TABL_SPAM_ENABLE == 1)
#define TEST_MASK_SPAM   (0x001F)                
/**
  * @brief Эта функция тестирования периодического запроса таблицы рассылки 
  * @param arg аргумент пользователя ((voice_gate_t *)arg)
  * @retval None
  */
static void UDPTestReqTableSpam(void *arg)
{
  if ( ( ( (uint16_t)(VoiceSpamGate->table_spam->ConticExportBox)) & TEST_MASK_SPAM ) == 0)
  {
  /* Копирование собственных данных идентификации устройства */  
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.dev_addr_array[0].ReqIPaddr       = 0x5D01A8C0;
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.dev_addr_array[0].ReqIPport       = VoiceSpamGate->table_spam->ReqIPport;	
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.dev_addr_array[0].PhyAddr         = ( ( (uint16_t)(VoiceSpamGate->table_spam->ConticExportBox)) & TEST_MASK_SPAM ) + 1;	
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.dev_addr_array[0].TypeDev         = TYPE_POZ;     
  }
  else
  {
  /* Копирование собственных данных идентификации устройства */  
    VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.dev_addr_array[0].ReqIPaddr       = ( ( VoiceSpamGate->table_spam->ReqIPaddr ) & 0x00FFFFFF ) + ( ( ( ( (uint32_t)(VoiceSpamGate->table_spam->ConticExportBox) ) & TEST_MASK_SPAM ) + 1 ) << 24);
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.dev_addr_array[0].ReqIPport       = VoiceSpamGate->table_spam->ReqIPport;	
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.dev_addr_array[0].PhyAddr         = ( ( (uint16_t)(VoiceSpamGate->table_spam->ConticExportBox)) & TEST_MASK_SPAM ) + 1;	
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.dev_addr_array[0].TypeDev         = TYPE_VRM | TYPE_ADD_SPAM;        
  }  
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.dev_addr_array[0].NGroup          = 0;    
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.dev_addr_array[0].PriorityInGroup = 0;
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.dev_addr_array[0].ModeRign        = 0;
  
  /* Если режим подчиненного - запрос на рассылку голоса не активируем */
  
//  if ( ( ((uint32_t)VoiceSpamGate->table_spam->ConticExportBox)&0x0000001 ) != 0 )
//  { 
//    VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.dev_addr_array[0].MaskChCoded     = 0;  
//  }
//  else
  {
    VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.dev_addr_array[0].MaskChCoded  = (((uint32_t)VoiceSpamGate->table_spam->ConticExportBox)&0x000003FF);    
  }  
  
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.dev_addr_array[0].MaskChEnCoded   = 0;//VoiceSpamGate->table_spam->MaskChEnCoded;
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.cmd_spam = REQ_CON_TIME_NO_GET;                        /* Команда управления таблицей                      */
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.status_cmd_spam = 0;                                   /* Статус выполнения команды                        */ 
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.rezerv = 0;                                            /* Резерв                                           */   
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.number_dev_addr = 1;                                   /* Число адресов в пакете                           */       
  
  /* Формирование пакета                        */
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.rs_head.pre = 0xAA55;                                  /* Преамбула  0x55 0xAA                             */
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.rs_head.id = ID_CMD_LOCAL_REQ;                         /* ID ответ команды управления локальными ресурсами */
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.rs_head.dest = 0x0000;                                 /* Локальный адрес получателя                       */
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.rs_head.src = VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.dev_addr_array[0].PhyAddr;     /* Устанавливаем свой физический адрес источника    */ 
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.rs_head.reserv = 0x00;      
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.rs_head.status_box = 0x00;  
  
  /* Формирование счетчика неприрывности */
  if (VoiceSpamGate->table_spam->ConticExportBox > 254) VoiceSpamGate->table_spam->ConticExportBox = 0;
  else  (VoiceSpamGate->table_spam->ConticExportBox)++;
  
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.rs_head.cnt = VoiceSpamGate->table_spam->ConticExportBox;               /* Cчетчик неприрывности пакетов 0..255             */ 
  /* Вычисление длинны пакета */
  VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.rs_head.lenght = 0x000A + sizeof(spam_dev_addr_t) + 4;        

  VoiceSpamGate->buf_ip_eth_rtr.IPaddr_reception = VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.dev_addr_array[0].ReqIPaddr;
  VoiceSpamGate->buf_ip_eth_rtr.IPport_reception = VoiceSpamGate->buf_ip_eth_rtr.buf_reception_spam.dev_addr_array[0].ReqIPport;  
    
  /* отправка пакета на регистрацию */         
  xQueueSend ( VoiceSpamGate->QueueReception, (void*)&(VoiceSpamGate->buf_ip_eth_rtr) , ( TickType_t ) 0 );
  /* Отправляем сообщение для задачи контроля шлюза */
  if ( VoiceSpamGate->udp_soc.HandleTask != NULL )
  {
    xTaskNotify( VoiceSpamGate->udp_soc.HandleTask,    /* Указатель на уведомлюемую задачу                         */
                RECEIVING_NOTE,                        /* Значения уведомления                                     */
                eSetBits );                            /* Текущее уведомление добавляются к уже прописанному       */
  } 
}
#endif

/**
  * @brief Эта функция обновления таблицы шлюзов кольца и формирования пакета тестироования магистрали RS/ETH  
  * @param arg аргумент пользователя ((voice_gate_t *)arg)
  * @retval None
  */
static void UDP_Update_Ring_Table(void *arg)
{
  uint16_t     size_box;
  dev_addr_t   dev_addr;
  
  /*========================================================================================================*/      
  /*==== Функция обновления таблицы шлюзов кольца и формирования пакета тестироования магистрали RS/ETH ====*/
  /*========================================================================================================*/
  /* обновляем таблицу - получаем пакет для тестирования */
  size_box = UpdateTableRing( VoiceSpamGate->cntrl_ring, &(VoiceSpamGate->buf_highway_eth_rtr) , &(VoiceSpamGate->mode_ring_notify) );
  /* если данные для тестирования сформированы - рассылаем пакет тестирования */
  if ( size_box > 0 )
  {
    /* Анализируем куда отправлять */
    if ( VoiceSpamGate->buf_eth_rtr.id == ID_DIAG_HIGHWAY_RS_REQ )
    { 
      /* Отправляетм пакет в роутер для тестироования магистрали RS */
      if ( VoiceSpamGate->set_port_router.QueueInRouter != NULL )
      {
        xQueueSend ( VoiceSpamGate->set_port_router.QueueInRouter, &(VoiceSpamGate->buf_eth_rtr ) , ( TickType_t ) 0 );
        if ( ( VoiceSpamGate->set_port_router.HandleTaskRouter ) != NULL )
        {
          xTaskNotify( VoiceSpamGate->set_port_router.HandleTaskRouter, /* Указатель на уведомлюемую задачу                         */
                      ( VoiceSpamGate->set_port_router.PortBitID ) << OFFSET_NOTE_PORD_ID, /* Значения уведомления                                     */
                      eSetBits );                                     /* Текущее уведомление добавляются к уже прописанному       */
        } 
      } 
    }
    else
    {
      /* Рассылка по ETH  пакета тестироования магистрали ETH       */
      while(GetAddrRingTable( VoiceSpamGate->table_spam, VoiceSpamGate->cntrl_ring->own_group, TypeBitID(TYPE_VRM)|TypeBitID(TYPE_PIETH)|TypeBitID(TYPE_GATE), TYPE_ADD_SPAM, &dev_addr )) 
      {  
#ifdef UDP_RING_DEBUG  
        printf( "|>> TEST_ETH_WAY 0x%.4x|%.3d|%.3d|%.3d|%.3d|%.5d|%.3d|\r\n",
               VoiceSpamGate->table_spam->own_dev_addr.PhyAddr,
               dev_addr.BtIPadr[0],
               dev_addr.BtIPadr[1],
               dev_addr.BtIPadr[2],
               dev_addr.BtIPadr[3],
               dev_addr.IPport,
               VoiceSpamGate->buf_highway_eth_rtr.number_nodes_heard );
#endif            
        /* Отправка пакета тестирования по заданному адресу */
        UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_eth_rtr) , size_box , &(VoiceSpamGate->udp_soc), dev_addr.IPaddr , dev_addr.IPport );
      } 
    }  
  }
  
  /*======================= Разослать всем что изменился режим кольца на мастер ===============================*/        
  if ( VoiceSpamGate->mode_ring_notify > 0)
  {
    /* Сброс флага изменения состояния */
    VoiceSpamGate->mode_ring_notify = 0;
    
    /* Функция формирования пакета запроса таблицы рассылки                           */
    size_box = ReqTableSpam( VoiceSpamGate->table_spam, (void*)&(VoiceSpamGate->buf_eth_rtr), REQ_CON_TIME_NO_GET, 0 ); 
    
    /* Рассылка по ETH  пакета всем слушателям канала о появлении нового слушателя/ мастера в кольце */
    while(GetAddrTable( VoiceSpamGate->table_spam, VoiceSpamGate->table_spam->MaskChCoded , VoiceSpamGate->table_spam->MaskChEnCoded  , TypeBitID(TYPE_REC), 0xFF, 0 , &dev_addr )) 
    {
#ifdef UDP_UPDATE_LISTING_DEBUG  
      printf( "|<< SPAM_UP_RING 0x%.8x|%.3d|%.3d|%.3d|%.3d|%.5d|\r\n",
             VoiceSpamGate->BufCMD.data_dword[1],
             dev_addr.BtIPadr[0],
             dev_addr.BtIPadr[1],
             dev_addr.BtIPadr[2],
             dev_addr.BtIPadr[3],
             dev_addr.IPport);
#endif            
      /* Рассылка пакета всем слушателям каналов которые активные на данном модуле */
      UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_eth_rtr) , size_box , &(VoiceSpamGate->udp_soc), dev_addr.IPaddr , dev_addr.IPport );
    } 
  }
}

/**
  * @brief Эта функция обработки очередь приема пакетов из callbaca ETH открыта
  * @param arg аргумент пользователя ((voice_gate_t *)arg)
  * @retval None
  */
static void UDP_Processing_Queue_Reception(void *arg)
{
  uint16_t     size_box;
  dev_addr_t   dev_addr;  
#if ( UDP_SPAM_DEBUG == 1 )   
  char STypeReq[] = "REQ_CON_NOTIME_STATUS_DEV    ";
#endif    
  /*================================================================================*/    
  /*===============Если очередь приема пакетов из callbaca ETH открыта==============*/
  /*================================================================================*/  
  if (VoiceSpamGate->QueueReception != NULL) 
  {
    /* Проверяем наличие входящих пакетов */
    while(xQueueReceive(VoiceSpamGate->QueueReception , VoiceSpamGate->pdamp_buf_rtr_eth , ( TickType_t ) 0) == pdTRUE)
    {
      /* Отправляетм пакет в задачу контроля соединения */
      if ( VoiceSpamGate->buf_ip_rtr_eth.buf_reception_eth_call.pre == CODE_PREAM  )
      {
        /*================================================================================================*/
        
        /*  Обработка пакетов c преамбулой 0xEDB7 полученных по рассылки из callbaca ETH принятых по UDP  */
        
        /*================================================================================================*/
        if ( VoiceSpamGate->buf_ip_rtr_eth.buf_reception_eth_call.flag_rec_to_eth == 1 ) 
        {
          /* Получили пакет от рекодера  транслируем всем остальным   */
          /* Транслируем всем кто подписан на сжатый голос            */
          switch(VoiceSpamGate->buf_ip_rtr_eth.buf_reception_eth_call.addr_gate_dst)
          {
          case 0x0000:/* не отправляем никому */ 
            break;
            
          case 0xFFFF:/* отправляем всем      */  
            /* Рассылка по ETH  пакета всем  */
            while(GetAddrTable( VoiceSpamGate->table_spam, 0xFFFFFFFF, 0x00000000, TypeBitID(TYPE_PDH)|TypeBitID(TYPE_VRM)|TypeBitID(TYPE_PIETH)|TypeBitID(TYPE_GATE), TYPE_ADD_SPAM, 1 , &dev_addr )) 
            {  
              /* Рассылка пакета всем слушателям появившегося канала  */
              UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_ip_rtr_eth.buf_reception_eth_call) , sizeof(eth_call_box_t) , &(VoiceSpamGate->udp_soc), dev_addr.IPaddr , dev_addr.IPport );
              /* Диагностическое сообщение */      
              diag_mes_call_eth("<<__ETH__ALL__",&(VoiceSpamGate->buf_ip_rtr_eth.buf_reception_eth_call),dev_addr.PhyAddr);         
            } 
            break;          
          default:    /* отправка только по указанному адресу, если адрес не указан - отправляем на шлюз */ 
            /* Функция запроса адреса из таблицы рассылки по физическому адресу устройства*/
            if (GetPhyAddrTable( VoiceSpamGate->table_spam, VoiceSpamGate->buf_ip_rtr_eth.buf_reception_eth_call.addr_gate_dst, &dev_addr ))
            {
              /* Отправка пакета */
              UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_ip_rtr_eth.buf_reception_eth_call) , sizeof(eth_call_box_t) , &(VoiceSpamGate->udp_soc), dev_addr.IPaddr , dev_addr.IPport );
              /* Диагностическое сообщение */      
              diag_mes_call_eth("<<__ETH__ADDR_",&(VoiceSpamGate->buf_ip_rtr_eth.buf_reception_eth_call),dev_addr.PhyAddr);        
            }    
            break;         
          }
        }
        else
        {
          /* Рассылка по ETH  пакета всем подписаным рекодерам  */
          while(GetAddrTable( VoiceSpamGate->table_spam, 0x00000000, 0xFFFFFFFF, TypeBitID(TYPE_REC), 0xFF, 1 ,&dev_addr )) 
          {  
            /* Рассылка пакета всем слушателям появившегося канала  */
            UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_ip_rtr_eth.buf_reception_eth_call_rec) , sizeof(eth_call_rec_box_t) , &(VoiceSpamGate->udp_soc), dev_addr.IPaddr , dev_addr.IPport );
            /* Диагностическое сообщение */      
            diag_mes_call_eth("<<__ETH_REC___",&(VoiceSpamGate->buf_ip_rtr_eth.buf_reception_eth_call),dev_addr.PhyAddr);             
          } 
        }  
        /* Проверка прохождения шлюзов полученным пакетом - если он уже получен из eth в rs роутер - пакет не обрабатываем */
        if ( VoiceSpamGate->buf_ip_rtr_eth.buf_reception_eth_call.flag_eth_to_rs != 1 )
        {
          /* Установка флага направления прохождения шлюза */
          VoiceSpamGate->buf_ip_rtr_eth.buf_reception_eth_call.flag_eth_to_rs = 1;
          /* Диагностическое сообщение - пакет получен из ETH */
          diag_mes_call_eth(">>__Ethernet__",(void*)&(VoiceSpamGate->buf_eth_call_eth_rtr),0x0000);   
          
          /* Если пакет пришел в шлюз по заданному адресу или адрес широковещательный */
          if ( ( ( VoiceSpamGate->buf_ip_rtr_eth.buf_reception_eth_call.addr_gate_dst ) == DataLoaderSto.Settings.phy_adr) || ( ( VoiceSpamGate->buf_ip_rtr_eth.buf_reception_eth_call.addr_gate_dst ) == 0xFFFF ) )
          {
            /* Получили пакет формата call eth - преобразовали в call rs. */
            call_eth_to_rs( &(VoiceSpamGate->buf_call_eth_rtr), &(VoiceSpamGate->buf_ip_rtr_eth.buf_reception_eth_call) );          
            /* Отправляетм пакет в роутер */
            if (VoiceSpamGate->set_port_router.QueueInRouter != NULL)
            {
              xQueueSend ( VoiceSpamGate->set_port_router.QueueInRouter, &(VoiceSpamGate->buf_call_eth_rtr) , ( TickType_t ) 0 );
              /* Диагностическое сообщение пакет отправлен в роутер */
              diag_mes_call_rs("<<_RS__Router_",&(VoiceSpamGate->buf_call_eth_rtr),0x0000);  
              /* Отправить уведомление в роутер */
              if ( ( VoiceSpamGate->set_port_router.HandleTaskRouter ) != NULL )
              {
                xTaskNotify( VoiceSpamGate->set_port_router.HandleTaskRouter, /* Указатель на уведомлюемую задачу                         */
                            ( VoiceSpamGate->set_port_router.PortBitID ) << OFFSET_NOTE_PORD_ID, /* Значения уведомления                                     */
                            eSetBits );                                     /* Текущее уведомление добавляются к уже прописанному       */
              }
            }
          }
        }
      }
      else
      {
        /* Проверка типа полученого пакета             */
        switch(VoiceSpamGate->buf_ip_rtr_eth.buf_reception_eth.id)
        {
        case ID_DIAG_HIGHWAY_ETH_REQ: /* Запросы команд диагностики магистрали ETH             */ 
          /*  Функция обработки полученного пакета тестирования магистрали ETH */
#ifdef UDP_RING_DEBUG  
          printf( "|<< TEST_ETH_WAY 0x%.4x|%.3d|%.3d|%.3d|%.3d|%.5d|%.3d|\r\n",
                 VoiceSpamGate->buf_ip_rtr_eth.buf_reception_highway.addr_src_unit_gate,
                 VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[0],
                 VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[1],
                 VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[2],
                 VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[3],
                 VoiceSpamGate->buf_ip_rtr_eth.IPport_reception,
                 VoiceSpamGate->buf_ip_rtr_eth.buf_reception_highway.number_nodes_heard );
#endif   
          SetTableRing( VoiceSpamGate->cntrl_ring, &(VoiceSpamGate->buf_ip_rtr_eth.buf_reception_highway) );
          break; 
          /**************************************************************************************************************************************************/
          /******************************* Запросы команд для управления локальными ресурсами    ************************************************************/        
          /**************************************************************************************************************************************************/        
        case ID_CMD_LOCAL_REQ:       
        case ID_CMD_LOCAL_RESP: 
#if ( UDP_SPAM_DEBUG == 1 )  
          
          switch(VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.cmd_spam)
          {
          case REQ_CON_NO_GET           : sprintf(STypeReq,"REQ_CON_NO_GET           "); break;  
          case REQ_CON_GET_TABL         : sprintf(STypeReq,"REQ_CON_GET_TABL         "); break;  
          case REQ_CON_TIME_GET_TABL    : sprintf(STypeReq,"REQ_CON_TIME_GET_TABL    "); break;  
          case REQ_CON_NOTIME_GET_TABL  : sprintf(STypeReq,"REQ_CON_NOTIME_GET_TABL  "); break;  
          case REQ_CON_SERVER_GET_TABL  : sprintf(STypeReq,"REQ_CON_SERVER_GET_TABL  "); break; 
          case REQ_GET_TABL             : sprintf(STypeReq,"REQ_GET_TABL             "); break;  
          case REQ_BROATCAST_CON_DEV    : sprintf(STypeReq,"REQ_BROATCAST_CON_DEV    "); break;  
          case REQ_CON_TIME_NO_GET      : sprintf(STypeReq,"REQ_CON_TIME_NO_GET      "); break;  
          case REQ_CON_NOTIME_NO_GET    : sprintf(STypeReq,"REQ_CON_NOTIME_NO_GET    "); break;  
          case REQ_CON_GET_STATUS_DEV   : sprintf(STypeReq,"REQ_CON_GET_STATUS_DEV   "); break;         
          case REQ_CON_TIME_STATUS_DEV  : sprintf(STypeReq,"REQ_CON_TIME_STATUS_DEV  "); break;  
          case REQ_CON_NOTIME_STATUS_DEV: sprintf(STypeReq,"REQ_CON_NOTIME_STATUS_DEV"); break;  
          case RESP_TABL_SPAM           : sprintf(STypeReq,"RESP_TABL_SPAM           "); break;  
          case RESP_STATUS_DEV          : sprintf(STypeReq,"RESP_STATUS_DEV          "); break;  
          default:                        sprintf(STypeReq,"-------------------------"); break;  
          }
          
          printf( "|>> %s |0x%.4x|%.3d|%.3d|%.3d|%.3d|%.5d|%.3d|\r\n",
                 STypeReq,
                 VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.rs_head.src,
                 VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[0],
                 VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[1],
                 VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[2],
                 VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[3],
                 VoiceSpamGate->buf_ip_rtr_eth.IPport_reception,
                 VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.number_dev_addr );
#endif 
          
          /* Проверка типа полученого пакета             */
          switch(VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.cmd_spam)
          {
          case REQ_BROATCAST_CON_DEV:
            /* Временое подключение к рассылке без запроса таблицы рассылки с трансляцией на сервер рассылки - используем для регистрации пульта позиционирования */
            /* Заменить порт для ответа - портом первого устройства в рассылке */
            VoiceSpamGate->buf_ip_rtr_eth.IPport_reception = VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.dev_addr_array[0].ReqIPport;
            VoiceSpamGate->buf_ip_rtr_eth.IPaddr_reception = VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.dev_addr_array[0].PhyAddr;
            VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[0] = ((uint8_t*)(&(VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.dev_addr_array[0].ReqIPaddr)))[0];
            VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[1] = ((uint8_t*)(&(VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.dev_addr_array[0].ReqIPaddr)))[1];
            VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[2] = ((uint8_t*)(&(VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.dev_addr_array[0].ReqIPaddr)))[2];
            VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[3] = ((uint8_t*)(&(VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.dev_addr_array[0].ReqIPaddr)))[3];
            
            /* Адрес этого клиента должен участвовать в рассылке */
            VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.dev_addr_array[1].TypeDev = ( VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.dev_addr_array[1].TypeDev ) | TYPE_ADD_SPAM; 
            
            /* Функция занесения адреса в таблицу рассылки */
            ImportTableSpam( VoiceSpamGate->table_spam, &(VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam) );     
            
            /* Перетранслируем данные о запросе на позиционирования на сервера рассылки */  
            size_box = ExportAddrSpam( VoiceSpamGate->table_spam, &(VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.dev_addr_array[1]) ,(void*)&(VoiceSpamGate->buf_eth_rtr), RESP_TABL_SPAM );          
            
            /* Запрос таблицы у серверов рассылки ETH                                         */ 
            /* Если адрес основного сервера не соответствует собственному - транслируем пакет */
            if (VoiceSpamGate->table_spam->sever_table_spam_main.IPaddr != VoiceSpamGate->table_spam->ReqIPaddr )
            {
              UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_spam_eth_rtr) , VoiceSpamGate->buf_spam_eth_rtr.rs_head.lenght + SIZE_LENGHT + SIZE_CRC, &(VoiceSpamGate->udp_soc), VoiceSpamGate->table_spam->sever_table_spam_main.IPaddr ,VoiceSpamGate->table_spam->sever_table_spam_main.IPport);
            }
            /* Если адрес резервного сервера не соответствует собственному - отправляем запрос */        
            if (VoiceSpamGate->table_spam->sever_table_spam_rezerv.IPaddr != VoiceSpamGate->table_spam->ReqIPaddr )
            {
              UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_spam_eth_rtr) , VoiceSpamGate->buf_spam_eth_rtr.rs_head.lenght + SIZE_LENGHT + SIZE_CRC, &(VoiceSpamGate->udp_soc), VoiceSpamGate->table_spam->sever_table_spam_rezerv.IPaddr ,VoiceSpamGate->table_spam->sever_table_spam_rezerv.IPport);
            }
            
            /* Формирует ответ таблица только с адресом позиционирования*/
            size_box = ExportOwnAddrSpam( VoiceSpamGate->table_spam, &(VoiceSpamGate->buf_spam_eth_rtr) , RESP_STATUS_DEV , 0 );
            if (size_box != 0)
            {
#if ( UDP_SPAM_DEBUG == 1 ) 
              printf( "|<< RESP_STATUS_DEV           |0x%.4x|%.3d|%.3d|%.3d|%.3d|%.5d|%.3d|\r\n",
                     VoiceSpamGate->buf_ip_rtr_eth.IPaddr_reception,
                     VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[0],
                     VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[1],
                     VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[2],
                     VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[3],
                     VoiceSpamGate->buf_ip_rtr_eth.IPport_reception,
                     VoiceSpamGate->buf_ip_rtr_eth.IPaddr_reception);
#endif   
              UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_spam_eth_rtr) , size_box , &(VoiceSpamGate->udp_soc), VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.dev_addr_array[0].ReqIPaddr ,VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.dev_addr_array[0].ReqIPport);
            }
            break; 
            
          case REQ_CON_NO_GET:		             
          case REQ_CON_GET_TABL:	             
          case REQ_CON_TIME_GET_TABL:	             
          case REQ_CON_NOTIME_GET_TABL:             
          case REQ_CON_SERVER_GET_TABL:             
          case REQ_CON_TIME_NO_GET:	             
          case REQ_CON_NOTIME_NO_GET:               
          case REQ_CON_GET_STATUS_DEV:	             
          case REQ_CON_TIME_STATUS_DEV:	     
          case REQ_CON_NOTIME_STATUS_DEV:     
          case RESP_TABL_SPAM: 
          case RESP_STATUS_DEV:          
            /* Функция занесения адреса в таблицу рассылки */
            ImportTableSpam( VoiceSpamGate->table_spam, &(VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam) );        
            break;           
          }        
          
          /*================================================================================================================================*/
          /*====================================== Формирование таблицы рассылки в ответ на запрос =========================================*/
          /*================================================================================================================================*/        
          /* Проверка типа полученого пакета             */
          switch(VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.cmd_spam)
          {
          case REQ_CON_NOTIME_GET_TABL:        /*  */    
          case REQ_CON_GET_TABL:               /*  */
          case REQ_CON_TIME_GET_TABL:          /*  */
          case REQ_GET_TABL: 
          case REQ_CON_SERVER_GET_TABL:          
            /* запуск экспорта таблицы рассылки */
            /* если ты сервер отправляешь таблицу иначе только свой адрес */
            if ( ( VoiceSpamGate->table_spam->sever_table_spam_main.IPaddr == VoiceSpamGate->table_spam->ReqIPaddr ) || (VoiceSpamGate->table_spam->sever_table_spam_rezerv.IPaddr == VoiceSpamGate->table_spam->ReqIPaddr ) )
            {
                /* Экспорт таблицы рассылки */
              size_box = ExportTableSpam( VoiceSpamGate->table_spam, &(VoiceSpamGate->buf_spam_eth_rtr) , VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.cmd_spam , VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.status_cmd_spam );
                if (size_box != 0)
                {
#if ( UDP_SPAM_DEBUG == 1 )   
                  printf( "|<< RESP_TABL_SPAM            |0x%.4x|%.3d|%.3d|%.3d|%.3d|%.5d|%.3d|\r\n",
                         VoiceSpamGate->table_spam->PhyAddr,
                         VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[0],
                         VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[1],
                         VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[2],
                         VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[3],
                         VoiceSpamGate->buf_ip_rtr_eth.IPport_reception,
                         VoiceSpamGate->buf_spam_eth_rtr.number_dev_addr );
#endif   
                  UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_spam_eth_rtr) , size_box , &(VoiceSpamGate->udp_soc), VoiceSpamGate->buf_ip_rtr_eth.IPaddr_reception ,VoiceSpamGate->buf_ip_rtr_eth.IPport_reception);
                }
              } 
            else
            {
              /* Формирует ответ таблица только с собственным адресом */
              size_box = ExportOwnAddrSpam( VoiceSpamGate->table_spam, &(VoiceSpamGate->buf_spam_eth_rtr) , RESP_STATUS_DEV , 0 );
              
              if (size_box != 0)
              {
#if ( UDP_SPAM_DEBUG == 1 )  
                printf( "|<< RESP_STATUS_DEV           |0x%.4x|%.3d|%.3d|%.3d|%.3d|%.5d|%.3d|\r\n",
                       VoiceSpamGate->table_spam->PhyAddr,
                       VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[0],
                       VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[1],
                       VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[2],
                       VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[3],
                       VoiceSpamGate->buf_ip_rtr_eth.IPport_reception,
                       VoiceSpamGate->buf_spam_eth_rtr.number_dev_addr );
#endif   
                UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_spam_eth_rtr) , size_box , &(VoiceSpamGate->udp_soc), VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.dev_addr_array[0].ReqIPaddr ,VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.dev_addr_array[0].ReqIPport);
              }
            }  
            break;     	
            
          case REQ_CON_GET_STATUS_DEV: /*  */           
          case REQ_CON_NOTIME_STATUS_DEV: /*  */ 
          case REQ_CON_TIME_STATUS_DEV:   /* REQ_CON_TIME_STATUS_DEV */  
            /* Формирует ответ таблица только с собственным адресом */
            size_box = ExportOwnAddrSpam( VoiceSpamGate->table_spam, &(VoiceSpamGate->buf_spam_eth_rtr) , RESP_STATUS_DEV , 0 );
            
            if (size_box != 0)
            {
#if ( UDP_SPAM_DEBUG == 1 )  
              printf( "|<< RESP_STATUS_DEV           |0x%.4x|%.3d|%.3d|%.3d|%.3d|%.5d|%.3d|\r\n",
                     VoiceSpamGate->table_spam->PhyAddr,
                     VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[0],
                     VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[1],
                     VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[2],
                     VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[3],
                     VoiceSpamGate->buf_ip_rtr_eth.IPport_reception,
                     VoiceSpamGate->buf_spam_eth_rtr.number_dev_addr );
#endif   
              UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_spam_eth_rtr) , size_box , &(VoiceSpamGate->udp_soc), VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.dev_addr_array[0].ReqIPaddr ,VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.dev_addr_array[0].ReqIPport);
            }
            break;     	
            
          case RESP_TABL_SPAM: 
            /* Запрос продолжения таблицы рассылки - если таблица считана не полностью */          
            if ( ( VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.status_cmd_spam != 0xFF) && ( VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.status_cmd_spam > 0 ) )
            {
              /* Функция формирования пакета запроса таблицы рассылки                           */
              if ( ( ( VoiceSpamGate->table_spam->TypeDev ) & (TYPE_ADD_TBL) ) != 0 )
              {
                /* Если это сервер рассылки - не запрашиваем таблицу */
                size_box = ReqTableSpam( VoiceSpamGate->table_spam, (void*)&(VoiceSpamGate->buf_eth_rtr), REQ_CON_SERVER_GET_TABL, VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.status_cmd_spam ); 
              }
              else
              {
                /* Если не сервер рассылки - запрашиваем таблицу */
                size_box = ReqTableSpam( VoiceSpamGate->table_spam, (void*)&(VoiceSpamGate->buf_eth_rtr), REQ_CON_TIME_GET_TABL, VoiceSpamGate->buf_ip_rtr_eth.buf_reception_spam.status_cmd_spam ); 
              }         
              
#if ( UDP_SPAM_DEBUG == 1 )   
              if ( ( ( VoiceSpamGate->table_spam->TypeDev ) & (TYPE_ADD_TBL) ) != 0 )
              {                                                         
                printf( "|>> REQ_CON_SERVER_GET_TABL   |%.3d|%.3d|%.3d|%.3d|%.5d|\r\n",
                       VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[0],
                       VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[1],
                       VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[2],
                       VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[3],
                       VoiceSpamGate->buf_ip_rtr_eth.IPport_reception );
              }
              else
              {                                                           
                printf( "|>> REQ_CON_TIME_GET_TABL     |%.3d|%.3d|%.3d|%.3d|%.5d|\r\n",
                       VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[0],
                       VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[1],
                       VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[2],
                       VoiceSpamGate->buf_ip_rtr_eth.BtIPadr_reception[3],
                       VoiceSpamGate->buf_ip_rtr_eth.IPport_reception );
              }  
              
#endif               
              if ( ( VoiceSpamGate->buf_ip_rtr_eth.IPaddr_reception != 0 ) && ( VoiceSpamGate->buf_ip_rtr_eth.IPport_reception !=0 ) )
              {
                UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_eth_rtr) , size_box , &(VoiceSpamGate->udp_soc), VoiceSpamGate->buf_ip_rtr_eth.IPaddr_reception ,VoiceSpamGate->buf_ip_rtr_eth.IPport_reception);
              }
            }
            break;  
            
            
          default: 
            /* Пакет оставляем без обработки */
            break;           
          }     
          /*================================================================================================================================*/
          /*================================================================================================================================*/
          /*================================================================================================================================*/          
          break;           
        }
      }
    }
  }
}

/**
  * @brief Эта функция обработки пакетов из роутера потоков   
  * @param arg аргумент пользователя ((voice_gate_t *)arg)
  * @retval None
  */
static void UDP_ProcessingQueueOutRouter(void *arg)
{
  uint32_t     masc_ch_box_voice;  
  dev_addr_t   dev_addr;
  /* Если очередь получения пакетов из роутера открыта  */
  if (VoiceSpamGate->set_port_router.QueueOutRouter != NULL) 
  {
    /* Проверяем наличие входящих пакетов */
    while(xQueueReceive(VoiceSpamGate->set_port_router.QueueOutRouter , ( void * )&(VoiceSpamGate->buf_rtr_eth) , ( TickType_t ) 0) == pdTRUE)
    {
      /* Запросы команд диагностики магистрали RS              */
      /* Проверка типа полученого пакета             */
      switch(VoiceSpamGate->buf_rtr_eth.id)
      {
        /*==========================================================================================================*/
      case ID_DIAG_HIGHWAY_RS_REQ:  /* Запросы команд диагностики магистрали RS              */ 
        /*  Функция обработки полученного пакета тестирования магистрали RS */
        SetTableRing( VoiceSpamGate->cntrl_ring, &(VoiceSpamGate->buf_highway_rtr_eth) );
        break;     	
      default: 
        /* Пакет оставляем без обработки */
        break;           
      }
      
      /* Проверка прохождения шлюзов полученным пакетом - если получен из eth в роутер - пакет не обрабатываем */
      if ( VoiceSpamGate->buf_rtr_eth.flag_rs_to_eth != 1 )
      {
        /* Установка флага направления прохождения шлюза */
        VoiceSpamGate->buf_rtr_eth.flag_rs_to_eth = 1;   
        
        /* Пропишем в RS свой PHY для любого пакета транслируемого в ETH */
        VoiceSpamGate->buf_rtr_eth.src = DataLoaderSto.Settings.phy_adr;
        
        /* Коррекция контрольной суммы */
        CalcCRC16FromISR((uint8_t*)(&(VoiceSpamGate->buf_rtr_eth.lenght)),
                         VoiceSpamGate->buf_rtr_eth.lenght,
                         &(VoiceSpamGate->buf_rtr_eth.data[(VoiceSpamGate->buf_rtr_eth.lenght) + SIZE_LENGHT + SIZE_CRC - sizeof(marker_box_t)]),
                         &(VoiceSpamGate->index_calc_crc));  
        
        /* ===================== Отработка тунеля UDP - RS =================================================================================== */
        /* Проверка маски трансляции */
        if ((DataSto.Settings.tabl_mask_box_id[DataSto.Settings.nmask_transl_config].mask_id[((VoiceSpamGate->buf_rtr_eth.id)>>4)])&(0x0001<<((VoiceSpamGate->buf_rtr_eth.id)&0x0F)))
        {
          /* Отправка всем зарегистрированным слушателям тунеля если пакет соответствует маске */
          while(GetOrTypeAddrTable( VoiceSpamGate->table_spam, TypeBitID(TYPE_SETUP), 0xFF, 0 , &dev_addr )) 
          {  
            /* Рассылка пакета позиционирования  */
            UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_rtr_eth) , VoiceSpamGate->buf_rtr_eth.lenght + SIZE_LENGHT + SIZE_CRC , &(VoiceSpamGate->udp_soc), dev_addr.IPaddr , dev_addr.IPport );
            diag_mes_pos("<<__ETH_POS__",(void*)&(VoiceSpamGate->buf_rtr_eth),dev_addr.PhyAddr);
          } 
        }        
        
        /* ===================== Обработка типов пакетов поддерживающих режим кольца ========================================================== */
        /* ===================== Если включено кольцо и установлен корректный приоритет при режиме кольца слейв голосовые пакеты не транслируем */
        if ( VoiceSpamGate->cntrl_ring->own_mode_ring == RING_MASTER )
        {   
          /* Анализ типа пакета */
          /* Проверка типа полученого пакета             */
          switch(VoiceSpamGate->buf_rtr_eth.id)
          {
          case ID_VOICE:  /* Голосовые данные                                      */
            /* Проверка коррекности голосового канала */
            if  ( ( VoiceSpamGate->buf_voice_rtr_eth.NwkFrameVoice.header.nwkDstPanId == 0xFF ) || ( ( ( VoiceSpamGate->buf_voice_rtr_eth.NwkFrameVoice.header.nwkDstPanId ) > 0 ) && ( ( VoiceSpamGate->buf_voice_rtr_eth.NwkFrameVoice.header.nwkDstPanId ) < 33 ) ) ) 
            {
              /* Если номер канала больше */
              if (VoiceSpamGate->buf_voice_rtr_eth.NwkFrameVoice.header.nwkDstPanId > 32) 
              {
                /* включение широковещательного режима */
                masc_ch_box_voice = 0xFFFFFFFF;
              }
              else
              {
                /* Формируем маску для канала вещания */
                masc_ch_box_voice = BitChID(VoiceSpamGate->buf_voice_rtr_eth.NwkFrameVoice.header.nwkDstPanId);
              } 
              /* Рассылка всем слушателям канала */          
              /* Вещаем по всем выданным на запросу адресам */
              while(GetAddrTable( VoiceSpamGate->table_spam, masc_ch_box_voice , 0x00000000, TypeBitID(TYPE_PDH)|TypeBitID(TYPE_VRM)|TypeBitID(TYPE_PIETH)|TypeBitID(TYPE_GATE), TYPE_ADD_SPAM, 1 , &dev_addr )) 
              {  
                /* Рассылка пакета всем слушателям канала  */
                UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_voice_rtr_eth) , sizeof(RSFrameVoice_t) , &(VoiceSpamGate->udp_soc), dev_addr.IPaddr , dev_addr.IPport );                
                /* Диагностическое сообщение */      
                diag_mes_voice("<<_TRSM_VOICE_",(void*)&(VoiceSpamGate->buf_voice_rtr_eth),dev_addr.PhyAddr);   
              }
            }
            break; 
            /*==========================================================================================================*/              
          case ID_RING_DISP:            /* Вызов диспетчера                                      */
          case ID_REQ_DISP:             /* Вызов диспетчера                                      */    
            
            /* Получили пакет формата call rs - преобразовали в call eth  */
            call_rs_to_eth( &(VoiceSpamGate->buf_eth_call_eth_rtr), &(VoiceSpamGate->buf_call_rtr_eth) );
            /* Диагностика полученного по RS пакета */ 
            diag_mes_call_eth(">>___RS485____",(void*)&(VoiceSpamGate->buf_eth_call_eth_rtr),0x0000);              
            
            /* Обнуление флага отправки пакета по адресу назначения */
            VoiceSpamGate->Flag_send_addr_dst_gate = 0;
            
            /* Рассылка по ETH  пакета всем аудио конверторам */
            while(GetAddrTable( VoiceSpamGate->table_spam, 0xFFFFFFFF, 0x00000000, TypeBitID(TYPE_GATE), 0xFF, 1 , &dev_addr )) 
            {  
              /* Рассылка пакета всем слушателям появившегося канала  */
              UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_eth_call_eth_rtr) , sizeof(eth_call_box_t) , &(VoiceSpamGate->udp_soc), dev_addr.IPaddr , dev_addr.IPport );
              /* Проверка на адрес назначения */
              if ( ( ( VoiceSpamGate->buf_eth_call_eth_rtr.addr_gate_dst ) == dev_addr.PhyAddr) && ( (VoiceSpamGate->buf_eth_call_eth_rtr.addr_gate_dst) != 0xFFFF ) ) VoiceSpamGate->Flag_send_addr_dst_gate = 1;
              /* Диагностическое сообщение */      
              diag_mes_call_eth("<<__ETH_GATE__",(void*)&(VoiceSpamGate->buf_eth_call_eth_rtr),dev_addr.PhyAddr);             
            } 
            /* Рассылка по ETH  пакета всем подписаным рекодерам  */
            while(GetAddrTable( VoiceSpamGate->table_spam, 0x00000000, 0xFFFFFFFF, TypeBitID(TYPE_REC), 0xFF, 1 , &dev_addr )) 
            {  
              /* Рассылка пакета всем слушателям появившегося канала  */
              UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_eth_call_rec_rtr) , sizeof(eth_call_rec_box_t) , &(VoiceSpamGate->udp_soc), dev_addr.IPaddr , dev_addr.IPport );
              /* Проверка на адрес назначения */
              if ( ( ( VoiceSpamGate->buf_eth_call_eth_rtr.addr_gate_dst ) == dev_addr.PhyAddr) && ( (VoiceSpamGate->buf_eth_call_eth_rtr.addr_gate_dst) != 0xFFFF ) ) VoiceSpamGate->Flag_send_addr_dst_gate = 1;
              /* Диагностическое сообщение */      
              diag_mes_call_eth("<<__ETH_REC___",(void*)&(VoiceSpamGate->buf_eth_call_rec_rtr),dev_addr.PhyAddr);           
            } 
            
            /* Если пакет не отправлен по назначению - продолжим рассылку */                    
            if ( ( VoiceSpamGate->Flag_send_addr_dst_gate ) == 0)
            {
              switch(VoiceSpamGate->buf_eth_call_eth_rtr.addr_gate_dst)
              {
              case 0x0000:/* не отправляем никому */ 
                break;
                
              case 0xFFFF:/* отправляем всем      */  
                /* Рассылка по ETH  пакета всем  */
                while(GetAddrTable( VoiceSpamGate->table_spam, 0xFFFFFFFF, 0x00000000,TypeBitID(TYPE_PDH)|TypeBitID(TYPE_VRM)|TypeBitID(TYPE_PIETH), TYPE_ADD_SPAM, 1 , &dev_addr )) 
                {  
                  /* Рассылка пакета всем слушателям появившегося канала  */
                  UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_eth_call_eth_rtr) , sizeof(eth_call_box_t) , &(VoiceSpamGate->udp_soc), dev_addr.IPaddr , dev_addr.IPport );
                  /* Диагностическое сообщение */      
                  diag_mes_call_eth("<<__ETH__ALL__",(void*)&(VoiceSpamGate->buf_eth_call_eth_rtr),dev_addr.PhyAddr);      
                } 
                break;          
              default:    /* отправка только по указанному адресу, если адрес не указан - отправляем на шлюз */ 
                /* Функция запроса адреса из таблицы рассылки по физическому адресу устройства*/
                if (GetPhyAddrTable( VoiceSpamGate->table_spam, VoiceSpamGate->buf_eth_call_eth_rtr.addr_gate_dst, &dev_addr ))
                {
                  /* Отправка пакета */
                  UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_eth_call_eth_rtr) , sizeof(eth_call_box_t) , &(VoiceSpamGate->udp_soc), dev_addr.IPaddr , dev_addr.IPport );
                  /* Диагностическое сообщение */      
                  diag_mes_call_eth("<<__ETH_ADDR__",(void*)&(VoiceSpamGate->buf_eth_call_eth_rtr),dev_addr.PhyAddr);          
                }    
                break;         
              }
            }
            break; 
            /*==========================================================================================================*/    
          case ID_NEW_POSITION:        /* Пакет новый                                       */
            /* Рассылка только модулям позиционировани */
            while(GetOrTypeAddrTable( VoiceSpamGate->table_spam, TypeBitID(TYPE_POZ), 0xFF, 0 , &dev_addr )) 
            {  
              /* Рассылка пакета позиционирования  */
              UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_rtr_eth) , VoiceSpamGate->buf_rtr_eth.lenght + SIZE_LENGHT + SIZE_CRC , &(VoiceSpamGate->udp_soc), dev_addr.IPaddr , dev_addr.IPport );
              diag_mes_pos("<<__ETH_POS__",(void*)&(VoiceSpamGate->buf_rtr_eth),dev_addr.PhyAddr);
            } 
            break;
          case ID_DIAGNOSTICS_RESP:    /* Ответы команд диагностики                         */
          case ID_CMD_TAG_RESP:        /* Ответы команд для управления тегами               */
          case ID_PGLR_STND:           /* Команды/данные от стенда PGLR                     */
            /* Рассылка только модулям позиционировани */
            while(GetOrTypeAddrTable( VoiceSpamGate->table_spam, TypeBitID(TYPE_POZ), 0xFF, 0 , &dev_addr )) 
            {  
              /* Рассылка пакета всем слушателям появившегося канала  */
              UDP_Send_Socket( (void*)&(VoiceSpamGate->buf_rtr_eth) , VoiceSpamGate->buf_rtr_eth.lenght + SIZE_LENGHT + SIZE_CRC , &(VoiceSpamGate->udp_soc), dev_addr.IPaddr , dev_addr.IPport );
            } 
            break; 
          default: 
            /* Пакет оставляем без обработки */
            break;           
          }
        }
      }
    }
  }
}
                                
/**
  * @brief Эта функция для отработки периодических функций задачи и передачи пакетов данных в UDP.
  * @param arg аргумент пользователя ((voice_gate_t *)arg)
  * @retval None
  */
void UDP_Body_Task_Voice(void *arg)
{
  /* Проверяем наличие сообщений */
  if ( VoiceSpamGate->udp_soc.NotifiedValue != 0 )
  {  
    if ( ( ( VoiceSpamGate->udp_soc.NotifiedValue ) & UPTIME_NOTE ) != 0 )
    {
      /* функция периодического запроса таблицы рассылки */
      UDP_ProcessingTableSpam( VoiceSpamGate );
    }     
    
    if ( ( ( VoiceSpamGate->udp_soc.NotifiedValue ) & REQ_DIAG_NOTE ) != 0 )
    {
      /* Функция обновления времени жизни рассылок */
      UpdateTimeLifeSpam( VoiceSpamGate->table_spam );
      
      /* функция обновления таблицы шлюзов кольца и формирования пакета тестироования магистрали RS/ETH  */
      UDP_Update_Ring_Table( VoiceSpamGate );  
      
#if ( TEST_REQ_TABL_SPAM_ENABLE == 1)
      /* функция тестирования периодического запроса таблицы рассылки */
      UDPTestReqTableSpam( VoiceSpamGate );
#endif      
    }      
    
    if ( ( ( VoiceSpamGate->udp_soc.NotifiedValue ) & ROUTER_NOTE ) != 0 )
    {
      /* функция обработки пакетов из роутера потоков */
      UDP_ProcessingQueueOutRouter( VoiceSpamGate );
    }   
      
    if ( ( ( VoiceSpamGate->udp_soc.NotifiedValue ) & RECEIVING_NOTE ) != 0 )
    {
      /* функция обработки очереди приема пакетов из callbaca ETH */
      UDP_Processing_Queue_Reception( VoiceSpamGate );
    }      
          
    if ( ( ( VoiceSpamGate->udp_soc.NotifiedValue ) & SOFT_ROUTER_NOTE ) != 0 )
    {
      /* функция для анализа буфера несжатого звука и передачи пакетов в UDP  */
      UDP_EncodeVoiceTransfer( VoiceSpamGate );
    }       
    
    if ( ( ( VoiceSpamGate->udp_soc.NotifiedValue ) & TIMER_NOTE ) != 0 )
    {
      /* Обработка внутренних команд */ 
      UDP_Local_CMD_Processing(VoiceSpamGate);
      
      /* функция для анализа буфера несжатого звука и передачи пакетов в UDP  */
      UDP_EncodeVoiceTransfer( VoiceSpamGate );
      
      /* функция обработки очереди приема пакетов из callbaca ETH */
      UDP_Processing_Queue_Reception( VoiceSpamGate );
      
      /* функция обработки пакетов из роутера потоков */
      UDP_ProcessingQueueOutRouter( VoiceSpamGate );
    }
  }  
}
/************************ (C) COPYRIGHT DEX *****END OF FILE************************/