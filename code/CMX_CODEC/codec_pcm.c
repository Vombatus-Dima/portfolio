/**
  ******************************************************************************
  * @file    codec_pcm.c
  * @author  Trembach Dmitry
  * @version V1.2.0
  * @date    17-07-2020
  * @brief   Функции контроля интерфейса PCM кодека CMX
  *
  ******************************************************************************
  * @attention
  *   Данные функции выполняют следующие действия:
  *    1.Получают пакеты несжатого голосового потока из програмного роутера 
  *       (согласно заданной маске каналов и портов) и помещает их в промежуточный   
  *       буфер. При накоплении в буфере   
  * <h2><center>&copy; COPYRIGHT 2020 DataExpress</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/  
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "hooks.h"
#include "codec_control.h"
#include "rs_frame_header.h"
#include "printf_dbg.h" 
#include "cmx_gate.h"
#include "codec_cmx.h" 
#include "hard_dma_gpio.h"
#include "board.h"


/* Структура для подготовки сообщения DMA_PCM передаваемого кодеку */
mes_codec_t pcm_dma_mes;


/* коэффициент увеличения уровня сигнала PCM с аппаратного кодека  */
#define GAIN_CODEC_PCM  (6.0)


/* обьявление указателя на группу событий xGate_PCM_Codec_Event */
EventGroupHandle_t xGate_PCM_Codec_Event;

/**
  * @brief  Инициализация интерфейса PCM голосового кодека
  * @param  pcm_cmx_t* pcm_spi - указатель на порт подключения
  * @retval None
  */
