/**
  ******************************************************************************
  * @file    audio_gate.c
  * @author  Trembach D.N.
  * @version V2.3.0
  * @date    15-06-2020
  * @brief   Файл содержит функции шлюза аудиокодека 
  ******************************************************************************
  * @attention
  *
  * 
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include <string.h>
#include "stm32f4x7_eth.h"
#include "FreeRTOS.h"
#include "task.h"
#include "printf_dbg.h" 
#include "hooks.h"
#include "board.h"
#include "settings.h"
#include "audio_gate.h"
#include "codec_i2s.h"
#include "codec_i2c.h"
#include "UDA1380.h"

/* Очередь сообщения для аудио кодека */
QueueHandle_t       x_audio_message; 
/* Указатель для уведомления задачи контроля данных аудио кодека */
TaskHandle_t        handle_codec_audio_task;
/* Создаем структуру контроля аудио кодека */
audio_codec_gate_t  GateAudioCodec;

/* Структура для подготовки сообщения DMA_PCM передаваемого кодеку */
mes_codec_t audio_dma_mes;

/**
  * @brief  Функция отработки програмного таймера 
  * @param  TimerHandle_t pxTimer - указатель на таймер вызвавщий функцию
  * @retval None
  */
void TimUpdateNoteAudioTask( TimerHandle_t pxTimer )
{
  /* Функция обработки програмного таймера.*/
  if ( ( handle_codec_audio_task ) != NULL  )
  {/* Устанавливаем событие срабатывания програмного таймера для периодического уведомления */
    xTaskNotify( handle_codec_audio_task,   /* Указатель на уведомлюемую задачу                         */
                TIMER_NOTE,                 /* Значения уведомления                                     */
                eSetBits );                 /* Текущее уведомление добавляются к уже прописанному       */
  }
}

/**
  * @brief  Функция функция загрузки пакетов из soft_router и контроля захвата потока канала
  * @param  audio_codec_gate_t* pcm_audio - указатель на структуру контроля кодека
  * @retval None
  */
