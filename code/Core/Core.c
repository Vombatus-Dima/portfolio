/**
 ******************************************************************************
 * @file    mb_gate.c
 * @author  Trembach Dmitry
 * @version V1.2.0
 * @date    27-03-2020
 * @brief   Инициализация порта шлюза модбас
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2019 DataExpress</center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "router_streams.h"
#include "FreeRTOS.h"
#include "task.h"
#include "loader_settings.h"
#include "board.h"
#include "GenerationCRC16.h"
#include "Core.h"
#include "core_cntrl_rsn.h"
#include "settings.h"
#include "core_cntrl_settings.h"
#include "core_cntrl_upload.h"
#include "eth_core_tube.h"
#include "core_cntrl_dev.h"

/* Включаем в ядре обработку пакетов модбас */
#ifdef MODBUS_CORE_MODE
#include "core_cntrl_modbus.h"
#endif

/**
  * @brief  Функция получения и обработки данных ADC
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  * @retval None
  */
extern void Core_ADC_Control(core_struct_t*  core_st);

// Функция обновления параметров диагностики ядра
extern void UpdateDiagnostic(core_struct_t*  core_st);

// Функция инициализхации параметров диагностики ядра
extern void InitDiagnostic( void );

// Функция обработка запросов диагностики
extern bool ProcessingDiagnosticBox(core_struct_t* core_st);


// Структура ядра
core_struct_t  st_core;

// Массив указателей очередей для команд управления ресурсами
queue_cntrl_cmd_t BaseQCMD[MAX_NUMBER_PortID]; 

// Определяем группу событий - команд запроса
// За каждым ресурсом закрепляем 3 бита
EventGroupHandle_t xCoreOutEvent;

// Определяем группу событий - команд ответа
// За каждым ресурсом закрепляем 3 бита
EventGroupHandle_t xCoreInEvent;

// Переопределение указателя аргуметра задачи
#define core_st ((core_struct_t*)pvParameters)

/**
  * @brief Функция вызываемая периодически задачей для обработки команд  
  * @param None
  * @retval None
  */
void ProcessingCoreCmdRX(void) 
{
  // Если очередь команд открыта 
  if (st_core.xQueueCoreCMD != NULL)
  {
    // Проверка очереди команд
    if ( xQueueReceive( st_core.xQueueCoreCMD, &(st_core.data_core_rx_cmd), ( TickType_t ) 0 ) == pdTRUE )
    {
      // Получена команда - запуск обработки
      switch(st_core.data_core_rx_cmd.CMD_ID)
      {
      case CMD_ALARM_PORT: // Получена команда аварии - устаналиваем флаги аварии
        // Получена команда - запуск обработки
        switch(st_core.data_core_rx_cmd.PortID)
        {
        case CorePortID:        // Команда получена от порта CorePortID
          break;          
         case VoiceETHPortID:    // Команда получена от порта VoiceETHPortID
          st_core.alarm_eth = 1;   // 8.Авария ETH
          break;              
        case CodecPortID:       // Команда получена от порта CodecPortID
          break;    	         
        } 
        break;
      }      
    }
  } 
}

/**
 * @brief  Функция инициализации при запуске задачи ядра
 * @param  None
 * @retval None
 */
void InitCoreTask( void )
{
 /* Открытие группы событий xCoreOutEvent */
 xCoreOutEvent = xEventGroupCreate();
 /* 0ткрытие группы событий xCoreInEvent  */
 xCoreInEvent = xEventGroupCreate();
 // Функция инициализхации параметров диагностики ядра
 InitDiagnostic(); 
 /* Обнуления всей таблицы тегов */
 ClearTableTag();
 /* Функция обнуления всей таблицы устройств */
 ClearTableDev();
}

/**
 * @brief  Функция обработки пакетов получаемых от роутера
 * @param  None
 * @retval None
 */
