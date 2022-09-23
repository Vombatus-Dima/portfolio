/**
  ******************************************************************************
  * @file    udp_voice.c
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

/* Обьявляем структуру рассылки */
spam_t  SpamTableCntrl;
/* Обьявляем структуру контроля кольца */
control_rign_unit_gate_t ring_cntrl;
   
/* Структура для шлюза ROUTER-ETH UDP */
voice_gate_t VoiceGateRouterEth;

#define VoiceSpamGate ((voice_gate_t *)arg)

/**
  * @brief  Функция диагностики параметров таблиц рассылки по UDP.
  * @param resp_eth_diag_t *resp_eth_diag  - указатель на пакет дагностики
  * @retval None
  */
void UDPSpamDiagnostic( resp_eth_diag_t *resp_eth_diag )
{
 uint8_t cntic_table_spam;
 /* Инициализация подсчета активных рассылок*/
 
 resp_eth_diag->NumSpamAddrAll = 0;        /*  Число зарегистрированных адресов рассылки                        */
 resp_eth_diag->NumSpamAddrTabl = 0;       /*  Число зарегистрированных серверов таблицы рассылки               */
 resp_eth_diag->NumSpamAddrPos = 0;        /*  Число зарегистрированных клиентов позиционирования и диагностики */
 resp_eth_diag->NumSpamAddrRec = 0;        /*  Число зарегистрированных клиентов регистрации голосовых сообщений*/
 resp_eth_diag->NumSpamAddrGate = 0;       /*  Число зарегистрированных аудиоконверторов                        */
 resp_eth_diag->NumSpamAddrPDH = 0;        /*  Число зарегистрированных пультов диспетчера                      */
 resp_eth_diag->NumSpamAddrPiEth = 0;      /*  Число зарегистрированных преобразователей интерфейсов            */
 resp_eth_diag->NumSpamAddrVRMV = 0;       /*  Число зарегистрированных выносных голосовых радио модулей        */
  
 /* Анализ собственных данных */ 

 /* Подсчет общего колличества регистраций */
 (resp_eth_diag->NumSpamAddrAll)++;
 
 /*  Число зарегистрированных серверов таблицы рассылки               */
 if ( ( (SpamTableCntrl.TypeDev)& TYPE_ADD_TBL ) > 0 )
 {
   (resp_eth_diag->NumSpamAddrTabl)++;
 }
 
 /*  Число зарегистрированных клиентов позиционирования и диагностики */
 if ( ( (SpamTableCntrl.TypeDev) & TYPE_MASK_ALL) == TYPE_POZ )
 {
   (resp_eth_diag->NumSpamAddrPos)++;
 }
 
 /*  Число зарегистрированных клиентов регистрации голосовых сообщений*/
 if ( ( (SpamTableCntrl.TypeDev) & TYPE_MASK_ALL) == TYPE_REC )
 {
   (resp_eth_diag->NumSpamAddrRec)++;
 }  
 
 /*  Число зарегистрированных аудиоконверторов                        */
 if ( ( (SpamTableCntrl.TypeDev) & TYPE_MASK_ALL) == TYPE_GATE )
 {
   (resp_eth_diag->NumSpamAddrGate)++;
 }
 /*  Число зарегистрированных пультов диспетчера                      */
 if ( ( (SpamTableCntrl.TypeDev) & TYPE_MASK_ALL) == TYPE_PDH )
 {
   (resp_eth_diag->NumSpamAddrPDH)++;
 }
 
 /*  Число зарегистрированных преобразователей интерфейсов            */
 if ( ( (SpamTableCntrl.TypeDev) & TYPE_MASK_ALL) == TYPE_PIETH )
 {
   (resp_eth_diag->NumSpamAddrPiEth)++;
 }
 
 /*  Число зарегистрированных выносных голосовых радио модулей        */
 if ( ( (SpamTableCntrl.TypeDev) & TYPE_MASK_ALL) == TYPE_VRM )
 {
   (resp_eth_diag->NumSpamAddrVRMV)++;
 }

 /* Инициализация индекса подсчета статистики */
 cntic_table_spam = 0;
 /* Выполняем обновление времени жизни рассылок */
 while( ( (SpamTableCntrl.table_dev_addr[cntic_table_spam].TimeLife) > 0 ) && ( cntic_table_spam < MAX_NUM_ADDR_TABLE_SPAM ) )
 {
   /* Подсчет общего колличества регистраций */
   (resp_eth_diag->NumSpamAddrAll)++;
   
   /*  Число зарегистрированных серверов таблицы рассылки               */
   if ( ( (SpamTableCntrl.table_dev_addr[cntic_table_spam].dev_addr.TypeDev)& TYPE_ADD_TBL ) > 0 )
   {
     (resp_eth_diag->NumSpamAddrTabl)++;
   }
   
   /*  Число зарегистрированных клиентов позиционирования и диагностики */
   if ( ( (SpamTableCntrl.table_dev_addr[cntic_table_spam].dev_addr.TypeDev) & TYPE_MASK_ALL ) == TYPE_POZ )
   {
     (resp_eth_diag->NumSpamAddrPos)++;
   }
   
   /*  Число зарегистрированных клиентов регистрации голосовых сообщений*/
   if ( ( (SpamTableCntrl.table_dev_addr[cntic_table_spam].dev_addr.TypeDev) & TYPE_MASK_ALL ) == TYPE_REC )
   {
     (resp_eth_diag->NumSpamAddrRec)++;
   }  
   
   /*  Число зарегистрированных аудиоконверторов                        */
   if ( ( (SpamTableCntrl.table_dev_addr[cntic_table_spam].dev_addr.TypeDev) & TYPE_MASK_ALL ) == TYPE_GATE )
   {
     (resp_eth_diag->NumSpamAddrGate)++;
   }
   /*  Число зарегистрированных пультов диспетчера                      */
   if ( ( (SpamTableCntrl.table_dev_addr[cntic_table_spam].dev_addr.TypeDev) & TYPE_MASK_ALL ) == TYPE_PDH )
   {
     (resp_eth_diag->NumSpamAddrPDH)++;
   }
      
   /*  Число зарегистрированных преобразователей интерфейсов            */
   if ( ( (SpamTableCntrl.table_dev_addr[cntic_table_spam].dev_addr.TypeDev) & TYPE_MASK_ALL ) == TYPE_PIETH )
   {
     (resp_eth_diag->NumSpamAddrPiEth)++;
   }

   /*  Число зарегистрированных выносных голосовых радио модулей        */
   if ( ( (SpamTableCntrl.table_dev_addr[cntic_table_spam].dev_addr.TypeDev) & TYPE_MASK_ALL ) == TYPE_VRM )
   {
     (resp_eth_diag->NumSpamAddrVRMV)++;
   }

   /* Инкрементирование индекса */
   cntic_table_spam++;
 }
}   
  
