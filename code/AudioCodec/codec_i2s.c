/**
  ******************************************************************************
  * @file    codec_i2s.c
  * @author  Trembach Dmitry
  * @date    20-07-2020
  * @brief   Функции инициализации интерфейса i2s управления кодеком UDA1380
  *
  ******************************************************************************
  * @attention
  * 
  *
  * <h2><center>&copy; COPYRIGHT 2020 DataExpress</center></h2>
  ******************************************************************************
  */

#include "codec_i2s.h"
#include "UDA1380.h"
#include "stm32f4xx_rcc.h"
#include "audio_gate.h"    


/* Codec audio Standards */
#ifdef I2S_STANDARD_PHILLIPS
 #define  CODEC_STANDARD                0x04
 #define I2S_STANDARD                   I2S_Standard_Phillips
#elif defined(I2S_STANDARD_MSB)
 #define  CODEC_STANDARD                0x00
 #define I2S_STANDARD                   I2S_Standard_MSB
#elif defined(I2S_STANDARD_LSB)
 #define  CODEC_STANDARD                0x08
 #define I2S_STANDARD                   I2S_Standard_LSB
#else
 #error "Error: No audio communication standard selected !"
#endif

void SET_I2S_PLL(uint32_t AudioFreq)
{
  RCC_PLLI2SCmd(DISABLE);
  
  switch(AudioFreq)
  {
  case I2S_AudioFreq_8k:  {RCC_PLLI2SConfig  (256,2,5);break;}
  case I2S_AudioFreq_16k: {RCC_PLLI2SConfig  (213,2,2);break;}
  case I2S_AudioFreq_11k: {RCC_PLLI2SConfig  (213,2,2);break;}
  case I2S_AudioFreq_32k: {RCC_PLLI2SConfig  (213,2,2);break;}
  case I2S_AudioFreq_48k: {RCC_PLLI2SConfig  (258,2,3);break;}
  case I2S_AudioFreq_96k: {RCC_PLLI2SConfig  (344,2,2);break;}
  case I2S_AudioFreq_22k: {RCC_PLLI2SConfig  (429,2,4);break;}
  case I2S_AudioFreq_44k: {RCC_PLLI2SConfig  (271,2,2);break;}
  }
  
  RCC_PLLI2SCmd(ENABLE);
}

/**
  * @brief  Инициализация интерфейса PCM аудио кодека
  * @param  None
  * @retval None
  */