void ProcessingRouterToCoreBox( void )
{
  bool Processed = false;
  
  // Получаем пакет от роутера
  if (st_core.xQueue_router_core != NULL)
  {
    //Проверка наличия данных от роутера
    while(xQueueReceive(st_core.xQueue_router_core,(void *)st_core.damp_data_rx, 0 ) == pdTRUE)
    {
      // Прием данных от роутера
      // Регистрация тегов нового формата позиционирования
      Processed = ProcessingNewPositionBox(&(st_core.data_rx));
      /* Функция парсинга пакета пинг */
      if (Processed == false) ProcessinRespPing(&(st_core.data_rx));   
      /* Функция парсинга пакета диагностики */
      if (Processed == false) ProcessingDiagDev(&(st_core.data_rx));          
      // Обработка запросов диагностики
      if (Processed == false) Processed = ProcessingDiagnosticBox(&st_core);
      // Обработка запросов конфигурирования
      if (Processed == false) Processed = ProcessingConfigBox(&st_core); 
      // Обработка запросов обновления ПО
      if (Processed == false) Processed = ProcessingUploadBox(&st_core);

      /* Включаем в ядре обработку пакетов модбас */
      #ifdef MODBUS_CORE_MODE
      // Обработка запросов модбас 
      if (Processed == false) Processed = ProcessingModbusBox(&st_core);      
      #endif         
    }
  }
}

/**
 * @brief  Функция вызываемая периодически задачей ядра
 * @param  None
 * @retval None
 */
void PeriodicFuncCoreTask( void )
{
  /* Функция обработки пакетов получаемых от роутера */
  ProcessingRouterToCoreBox();
  
  Core_ADC_Control(&st_core);  
  /* Функция обновляет время жизни зарегистрированного тега, если время жизни исчерпано удаляем тег из таблицы */
  /* Эта функция обновления времени жизни канала голосового тега */
  UpdateTagTable( st_core.control_time );  
  /* Функция обновляет маску каналов прослушки */
  UpdateChanelMask( &st_core,st_core.control_time );
  /* Функция вызываемая периодически задачей для обработки команд для ядра      */
  ProcessingCoreCmdRX(); 
}

/**
 * @brief  Задача для контроля RS_UART
 * @param  pvParameters not used
 * @retval None
 */
void Core_Control_Task(void* pvParameters)
{
  // Регистрирование очереди команд в таблице
  if (core_st->xQueueCoreCMD != NULL)
  {
    // Ожидание инициализации указателя в таблице команд
    while(BaseQCMD[core_st->set_port_core_router.PortID].Status_QCMD != QCMD_INIT)
    {
      // Ожидание 1 млсек
      vTaskDelay(1);
    }
    // Обнуление указaтеля
    BaseQCMD[core_st->set_port_core_router.PortID].QueueCMD = core_st->xQueueCoreCMD;
    // Установка статуса
    BaseQCMD[core_st->set_port_core_router.PortID].Status_QCMD = QCMD_ENABLE;          
  }  
  
  // Инициализации ресурсов задачи
  /*--------------------------------------------------------------------------*/
  // Функция инициализации при запуске задачи ядра
  InitCoreTask();
    
  /* Запуск програмных таймеров */
  xTimerStart( core_st->xSoftTimer     , 0 );
  xTimerStart( core_st->xSoftTimerDiag , 0 );  
  xTimerStart( core_st->xSoftTimerPosition , 0 );    
  xTimerStart( core_st->xSoftTimerPing , 0 );   
   
  /*---------- Подключение данного ресурса к мультиплексору ------------------*/
  // Запрашиваем ID порта маршрутизации
  while(request_port_pnt(&(core_st->index_router_port)) != true) vTaskDelay(1);
  // Инициализация порта
  settings_port_pnt(core_st->index_router_port,&(core_st->set_port_core_router));
  // Включаем порт
  enable_port_pnt(core_st->index_router_port);
  // Цикл периодического запуска задачи
  for( ;; )
  {
    /* Обнуляем сообщение */
    core_st->NotifiedValue = 0;
    /*================================== Проверка наличия сообщений ========================================*/
    xTaskNotifyWait(0x00000000,                 /* Не очищайте биты уведомлений при входе               */
                    0xFFFFFFFF,                 /* Сбросить значение уведомления до 0 при выходе        */
                    &(core_st->NotifiedValue),  /* Значение уведомленное передается в  NotifiedValue    */
                    portMAX_DELAY  );           /* Блокировка задачи до появления уведомления           */
    /* Получено уведомление. Проверяем, какие биты были установлены. */
    /*=========================================================================*/
    /*=========================================================================*/
    /*=========================================================================*/ 
    if( ( ( core_st->NotifiedValue ) & ROUTER_NOTE ) != 0 )
    {  
      /* Функция обработки пакетов получаемых от роутера */
      ProcessingRouterToCoreBox();
    }        
    /*=========================================================================*/
    /*=========================================================================*/      
    if( ( ( core_st->NotifiedValue ) & TIMER_NOTE ) != 0 )
    { /* Периодическое уведомление по таймеру */
      PeriodicFuncCoreTask();
      /* Функция обновляет время жизни зарегистрированного устройства, если время жизни исчерпано */
      UpdateDevTable();      
    }   
    /*=========================================================================*/
    /*=========================================================================*/      
    if( ( ( core_st->NotifiedValue ) & REQ_DIAG_NOTE ) != 0 )
    { /* Запрос диагностики */
      UpdateDiagnostic(&st_core);
    }       
    /*=========================================================================*/
    /*=========================================================================*/      
    if( ( ( core_st->NotifiedValue ) & POSITION_NOTE ) != 0 )
    { /* Запрос  формирования пакета отчета о позиционировании */
      ProcessingPositionReport( &st_core );
    }      
    /*=========================================================================*/
    /*=========================================================================*/      
    if( ( ( core_st->NotifiedValue ) & UPTIME_NOTE ) != 0 )
    { /* Диагностика сети RS по запросу */
      UpdateProcDiagramDev();
    }         
  }
}