/**
* @brief Эта функция .
* @param arg аргумент пользователя ((spam_gate_t *)arg)
* @retval None
*/
void UDP_Init_Task_Voice(void *arg)
{  
  /*=== Регистрация в массиве указателей на очереди команд для задачи порта ====*/  
  /* Регистрирование очереди команд в таблице */
  if (VoiceSpamGate->xQueueCMD != NULL)
  {
    /* Ожидание инициализации указателя в таблице команд */
    while(BaseQCMD[VoiceSpamGate->set_port_router.PortID].Status_QCMD != QCMD_INIT)
    {
      /* Ожидание 1 млсек */
      vTaskDelay(1);
    }
    /* Обнуление укаазтеля */
    BaseQCMD[VoiceSpamGate->set_port_router.PortID].QueueCMD = VoiceSpamGate->xQueueCMD;
    /* Установка статуса */
    BaseQCMD[VoiceSpamGate->set_port_router.PortID].Status_QCMD = QCMD_ENABLE;          
  }    
  /*============================================================================*/  
  /*============================================================================*/   
  /*=================== Инициализация порта к RS мультиплексору ================*/
  /* Прописываем указатель на задачу контроля сокета */
  VoiceSpamGate->set_port_router.HandleTask = VoiceSpamGate->udp_soc.HandleTask;
  
  /* Запрашиваем ID порта маршрутизации */
  while(request_port_pnt(&(VoiceSpamGate->port_index_tab)) != true)
  {
    vTaskDelay(1);
  }
  /* Инициализация порта                */
  settings_port_pnt(VoiceSpamGate->port_index_tab,&(VoiceSpamGate->set_port_router));
  /* ожидаем соединения по ETH          */
  while(link_ethernet() == false)
  {
    vTaskDelay(1);
  } 
  /* Включаем порт                      */
  enable_port_pnt(VoiceSpamGate->port_index_tab);
  
  /*=====================================================================================*/    
  /* Функция инициализации порта сопряжения кодека и програмного роутера */
  /*=====================================================================================*/     
#if ( ROUTER_SOFT_ENABLE == 1 )  
  /* Функция регистрации на рассылку. */
  reg_notify_port( UDPGate_SoftPID , VoiceSpamGate->set_port_router.HandleTask );  
#endif //( ROUTER_SOFT_ENABLE == 1 )  
  /*============================================================================*/  
  /*============================================================================*/  
}