void ProcessingReadBoxForAudio( audio_codec_gate_t* pcm_audio )
{
  uint16_t cnt_index_data;
  uint8_t temp_priority;  

  /* Проверка наличия данных в   */    
  /* Функция получения указателя на текущий буффер для чтения */
  pcm_audio->pcm_pfifo_box = GetRDPointFIFO( &(pcm_audio->pcm_index_rd_box) , pcm_audio->pcm_mask_chanel , pcm_audio->pcm_mask_index_port );
  /* Если указатель корректный */      
  if ( (pcm_audio->pcm_pfifo_box) != NULL )
  { 
    /* подсчет полученных пакетов */
    (pcm_audio->pcm_cnt_rx_box)++;
    
    /* Нормируем приоритет канала полученного пакета */
    temp_priority = pcm_audio->pcm_pfifo_box->data_box.priority_box;  
    /* Если приоритет выходит за диапазон - устанавливаем приоритет по умолчанию */
    if ( ( temp_priority < VOICE_PRIOR_HIGH ) || ( temp_priority > VOICE_PRIOR_LOW ) )
    {
      /* Назначение приоритета по умолчанию */
      temp_priority = pcm_audio->pcm_ch_priority;
    }
    
    /* Анализ счетчика пустых интервал в захваченом потоке канала и появления канала с более высоким приоритетом */ 
    /* Если число пропущенных пакетов больше MAX_EMPTY_INTERVAL или появились в канале пакеты с более высоким    */
    /* приоритетом - сбрасываем захват канала и запускаем алгоритм захвата канала                                */
    if ( ( pcm_audio->pcm_empty_interval_counter >= MAX_EMPTY_INTERVAL ) || ( temp_priority < pcm_audio->pcm_lock_ch_priority ) )
    {
      /* Обнуление индексов буфера */
      pcm_audio->pcm_wr_buf_index = 0;
      pcm_audio->pcm_rd_buf_index = 0; 
      /* Обнуление флагов буфера */
      for(cnt_index_data = 0;  cnt_index_data  < MAX_BUF_RX_PCM_BOX ; cnt_index_data++)
      {
        /* Маркируем пакет как пустой */
        pcm_audio->pcm_buf_box[cnt_index_data].flag_box = BOX_EMPTY;             
      }      
      /* Поток канала потерян - канал захвачен*/
      pcm_audio->pcm_status_wr = SLEEP_STREAM;
      pcm_audio->pcm_empty_interval_counter = 0;
      
      /* Предварительные захват канала */
      pcm_audio->pcm_lock_ch_id            = pcm_audio->pcm_pfifo_box->data_box.ch_id;          /* Идентификатор канала                    */
      pcm_audio->pcm_lock_dest_phy_addr    = pcm_audio->pcm_pfifo_box->data_box.dest_phy_addr;  /* Физический адрес получателя             */
      pcm_audio->pcm_lock_src_phy_addr     = pcm_audio->pcm_pfifo_box->data_box.src_phy_addr;   /* Физический адрес источника              */
      pcm_audio->pcm_lock_codec_phy_addr   = pcm_audio->codec_phy_addr;                         /* Физический адрес шлюза/кодека           */   
      pcm_audio->pcm_lock_ch_priority      = temp_priority;                                     /* Приоритет канала                        */
    } 
    
    /* Если пакет данных не соответствует захваченному каналу - выход */ 
    if ( pcm_audio->pcm_lock_src_phy_addr  != pcm_audio->pcm_pfifo_box->data_box.src_phy_addr  ) return;
    if ( pcm_audio->pcm_lock_dest_phy_addr != pcm_audio->pcm_pfifo_box->data_box.dest_phy_addr ) return;
    
    /* Копирование данных в промежуточный буфер */
    for(cnt_index_data = 0;  cnt_index_data < (sizeof(soft_router_box_t))/4 ; cnt_index_data++)
    {
      ((uint32_t*)(&(pcm_audio->pcm_buf_box[pcm_audio->pcm_wr_buf_index].router_box)))[cnt_index_data] = ((uint32_t*)(pcm_audio->pcm_pfifo_box))[cnt_index_data];               
    }
    /* Маркируем пакет в буфере как полный */
    pcm_audio->pcm_buf_box[pcm_audio->pcm_wr_buf_index].flag_box = BOX_FULL;
        
    /* Инкремент индекса записи пакета */
    (pcm_audio->pcm_wr_buf_index)++;   
    /* Проверка на выход за границу чтения */
    if ( pcm_audio->pcm_wr_buf_index >= MAX_BUF_RX_PCM_BOX )   
    {
      /* Переход на нулевой индекс */
      pcm_audio->pcm_wr_buf_index = 0;
    }    
    
    /* Если канал не захвачен */          
    if ( pcm_audio->pcm_status_wr == SLEEP_STREAM )
    {
      /* Если в буфере накоплено MAX_START_BUFFER_STREAM пакетов  */
      if (pcm_audio->pcm_wr_buf_index >= ( MAX_START_BUFFER_STREAM ))
      {
        /* Меняем статут - поток захвачен */
        pcm_audio->pcm_status_wr = ACTIVE_STREAM;
      }
    }

    /* Если канал захвачен отправляем сообщение кодеку */
    if ( pcm_audio->pcm_status_wr == ACTIVE_STREAM )
    {
      /* Отправляем сообщение кодеку на переключение в режим микрофона */
      audio_dma_mes.data_event = MIC_EV;
      audio_dma_mes.index_codec = 0; 
      audio_dma_mes.data_codec = pcm_audio->audio_lock_ch_priority; /* Установить уровень запроса */
      xQueueSend( x_audio_message , (void*)&(audio_dma_mes), ( TickType_t ) 0 );
    }
  }  
}

/**
  * @brief  Функция функция обработки уведомления из прерывания DMA PCM
  * @param  audio_codec_gate_t* pcm_audio  - указатель на структуру контроля кодека
  * @param  uint8_t n_buffer - номер обрабатываемого буфера
  * @retval None
  */