void pcm_spi_init(pcm_cmx_t* pcm_spi)
{  
  GPIO_InitTypeDef            GPIO_InitStructure;  
  SPI_InitTypeDef             SPI_InitStructure;
  DMA_InitTypeDef             DMA_InitStructure;    
  NVIC_InitTypeDef            NVIC_InitStructure;   
  
  /* Включить тактирование PCM_SPI */
  pcm_spi->RCC_SPI_ClockCmd( pcm_spi->SPI_RCC_APB_Periph , ENABLE);
  
  /* Включить тактирование GPIO PCM_SPI */
  RCC_AHB1PeriphClockCmd(  pcm_spi->MOSI_CLK_GPIO | pcm_spi->MISO_CLK_GPIO | pcm_spi->SCK_CLK_GPIO | pcm_spi->SEL_CLK_GPIO , ENABLE);
  
  /* Включение тактирования DMA */
  RCC_AHB1PeriphClockCmd( pcm_spi->RCC_AHBPeriph_DMA , ENABLE);
  
  /*============================*/  
  /* *** Настройка портов ***   */
  /*============================*/
  /* Установка Альтернативной ф-ции PCM_SPI */
  GPIO_PinAFConfig( pcm_spi->MOSI_PORT, pcm_spi->MOSI_PinSource, pcm_spi->SPI_GPIO_AF );
  GPIO_PinAFConfig( pcm_spi->MISO_PORT, pcm_spi->MISO_PinSource, pcm_spi->SPI_GPIO_AF );  
  GPIO_PinAFConfig( pcm_spi->SCK_PORT , pcm_spi->SCK_PinSource , pcm_spi->SPI_GPIO_AF );  
  GPIO_PinAFConfig( pcm_spi->SEL_PORT , pcm_spi->SEL_PinSource , pcm_spi->SPI_GPIO_AF );  
  
  /* Hастройка выводов MOSI_PIN, MISO_PIN, SCK_PIN, SEL_PIN */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;         /* Используется альтернативная функция порта    */
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    /* Скорость                                     */
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;       /* Push pool                                    */
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;      /*                                              */
  /*  MOSI */
  GPIO_InitStructure.GPIO_Pin = pcm_spi->MISO_Pin;     /* Биты сигналов                                */  
  GPIO_Init(pcm_spi->MOSI_PORT, &GPIO_InitStructure);  /* Установка настроек битов порта               */  
  /*  MISO */
  GPIO_InitStructure.GPIO_Pin = pcm_spi->MOSI_Pin;     /* Биты сигналов                                */ 
  GPIO_Init(pcm_spi->MISO_PORT, &GPIO_InitStructure);  /* Установка настроек битов порта               */ 
  /* SCK  */
  GPIO_InitStructure.GPIO_Pin = pcm_spi->SCK_Pin;      /* Биты сигналов                                */
  GPIO_Init(pcm_spi->SCK_PORT, &GPIO_InitStructure);   /* Установка настроек битов порта               */
  /* SEL */ 
  GPIO_InitStructure.GPIO_Pin = pcm_spi->SEL_Pin;      /* Биты сигналов                                */
  GPIO_Init(pcm_spi->SEL_PORT, &GPIO_InitStructure);   /* Установка настроек битов порта               */

  /* --------------------------------------------------------------------------- */
  
  /* *** Настройка SPI ***         */
  /* DISABLE PCM_SPI               */
  SPI_Cmd(pcm_spi->SPI_PORT, DISABLE); 
  /* Конфигураци SPI configuration */
  SPI_DeInit(pcm_spi->SPI_PORT);
  /* Конфигурация SPI              */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;
  
  /* * Конфигурация предделителя для SPI * */  
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16; /* The slave clock does not need to be se */
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;  
  /* Инициализация SPI */
  SPI_Init(pcm_spi->SPI_PORT, &SPI_InitStructure);
  
  /* Инициализация режима DMA для SPI */
  /* Инициализация DMA SPI TX         */ 

  /* Де-инициализация DMA (на всякий случай) */
  DMA_DeInit(pcm_spi->DMA_Stream_TX);
 
  /* Длина передаваемого блока данных. Не может превышать 65535 байт.*/ 
  DMA_InitStructure.DMA_BufferSize = SIZE_SAMPLE_BUFF * N_SAMPLE_BUFF;  
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;  
  /* Размер передаваемых данных памяти */
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;  
  /* Надо ли увеличивать адрес памяти - да */
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  
  /* Включить циклический режим            */
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  
  /* Уст. адрес периферии регистра данных SPI */
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) (&(pcm_spi->SPI_PORT->DR));
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  /* Размер передаваемых данных периферии     */
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; 
  /* Надо ли увеличивать адрес периферии - нет */
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;   
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  /* Установить канал DMA                      */
  DMA_InitStructure.DMA_Channel = pcm_spi->DMA_Channel_TX;  
  /* Направление передачи — из памяти в периферию  */
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  /* Адрес памяти */
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&(pcm_spi->data_tx_spi[0]);  
  /* Инициализация DMA SPI */
  DMA_Init(pcm_spi->DMA_Stream_TX, &DMA_InitStructure);  

  /*      * Инициализация DMA SPI RX *        */
  /* Де-инициализация DMA (на всякий случай)  */
  DMA_DeInit(pcm_spi->DMA_Stream_RX);

  /* Длина передаваемого блока данных. Не может превышать 65535 байт. */  
  DMA_InitStructure.DMA_BufferSize = SIZE_SAMPLE_BUFF * N_SAMPLE_BUFF;  
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;  
  /* Размер передаваемых данных памяти       */
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;  
  /* Надо ли увеличивать адрес памяти - да   */
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  
  /* Включить циклический режим              */
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  
  /* Уст. адрес периферии регистра данных SPI*/
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(pcm_spi->SPI_PORT->DR));
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  /* Размер передаваемых данных периферии    */
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; 
  /* Надо ли увеличивать адрес периферии - нет */
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;   
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  /* Установить канал DMA                      */ 
  DMA_InitStructure.DMA_Channel = pcm_spi->DMA_Channel_RX;  
  /* Адрес памяти                              */
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&(pcm_spi->data_rx_spi[0]);
  /* Направление передачи — из периферии в память */
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  /* Инициализация DMA SPI                        */ 
  DMA_Init(pcm_spi->DMA_Stream_RX, &DMA_InitStructure);  

  /* * Настройка прерывания DMA SPI_RX *    */
  /* канал IRQ                              */
  NVIC_InitStructure.NVIC_IRQChannel = pcm_spi->DMA_Stream_RX_IRQn;
  /* приоритет канала (0 (самый приоритетный) - 15)  */
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8; /* при Priority <= 4 не работает RTOS */
  /* приоритет подгруппы           */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
  /* включить прерывание           */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  /* Инициализация NVIC в соответствии с заданными параметрами в NVIC_InitStruct. */  
  NVIC_Init(&NVIC_InitStructure);
  /* Включение прерывания по приёму DMA - принят весь буфер                       */   
  DMA_ITConfig(pcm_spi->DMA_Stream_RX, DMA_IT_TC, ENABLE);
  /* Включение прерывания по приёму DMA - принято половина буфера                 */
  DMA_ITConfig(pcm_spi->DMA_Stream_RX, DMA_IT_HT, ENABLE);  

   /* Enable DMA SPI TX Stream */
  DMA_Cmd(pcm_spi->DMA_Stream_TX,ENABLE);

  /* Enable DMA SPI RX Stream */
  DMA_Cmd(pcm_spi->DMA_Stream_RX,ENABLE);  
   
  /* Подключить DMA к SPI TX */
  SPI_I2S_DMACmd(pcm_spi->SPI_PORT, SPI_I2S_DMAReq_Tx, ENABLE); 
  /* Подключить DMA к SPI RX */
  SPI_I2S_DMACmd(pcm_spi->SPI_PORT, SPI_I2S_DMAReq_Rx, ENABLE);  
  
  /* Включить PCM_SPI */
  SPI_Cmd(pcm_spi->SPI_PORT, ENABLE);

  /* Инициализация завершена - включаем режим ожидания */
  pcm_spi->codec_status_speaker = SLEEP_STREAM;
  
  /* Инициализация завершена - включаем режим ожидания */  
  pcm_spi->pcm_status_wr = SLEEP_STREAM;
}