/**
  * @brief  Функция отработки програмного таймера 
  * @param  TimerHandle_t pxTimer - указатель на таймер вызвавщий функцию
  * @retval None
  */
void TimCallbackCore( TimerHandle_t pxTimer )
{
  /* Функция обработки програмного таймера.*/
  if ( ( st_core.set_port_core_router.HandleTask ) != NULL  )
  {
    if ( st_core.xSoftTimer == pxTimer )
    {/* Устанавливаем событие срабатывания програмного таймера для периодического уведомления */
      xTaskNotify( st_core.set_port_core_router.HandleTask,  /* Указатель на уведомлюемую задачу                         */
                  TIMER_NOTE,                                /* Значения уведомления                                     */
                  eSetBits );                                /* Текущее уведомление добавляются к уже прописанному       */
    }
    if ( st_core.xSoftTimerDiag == pxTimer )
    {/* Устанавливаем событие срабатывания програмного таймера для периодического уведомления */
      xTaskNotify( st_core.set_port_core_router.HandleTask,  /* Указатель на уведомлюемую задачу                         */
                  REQ_DIAG_NOTE,                             /* Значения уведомления                                     */
                  eSetBits );                                /* Текущее уведомление добавляются к уже прописанному       */
    }   
    if ( st_core.xSoftTimerPosition == pxTimer )
    {/* Устанавливаем событие срабатывания програмного таймера для формирования отчета о позиционировании                */
      xTaskNotify( st_core.set_port_core_router.HandleTask,  /* Указатель на уведомлюемую задачу                         */
                  POSITION_NOTE,                             /* Значения уведомления                                     */
                  eSetBits );                                /* Текущее уведомление добавляются к уже прописанному       */
    }      
    
    if ( st_core.xSoftTimerPing == pxTimer )
    {/* Устанавливаем событие срабатывания програмного таймера для формирования отчета о позиционировании                */
      xTaskNotify( st_core.set_port_core_router.HandleTask,  /* Указатель на уведомлюемую задачу                         */
                  UPTIME_NOTE,                               /* Значения уведомления                                     */
                  eSetBits );                                /* Текущее уведомление добавляются к уже прописанному       */
    }      
  }
}

/**
 * @brief  Функция инициализации задачи ядра
 * @param  None
 * @retval None
 */