void ProcessingNotifyDMAForAudio( audio_codec_gate_t* pcm_audio , uint8_t n_buffer )
{ 
uint16_t cnt_index_data;

  /*======================= Отработка записи в DMA ==========================*/
  /*== Данные несжатого голосового потока из области памяти загруженой DMA ==*/
  /*== дополняется шапкой и переносятся в программый роутер потоков.       ==*/
  if ( ( pcm_audio->pcm_status_wr == ACTIVE_STREAM ) && ( pcm_audio->pcm_buf_box[pcm_audio->pcm_rd_buf_index].flag_box == BOX_FULL ) && ( ( pcm_audio->analog_codec_mode == MODE_ANALOG_OUT ) || ( pcm_audio->analog_codec_mode == MODE_ANALOG_AUTO ) ) )
  { /* --------------       Загружаем в DMA пакет с данными    ---------------  */
    /* Копирование данных */
    for( cnt_index_data = 0 ; cnt_index_data < SIZE_SAMPLE_AUDIO_BUFF ; cnt_index_data = cnt_index_data + 2  )
    {
      pcm_audio->data_tx_i2s[n_buffer][cnt_index_data + 1] = pcm_audio->pcm_buf_box[pcm_audio->pcm_rd_buf_index].router_box.data_box.data[cnt_index_data>>1]; 
    }
    /* Сброс флага пакета */
    pcm_audio->pcm_buf_box[pcm_audio->pcm_rd_buf_index].flag_box = BOX_EMPTY;
    /* Инкремент индекса чтения пакета */
    (pcm_audio->pcm_rd_buf_index)++;
    /* Проверка на выход за границу чтения */
    if ( pcm_audio->pcm_rd_buf_index >= MAX_BUF_RX_PCM_BOX )   
    {
      /* Переход на нулевой индекс */
      pcm_audio->pcm_rd_buf_index = 0;
    }
    /* Сброс счетчика пустых интервалов */
    pcm_audio->pcm_empty_interval_counter = 0;
    
    /* Переключение статуса  */
    pcm_audio->codec_status_speaker = SLEEP_STREAM;
    /* Переключение аналоговой линии на выход */
    V_RX_SW_LINE_HIGH(); 
  }
  else
  { /* --------------       Загружаем в DMA пустой пакет       ---------------  */
    /* Копирование данных */
    for( cnt_index_data = 0 ; cnt_index_data < SIZE_SAMPLE_AUDIO_BUFF ; cnt_index_data = cnt_index_data + 2 )
    {
      pcm_audio->data_tx_i2s[n_buffer][cnt_index_data + 1] = 0; 
      pcm_audio->data_tx_i2s[n_buffer][cnt_index_data] = 0;       
    }    
    /* Подсчет пропущеных интервалов счет ограничиваем 10 */
    if ( pcm_audio->pcm_empty_interval_counter <= 10 ) (pcm_audio->pcm_empty_interval_counter)++;  
        
    /* В буфере есть пакет - освобождаем */
    if ( ( pcm_audio->pcm_buf_box[pcm_audio->pcm_rd_buf_index].flag_box == BOX_FULL ) )
    {
      /* Сброс флага пакета */
      pcm_audio->pcm_buf_box[pcm_audio->pcm_rd_buf_index].flag_box = BOX_EMPTY;
      /* Инкремент индекса чтения пакета */
      (pcm_audio->pcm_rd_buf_index)++;
      /* Проверка на выход за границу чтения */
      if ( pcm_audio->pcm_rd_buf_index >= MAX_BUF_RX_PCM_BOX )   
      {
        /* Переход на нулевой индекс */
        pcm_audio->pcm_rd_buf_index = 0;
      }
    }
    
    /* Управление статусом режима */
    if ( ( pcm_audio->analog_codec_mode == MODE_ANALOG_IN ) || ( pcm_audio->analog_codec_mode == MODE_ANALOG_AUTO ) )
    {
      /* Переключение статуса  */
      pcm_audio->codec_status_speaker = ACTIVE_STREAM;
      /* Переключение аналоговой линии на выход */
      V_RX_SW_LINE_LOW();    
    }
    else
    {
      /* Переключение статуса  */
      pcm_audio->codec_status_speaker = SLEEP_STREAM;
      /* Переключение аналоговой линии на выход */
      V_RX_SW_LINE_HIGH(); 
    }  
  }   
  /*=========================================================================*/
  
  /*== Данные несжатого голосового потока из области памяти загруженой DMA ==*/
  /*== дополняется шапкой и переносятся в программый роутер потоков.       ==*/
  if(pcm_audio->codec_status_speaker == ACTIVE_STREAM)
  {    
    /* Запрос указателя на пакет */
    pcm_audio->pcm_pfifo_box = GetWRPointFIFO();
    /* Если указатель корректный */
    if ( pcm_audio->pcm_pfifo_box != NULL)
    {
      /* Копирование данных */
      for( cnt_index_data = 0 ; cnt_index_data < SIZE_SAMPLE_AUDIO_BUFF ; cnt_index_data = cnt_index_data + 2 )
      {
        /* Считываем правый канал */
        pcm_audio->pcm_pfifo_box->data_box.data[cnt_index_data>>1] = pcm_audio->data_rx_i2s[n_buffer][cnt_index_data + 1];  
      }  
      
      /* Заполнение шапки пакета */
      pcm_audio->pcm_pfifo_box->source_port             = pcm_audio->pcm_source_port;          /* Порт источник                          */             
      pcm_audio->pcm_pfifo_box->data_box.ch_id          = pcm_audio->audio_lock_ch_id;         /* Идентификатор канала                   */
      pcm_audio->pcm_pfifo_box->data_box.dest_phy_addr  = pcm_audio->audio_lock_dest_phy_addr; /* Физический адрес получателя            */
      pcm_audio->pcm_pfifo_box->data_box.src_phy_addr   = pcm_audio->audio_lock_src_phy_addr;  /* Физический адрес источника             */
      pcm_audio->pcm_pfifo_box->data_box.codec_phy_addr = pcm_audio->codec_phy_addr;           /* Физический адрес шлюза/кодека          */   
      pcm_audio->pcm_pfifo_box->data_box.priority_box   = pcm_audio->audio_lock_ch_priority;   /* Приоритет канала                       */
      pcm_audio->pcm_pfifo_box->data_box.cnt            = pcm_audio->pcm_cnt++;                /* Cчетчик неприрывности пакетов 0..255   */
      pcm_audio->pcm_pfifo_box->data_box.time_id        = ulGetRuntimeCounterValue();          /* Временной идентификатор пакета         */
      
      /* Разрешение пакета для чтения */
      pcm_audio->pcm_pfifo_box->enable_read = 1;
      
      /* Рассылка оповешения зарегистрировавшихся на рассылку.*/
      notify_soft_port(pcm_audio->pcm_source_port);
      
      /* Подсчет переданных пакетов */
      pcm_audio->pcm_cnt_tx_box++;
    }
  }
  /*==========================================================================*/
}