/**
  * @brief  Функция функция загрузки пакетов из soft_router и контроля захвата потока канала
  * @param  pcm_cmx_t* pcm_cmx - указатель на структуру контроля кодека
  * @retval None
  */
void ProcessingReadBox( pcm_cmx_t* pcm_cmx )
{
  uint16_t cnt_index_data;
  uint8_t temp_priority;  
  /* Проверка наличия данных в   */    
  /* Функция получения указателя на текущий буффер для чтения */
  pcm_cmx->pcm_pfifo_box = GetRDPointFIFO( &(pcm_cmx->pcm_index_rd_box) , pcm_cmx->pcm_mask_chanel , pcm_cmx->pcm_mask_index_port );
  /* Если указатель корректный */      
  if ( (pcm_cmx->pcm_pfifo_box) != NULL )
  { 
    /* подсчет полученных пакетов */
    (pcm_cmx->pcm_cnt_rx_box)++;
    
    /* Нормируем приоритет канала полученного пакета */
    temp_priority = pcm_cmx->pcm_pfifo_box->data_box.priority_box;  
    /* Если приоритет выходит за диапазон - устанавливаем приоритет по умолчанию */
    if ( ( temp_priority < VOICE_PRIOR_HIGH ) || ( temp_priority > VOICE_PRIOR_LOW ) )
    {
      /* Назначение приоритета по умолчанию */
      temp_priority = pcm_cmx->pcm_ch_priority;
    }
    
    /* Анализ счетчика пустых интервал в захваченом потоке канала и появления канала с более высоким приоритетом */ 
    /* Если число пропущенных пакетов больше MAX_EMPTY_INTERVAL или появились в канале пакеты с более высоким    */
    /* приоритетом - сбрасываем захват канала и запускаем алгоритм захвата канала                                */
    if ((pcm_cmx->pcm_empty_interval_counter >= MAX_EMPTY_INTERVAL) || (temp_priority < pcm_cmx->pcm_lock_ch_priority))
    {
      /* Обнуление индексов буфера */
      pcm_cmx->pcm_wr_buf_index = 0;
      pcm_cmx->pcm_rd_buf_index = 0; 
      /* Обнуление флагов буфера */
      for(cnt_index_data = 0;  cnt_index_data  < MAX_BUF_RX_PCM_BOX ; cnt_index_data++)
      {
        /* Маркируем пакет как пустой */
        pcm_cmx->pcm_buf_box[cnt_index_data].flag_box = BOX_EMPTY;             
      }      
      /* Поток канала потерян - канал захвачен*/
      pcm_cmx->pcm_status_wr = SLEEP_STREAM;
      pcm_cmx->pcm_empty_interval_counter = 0;
      
      /* Предварительные захват канала */
      pcm_cmx->pcm_lock_ch_id            = pcm_cmx->pcm_pfifo_box->data_box.ch_id;          /* Идентификатор канала                    */
      pcm_cmx->pcm_lock_dest_phy_addr    = pcm_cmx->pcm_pfifo_box->data_box.dest_phy_addr;  /* Физический адрес получателя             */
      pcm_cmx->pcm_lock_src_phy_addr     = pcm_cmx->pcm_pfifo_box->data_box.src_phy_addr;   /* Физический адрес источника              */
      pcm_cmx->pcm_lock_codec_phy_addr   = pcm_cmx->codec_phy_addr;                         /* Физический адрес шлюза/кодека           */   
    } 

    pcm_cmx->pcm_lock_ch_priority      = temp_priority;                                     /* Обновить приоритет канала               */    
    
    /* Если пакет данных не соответствует захваченному каналу - выход */ 
    if ( pcm_cmx->pcm_lock_src_phy_addr  != pcm_cmx->pcm_pfifo_box->data_box.src_phy_addr  ) return;
    if ( pcm_cmx->pcm_lock_dest_phy_addr != pcm_cmx->pcm_pfifo_box->data_box.dest_phy_addr ) return;
    
    /* Копирование данных в промежуточный буфер */
    for(cnt_index_data = 0;  cnt_index_data < (sizeof(soft_router_box_t))/4 ; cnt_index_data++)
    {
      ((uint32_t*)(&(pcm_cmx->pcm_buf_box[pcm_cmx->pcm_wr_buf_index].router_box)))[cnt_index_data] = ((uint32_t*)(pcm_cmx->pcm_pfifo_box))[cnt_index_data];               
    }
    /* Маркируем пакет в буфере как полный */
    pcm_cmx->pcm_buf_box[pcm_cmx->pcm_wr_buf_index].flag_box = BOX_FULL;
    
    /* Инкремент индекса записи пакета */
    (pcm_cmx->pcm_wr_buf_index)++;   
    /* Проверка на выход за границу чтения */
    if ( pcm_cmx->pcm_wr_buf_index >= MAX_BUF_RX_PCM_BOX )   
    {
      /* Переход на нулевой индекс */
      pcm_cmx->pcm_wr_buf_index = 0;
    }    
    
    /* Если канал не захвачен */          
    if ( pcm_cmx->pcm_status_wr == SLEEP_STREAM )
    {
      /* Если в буфере накоплено MAX_START_BUFFER_STREAM пакетов  */
      if (pcm_cmx->pcm_wr_buf_index >= ( MAX_START_BUFFER_STREAM ))
      {
        /* Меняем статут - поток захвачен */
        pcm_cmx->pcm_status_wr = ACTIVE_STREAM;
      }
    }
    
    /* Если канал захвачен отправляем сообщение кодеку */
    if ( pcm_cmx->pcm_status_wr == ACTIVE_STREAM )
    {
      /* Отправляем сообщение кодеку на переключение в режим микрофона */
      pcm_dma_mes.data_event = MIC_EV;
      pcm_dma_mes.index_codec = pcm_cmx->index_codec; 
      pcm_dma_mes.data_codec = pcm_cmx->pcm_lock_ch_priority; /* Установить уровень запроса */
      xQueueSend( x_codec_message , (void*)&(pcm_dma_mes), ( TickType_t ) 0 );
    }
  }  
}

