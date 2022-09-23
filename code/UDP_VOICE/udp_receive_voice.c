/**
  ******************************************************************************
  * @file    udp_receive_voice.c
  * @author  Trembach D.N.
  * @version V3.0.0
  * @date    28-08-2020
  * @brief   Файл контроля UDP шлюза RS/ETH для голосовых потоков
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
#include "soft_router.h" 
#include "GenerationCRC16.h" 
#include "udp_spam_debug_gate.h" 


#if (UDP_RECEIVE_DEBUG==1)
/**
  * @brief Функция для формирования диагностического сообщения по принятому пакету.
  * @param const char *header_mess - шапка сообщения 
  * @param uint32_t addr - удаленное IP address с которого пакет был принят
  * @param uint16_t port - удаленный порт с которого пакет был принят
  * @param uint16_t tot_len - длинна принятого пакета
  * @param uint8_t type_id - тип или канал пакета 
  * @retval None
  */
void diag_receive( const char *header_mess, uint32_t addr, uint16_t port, uint16_t tot_len, uint8_t type_id );
#else
#define diag_receive( a, b, c, d, e );
#endif /* UDP_RECEIVE_DEBUG */

#define VoiceSpamGate ((voice_gate_t *)arg)
  
/**
  * @brief Эта функция приема пакета на порт UDP.
  * @param arg аргумент пользователя ((spam_gate_t *)arg)
  * @param pcb udp_pcb который принял данные
  * @param p буфер пакета который был принят
  * @param addr удаленное IP address с которого пакет был принят
  * @param port удаленный порт с которого пакет был принят
  * @retval None
  */