/**
  * @brief  Функция инициализации и предустановки вводов/ выводов управления двухпроводной линией
  * @param  None 
  * @retval None
  */
void Init_GPIO_Line_Control(void)
{
  GPIO_InitTypeDef    GPIO_InitStructure;
  
  /* Управляющие выводы */
  /*   Заполнение структуры  */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    

  /* Подключение выхода усилителя кодека к линии */  
  /*   Вкл. тактирование вывода                 */
  RCC_AHB1PeriphClockCmd( V_RX_SW_LINE_GPIO_CLK, ENABLE );
  /*   Конфигурирование вывода                  */  
  GPIO_InitStructure.GPIO_Pin = V_RX_SW_LINE_PIN;
  GPIO_Init( V_RX_SW_LINE_GPIO_PORT, &GPIO_InitStructure );
  /*    Предустановка вывода                    */
  V_RX_SW_LINE_LOW();  

  /* Отключение линии FXO*/
  /*   Вкл. тактирование вывода                 */
  RCC_AHB1PeriphClockCmd( NUMBER_LINE_GPIO_CLK, ENABLE );
  /*   Конфигурирование вывода                  */  
  GPIO_InitStructure.GPIO_Pin = NUMBER_LINE_PIN;
  GPIO_Init( NUMBER_LINE_GPIO_PORT, &GPIO_InitStructure );
  /*    Предустановка вывода                    */
  NUMBER_LINE_LOW(); 

  /* Вводы контроля полярности */
  /*   Заполнение структуры  */    
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;  
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;   
    
   /*   Вкл. тактирование вывода                 */
  RCC_AHB1PeriphClockCmd( POLARITY1_LINE_GPIO_CLK, ENABLE );
  /*   Конфигурирование вывода                  */  
  GPIO_InitStructure.GPIO_Pin = POLARITY1_LINE_PIN;
  GPIO_Init( POLARITY1_LINE_GPIO_PORT, &GPIO_InitStructure );

   /*   Вкл. тактирование вывода                 */
  RCC_AHB1PeriphClockCmd( POLARITY2_LINE_GPIO_CLK, ENABLE );
  /*   Конфигурирование вывода                  */  
  GPIO_InitStructure.GPIO_Pin = POLARITY2_LINE_PIN;
  GPIO_Init( POLARITY2_LINE_GPIO_PORT, &GPIO_InitStructure );
}