/**
  * @brief  Функция функция обработки уведомления из прерывания DMA PCM
  * @param  pcm_cmx_t* cmx_pcm - указатель на структуру контроля кодека
  * @param  uint8_t n_buffer - номер обрабатываемого буфера
  * @retval None
  */
void ProcessingNotifyDMA( pcm_cmx_t* pcm_cmx , uint8_t n_buffer )
{ 
  uint16_t cnt_index_data;
  /*== Данные несжатого голосового потока из области памяти загруженой DMA ==*/
  /*== дополняется шапкой и переносятся в программый роутер потоков.       ==*/
  /*======================= Отработка записи в DMA ==========================*/
  if ( pcm_cmx->pcm_buf_box[pcm_cmx->pcm_rd_buf_index].flag_box == BOX_FULL )
  {
    if ( pcm_cmx->pcm_status_wr == ACTIVE_STREAM )
    {
      /* Копирование данных */
      for( cnt_index_data = 0 ; cnt_index_data < MAX_SIZE_BOX_SAMPLE ; cnt_index_data++ )
      {
        pcm_cmx->data_tx_spi[n_buffer][cnt_index_data] = (pcm_cmx->pcm_buf_box[pcm_cmx->pcm_rd_buf_index].router_box.data_box.data[cnt_index_data]);                
      }
      //    for( cnt_index_data = 0 ; cnt_index_data < MAX_SIZE_BOX_SAMPLE/2 ; cnt_index_data++ )
      //    {
      //      ((uint32_t*)(&(pcm_cmx->data_tx_spi[n_buffer][0])))[cnt_index_data] = ((uint32_t*)(&(pcm_cmx->pcm_buf_box[pcm_cmx->pcm_rd_buf_index].router_box.data_box.data[0])))[cnt_index_data]; 
      //    }    
      /* Сброс флага пакета */
      pcm_cmx->pcm_buf_box[pcm_cmx->pcm_rd_buf_index].flag_box = BOX_EMPTY;
      /* Инкремент индекса чтения пакета */
      (pcm_cmx->pcm_rd_buf_index)++;
      /* Проверка на выход за границу чтения */
      if ( pcm_cmx->pcm_rd_buf_index >= MAX_BUF_RX_PCM_BOX )   
      {
        /* Переход на нулевой индекс */
        pcm_cmx->pcm_rd_buf_index = 0;
      }
      /* Сброс счетчика пустых интервалов */
      pcm_cmx->pcm_empty_interval_counter = 0;
    }
    else
    {
      /* Вычитываем из буфера пакет */
      /* Сброс флага пакета */
      pcm_cmx->pcm_buf_box[pcm_cmx->pcm_rd_buf_index].flag_box = BOX_EMPTY;
      /* Инкремент индекса чтения пакета */
      (pcm_cmx->pcm_rd_buf_index)++;
      /* Проверка на выход за границу чтения */
      if ( pcm_cmx->pcm_rd_buf_index >= MAX_BUF_RX_PCM_BOX )   
      {
        /* Переход на нулевой индекс */
        pcm_cmx->pcm_rd_buf_index = 0;
      }      

      /* Загружаем в DMA пустой пакет */  
      /* Копирование данных */
      //    for( cnt_index_data = 0 ; cnt_index_data < MAX_SIZE_BOX_SAMPLE ; cnt_index_data++ )
      //    {
      //      pcm_cmx->data_tx_spi[n_buffer][cnt_index_data] = PCM_NOISE_16_CONST;  
      //    }
      for( cnt_index_data = 0 ; cnt_index_data < MAX_SIZE_BOX_SAMPLE/2 ; cnt_index_data++ )
      {
        ((uint32_t*)(&(pcm_cmx->data_tx_spi[n_buffer][0])))[cnt_index_data] = PCM_NOISE_32_CONST;  
      }    
      
      /* Подсчет пропущеных интервалов счет ограничиваем 10 */
      if ( pcm_cmx->pcm_empty_interval_counter <= 10 ) (pcm_cmx->pcm_empty_interval_counter)++;
    }
  }
  else
  {
    /* Загружаем в DMA пустой пакет */  
    /* Копирование данных */
    //    for( cnt_index_data = 0 ; cnt_index_data < MAX_SIZE_BOX_SAMPLE ; cnt_index_data++ )
    //    {
    //      pcm_cmx->data_tx_spi[n_buffer][cnt_index_data] = PCM_NOISE_16_CONST;  
    //    }
    for( cnt_index_data = 0 ; cnt_index_data < MAX_SIZE_BOX_SAMPLE/2 ; cnt_index_data++ )
    {
      ((uint32_t*)(&(pcm_cmx->data_tx_spi[n_buffer][0])))[cnt_index_data] = PCM_NOISE_32_CONST;  
    }    
    
    /* Подсчет пропущеных интервалов счет ограничиваем 10 */
    if ( pcm_cmx->pcm_empty_interval_counter <= 10 ) (pcm_cmx->pcm_empty_interval_counter)++;
  }
  /*=========================================================================*/
  
  /*== Данные несжатого голосового потока из области памяти загруженой DMA ==*/
  /*== дополняется шапкой и переносятся в программый роутер потоков.       ==*/
  if(pcm_cmx->codec_status_speaker == ACTIVE_STREAM)
  {
    /* Запрос указателя на пакет */
    pcm_cmx->pcm_pfifo_box = GetWRPointFIFO();
    /* Если указатель корректный */
    if ( pcm_cmx->pcm_pfifo_box != NULL)
    {
      /* Копирование данных */
      for(cnt_index_data = 0 ;  cnt_index_data < MAX_SIZE_BOX_SAMPLE ; cnt_index_data++)
      {
        pcm_cmx->pcm_pfifo_box->data_box.data[cnt_index_data] = (uint16_t)(((int16_t)(pcm_cmx->data_rx_spi[n_buffer][cnt_index_data])) * GAIN_CODEC_PCM); 
      }
      //      /* Копирование данных */
      //      for(cnt_index_data = 0 ;  cnt_index_data < MAX_SIZE_BOX_SAMPLE/2 ; cnt_index_data++)
      //      {
      //        ((uint32_t*)(&(pcm_cmx->pcm_pfifo_box->data_box.data[0])))[cnt_index_data] = ((uint32_t*)(&(pcm_cmx->data_rx_spi[n_buffer][0])))[cnt_index_data];  
      //      }  
      /* Заполнение шапки пакета */
      pcm_cmx->pcm_pfifo_box->source_port             = pcm_cmx->pcm_source_port;                            /* Порт источник                          */   
#if TEST_CODEC_BOARD == 1     
      /* Режим теста кодеков */
      if (pcm_cmx->cmx_lock_ch_id == 3)  pcm_cmx->pcm_pfifo_box->data_box.ch_id = 1;
      else if (pcm_cmx->cmx_lock_ch_id == 2)  pcm_cmx->pcm_pfifo_box->data_box.ch_id = 3;
      else if (pcm_cmx->cmx_lock_ch_id == 1)  pcm_cmx->pcm_pfifo_box->data_box.ch_id = 2;
#else
      pcm_cmx->pcm_pfifo_box->data_box.ch_id          = pcm_cmx->cmx_lock_ch_id;                            /* Идентификатор канала                   */
#endif     
      pcm_cmx->pcm_pfifo_box->data_box.dest_phy_addr  = pcm_cmx->cmx_lock_dest_phy_addr;                    /* Физический адрес получателя            */
      pcm_cmx->pcm_pfifo_box->data_box.src_phy_addr   = pcm_cmx->cmx_lock_src_phy_addr;                     /* Физический адрес источника             */
      pcm_cmx->pcm_pfifo_box->data_box.codec_phy_addr = pcm_cmx->codec_phy_addr;                            /* Физический адрес шлюза/кодека          */   
      pcm_cmx->pcm_pfifo_box->data_box.priority_box   = pcm_cmx->cmx_lock_ch_priority;                      /* Приоритет данных пакета                */
      pcm_cmx->pcm_pfifo_box->data_box.cnt            = pcm_cmx->pcm_cnt++;                                 /* Cчетчик неприрывности пакетов 0..255   */
      pcm_cmx->pcm_pfifo_box->data_box.time_id        = ulGetRuntimeCounterValue();                         /* Временной идентификатор пакета         */
      
      /* Разрешение пакета для чтения */
      pcm_cmx->pcm_pfifo_box->enable_read = 1;
      
      /* Рассылка оповешения зарегистрировавшихся на рассылку.*/
      notify_soft_port(pcm_cmx->pcm_source_port);
      
      /* Подсчет переданных пакетов */
      pcm_cmx->pcm_cnt_tx_box++;
    }
  } 
//  else
//  {
//    /* Запрос указателя на пакет */
//    pcm_cmx->pcm_pfifo_box = GetWRPointFIFO();
//    /* Если указатель корректный */
//    if ( pcm_cmx->pcm_pfifo_box != NULL)
//    {
//      /* Копирование данных */
//      for(cnt_index_data = 0 ;  cnt_index_data < MAX_SIZE_BOX_SAMPLE ; cnt_index_data++)
//      {
//        pcm_cmx->pcm_pfifo_box->data_box.data[cnt_index_data] = 0x0000;
//      }
//      //      /* Копирование данных */
//      //      for(cnt_index_data = 0 ;  cnt_index_data < MAX_SIZE_BOX_SAMPLE/2 ; cnt_index_data++)
//      //      {
//      //        ((uint32_t*)(&(pcm_cmx->pcm_pfifo_box->data_box.data[0])))[cnt_index_data] = ((uint32_t*)(&(pcm_cmx->data_rx_spi[n_buffer][0])))[cnt_index_data];  
//      //      }  
//      /* Заполнение шапки пакета */
//      pcm_cmx->pcm_pfifo_box->source_port             = pcm_cmx->pcm_source_port;                            /* Порт источник                          */   
//      pcm_cmx->pcm_pfifo_box->data_box.ch_id          = pcm_cmx->cmx_lock_ch_id;                            /* Идентификатор канала                   */
//
//      pcm_cmx->pcm_pfifo_box->data_box.dest_phy_addr  = 0x0000;                                             /* Физический адрес получателя            */
//      pcm_cmx->pcm_pfifo_box->data_box.src_phy_addr   = 0x0000;                                             /* Физический адрес источника             */
//      pcm_cmx->pcm_pfifo_box->data_box.codec_phy_addr = pcm_cmx->codec_phy_addr;                            /* Физический адрес шлюза/кодека          */   
//      pcm_cmx->pcm_pfifo_box->data_box.priority_box   = 0;                                                  /* Приоритет данных пакета                */
//      pcm_cmx->pcm_pfifo_box->data_box.cnt            = pcm_cmx->pcm_cnt++;                                 /* Cчетчик неприрывности пакетов 0..255   */
//      pcm_cmx->pcm_pfifo_box->data_box.time_id        = ulGetRuntimeCounterValue();                         /* Временной идентификатор пакета         */
//      
//      /* Разрешение пакета для чтения */
//      pcm_cmx->pcm_pfifo_box->enable_read = 1;
//      
//      /* Рассылка оповешения зарегистрировавшихся на рассылку.*/
//      notify_soft_port(pcm_cmx->pcm_source_port);
//      
//      /* Подсчет переданных пакетов */
//      pcm_cmx->pcm_cnt_tx_box++;
//    }
//  }  
  /*==========================================================================*/
}
/**
  * @brief  Функция контроля интерфейсов PCM кодека
  * @param  None
  * @retval None
  */
void ProcessingPCM( void )
{
  /* Проверка всех кодеков */  
  for(uint8_t index_codec = 0 ; index_codec < MAX_CODEC; index_codec++ )
  {
    /*======= ==== Функция функция загрузки PCM пакетов из soft_router и контроля захвата потока канала ====*/
    ProcessingReadBox( &(cmx_pcm[index_codec]) );
  }
  /*============================================================================================================*/   
}

/******************* (C) COPYRIGHT 2020 DataExpress *****END OF FILE****/