void InitCore( void )
{
  // Подготовка массива очередей команд к инициализации
  for( uint8_t cnt_queue_cmd = 0; cnt_queue_cmd < MAX_NUMBER_PortID ;cnt_queue_cmd++)
  {
    // Обнуление укаазтеля
    BaseQCMD[cnt_queue_cmd].QueueCMD = NULL;
    // Установка статуса
    BaseQCMD[cnt_queue_cmd].Status_QCMD = QCMD_INIT;        
  }  
  // Установка размера входящей / исходящей очереди
  st_core.size_queue_core_router = 4;
  st_core.size_queue_router_core = 8;
  
  // Период вызова задачи ядра
  st_core.control_time = TIME_CORE_UPDATE;
  
  // Установим имя задачи для отладки
  st_core.pcName = "CORE";
  
  // Открытие очереди для пакетов router to core
  if ((st_core.xQueue_core_router == NULL) && (st_core.size_queue_core_router > 0))
  {
    //открытие очереди для size_queue_core_router пакетов
    st_core.xQueue_core_router = xQueueCreate( st_core.size_queue_core_router , sizeof(router_box_t));
  }
  // Открытие очереди для пакетов core to router
  if ((st_core.xQueue_router_core == NULL) && (st_core.size_queue_router_core > 0))
  {
    //открытие очереди для size_queue_router_core пакетов
    st_core.xQueue_router_core = xQueueCreate( st_core.size_queue_router_core , sizeof(router_box_t));
  }
  // Открытие очереди для приема комманд
  if ( st_core.xQueueCoreCMD == NULL )
  {
    //открытие очереди для приема MAX_SIZE_QUEUE_CNTRL_CMD комманд
    st_core.xQueueCoreCMD = xQueueCreate( MAX_SIZE_QUEUE_CNTRL_CMD , sizeof(cntrl_cmd_t));
  }
  // Проводим инициализацию подключения к мультиплексору
  st_core.set_port_core_router.FlagLockAddr = UNLOCK_ADDR;                      //не блокирует трансляцию в порт со своим адресом
  st_core.set_port_core_router.MaskPortID =  DataSto.Settings.mask_inpup_port_id_core; //установка маску ID портов
  st_core.set_port_core_router.NumMaskBoxID = DataSto.Settings.nmask_transl_core;                    //используем маску ID пакетов
  st_core.set_port_core_router.PortID = CorePortID;                       //Идентификатор порта  
  st_core.set_port_core_router.PortBitID = BitID(CorePortID);                       //Идентификатор порта
  st_core.set_port_core_router.QueueInRouter = st_core.xQueue_core_router;      //очередь исходящих
  st_core.set_port_core_router.QueueOutRouter = st_core.xQueue_router_core;     //очередь входящих
  st_core.set_port_core_router.HandleTaskRouter = NULL;                         /* Обнуление указателя на задачу                */ 
  
  st_core.tag_ch_stat[0].SrcPortID = RSAPortID;    //  порт RFPortID
  st_core.tag_ch_stat[0].NameSrcPortID = "Видимые теги по RSA";
  st_core.tag_ch_stat[0].ShotNameSrcPortID = "RSPortA";  
  st_core.tag_ch_stat[1].SrcPortID = RSBPortID;    //  порт RSPortID
  st_core.tag_ch_stat[1].NameSrcPortID = "Видимые теги по RSB";
  st_core.tag_ch_stat[1].ShotNameSrcPortID = "RSPortB";
  
  /* Открытие таймера периодического уведомления задачи */
  st_core.xSoftTimer = xTimerCreate( "TmNtCore",       /* Текстовое имя, не используется в RTOS kernel. */
                                    (TickType_t)st_core.control_time, /* Период таймера в тиках. */
                                    pdTRUE,            /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                    NULL,              /* В функцию параметры не передаем */
                                    TimCallbackCore ); /* Указатель на функцию , которую вызывает таймер. */

  /* Открытие таймера запроса подсчета статистики */
  st_core.xSoftTimerDiag = xTimerCreate( "TmDgCore",   /* Текстовое имя, не используется в RTOS kernel. */
                                    (TickType_t)DataSto.Settings.time_update_diagn,              /* Период таймера в тиках. */
                                    pdTRUE,            /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                    NULL,              /* В функцию параметры не передаем */
                                    TimCallbackCore ); /* Указатель на функцию , которую вызывает таймер. */  
  
  /* Открытие таймера запроса формирования отчета позиционирования */
  st_core.xSoftTimerPosition = xTimerCreate( "TmPsCore",   /* Текстовое имя, не используется в RTOS kernel. */
                                    5000,              /* Период таймера в тиках. */
                                    pdTRUE,            /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                    NULL,              /* В функцию параметры не передаем */
                                    TimCallbackCore ); /* Указатель на функцию , которую вызывает таймер. */   
  
  /* Открытие таймера запроса пинга для построения сети */
  st_core.xSoftTimerPing = xTimerCreate( "TmPing",              /* Текстовое имя, не используется в RTOS kernel. */
                                    MAX_INTERAL_TEST_TABLE_DEV, /* Период таймера в тиках. */
                                    pdTRUE,                     /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                    NULL,                       /* В функцию параметры не передаем */
                                    TimCallbackCore );          /* Указатель на функцию , которую вызывает таймер. */     
  
  // Открытие задачи контроля RS_UART
  xTaskCreate( Core_Control_Task, st_core.pcName, configMINIMAL_STACK_SIZE * 2, (void *)&st_core, CNTRL_CORE_TASK_PRIO, &(st_core.set_port_core_router.HandleTask) );
}
/**
 * @brief  Функция предназначена тестовых таблиц маршрутизации по ID пакета.
 * @param  none
 * @retval none
 */