void CODEC_I2S_Configuration(void)
{
  I2S_InitTypeDef   I2S_InitStructure;
  GPIO_InitTypeDef  GPIO_InitStructure;
  NVIC_InitTypeDef  NVIC_InitStructure;
  DMA_InitTypeDef   DMA_InitStructure;
  
  /*======================================================================================*/
  /*======================================================================================*/    
  
  /* Включить тактирование пинов CODEC_I2S */
  RCC_AHB1PeriphClockCmd( CODEC_I2S_WS_CLOCK | CODEC_I2S_SCK_CLOCK | CODEC_I2S_SD_CLOCK | CODEC_I2S_EXT_CLOCK , ENABLE);
  
  /* Конфигурация WS, SCK, SD, EXT_SD */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  
  /* WS     */
  GPIO_InitStructure.GPIO_Pin = CODEC_I2S_WS_PIN;
  GPIO_Init(CODEC_I2S_WS_GPIO, &GPIO_InitStructure);
  GPIO_PinAFConfig(CODEC_I2S_WS_GPIO, CODEC_I2S_WS_PINSRC, CODEC_I2S_WS_AF);	
  /* SCK    */
  GPIO_InitStructure.GPIO_Pin = CODEC_I2S_SCK_PIN;
  GPIO_Init(CODEC_I2S_SCK_GPIO, &GPIO_InitStructure);
  GPIO_PinAFConfig(CODEC_I2S_SCK_GPIO, CODEC_I2S_SCK_PINSRC, CODEC_I2S_SCK_AF);	
  /* SD     */
  GPIO_InitStructure.GPIO_Pin = CODEC_I2S_SD_PIN;
  GPIO_Init(CODEC_I2S_SD_GPIO, &GPIO_InitStructure);
  GPIO_PinAFConfig(CODEC_I2S_SD_GPIO, CODEC_I2S_SD_PINSRC, CODEC_I2S_SD_AF);	
  /* EXT_SD */
  GPIO_InitStructure.GPIO_Pin =  CODEC_I2S_EXT_SD_PIN;
  GPIO_Init(CODEC_I2S_EXT_SD_GPIO_PORT, &GPIO_InitStructure);
  GPIO_PinAFConfig(CODEC_I2S_EXT_SD_GPIO_PORT, CODEC_I2S_EXT_SD_SOURCE, CODEC_I2S_EXT_AF);	
  /* Включение тактировани CODEC_I2S (I2S) */
  CODEC_I2S_CLK_CMD(CODEC_I2S_CLK, ENABLE);
  
  /*======================================================================================*/
  /*======================================================================================*/        
  /* Деинициализация I2S */
  SPI_I2S_DeInit(CODEC_I2S);
  /* Частота - 8кГц */
  I2S_InitStructure.I2S_AudioFreq = I2S_AudioFreq_8k;
  /* Стантард I2S - см. определение дефайнов            */
  I2S_InitStructure.I2S_Standard = I2S_STANDARD;
  /* формат данных 16-бит                               */
  I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
  /* Уровень I2S clock в idle - низкий                  */
  I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
  /* Режим - Масетер TX                                 */
  I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;
  /* Вывод тактирование MCLK - выкл.                    */
  I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;
  /* Инициализация тактирования                         */
  SET_I2S_PLL(I2S_InitStructure.I2S_AudioFreq);
  /* Записать параметры настроек структуры              */
  I2S_Init(CODEC_I2S, &I2S_InitStructure);
  /* Отключение прерываний I2S2 TXE (!!! включить если не используется DMA) */
  SPI_I2S_ITConfig(CODEC_I2S, SPI_I2S_IT_TXE, DISABLE);
  /* Настройка I2Sx_ext (2-ой интерфейс) в режиме Slave приемника           */
  I2S_FullDuplexConfig(CODEC_I2S_EXT, &I2S_InitStructure);
  /*======================================================================================*/
  /*======================================================================================*/            
  /* Вкл. тактирование DMA                                                  */
  AUDIO_DMA_CLOCK_CMD( AUDIO_DMA_CLOCK, ENABLE );
  /* Настройка DMA потока по передаче TX                                    */
  /* Отключение ДМА */
  DMA_Cmd( AUDIO_TX_DMA_STREAM, DISABLE );
  /* Деинициализация ресурсов ДМА */
  DMA_DeInit( AUDIO_TX_DMA_STREAM );
  /*============================ Заполнение структуры ДМА TX ==============================*/     
  /* Установить канал DMA                      */
  DMA_InitStructure.DMA_Channel = AUDIO_TX_DMA_CHANNEL; 
  /* Уст. адрес периферии регистра данных SPI */
  DMA_InitStructure.DMA_PeripheralBaseAddr = CODEC_I2S_ADDRESS; 
  /* Адрес памяти */
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&(GateAudioCodec.data_tx_i2s[0][0]);
  /* Направление передачи — из памяти в периферию  */
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;  
  
  DMA_InitStructure.DMA_BufferSize = SIZE_SAMPLE_AUDIO_BUFF * N_SAMPLE_AUDIO_BUFF; 
  /* Надо ли увеличивать адрес периферии - нет */
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;    
  /* Надо ли увеличивать адрес памяти - да */
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  /* Размер передаваемых данных периферии     */
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; 
  /* Размер передаваемых данных памяти */
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  /* Включить циклический режим            */
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;     
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;   
  /* Инициализация DMA SPI */
  DMA_Init( AUDIO_TX_DMA_STREAM, &DMA_InitStructure );    
   /* Включение прерывания по приёму DMA - принято половина буфера                 */
  DMA_ITConfig( AUDIO_RX_DMA_STREAM, DMA_IT_TC, ENABLE );     
  /* Подключить DMA к SPI TX */
  SPI_I2S_DMACmd( CODEC_I2S , SPI_I2S_DMAReq_Tx, ENABLE);
  /*======================================================================================*/
  /*======================================================================================*/         
  /* Настройка DMA потока по приему RX                                    */        
  /* Отключение ДМА */
  DMA_Cmd( AUDIO_RX_DMA_STREAM , DISABLE );
  /* Деинициализация ресурсов ДМА */
  DMA_DeInit( AUDIO_RX_DMA_STREAM );
  /*============================ Заполнение структуры ДМА RX ==============================*/    
  /* Установить канал DMA                      */ 
  DMA_InitStructure.DMA_Channel = AUDIO_RX_DMA_CHANNEL; 
  /* Уст. адрес периферии регистра данных SPI*/
  DMA_InitStructure.DMA_PeripheralBaseAddr = CODEC_I2S_EXT_ADDRESS;
  /* Адрес памяти                              */
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&(GateAudioCodec.data_rx_i2s[0][0]);
  /* Направление передачи — из периферии в память */
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = SIZE_SAMPLE_AUDIO_BUFF * N_SAMPLE_AUDIO_BUFF; 
  /* Надо ли увеличивать адрес периферии - нет */
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  
  /* Надо ли увеличивать адрес памяти - да   */
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  /* Размер передаваемых данных периферии    */
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; 
  /* Размер передаваемых данных памяти       */
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  /* Включить циклический режим              */
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; 
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable; 
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;         
  /* Инициализация DMA SPI                        */ 
  DMA_Init( AUDIO_RX_DMA_STREAM , &DMA_InitStructure );  
  /*======================================================================================*/
  /* Подключить DMA к SPI RX */
  SPI_I2S_DMACmd( CODEC_I2S_EXT , SPI_I2S_DMAReq_Rx, ENABLE);    
  /* Enable DMA SPI TX Stream */
  DMA_Cmd( AUDIO_TX_DMA_STREAM,ENABLE );  
  /* Включить I2S */
  I2S_Cmd(CODEC_I2S, ENABLE);
  /* Enable DMA SPI RX Stream */
  DMA_Cmd( AUDIO_RX_DMA_STREAM,ENABLE );
  /* Включить I2S_EXT */        
  I2S_Cmd(CODEC_I2S_EXT, ENABLE);    
  /*======================================================================================*/        
  /* * Настройка прерывания DMA SPI_RX *    */
  /* канал IRQ                              */
  NVIC_InitStructure.NVIC_IRQChannel = AUDIO_RX_DMA_IRQ;
  /* приоритет канала (0 (самый приоритетный) - 15)  */
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8; /* при Priority <= 4 не работает RTOS */
  /* приоритет подгруппы           */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
  /* включить прерывание           */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  /* Инициализация NVIC в соответствии с заданными параметрами в NVIC_InitStruct. */  
  NVIC_Init(&NVIC_InitStructure);
  /* Включение прерывания по приёму DMA - принят весь буфер                       */   
  DMA_ITConfig( AUDIO_RX_DMA_STREAM, DMA_IT_TC, ENABLE );
  /* Включение прерывания по приёму DMA - принято половина буфера                 */
  DMA_ITConfig( AUDIO_RX_DMA_STREAM, DMA_IT_HT, ENABLE );  
  /*======================================================================================*/
  /*======================================================================================*/   
}