void UDP_Receive_Voice(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
  /* Заполняем поля IP адреса и порта */
  VoiceSpamGate->buf_callback_reception.IPaddr_reception = addr->addr;
  VoiceSpamGate->buf_callback_reception.IPport_reception = port;      
  
  /*================================================================================================*/  
  /*================================================================================================*/
  /* Проверяем длинну пакета - если пакет с несжатым звуком - принимаем*/
  if (p->tot_len == sizeof(sample_voice_box_t) )
  {  
    
    /* Диагностическое сообщение */      
    diag_receive(">>_RECV_SOFTBX", addr->addr, port,p->tot_len,0 );
    
#if ( ROUTER_SOFT_ENABLE == 1 )   
    /*================================================================================================*/  
    /*                   Обработка пакетов несжатого звука полученных по UDP                          */
    /*================================================================================================*/
    /* Получаем указатель на доступный буфер */
    VoiceSpamGate->pfifo_box = GetWRPointFIFO();
    /* Если указатель получен корректный */
    if ( VoiceSpamGate->pfifo_box != NULL)
    {
      /* Копируем принятые данные во временный буфер */
      pbuf_copy_partial(p, (void*)(VoiceSpamGate->pfifo_box) , p->tot_len, 0);  
      
      /* Прописать индекс порта источника */
      VoiceSpamGate->pfifo_box->source_port = VoiceSpamGate->index_port; 
      
#ifdef UDP_RX_ENCODE_VOICE_DEBUG  
      printf( "|>> RESV_SFT_BOX 0x%.4x|%.3d|%.3d|%.3d|%.3d|%.5d|%.3d|\r\n",
             VoiceSpamGate->pfifo_box->data_box.src_phy_addr,
             VoiceSpamGate->buf_callback_reception.BtIPadr_reception[0],
             VoiceSpamGate->buf_callback_reception.BtIPadr_reception[1],
             VoiceSpamGate->buf_callback_reception.BtIPadr_reception[2],
             VoiceSpamGate->buf_callback_reception.BtIPadr_reception[3],
             VoiceSpamGate->buf_callback_reception.IPport_reception,
             VoiceSpamGate->pfifo_box->data_box.ch_id); 
#endif       
      
      /* Разрешение пакета для чтения                  */
      VoiceSpamGate->pfifo_box->enable_read = 1;
      
      /* Рассылка оповешения зарегистрировавшихся на рассылку.*/
      notify_soft_port( VoiceSpamGate->index_port );
      
      /* Счетчик не переданных пакетов                 */ 
      (VoiceSpamGate->cnt_rx_soft_box)++;
    } 
    else
    {  
      /* Если указатель не получен - отбрасываем пакет */
      /* Счетчик не переданных пакетов                 */ 
      (VoiceSpamGate->cnt_rx_soft_err)++;
    }
#endif /* ROUTER_SOFT_ENABLE == 1 */      
  }/*================================================================================================*/
  else
  {  
    /* Проверка на переполнение приемного буфера */
    if ((p->tot_len) <= (sizeof(router_box_t)))
    {
      /* Копируем принятые данные во временный буфер */
      pbuf_copy_partial(p, &(VoiceSpamGate->buf_callback_reception.pdamp_buf_reception_eth) , p->tot_len, 0); 
      
      /* Диагностическое сообщение */      
      diag_receive(">>_RECV_RS_BOX", addr->addr, port,p->tot_len,VoiceSpamGate->buf_callback_reception.buf_reception_eth.id );
      
      /* Eсли преамбула соответствует коду CODE_PREAM */
      if ( VoiceSpamGate->buf_callback_reception.buf_reception_eth_call.pre == CODE_PREAM  )
      {
        /* ======================================================================================== */          
        /* Трансляция пакета с преамбулой CODE_PREAM в задачу контроля рассылки голосовых пакетов   */
        /* ======================================================================================== */    
        /* Если получен короткий пакет - значит пакет от роутера  */
        if ( (p->tot_len) == (sizeof(eth_call_rec_box_t)) )
        {
          /* Установка флага получения пакета от рекодера */
          VoiceSpamGate->buf_callback_reception.buf_reception_eth_call.reserv = 0;
          VoiceSpamGate->buf_callback_reception.buf_reception_eth_call.status_box = 0;
          VoiceSpamGate->buf_callback_reception.buf_reception_eth_call.flag_rec_to_eth = 1;          
        }
        
        if (VoiceSpamGate->QueueReception != NULL)
        {
          xQueueSend ( VoiceSpamGate->QueueReception, (void*)&(VoiceSpamGate->buf_callback_reception) , ( TickType_t ) 0 );
          /* Отправляем сообщение для задачи контроля шлюза */
          if ( VoiceSpamGate->udp_soc.HandleTask != NULL )
          {
            xTaskNotify( VoiceSpamGate->udp_soc.HandleTask,        /* Указатель на уведомлюемую задачу                         */
                        RECEIVING_NOTE,                            /* Значения уведомления                                     */
                        eSetBits );                                /* Текущее уведомление добавляются к уже прописанному       */
          }
        }
      }      
      else
      {
        /* Eсли преамбула  */
        if ( VoiceSpamGate->buf_callback_reception.buf_reception_eth.pre == 0xAA55  )
        {
          /* ======================================================================================== */          
          /* Проверка типа полученого пакета формата RS не учитывая адрес назначения                  */
          /* ======================================================================================== */           
          switch( VoiceSpamGate->buf_callback_reception.buf_reception_eth.id )
          {
          case ID_CMD_LOCAL_REQ:        /* Запросы команд для управления локальными ресурсами    */
          case ID_CMD_LOCAL_RESP:       /* Ответы команд для управления локальными ресурсами     */
            /* Отправляетм пакет в задачу контроля соединения */
            if (VoiceSpamGate->QueueReception != NULL)
            {
              xQueueSend ( VoiceSpamGate->QueueReception, (void*)&(VoiceSpamGate->buf_callback_reception) , ( TickType_t ) 0 );
              /* Отправляем сообщение для задачи контроля шлюза */
              if ( VoiceSpamGate->udp_soc.HandleTask != NULL )
              {
                xTaskNotify( VoiceSpamGate->udp_soc.HandleTask,    /* Указатель на уведомлюемую задачу                         */
                            RECEIVING_NOTE,                            /* Значения уведомления                                     */
                            eSetBits );                                /* Текущее уведомление добавляются к уже прописанному       */
              }
            }   
            break;  
          default: 
            /* Пакет оставляем без обработки */
            break;           
          } 
          
          /* Проверка адреса назначения пакета для локальных пакетов*/
          if (VoiceSpamGate->buf_callback_reception.buf_reception_spam.rs_head.dest == 0x0000)
          { 
            /* ======================================================================================== */          
            /* Проверка типа полученого пакета формата RS для локального адреса назначения              */
            /* ======================================================================================== */ 
            switch( VoiceSpamGate->buf_callback_reception.buf_reception_eth.id )
            {
            case ID_MARKER:        /* Пакет маркерный или конфигурации */
            case ID_SETUP_REQ:     
              /* анализ типа регистраци и дозаполнение пакета до нового формата */
              /* Формирование пакета                        */
              VoiceSpamGate->buf_callback_reception.buf_reception_spam.rs_head.id = ID_CMD_LOCAL_REQ;                         /* ID ответ команды управления локальными ресурсами */
              VoiceSpamGate->buf_callback_reception.buf_reception_spam.rs_head.status_box = 0x0000;   
              VoiceSpamGate->buf_callback_reception.buf_reception_spam.rs_head.reserv = 0x0000;               
              
              /* Копирование собственных данных идентификации устройства */
              VoiceSpamGate->buf_callback_reception.buf_reception_spam.dev_addr_array[0].MaskChCoded     = 0x00000000;    
              VoiceSpamGate->buf_callback_reception.buf_reception_spam.dev_addr_array[0].MaskChEnCoded   = 0x00000000;  
              VoiceSpamGate->buf_callback_reception.buf_reception_spam.dev_addr_array[0].ReqIPaddr       = addr->addr;	
              VoiceSpamGate->buf_callback_reception.buf_reception_spam.dev_addr_array[0].ReqIPport       = port;	
              VoiceSpamGate->buf_callback_reception.buf_reception_spam.dev_addr_array[0].PhyAddr         = VoiceSpamGate->buf_callback_reception.buf_reception_spam.rs_head.src;	
              VoiceSpamGate->buf_callback_reception.buf_reception_spam.dev_addr_array[0].TypeDev         = TYPE_POZ;        
              VoiceSpamGate->buf_callback_reception.buf_reception_spam.dev_addr_array[0].NGroup          = 0;    
              VoiceSpamGate->buf_callback_reception.buf_reception_spam.dev_addr_array[0].PriorityInGroup = 0;
              VoiceSpamGate->buf_callback_reception.buf_reception_spam.dev_addr_array[0].ModeRign        = RING_MASTER;
              
              VoiceSpamGate->buf_callback_reception.buf_reception_spam.cmd_spam = REQ_CON_TIME_NO_GET;                        /* Команда управления таблицей                      */
              VoiceSpamGate->buf_callback_reception.buf_reception_spam.status_cmd_spam = 0;                                   /* Статус выполнения команды                        */ 
              VoiceSpamGate->buf_callback_reception.buf_reception_spam.rezerv = 0;                                            /* Резерв                                           */   
              VoiceSpamGate->buf_callback_reception.buf_reception_spam.number_dev_addr = 1;                                   /* Число адресов в пакете                           */       
              
              /* Вычисление длинны пакета */
              VoiceSpamGate->buf_callback_reception.buf_reception_spam.rs_head.lenght = 0x000A + sizeof(spam_dev_addr_t) + 4;    
              
              /* отправка пакета на регистрацию */         
              xQueueSend ( VoiceSpamGate->QueueReception, (void*)&(VoiceSpamGate->buf_callback_reception) , ( TickType_t ) 0 );
              /* Отправляем сообщение для задачи контроля шлюза */
              if ( VoiceSpamGate->udp_soc.HandleTask != NULL )
              {
                xTaskNotify( VoiceSpamGate->udp_soc.HandleTask,    /* Указатель на уведомлюемую задачу                         */
                            RECEIVING_NOTE,                            /* Значения уведомления                                     */
                            eSetBits );                                /* Текущее уведомление добавляются к уже прописанному       */
              } 
              break;
            default: 
              /* Пакет оставляем без обработки */
              break;           
            } 
          }
          else
          {     
            /* ====================================================================================================== */          
            /* Проверка типа полученого пакета формата RS для широковещательного и индивидуального адреса назначения  */
            /* ====================================================================================================== */ 
            /* Проверка прохождения шлюзов полученным пакетом - если уже проходил из eth в роутер - пакет не обрабатываем */
            if ( VoiceSpamGate->buf_callback_reception.buf_reception_eth.flag_eth_to_rs != 1 )
            {
              /* Установка флага направления прохождения шлюза */
              VoiceSpamGate->buf_callback_reception.buf_reception_eth.flag_eth_to_rs = 1;   
              /* Коррекция контрольной суммы */
              CalcCRC16FromISR((uint8_t*)(&(VoiceSpamGate->buf_callback_reception.buf_reception_eth.lenght)),
                               VoiceSpamGate->buf_callback_reception.buf_reception_eth.lenght,
                               &(VoiceSpamGate->buf_callback_reception.buf_reception_eth.data[(VoiceSpamGate->buf_callback_reception.buf_reception_eth.lenght) + SIZE_LENGHT + SIZE_CRC - sizeof(marker_box_t)]),
                               &(VoiceSpamGate->index_calc_crc_reception));     
              
              /* ================================================================================================================= */ 
              /* == Обработка типов пакетов не поддерживающих режим кольца ======================================================= */
              /* ================================================================================================================= */ 
              
              switch( VoiceSpamGate->buf_callback_reception.buf_reception_eth.id )
              {
              case ID_DIAGNOSTICS_REQ:         /* Запросы команд диагностики               */
              case ID_SETUP_RF_REQ:            /* Запросы команд настроек через RF         */
              case ID_BOOTLOADER_RF_REQ:       /* Запросы команд бутлоадера                */
              case ID_CMD_TAG_REQ:             /* Запросы команд для управления тегами     */
                //case ID_REG_MASK_REQ:            /* Запросы команд с регистрами масок портов */
              case ID_SETUP_REQ:               /* Запросы команд настроек                  */
              case ID_BOOTLOADER_REQ:          /* Запросы команд бутлоадера                */
              case ID_INDIVID_RSN_CH:       /* Команда запроса на формировани индивидуального канала */
              case ID_PGLR_SERVER:             /* Команды/данные PGLR от сервера                        */          
                /* Отправляетм пакет в роутер */
                if (VoiceSpamGate->set_port_router.QueueInRouter != NULL)
                {
                  xQueueSend ( VoiceSpamGate->set_port_router.QueueInRouter, &(VoiceSpamGate->buf_callback_reception.buf_reception_eth) , ( TickType_t ) 0 );
                  if ( ( VoiceSpamGate->set_port_router.HandleTaskRouter ) != NULL )
                  {
                    xTaskNotify( VoiceSpamGate->set_port_router.HandleTaskRouter, /* Указатель на уведомлюемую задачу                         */
                                ( VoiceSpamGate->set_port_router.PortBitID ) << OFFSET_NOTE_PORD_ID, /* Значения уведомления                                     */
                                eSetBits );                                     /* Текущее уведомление добавляются к уже прописанному       */
                  } 
                }        
                break; 
              case ID_DIAG_HIGHWAY_RS_REQ:  /* Запросы команд диагностики магистрали RS              */  	
              case ID_DIAG_HIGHWAY_ETH_REQ: /* Запросы команд диагностики магистрали ETH             */ 
                /* Отправляетм пакет в задачу контроля соединения */
                if (VoiceSpamGate->QueueReception != NULL)
                {
                  xQueueSend ( VoiceSpamGate->QueueReception, (void*)&(VoiceSpamGate->buf_callback_reception) , ( TickType_t ) 0 );
                  /* Отправляем сообщение для задачи контроля шлюза */
                  if ( VoiceSpamGate->udp_soc.HandleTask != NULL )
                  {
                    xTaskNotify( VoiceSpamGate->udp_soc.HandleTask,    /* Указатель на уведомлюемую задачу                         */
                                RECEIVING_NOTE,                            /* Значения уведомления                                     */
                                eSetBits );                                /* Текущее уведомление добавляются к уже прописанному       */
                  }
                }   
                break; 
              default: 
                /* Пакет оставляем без обработки */
                break;           
              }
              
              /* ================================================================================================================= */ 
              /* == Обработка типов пакетов поддерживающих режим кольца ========================================================== */
              /* == Если включено кольцо и установлен корректный приоритет при режиме кольца слейв голосовые пакеты не транслируем */
              /* ================================================================================================================= */ 
              switch( VoiceSpamGate->buf_callback_reception.buf_reception_eth.id )
              {
              case ID_VOICE:                /* Голосовые данные                                      */
                /* Транслирование в RS только в режиме мастер кольца                                 */
                if ( VoiceSpamGate->cntrl_ring->own_mode_ring == RING_MASTER )
                {
                  /* Проверка наличия прослушки на данном канале */
                  if (TestListingVoiceBoxCh( VoiceSpamGate->table_spam, &(VoiceSpamGate->buf_callback_reception.buf_reception_voice)  ))
                  {
                    /* Отправляетм пакет в роутер */
                    if (VoiceSpamGate->set_port_router.QueueInRouter != NULL)
                    {
                      xQueueSend ( VoiceSpamGate->set_port_router.QueueInRouter, &(VoiceSpamGate->buf_callback_reception.buf_reception_eth) , ( TickType_t ) 0 );
                      if ( ( VoiceSpamGate->set_port_router.HandleTaskRouter ) != NULL )
                      {
                        xTaskNotify( VoiceSpamGate->set_port_router.HandleTaskRouter, /* Указатель на уведомлюемую задачу                         */
                                    ( VoiceSpamGate->set_port_router.PortBitID ) << OFFSET_NOTE_PORD_ID, /* Значения уведомления                                     */
                                    eSetBits );                                     /* Текущее уведомление добавляются к уже прописанному       */
                      } 
                      /* Диагностическое сообщение */      
                      diag_mes_voice(">>_RECV_VOICE_",&(VoiceSpamGate->buf_callback_reception.buf_reception_voice),VoiceSpamGate->buf_callback_reception.BtIPadr_reception[3]);
                    } 
                  }
                }
              default: 
                /* Пакет оставляем без обработки */
                break;           
              }
            }
          }        
        }
      }
    }
  }
  /* освобождаем принятый буфер */
  pbuf_free(p);
}

#if (UDP_RECEIVE_DEBUG==1)
/**
  * @brief Функция для формирования диагностического сообщения по принятому пакету.
  * @param const char *header_mess - шапка сообщения 
  * @param uint32_t addr - удаленное IP address с которого пакет был принят
  * @param uint16_t port - удаленный порт с которого пакет был принят
  * @param uint16_t tot_len - длинна принятого пакета
  * @param uint8_t type_id - тип или канал пакета 
  * @retval None
  */
void diag_receive( const char *header_mess, uint32_t addr, uint16_t port, uint16_t tot_len, uint8_t type_id )
{  
  printf("|%.16s|0x%.8X|%.5d|%.4d|0x%.2X|\r\n",
         header_mess,
         addr,
         port,         
         tot_len,
         type_id);        
}
#endif
/************************ (C) COPYRIGHT DEX *****END OF FILE************************/