/**
  * @brief  Задача данными кодеков CMX
  * @param  pvParameters not used
  * @retval None
  */
void codec_audio_task( void* pvParameters )
{
  /*=========== Функции инициализации аппаратных ресурсов интерфейса аудио кодека ========*/
  /* Функция инициализации и предустановки вводов/ выводов управления двухпроводной линией */ 
  Init_GPIO_Line_Control();
  
  
  /* Функция инициализации аппаратных ресурсов аудио кодека                   */
  /* Функции инициализация вывода сброса UDA1380  */
  UDA1380ResetInit();  
  /* Функции состояния сброса UDA1380 */
  /*  */
  UDA1380Reset( ENABLE );
  /*  */
  vTaskDelay(20);
  /*=========== Функции инициализации аппаратных ресурсов cmx интерфейса кодека==========*/   
  /* Инициализация I2S */
  CODEC_I2S_Configuration();  
  /*  */
  vTaskDelay(20);  
  /*  */
  UDA1380Reset( DISABLE ); 
  vTaskDelay(20);  
  /* Функция инициализации аудио кодека UDA1380 */
  UDA1380Init();
  /*=====================================================================================*/ 
  /* Инициализация I2C */
  CodecI2CInit(); 
  /* Открытие очереди сообщения */
  x_audio_message = xQueueCreate( 20, sizeof(mes_codec_t) );   
  /*=====================================================================================*/    
  /* Функция инициализации порта сопряжения кодека и програмного роутера */
  /*=====================================================================================*/     

  /* Функция регистрации на рассылку. */
  reg_notify_port( AUDIO_CODEC_SoftPID , handle_codec_audio_task );
  
  /*=====================================================================================*/    
  /* Предустановка режима работы */
  /*=====================================================================================*/  
  switch(GateAudioCodec.analog_codec_mode)
  {
  case MODE_ANALOG_IN:
    break;
  case MODE_ANALOG_OUT:
    break;    
  case MODE_ANALOG_AUTO:
    break;    
  }
  
  GateAudioCodec.PeriodSoftTimer = 10;
  
  /* Открытие таймера периодического уведомления задачи */
  GateAudioCodec.xSoftTimer = xTimerCreate( "TmNoCodec",             /* Текстовое имя, не используется в RTOS kernel.                    */
                                           GateAudioCodec.PeriodSoftTimer,  /* Период таймера в тиках.                                          */
                                           pdTRUE,                   /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                           NULL,                     /* В функцию параметры не передаем                                  */
                                           TimUpdateNoteAudioTask ); /* Указатель на функцию , которую вызывает таймер.                  */

  if ( GateAudioCodec.xSoftTimer != NULL ) 
  {
    /* Запуск програмных таймеров */
    xTimerStart( GateAudioCodec.xSoftTimer , 0 );
  }
  else
  {
    /* Ошибка запуска таймера */
    while(1);
  }  
  
  /* запускаем периодический запуск задачи 	                  */
  for( ;; )
  {
    /* Обнуляем сообщение */
    GateAudioCodec.NotifiedValue = 0;
    /*================================== Проверка наличия сообщений ========================================*/
    xTaskNotifyWait( 0x00000000,    /* Не очищайте биты уведомлений при входе               */
                    0xFFFFFFFF,    /* Сбросить значение уведомления до 0 при выходе        */
                    &(GateAudioCodec.NotifiedValue),/* Значение уведомленное передается в  ulNotifiedValue  */
                    portMAX_DELAY );
    
    /* Получено уведомление. Проверяем, какие биты были установлены. */
    if ( ( (GateAudioCodec.NotifiedValue) & ( PCM_BUFF_A_CMPL << ( CODEC_ANALOG_PCM_NOTE ) ) ) != 0 )
    {/* Буфер A PCM DMA отработан. */
      /* Функция функция обработки уведомления из прерывания DMA PCM */
      ProcessingNotifyDMAForAudio( &GateAudioCodec , 0);
    }
    
    if ( ( (GateAudioCodec.NotifiedValue) & ( PCM_BUFF_B_CMPL << ( CODEC_ANALOG_PCM_NOTE ) ) ) != 0 )
    {/* Буфер B PCM DMA отработан. */
      /* Функция функция обработки уведомления из прерывания DMA PCM */
      ProcessingNotifyDMAForAudio( &GateAudioCodec , 1);
    }
    
    if( ( (GateAudioCodec.NotifiedValue) & SOFT_ROUTER_NOTE ) != 0 )
    {
      /*========= Функция функция загрузки PCM пакетов из soft_router и контроля захвата потока канала =======*/
      ProcessingReadBoxForAudio( &GateAudioCodec );
    }
    else
    {
      /*========= Функция функция загрузки PCM пакетов из soft_router и контроля захвата потока канала =======*/
      ProcessingReadBoxForAudio( &GateAudioCodec ); 
    }     
    
    if ( ( (GateAudioCodec.NotifiedValue) & TIMER_NOTE ) != 0 )
    {/* Обновление задачи по таймеру. */

      /* Пока пусто */
      
    }    
  }    
}