/**
  * @brief  Функция обратного вызова таймера организации голосового шлюза
  * @param  TimerHandle_t xTimer  - указатель на таймера кодека вызвавшего функции
  * @retval None
  * 
  */
void SfTimerVoiceGateCB( TimerHandle_t xTimer )
{
  if ( ( xTimer != NULL) && ( ( VoiceGateRouterEth.udp_soc.HandleTask ) != NULL ) )
  {
    if ( VoiceGateRouterEth.udp_soc.SoftTimerForTask == xTimer )
    {/* Отправляем сообщение для задачи контроля шлюза */
    xTaskNotify( VoiceGateRouterEth.udp_soc.HandleTask,    /* Указатель на уведомлюемую задачу                         */
                TIMER_NOTE,                                /* Значения уведомления                                     */
                eSetBits );                                /* Текущее уведомление добавляются к уже прописанному       */
  }

    if ( VoiceGateRouterEth.xSoftTimerUpdate == xTimer )
    {/* Устанавливаем событие срабатывания програмного таймера для периодического обновления таблиц */
      xTaskNotify( VoiceGateRouterEth.udp_soc.HandleTask,    /* Указатель на уведомлюемую задачу                         */
                  UPTIME_NOTE,                               /* Значения уведомления                                     */
                  eSetBits );                                /* Текущее уведомление добавляются к уже прописанному       */
    }    
    if ( VoiceGateRouterEth.xSoftTimerDiag == xTimer )
    {/* Устанавливаем событие срабатывания програмного таймера для запуска функций диагностики */
      xTaskNotify( VoiceGateRouterEth.udp_soc.HandleTask,    /* Указатель на уведомлюемую задачу                         */
                  REQ_DIAG_NOTE,                             /* Значения уведомления                                     */
                  eSetBits );                                /* Текущее уведомление добавляются к уже прописанному       */
    }       
  }
}

/**
  * @brief  Функция создания сокета UDP для организации шлюза
  * @param  voice_gate_t* GateRouterEth указатель на структуру для шлюза ROUTER-ETH UDP VOICE
  * @retval None
  */