void init_mask_id(void)
{
 
        //#define MaskBoxIDCore          (0x00)
	//#define NEW_POSITION            0x05 //
	//#define DIAGNOSTICS_REQ         0x14 //20 Запросы команд диагностики
	//#define DIAGNOSTICS_RESP        0x15 //21 Ответы команд диагностики
	//#define CMD_TAG_REQ             0xF5 //245 Запросы команд для управления тегами
	//#define CMD_TAG_RESP            0xF6 //246 Ответы команд для управления тегами
	//#define REG_MASK_REQ            0xF7 //247 Запросы команд с регистрами масок портов
	//#define REG_MASK_RESP           0xF8 //248 Ответы команд с регистрами масок портов
	//#define SETUP_REQ               0xFB //251 Запросы команд настроек
	//#define SETUP_RESP              0xFC //252 Ответы команд настроек
	//#define BOOTLOADER_REQ          0xFD //253 Запросы команд бутлоадера
        //#define ID_MODBUS_MASTER        0xF9 //249 Пакет MODBUS от мастера к подчиненному
        //#define ID_SETUP_RF_REQ         0xE2 //251 Запросы команд настроек через RF
        //#define ID_BOOTLOADER_RF_REQ    0xE4 // Запросы команд бутлоадера

	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCore].mask_id[0x00] = CIDX5                                                            ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCore].mask_id[0x01] = CIDX4 | CIDX5                                                    ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCore].mask_id[0x02] = 0x0000                                                           ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCore].mask_id[0x03] = 0x0000                                                           ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCore].mask_id[0x04] = 0x0000                                                           ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCore].mask_id[0x05] = 0x0000                                                           ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCore].mask_id[0x06] = 0x0000                                                           ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCore].mask_id[0x07] = 0x0000                                                           ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCore].mask_id[0x08] = 0x0000                                                           ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCore].mask_id[0x09] = 0x0000                                                           ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCore].mask_id[0x0a] = 0x0000                                                           ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCore].mask_id[0x0b] = 0x0000                                                           ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCore].mask_id[0x0c] = 0x0000                                                           ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCore].mask_id[0x0d] = 0x0000                                                           ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCore].mask_id[0x0e] = CIDX2 | CIDX4                                                    ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCore].mask_id[0x0f] = CIDX5 | CIDX6 | CIDX7 | CIDX8 | CIDX9 | CIDXB |  CIDXC |  CIDXD  ;

	//#define MaskBoxIDCodec          (0x01)  
	//#define VOICE                   0x01 //1 Голосовые данные
	//#define CALL_DISP               0x03 //3 Вызов диспетчера
	//#define REQ_DISP                0x04 //4 Подтверждение вызова диспетчера

	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCodec].mask_id[0x00] = CIDX1 | CIDX3 | CIDX4  ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCodec].mask_id[0x01] = 0x0000                 ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCodec].mask_id[0x02] = 0x0000                 ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCodec].mask_id[0x03] = 0x0000                 ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCodec].mask_id[0x04] = 0x0000                 ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCodec].mask_id[0x05] = 0x0000                 ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCodec].mask_id[0x06] = 0x0000                 ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCodec].mask_id[0x07] = 0x0000                 ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCodec].mask_id[0x08] = 0x0000                 ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCodec].mask_id[0x09] = 0x0000                 ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCodec].mask_id[0x0a] = 0x0000                 ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCodec].mask_id[0x0b] = 0x0000                 ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCodec].mask_id[0x0c] = 0x0000                 ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCodec].mask_id[0x0d] = 0x0000                 ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCodec].mask_id[0x0e] = 0x0000                 ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDCodec].mask_id[0x0f] = 0x0000                 ;        
        
	//#define MaskBoxIDVoiceETH       (0x02)
	//#define VOICE                   0x01 //1 Голосовые данные
	//#define CALL_DISP               0x03 //3 Вызов диспетчера
	//#define REQ_DISP                0x04 //4 Подтверждение вызова диспетчера
        //#define NEW_POSITION            0x05 //
        //#define ID_INDIVID_RSN_CH       0x06 //6 Команда запроса на формировани индивидуального канала
                             
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceETH].mask_id[0x00] = CIDX1 | CIDX3 | CIDX4 | CIDX5 | CIDX6 ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceETH].mask_id[0x01] = 0x0000                                ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceETH].mask_id[0x02] = 0x0000                                ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceETH].mask_id[0x03] = 0x0000                                ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceETH].mask_id[0x04] = 0x0000                                ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceETH].mask_id[0x05] = 0x0000                                ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceETH].mask_id[0x06] = 0x0000                                ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceETH].mask_id[0x07] = 0x0000                                ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceETH].mask_id[0x08] = 0x0000                                ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceETH].mask_id[0x09] = 0x0000                                ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceETH].mask_id[0x0a] = 0x0000                                ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceETH].mask_id[0x0b] = 0x0000                                ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceETH].mask_id[0x0c] = 0x0000                                ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceETH].mask_id[0x0d] = 0x0000                                ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceETH].mask_id[0x0e] = 0x0000                                ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceETH].mask_id[0x0f] = 0x0000                                ; 
        
	//#define MaskBoxDublicate                (0x03)  
        //#define ID_VOICE                   0x01 //1 Голосовые данные
        //#define ID_RING_DISP               0x03 //3 Вызов диспетчера
        //#define ID_REQ_DISP                0x04 //4 Подтверждение вызова диспетчера
        //#define ID_INDIVID_RSN_CH          0x06 //6 Команда запроса на формировани индивидуального канала
        //#define ID_CMD_TAG_REQ             0xF5 //245 Запросы команд для управления тегами
        //#define ID_CMD_TAG_RESP            0xF6 //246 Ответы команд для управления тегами
        
	DataSto.Settings.tabl_mask_box_id[MaskBoxDublicate].mask_id[0x00] = CIDX1 | CIDX3 | CIDX4 | CIDX6 ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxDublicate].mask_id[0x01] = 0x0000                        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxDublicate].mask_id[0x02] = 0x0000                        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxDublicate].mask_id[0x03] = 0x0000                        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxDublicate].mask_id[0x04] = 0x0000                        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxDublicate].mask_id[0x05] = 0x0000                        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxDublicate].mask_id[0x06] = 0x0000                        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxDublicate].mask_id[0x07] = 0x0000                        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxDublicate].mask_id[0x08] = 0x0000                        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxDublicate].mask_id[0x09] = 0x0000                        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxDublicate].mask_id[0x0a] = 0x0000                        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxDublicate].mask_id[0x0b] = 0x0000                        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxDublicate].mask_id[0x0c] = 0x0000                        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxDublicate].mask_id[0x0d] = 0x0000                        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxDublicate].mask_id[0x0e] = 0x0000                        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxDublicate].mask_id[0x0f] = CIDX5 | CIDX6                 ;
        
        //#define MaskBoxIDETH                    (0x04)     
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDETH].mask_id[0x00] = 0x0000            ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDETH].mask_id[0x01] = 0x0000            ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDETH].mask_id[0x02] = 0x0000            ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDETH].mask_id[0x03] = 0x0000            ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDETH].mask_id[0x04] = 0x0000            ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDETH].mask_id[0x05] = 0x0000            ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDETH].mask_id[0x06] = 0x0000            ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDETH].mask_id[0x07] = 0x0000            ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDETH].mask_id[0x08] = 0x0000            ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDETH].mask_id[0x09] = 0x0000            ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDETH].mask_id[0x0a] = 0x0000            ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDETH].mask_id[0x0b] = 0x0000            ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDETH].mask_id[0x0c] = 0x0000            ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDETH].mask_id[0x0d] = 0x0000            ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDETH].mask_id[0x0e] = 0x0000            ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDETH].mask_id[0x0f] = 0x0000            ;        
                
        //#define MaskBoxIDtubeETH                (0x05) 
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDtubeETH].mask_id[0x00] = 0x0000        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDtubeETH].mask_id[0x01] = 0x0000        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDtubeETH].mask_id[0x02] = 0x0000        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDtubeETH].mask_id[0x03] = 0x0000        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDtubeETH].mask_id[0x04] = 0x0000        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDtubeETH].mask_id[0x05] = 0x0000        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDtubeETH].mask_id[0x06] = 0x0000        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDtubeETH].mask_id[0x07] = 0x0000        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDtubeETH].mask_id[0x08] = 0x0000        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDtubeETH].mask_id[0x09] = 0x0000        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDtubeETH].mask_id[0x0a] = 0x0000        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDtubeETH].mask_id[0x0b] = 0x0000        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDtubeETH].mask_id[0x0c] = 0x0000        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDtubeETH].mask_id[0x0d] = 0x0000        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDtubeETH].mask_id[0x0e] = 0x0000        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDtubeETH].mask_id[0x0f] = CIDXC | CIDXE ;              


        //#define MaskBoxIDRSAPort                (0x06)   
	//#define ID_VOICE                   0x01 //1 Голосовые данные
	//#define ID_REQ_DISP                0x04 //4 Подтверждение вызова диспетчера
        //#define ID_INDIVID_RSN_CH          0x06 //6 Команда запроса на формировани индивидуального канала
	//#define ID_DIAGNOSTICS_REQ         0x14 //20 Запросы команд диагностики
        //#define ID_DIAG_HIGHWAY_RS_REQ     0x16 //22 Запросы команд диагностики магистрали RS
        //#define ID_DIAG_HIGHWAY_RS_RESP    0x17 //23 Ответы команд диагностики  магистрали RS
	//#define ID_CMD_TAG_REQ             0xF5 //245 Запросы команд для управления тегами
	//#define ID_REG_MASK_REQ            0xF7 //247 Запросы команд с регистрами масок портов
        //#define ID_MODBUS_MASTER           0xF9 //249 Пакет MODBUS от мастера к подчиненному
        //#define ID_MODBUS_SLAVE            0xFA //250 Пакет MODBUS от подчиненному к мастеру
	//#define ID_SETUP_REQ               0xFB //251 Запросы команд настроек
	//#define ID_BOOTLOADER_REQ          0xFD //253 Запросы команд бутлоадера
        
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSAPort].mask_id[0x00] = CIDX1 | CIDX4 | CIDX6                        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSAPort].mask_id[0x01] = CIDX4 | CIDX6 | CIDX7                        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSAPort].mask_id[0x02] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSAPort].mask_id[0x03] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSAPort].mask_id[0x04] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSAPort].mask_id[0x05] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSAPort].mask_id[0x06] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSAPort].mask_id[0x07] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSAPort].mask_id[0x08] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSAPort].mask_id[0x09] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSAPort].mask_id[0x0a] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSAPort].mask_id[0x0b] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSAPort].mask_id[0x0c] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSAPort].mask_id[0x0d] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSAPort].mask_id[0x0e] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSAPort].mask_id[0x0f] = CIDX5 | CIDX7 | CIDX9 | CIDXA | CIDXB | CIDXD;         
        
        //#define MaskBoxIDRSBPort                (0x07)   
	//#define ID_VOICE                   0x01 //1 Голосовые данные
	//#define ID_REQ_DISP                0x04 //4 Подтверждение вызова диспетчера
        //#define ID_INDIVID_RSN_CH          0x06 //6 Команда запроса на формировани индивидуального канала
	//#define ID_DIAGNOSTICS_REQ         0x14 //20 Запросы команд диагностики
        //#define ID_DIAG_HIGHWAY_RS_REQ     0x16 //22 Запросы команд диагностики магистрали RS
        //#define ID_DIAG_HIGHWAY_RS_RESP    0x17 //23 Ответы команд диагностики  магистрали RS
	//#define ID_CMD_TAG_REQ             0xF5 //245 Запросы команд для управления тегами
	//#define ID_REG_MASK_REQ            0xF7 //247 Запросы команд с регистрами масок портов
        //#define ID_MODBUS_MASTER           0xF9 //249 Пакет MODBUS от мастера к подчиненному
        //#define ID_MODBUS_SLAVE            0xFA //250 Пакет MODBUS от подчиненному к мастеру
	//#define ID_SETUP_REQ               0xFB //251 Запросы команд настроек
	//#define ID_BOOTLOADER_REQ          0xFD //253 Запросы команд бутлоадера
        
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSBPort].mask_id[0x00] = CIDX1 | CIDX4 | CIDX6                        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSBPort].mask_id[0x01] = CIDX4 | CIDX6 | CIDX7                        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSBPort].mask_id[0x02] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSBPort].mask_id[0x03] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSBPort].mask_id[0x04] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSBPort].mask_id[0x05] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSBPort].mask_id[0x06] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSBPort].mask_id[0x07] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSBPort].mask_id[0x08] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSBPort].mask_id[0x09] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSBPort].mask_id[0x0a] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSBPort].mask_id[0x0b] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSBPort].mask_id[0x0c] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSBPort].mask_id[0x0d] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSBPort].mask_id[0x0e] = 0x0000                                       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDRSBPort].mask_id[0x0f] = CIDX5 | CIDX7 | CIDX9 | CIDXA | CIDXB | CIDXD;   

        //#define MaskBoxIDVoiceInfo              (0x08)  
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceInfo].mask_id[0x00] = CIDX1    ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceInfo].mask_id[0x01] = 0x0000   ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceInfo].mask_id[0x02] = 0x0000   ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceInfo].mask_id[0x03] = 0x0000   ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceInfo].mask_id[0x04] = 0x0000   ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceInfo].mask_id[0x05] = 0x0000   ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceInfo].mask_id[0x06] = 0x0000   ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceInfo].mask_id[0x07] = 0x0000   ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceInfo].mask_id[0x08] = 0x0000   ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceInfo].mask_id[0x09] = 0x0000   ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceInfo].mask_id[0x0a] = 0x0000   ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceInfo].mask_id[0x0b] = 0x0000   ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceInfo].mask_id[0x0c] = 0x0000   ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceInfo].mask_id[0x0d] = 0x0000   ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceInfo].mask_id[0x0e] = 0x0000   ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxIDVoiceInfo].mask_id[0x0f] = 0x0000   ;          
        
        
        //#define MaskBoxRSPOWER                  (0x09)      
	//#define ID_DIAGNOSTICS_REQ         0x14 //20 Запросы команд диагностики
	//#define ID_SETUP_REQ               0xFB //251 Запросы команд настроек
	//#define ID_BOOTLOADER_REQ          0xFD //253 Запросы команд бутлоадера
        
	DataSto.Settings.tabl_mask_box_id[MaskBoxRSPOWER].mask_id[0x00] = 0x0000       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxRSPOWER].mask_id[0x01] = CIDX4        ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxRSPOWER].mask_id[0x02] = 0x0000       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxRSPOWER].mask_id[0x03] = 0x0000       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxRSPOWER].mask_id[0x04] = 0x0000       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxRSPOWER].mask_id[0x05] = 0x0000       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxRSPOWER].mask_id[0x06] = 0x0000       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxRSPOWER].mask_id[0x07] = 0x0000       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxRSPOWER].mask_id[0x08] = 0x0000       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxRSPOWER].mask_id[0x09] = 0x0000       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxRSPOWER].mask_id[0x0a] = 0x0000       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxRSPOWER].mask_id[0x0b] = 0x0000       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxRSPOWER].mask_id[0x0c] = 0x0000       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxRSPOWER].mask_id[0x0d] = 0x0000       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxRSPOWER].mask_id[0x0e] = 0x0000       ;
	DataSto.Settings.tabl_mask_box_id[MaskBoxRSPOWER].mask_id[0x0f] = CIDXB | CIDXD;           

}
/******************* (C) COPYRIGHT 2019 DataExpress *****END OF FILE****/