/**
  * @brief  Функция создания задачи и организации шлюза аудио кодека
  * @param  None 
  * @retval None
  */
void Init_Gate_Audio_Codec(void)
{
  /*======================================= Переменные настройки общие ==============================================================================*/   
  GateAudioCodec.codec_ch_id = DataSto.Settings.analog_codec_chanel;                          /* Идентификатор канала захвата                        */                
  GateAudioCodec.analog_codec_mode = (analog_mode_e)(DataSto.Settings.analog_codec_mode);     /* Режим работы кодека                                 */

  /*=================================================================================================================================================*/ 
  /*======================================= Переменные настройки интерфейса pcm =====================================================================*/  

  GateAudioCodec.pcm_ch_priority       = DataSto.Settings.analog_codec_priority_discrt;       /* Приоритет канала интерфейса PCM по умолчанию   */ 
  
  GateAudioCodec.pcm_source_port       = AUDIO_CODEC_SoftPID;
  GateAudioCodec.pcm_mask_index_port   = DataSto.Settings.analog_codec_mask_source_soft_port; /* Маска доступных для приема портов              */
  GateAudioCodec.pcm_mask_chanel       = DataSto.Settings.analog_codec_mask_chanel_soft_port; /* Маска доступных каналов                        */   
  GateAudioCodec.pcm_type_box          = PCM_TYPE_BOX;                                        /* Формат данных пакета                           */ 
  /*=================================================================================================================================================*/     
  /*======================================= Переменные настройки интерфейса cmx =====================================================================*/  
  GateAudioCodec.audio_ch_priority     = DataSto.Settings.codec_a_priority_ch_cmx;   /* Приоритет канала интерфейса СМХ по умолчанию       */   
  /*=================================================================================================================================================*/  
  GateAudioCodec.audio_lock_ch_id = GateAudioCodec.codec_ch_id;                              /* Идентификатор канала                   */
  GateAudioCodec.audio_lock_dest_phy_addr = 0xFFFF;                                          /* Физический адрес получателя            */  
  GateAudioCodec.audio_lock_src_phy_addr = DataLoaderSto.Settings.phy_adr;                   /* Физический адрес источника             */  
  GateAudioCodec.codec_phy_addr = DataLoaderSto.Settings.phy_adr;                            /* Физический адрес шлюза/кодека          */     
  GateAudioCodec.audio_lock_ch_priority = DataSto.Settings.analog_codec_priority_analog;       /* Формат данных пакета | Приоритет канала */  
  /*=================================================================================================================================================*/ 
  /* Открытие задачи сопряжения аудио кодека и мультиплексора */
  xTaskCreate( codec_audio_task, "AUDIOC", configMINIMAL_STACK_SIZE*2, NULL, CNTRL_CODEC_AUDIO_TASK_PRIO, &handle_codec_audio_task );
}
/************************ (C) COPYRIGHT DEX *****END OF FILE****/