void Init_Voice_UDP(voice_gate_t* VoiceRouterEth)
{
  /*===================== Инициализация UDP сокета =============================*/
  VoiceRouterEth->udp_soc.status = NONE_UDP;          
  /* Указатель на блок контроля соединения                                      */
  VoiceRouterEth->udp_soc.pcb = NULL;
  /* Локальный порт сервера                                                     */
  VoiceRouterEth->udp_soc.local_port = DataSto.Settings.ip_port_voice;
  /* Время формирования запросов задачи в тиках системы                         */
  VoiceRouterEth->udp_soc.control_time = 20;		
  /* Имя для задачи                                                             */
  VoiceRouterEth->udp_soc.pcName = "VGATE"; 
  /* Установка функции инициализации задачи                                     */
  VoiceRouterEth->udp_soc.init_task = UDP_Init_Task_Voice;
  /* Установка функции тела задачи                                              */
  VoiceRouterEth->udp_soc.body_task = UDP_Body_Task_Voice;
  /* Установка обработчика принятых пакетов                                     */
  VoiceRouterEth->udp_soc.recv = UDP_Receive_Voice;
  /* Имя таймера - для отладки                                                  */
  VoiceRouterEth->udp_soc.pcSoftTimerName = "VGTIM";  
  /* Сallback програмного таймера вызова задачи                                 */
  VoiceRouterEth->udp_soc.SoftTimerCB = SfTimerVoiceGateCB;
  /*============================================================================*/  
  /*============================================================================*/   
  /* Открытие очереди для приема комманд */
  if ( VoiceRouterEth->xQueueCMD == NULL )
  {
    /* открытие очереди для приема до 4 комманд */
    VoiceRouterEth->xQueueCMD = xQueueCreate( 4 , sizeof(cntrl_cmd_t));
  }  
  /*============================================================================*/    
  /*=================== Инициализация порта RS мультиплексору ==================*/
  /* Открытие очереди для пакетов ROUTER_ETH для шлюза  */
  if (VoiceRouterEth->set_port_router.QueueOutRouter == NULL)
  {
    VoiceRouterEth->set_port_router.QueueOutRouter = xQueueCreate( 6 , sizeof(router_box_t)); 
  }	
  
  /* Открытие очереди для пакетов ETH_ROUTER для шлюза   */
  if (VoiceRouterEth->set_port_router.QueueInRouter == NULL)
  { 
    VoiceRouterEth->set_port_router.QueueInRouter = xQueueCreate( 6 , sizeof(router_box_t)); 
  }  
  VoiceRouterEth->set_port_router.FlagLockAddr = LOCK_ADDR;                                  /* блокирует трансляцию в порт со своим адресом */
  VoiceRouterEth->set_port_router.MaskPortID = DataSto.Settings.mask_inpup_port_id_voice;    /* установка маску ID портов                    */
  VoiceRouterEth->set_port_router.NumMaskBoxID = DataSto.Settings.nmask_transl_voice;        /* используем маску ID пакетов                  */
  VoiceRouterEth->set_port_router.PortID = VoiceETHPortID;                                   /* идентификатор порта                          */
  VoiceRouterEth->set_port_router.PortBitID = BitID(VoiceETHPortID);                         /* идентификатор порта                          */
  VoiceRouterEth->set_port_router.HandleTask = NULL;                                         /* Обнуление указателя на задачу                */ 
  /*============================================================================*/  
  /*============================================================================*/    
  /*== Открытие очереди для данных передаваемых из callback_reception ==========*/
  if (VoiceRouterEth->QueueReception == NULL)
  { 
#if  DEVICE_TYPE == PDH_B    
    VoiceRouterEth->QueueReception = xQueueCreate( 8 , sizeof(reception_ip_buf_t)); 
#else
    VoiceRouterEth->QueueReception = xQueueCreate( 12 , sizeof(reception_ip_buf_t));
#endif    
  }    
  /*============================================================================*/
  /*============================================================================*/  
  /*================ Инициализация таблицы резервирования ======================*/
  VoiceRouterEth->cntrl_ring = &ring_cntrl;
  
  /* Функция инициализации структуры контроля кольца */
  InitRing( VoiceRouterEth->cntrl_ring,                        /*  указатель на структуру контроля кольца         */
            DataLoaderSto.Settings.phy_adr,                    /*  собственный адрес шлюза                        */
            DataSto.Settings.group_ring,                       /*  номер группы кольца (0 - без группы)           */
            DataSto.Settings.priority_ring);                   /*  собственный приоритет в кольце                 */
  
  /*================== Инициализация таблицы рассылки ==========================*/
  VoiceRouterEth->table_spam = &SpamTableCntrl;

  /* Определение типа устройства */
  uint8_t spam_type_device;
  /* Тип устройства  */
  spam_type_device = MaskTypeDev( DEVICE_TYPE );

  /* Проверка является ли данное устройство основным сервером рассылки */  
  if ( ( CNVRT_IP_ADDR4(DataLoaderSto.Settings.ip_ad0, DataLoaderSto.Settings.ip_ad1,  DataLoaderSto.Settings.ip_ad2, DataLoaderSto.Settings.ip_ad3 )) == ( CNVRT_IP_ADDR4(DataSto.Settings.ip_table_main_srv_ad0,DataSto.Settings.ip_table_main_srv_ad1,DataSto.Settings.ip_table_main_srv_ad2,DataSto.Settings.ip_table_main_srv_ad3 ))  ) 
  {
    spam_type_device = spam_type_device | TYPE_ADD_TBL;
  }
  /* Проверка является ли данное устройство резервным сервером рассылки */  
  if ( ( CNVRT_IP_ADDR4(DataLoaderSto.Settings.ip_ad0, DataLoaderSto.Settings.ip_ad1,  DataLoaderSto.Settings.ip_ad2, DataLoaderSto.Settings.ip_ad3 )) == ( CNVRT_IP_ADDR4(DataSto.Settings.ip_table_rezv_srv_ad0,DataSto.Settings.ip_table_rezv_srv_ad1,DataSto.Settings.ip_table_rezv_srv_ad2,DataSto.Settings.ip_table_rezv_srv_ad3 ))  ) 
  {
    spam_type_device = spam_type_device | TYPE_ADD_TBL;
  }  
  
  InitTableSpam(VoiceRouterEth->table_spam,                           /* указатель на структуру контроля таблицы рассылки  */
                CNVRT_IP_ADDR4(DataLoaderSto.Settings.ip_ad0,
                               DataLoaderSto.Settings.ip_ad1,
                               DataLoaderSto.Settings.ip_ad2,
                               DataLoaderSto.Settings.ip_ad3),        /* собственный IP address                            */
                DataSto.Settings.ip_port_voice,                       /* собственный UDP port                              */
                DataLoaderSto.Settings.phy_adr,                       /* IP address основного сервера рассылки             */    
                CNVRT_IP_ADDR4(DataSto.Settings.ip_table_main_srv_ad0,
                               DataSto.Settings.ip_table_main_srv_ad1,
                               DataSto.Settings.ip_table_main_srv_ad2,
                               DataSto.Settings.ip_table_main_srv_ad3),/* UDP port основного сервера рассылки               */
                DataSto.Settings.ip_table_main_srv_port,               /* IP address резервного сервера рассылки            */    
                CNVRT_IP_ADDR4(DataSto.Settings.ip_table_rezv_srv_ad0,
                               DataSto.Settings.ip_table_rezv_srv_ad1,
                               DataSto.Settings.ip_table_rezv_srv_ad2,
                               DataSto.Settings.ip_table_rezv_srv_ad3),/* UDP port резервного сервера рассылки              */    
                DataSto.Settings.ip_table_rezv_srv_port,               /* Физический адрес устройства                       */
                spam_type_device,                           /* Тип адреса рассылки                               */
                VoiceRouterEth->cntrl_ring);                           /*  указатель на структуру контроля кольца           */
  
  
  
  /* Открытие таймера запроса запуска диагностики кольца и обновления таблицы рассылки*/
  VoiceRouterEth->xSoftTimerDiag = xTimerCreate( "TmUDPRG",    /* Текстовое имя, не используется в RTOS kernel. */
                                 PERIOD_UPDATE_TIME_LIFE_SPAM, /* Период таймера в тиках. */
                                    pdTRUE,                    /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                    NULL,                      /* В функцию параметры не передаем */
                                    SfTimerVoiceGateCB );      /* Указатель на функцию , которую вызывает таймер. */  
  
  xTimerStart( VoiceRouterEth->xSoftTimerDiag , 0 );    
  
  /* Открытие таймера запроса запуска запроса таблицы рассылки */
  VoiceRouterEth->xSoftTimerUpdate = xTimerCreate( "TmUDPUP",         /* Текстовое имя, не используется в RTOS kernel. */
                                                TIME_PERIOD_REQ_TABLE_SPAM, /* Период таймера в тиках. */
                                                pdTRUE,               /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                                NULL,                 /* В функцию параметры не передаем */
                                                SfTimerVoiceGateCB ); /* Указатель на функцию , которую вызывает таймер. */  
  
  xTimerStart( VoiceRouterEth->xSoftTimerUpdate , 0 );  
  /*============================================================================*/   
  /*============================================================================*/   

#if ( ROUTER_SOFT_ENABLE == 1 ) 
  /*============ Инициализация переменных работы с програмным роутером =========*/  
  /* Инициализация порта FIFO */
  /* Индекс для приема из буфер роутера            */
  VoiceRouterEth->index_rd_box = 0;                  
  /* Переменная хранения указателя */
  VoiceRouterEth->pfifo_box = NULL;                   
  /* Счетчик переданных пакетов                    */
  VoiceRouterEth->cnt_tx_soft_box = 0;                
  /* Счетчик не переданных пакетов                 */ 
  VoiceRouterEth->cnt_tx_soft_err = 0;         
  /* Счетчик принятых пакетов                      */
  VoiceRouterEth->cnt_rx_soft_box = 0;                  
  /* Счетчик принятых пакетов c ошибкой            */ 
  VoiceRouterEth->cnt_rx_soft_err = 0;                  
  /* Собственные переменные идентификации порта    */
  /* Собственный индекс порта                      */
  VoiceRouterEth->index_port = UDPGate_SoftPID;               
  /* Маска доступных для приема портов             */
  VoiceRouterEth->mask_index_port = DataSto.Settings.udp_mask_source_soft_port;           
  /* Маска доступных каналов                       */
  VoiceRouterEth->mask_chanel = DataSto.Settings.udp_mask_chanel_soft_port;   
  /*============================================================================*/
  /*============================================================================*/  
#endif /* ROUTER_SOFT_ENABLE == 1 */
  
  /* Открытие задачи контроля шлюза                                             */
  xTaskCreate(UDP_Task_Socket, VoiceRouterEth->udp_soc.pcName, configMINIMAL_STACK_SIZE * 2.5, (void *)VoiceRouterEth, _ETH_THREAD_PRIO_, &(VoiceRouterEth->udp_soc.HandleTask));
}
/**
  * @brief  Функция создания сокета UDP для рассылки голосовых пакетов
  * @param  None
  * @retval None
  */
void Init_UDP_voice(void)
{
  Init_Voice_UDP(&VoiceGateRouterEth);
}
/************************ (C) COPYRIGHT DEX *****END OF FILE************************/