/* Прерывание I2S RX DMA  */
void Audio_RX_IRQHandler()
{
  /* Проверка, на завершение прерывание потока DMA приёма(DMA Stream Transfer ) */
  if (DMA_GetITStatus(AUDIO_RX_DMA_STREAM, AUDIO_RX_DMA_IT_FLAG_TCIF)) 
  {
    /* Очистка бита прерывания потока DMA */
    DMA_ClearITPendingBit(AUDIO_RX_DMA_STREAM, AUDIO_RX_DMA_IT_FLAG_TCIF );
    if ( handle_codec_audio_task != NULL )
    {
      xTaskNotifyFromISR( handle_codec_audio_task, /* Указатель на уведомлюемую задачу                         */
                         PCM_BUFF_B_CMPL<<(CODEC_ANALOG_PCM_NOTE),      /* Значения уведомления                                     */
                         eSetBits,             /* Текущее уведомление добавляются к уже прописанному       */
                         NULL );               /* Внеочередного переключения планировщика не запрашиваем   */
    }     
  }
  
  /* Проверка, на завершение прерывание потока DMA приёма(DMA Stream Transfer ) */
  if (DMA_GetITStatus(AUDIO_RX_DMA_STREAM, AUDIO_RX_DMA_IT_FLAG_HTIF)) 
  {
    /* Очистка бита прерывания потока DMA */
    DMA_ClearITPendingBit(AUDIO_RX_DMA_STREAM,  AUDIO_RX_DMA_IT_FLAG_HTIF );
    if ( handle_codec_audio_task != NULL )
    {
      xTaskNotifyFromISR( handle_codec_audio_task, /* Указатель на уведомлюемую задачу                         */
                         PCM_BUFF_A_CMPL<<(CODEC_ANALOG_PCM_NOTE),      /* Значения уведомления                                     */
                         eSetBits,             /* Текущее уведомление добавляются к уже прописанному       */
                         NULL );               /* Внеочередного переключения планировщика не запрашиваем   */
    }      
  }
}

