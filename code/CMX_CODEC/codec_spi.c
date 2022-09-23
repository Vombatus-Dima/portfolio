/**
  ******************************************************************************
  * @file    codec_spi.c
  * @author  Trembach Dmitry
  * @version V1.0.0
  * @date    12-04-2020
  * @brief   Инициализация интерфейса SPI кодека
  *
  ******************************************************************************
  * @attention
  * 
  * 
  *
  * <h2><center>&copy; COPYRIGHT 2020 DataExpress</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "spi_base.h"
#include "codec_spi.h"    
#include "task.h" 
#include "cmx618_reg.h"
#include "codec_control.h"
#include "board.h"

/*xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx*/
QueueHandle_t xCNTRL_CDC_Ev_Queue;  /* Указатель для группы событий контроля кодеков */

TaskHandle_t xHandlingTask = NULL;

/* Фиктивное значение, для записи в SPDR, чтобы получить данные по SPI */
#define SPI_DUMMY_VALUE_8         (0x00)
/* Фиктивное значение, для записи в SPDR, чтобы получить данные по SPI */
#define SPI_DUMMY_VALUE_16        (0x00)
/* Переменная dummy для приёма/передачи                                */
static uint8_t    cmx618_spi_dummy = SPI_DUMMY_VALUE_8;
/* Переменная для пустого чтения из SPI                                */
static uint16_t    data_spi_rd;


/**
 * @brief  Основная задача контроля трансивера. 
 * @param  uint16_t time_val
 * @retval None
  ******************************************************************************
  * @attention
  * результаты измерения
  *  time_val=3     - 0.29 мкс 
  *  time_val=100   - 10.1 мкс
  *     ошибка ~ 1%
  ******************************************************************************
 */
#pragma optimize=none 
void ns100_delay(uint16_t time_val)
{
  for(uint16_t cnt_time = 1;cnt_time < time_val;cnt_time++)
  {
    asm("nop");  asm("nop");   asm("nop");  asm("nop");       
    asm("nop");  asm("nop");   asm("nop");  asm("nop");    
  }
}

/**
  * @brief  Инициализация интерфейса spi. 
  * @param  uint32_t spi_bitrate  желаемая (макс.) скорость SPI
  * @retval uint32_t - установленная скорость
  * 
  */
uint32_t spi_interface_init(uint32_t spi_bitrate)
{
/* Переменная для значения установленного битрейта */
uint32_t spi_bitrate_real;  

  GPIO_InitTypeDef              GPIO_InitStructure;  
  NVIC_InitTypeDef              NVIC_InitStructure;  
  SPI_InitTypeDef               SPI_InitStructure;
  TIM_TimeBaseInitTypeDef       TIM_TimeBaseStructure;
  TIM_OCInitTypeDef             TIM_OCInitStructure;
  
  /* Настройка портов                                         */
  /* Включить тактирование GPIO ножек CODEC_SPI, IRQ, BUSY   */
  RCC_AHB1PeriphClockCmd(CODEC_PORTs_RCC, ENABLE);
  
  /* Установка Альтернативной ф-ции CODEC_SPI     */
  GPIO_PinAFConfig( CODEC_MOSI_GPIO_PORT , CODEC_MOSI_SOURCE , CODEC_SPI_GPIO_AF );
  GPIO_PinAFConfig( CODEC_MISO_GPIO_PORT , CODEC_MISO_SOURCE , CODEC_SPI_GPIO_AF );  
  GPIO_PinAFConfig( CODEC_CLK_GPIO_PORT  , CODEC_CLK_SOURCE  , CODEC_SPI_GPIO_AF ); 	
  
  /* настройка выводов MOSI_PIN, MISO_PIN, CLK_PIN */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;          /* Используется альтернативная функция порта  */
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     /* Скорость                                   */
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;        /* подтяжка резисторами                       */
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;	/* pull_down                                  */
  /* MOSI */
  GPIO_InitStructure.GPIO_Pin = CODEC_MOSI_PIN;         /* Биты сигналов                              */
  GPIO_Init(CODEC_MOSI_GPIO_PORT, &GPIO_InitStructure); /* Установка настроек битов порта             */
  /* MISO */
  GPIO_InitStructure.GPIO_Pin = CODEC_MISO_PIN;         /* Биты сигналов                              */
  GPIO_Init(CODEC_MISO_GPIO_PORT, &GPIO_InitStructure); /* Установка настроек битов порта             */
  /* CLK  */
  GPIO_InitStructure.GPIO_Pin = CODEC_CLK_PIN;          /* Биты сигналов                              */
  GPIO_Init(CODEC_CLK_GPIO_PORT, &GPIO_InitStructure);  /* Установка настроек битов порта             */
  
  /* Настройка SPI */
  
  /* Включить тактирование CODEC_SPI */
  CODEC_RCC_APBPeriphClockCmd(CODEC_SPI_RCC, ENABLE);
  
  /* Конфигураци SPI configuration    */
  SPI_DeInit(CODEC_SPI);
  /* DISABLE CODEC_SPI               */
  SPI_Cmd(CODEC_SPI, DISABLE); 
  /* Конфигурация SPI                */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  
  /* Конфигурация предделителя для SPI */    
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_GetPrescalerMin(CODEC_SPI, spi_bitrate, &spi_bitrate_real);
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;  
  /* Инициализация SPI                 */
  SPI_Init(CODEC_SPI, &SPI_InitStructure);
  
  /* Очистка флага готовности к приёму новых данных                         */
  SPI_I2S_ClearFlag(CODEC_SPI, SPI_I2S_FLAG_TXE);  
  
  /* Включить CODEC_SPI                                                     */
  SPI_Cmd(CODEC_SPI, ENABLE);
  
  /*====         инициализация таймера для генерации сигнала SYNC        =====*/
  /* TIM clock enable */
  CODEC_TIM_CLK_CMD( CODEC_TIM_CLK, ENABLE );
  TIM_DeInit(CODEC_TIM);
  /* Time base configuration */	
  TIM_TimeBaseStructure.TIM_Period = (CODEC_TIME_UPDATE_SYNC/MAX_CODEC)-1;
  TIM_TimeBaseStructure.TIM_Prescaler = CODEC_TIMER_Prescaler-1;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(CODEC_TIM, &TIM_TimeBaseStructure);
  
  /* Output Compare Active Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
  TIM_OCInitStructure.TIM_Pulse = 1000;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC1Init(CODEC_TIM, &TIM_OCInitStructure);
  TIM_OC1PreloadConfig(CODEC_TIM, TIM_OCPreload_Enable);  
  
  /* Clear  */
  TIM_SetCounter( CODEC_TIM, 0 );
    
  /* Enable the TIM global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = CODEC_TIM_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00; 
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);			
  
  /* Enable the interrupt. */
  TIM_ITConfig( CODEC_TIM, TIM_IT_Update , ENABLE );
  TIM_ITConfig( CODEC_TIM, TIM_IT_CC1 , ENABLE );  
  
  /* Сброс флагов прерывания таймера */
  TIM_ClearITPendingBit( CODEC_TIM, TIM_IT_Update );
  
  /* Сброс флагов сравнения таймера */  
  TIM_ClearITPendingBit( CODEC_TIM, TIM_IT_CC1 );  

  /* Включаем таймер */
  TIM_Cmd( CODEC_TIM, ENABLE );    
  /*==========================================================================*/  
  /* Возвращаем реально установленный битрейт */
  return spi_bitrate_real;
}

/**
  * @brief  Запись/чтение байта по spi
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @param  uint8_t data - байт для записи
  * @retval uint8_t прочитаный байт 
  * 
  */
uint8_t spi_wr_rd_byte(pcm_cmx_t* CODECx , uint8_t wr_data)
{
  uint8_t temp_data;
  /* Обнуляем флаги                                                         */
  temp_data = (uint8_t)(CODEC_SPI->DR);                                     
  /* Стартовать передачу SPI, установив CS в 0                              */
  CODECx->CS_PORTx->BSRRH = CODECx->CS_Pin;                                 
  /* Ждем 0.5 мкс                                                           */
  ns100_delay(5);                                                           
  /* Ожидание пока не освободится регистр DR                                */
  while((CODEC_SPI->SR & SPI_I2S_FLAG_TXE) == RESET);                       
  /* Записать данные в регистр DR для передачи                              */
  CODEC_SPI->DR = (uint16_t)wr_data;                                        
  /* Ожидание принятие данных                                               */
  while((CODEC_SPI->SR & SPI_I2S_FLAG_RXNE) == RESET);                      
  temp_data = (uint8_t)(CODEC_SPI->DR);                                     
  /* Ждем 1 мкс                                                             */
  ns100_delay(10);                                                          
  /* Завершить передачу SPI, установив CS в 1                               */
  CODECx->CS_PORTx->BSRRL = CODECx->CS_Pin;                                 
  /* Вернуть полученные данные, прочитанные из регистр DR                   */
  return temp_data;
}
 
/**
  * @brief  Запись/чтение слова по spi
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @param  uint16_t data - слово для записи
  * @retval uint16_t прочитанное слово 
  * 
  */
uint16_t spi_wr_rd_word( pcm_cmx_t* CODECx , uint16_t wr_data)
{
  uint16_t temp_data;
  /* Обнуляем флаги                                                         */
  temp_data = CODEC_SPI->DR;                                                
  /* Стартовать передачу SPI, установив CS в 0                              */
  CODECx->CS_PORTx->BSRRH = CODECx->CS_Pin;                                 
  /* Ждем 0.5 мкс                                                           */
  ns100_delay(5);                                                           
                                                                            
  /* Установка запись/чтения данных                                         */
  /* Ожидание пока не освободится регистр DR                                */
  while((CODEC_SPI->SR & SPI_I2S_FLAG_TXE) == RESET);                       
  /* Записать данные в регистр DR для передачи                              */
  CODEC_SPI->DR = (( wr_data ) >> 8) & 0x00FF;                              
  /* Ожидание принятие данных                                               */
  while((CODEC_SPI->SR & SPI_I2S_FLAG_RXNE) == RESET);                      
  /* Чтение полученных данных                                               */
  temp_data = ((CODEC_SPI->DR) << 8) & 0xFF00 ;                             
                                                                            
  /* Ждем 0.5 мкс                                                           */
  ns100_delay(5);                                                           
                                                                            
  /* Установка запись/чтения данных */                                      
  /* Ожидание пока не освободится регистр DR                                */
  while((CODEC_SPI->SR & SPI_I2S_FLAG_TXE) == RESET);                       
  /* Записать данные в регистр DR для передачи                              */
  CODEC_SPI->DR = wr_data & 0x00FF;                                         
  /* Ожидание принятие данных                                               */
  while((CODEC_SPI->SR & SPI_I2S_FLAG_RXNE) == RESET);                      
  /* Чтение полученных данных                                               */
  temp_data = temp_data | ((CODEC_SPI->DR) & 0x00FF);                       
                                                                            
  /* Ждем 1 мкс                                                             */  
  ns100_delay(10);                                                          
  /* Завершить передачу SPI, установив CS в 1                               */
  CODECx->CS_PORTx->BSRRL = CODECx->CS_Pin;                                 
  /* Вернуть полученные данные, прочитаные из регистр DR                    */
  return temp_data; 
}

/**
  * @brief  Запись/чтение регистра байта по заданному адресу
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @param  uint8_t addr - адрес для записи
  * @param  uint8_t data - байт для записи
  * @retval uint8_t прочитанный байт 
  * 
  */
uint8_t spi_wr_rd_reg_byte( pcm_cmx_t*  CODECx , uint8_t addr, uint8_t data)
{
  uint8_t temp_data;
  /* Обнуляем флаги                                                         */
  temp_data = (uint8_t)(CODEC_SPI->DR);                                     
  /* Стартовать передачу SPI, установив CS в 0                              */
  CODECx->CS_PORTx->BSRRH = CODECx->CS_Pin;                                 
  /* Ждем 0.5 мкс                                                           */
  ns100_delay(5);
  
  /* Установка адреса */
  /* Ожидание пока не освободится регистр DR                                */
  while((CODEC_SPI->SR & SPI_I2S_FLAG_TXE) == RESET);                       
  /* Записать данные в регистр DR для передачи                              */
  CODEC_SPI->DR = (uint16_t)addr;                                           
  /* Ожидание принятие данных                                               */
  while((CODEC_SPI->SR & SPI_I2S_FLAG_RXNE) == RESET);
  /* Чтение полученных данных                                               */                                                        
  temp_data = (uint8_t)(CODEC_SPI->DR);
  
  /* Ждем 0.5 мкс                                                           */
  ns100_delay(5);   
    
  /* Установка запись/чтения данных */
  /* Ожидание пока не освободится регистр DR                                */
  while((CODEC_SPI->SR & SPI_I2S_FLAG_TXE) == RESET);
  /* Записать данные в регистр DR для передачи                              */
  CODEC_SPI->DR = (uint16_t)data;
  /* Ожидание принятие данных                                               */
  while((CODEC_SPI->SR & SPI_I2S_FLAG_RXNE) == RESET);
  
  /* Чтение полученных данных                                               */
  temp_data = (uint8_t)(CODEC_SPI->DR);                                     
  /* Ждем 1 мкс                                                             */
  ns100_delay(10);                                                          
  /* Завершить передачу SPI, установив CS в 1                               */
  CODECx->CS_PORTx->BSRRL = CODECx->CS_Pin;                                 
  /* Вернуть полученные данные, прочитанные из регистр DR                   */
  return temp_data;   
}

/**
  * @brief  Запись/чтение регистра 16 бит по заданному адресу
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @param  uint8_t addr - адрес для записи
  * @param  uint16_t data - слово для записи
  * @retval uint16_t прочитанный байт 
  * 
  */
uint16_t spi_wr_rd_reg_word( pcm_cmx_t*  CODECx , uint8_t addr, uint16_t data)
{
  uint16_t temp_data;
  /* Обнуляем флаги                                                         */
  temp_data = CODEC_SPI->DR;                                                
  /* Стартовать передачу SPI, установив CS в 0                              */
  CODECx->CS_PORTx->BSRRH = CODECx->CS_Pin;                                 
  /* Ждем 0.5 мкс                                                           */
  ns100_delay(5);
  
  /* Установка адреса */
  /* Ожидание пока не освободится регистр DR                                */
  while((CODEC_SPI->SR & SPI_I2S_FLAG_TXE) == RESET);                       
  /* Записать данные в регистр DR для передачи                              */
  CODEC_SPI->DR = (uint16_t)addr;                                           
  /* Ожидание принятие данных                                               */
  while((CODEC_SPI->SR & SPI_I2S_FLAG_RXNE) == RESET);                      
  /* Чтение полученных данных                                               */
  temp_data = CODEC_SPI->DR;
  
  /* Ждем 0.5 мкс                                                           */
  ns100_delay(5);   
  
  /* Установка запись/чтения данных */
  /* Ожидание пока не освободится регистр DR                                */
  while((CODEC_SPI->SR & SPI_I2S_FLAG_TXE) == RESET);                       
  /* Записать данные в регистр DR для передачи                              */
  CODEC_SPI->DR = (( data ) >> 8) & 0x00FF;                                 
  /* Ожидание принятие данных                                               */
  while((CODEC_SPI->SR & SPI_I2S_FLAG_RXNE) == RESET);                      
  /* Чтение полученных данных                                               */
  temp_data = ((CODEC_SPI->DR) << 8) & 0xFF00 ; 
  
  /* Ждем 0.5 мкс                                                           */
  ns100_delay(5);   
  
  /* Установка запись/чтения данных */
  /* Ожидание пока не освободится регистр DR                                */
  while((CODEC_SPI->SR & SPI_I2S_FLAG_TXE) == RESET);                       
  /* Записать данные в регистр DR для передачи                              */
  CODEC_SPI->DR = data & 0x00FF;                                            
  /* Ожидание принятие данных                                               */
  while((CODEC_SPI->SR & SPI_I2S_FLAG_RXNE) == RESET);                      
  /* Чтение полученных данных                                               */
  temp_data = temp_data | ((CODEC_SPI->DR) & 0x00FF);      
  
  /* Ждем 1 мкс                                                             */
  ns100_delay(10);                                                          
  /* Завершить передачу SPI, установив CS в 1                               */
  CODECx->CS_PORTx->BSRRL = CODECx->CS_Pin;                                 
  /* Вернуть полученные данные, прочитанные из регистр DR                   */
  return temp_data;   
}

/**
  * @brief  Вкл. режим микрофона
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @retval None 
  * 
  */
void Set_Mic( pcm_cmx_t*  CODECx )
{
  spi_wr_rd_reg_word( CODECx, VCTRL, 0x0002 );
  /* Ждем 1 мкс */
  ns100_delay(10);
}

/**
  * @brief  Вкл. режим спикера
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @retval None 
  * 
  */
void Set_Speaker( pcm_cmx_t*  CODECx )
{
  spi_wr_rd_reg_word( CODECx, VCTRL, 0x0009 );
  /* Ждем 1 мкс                                                            */
  ns100_delay(10);
}

/**
  * @brief  Запись данных голоса в кодек CMX
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @retval None 
  */
void Send_Voice(pcm_cmx_t* CODECx)
{
  uint16_t cnt_wr_byte;
  /* Стартовать передачу SPI, установив CS в 0                             */
  CODECx->CS_PORTx->BSRRH = CODECx->CS_Pin;                                
  /* Ждем 0.5 мкс                                                          */
  ns100_delay(5);                                                          
  /* Ожидание пока не освободится регистр DR                               */
  while((CODEC_SPI->SR & SPI_I2S_FLAG_TXE) == RESET);                      
  /* Записать данные в регистр DR для передачи                             */
  CODEC_SPI->DR = (uint16_t)DECFRAME;                                      
  /* Ожидание принятие данных                                              */
  while((CODEC_SPI->SR & SPI_I2S_FLAG_RXNE) == RESET);                     
  /* Чтение регистра для обнуления флагов                                  */
  data_spi_rd = CODEC_SPI->DR;                                             
  /* Ждем 0.5 мкс                                                          */
  ns100_delay(5);     
  /* Цикл загрузки пакета */
  for(cnt_wr_byte = 0 ; cnt_wr_byte < CMX618_VOICE_LEN ; cnt_wr_byte++ )
  {
    /* Ожидание пока не освободится регистр DR                             */
    while((CODEC_SPI->SR & SPI_I2S_FLAG_TXE) == RESET);                    
    /* Записать данные в регистр DR для передачи                           */
    CODEC_SPI->DR = (uint16_t)(data_to_codec[cnt_wr_byte]);                
    /* Ожидание принятие данных                                            */
    while((CODEC_SPI->SR & SPI_I2S_FLAG_RXNE) == RESET);                   
    /* Чтение регистра для обнуления флагов                                */
    data_spi_rd = CODEC_SPI->DR;                                           
  }                                                                        
  /* Ждем 1 мкс                                                            */
  ns100_delay(10);                                                         
  /* Завершить передачу SPI, установив CS в 1                              */
  CODECx->CS_PORTx->BSRRL = CODECx->CS_Pin;
}

/**
  * @brief  Чтения данных голоса из кодека CMX
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @retval None 
  */
void Read_Voice(pcm_cmx_t*  CODECx)
{
  uint16_t cnt_wr_byte;
  /* Стартовать передачу SPI, установив CS в 0                             */
  CODECx->CS_PORTx->BSRRH = CODECx->CS_Pin;                                
  /* Ждем 0.5 мкс                                                          */
  ns100_delay(5);                                                          
  /* Ожидание пока не освободится регистр DR                               */
  while((CODEC_SPI->SR & SPI_I2S_FLAG_TXE) == RESET);                      
  /* Записать данные в регистр DR для передачи                             */
  CODEC_SPI->DR = (uint16_t)ENCFRAME;                                      
  /* Ожидание принятие данных                                              */
  while((CODEC_SPI->SR & SPI_I2S_FLAG_RXNE) == RESET);                     
  /* Чтение регистра для обнуления флагов                                  */
  data_spi_rd = CODEC_SPI->DR;                                             
  /* Ждем 0.5 мкс                                                          */
  ns100_delay(5);                                                          
  /* Цикл загрузки пакета */                                               
  for(cnt_wr_byte = 0 ; cnt_wr_byte < CMX618_VOICE_LEN ; cnt_wr_byte++ )   
  {                                                                        
    /* Ожидание пока не освободится регистр DR                             */
    while((CODEC_SPI->SR & SPI_I2S_FLAG_TXE) == RESET);                    
    /* Записать данные в регистр DR для передачи                           */
    CODEC_SPI->DR = SPI_DUMMY_16;                                          
    /* Ожидание принятие данных                                            */
    while((CODEC_SPI->SR & SPI_I2S_FLAG_RXNE) == RESET);                   
    /* Чтение регистра для обнуления флагов                                */
    codec_to_data[cnt_wr_byte] = (uint16_t)CODEC_SPI->DR;                  
  }                                                                        
  /* Ждем 1 мкс                                                            */
  ns100_delay(10);                                                         
  /* Завершить передачу SPI, установив CS в 1                              */
  CODECx->CS_PORTx->BSRRL = CODECx->CS_Pin;    
  
  /* если очередь открыта */
  if (CODECx->QueueOutCodecCMX != NULL)
  {       
    /* если есть данные, то ...                                            */
    /* Отправка пакета в очередь кодека                                    */
    xQueueSend ( CODECx->QueueOutCodecCMX , codec_to_data , ( TickType_t ) 0 );    
    
    /* Отправка уведомления задаче */
    if ( ( PortCMX.set_port_cmx_router.HandleTask ) != NULL  )
    {
      xTaskNotify( PortCMX.set_port_cmx_router.HandleTask,   /* Указатель на уведомлюемую задачу                         */
                  RECEIVING_NOTE,                             /* Значения уведомления                                     */
                  eSetBits );                                 /* Текущее уведомление добавляются к уже прописанному       */
    }
  }
}
/************** (C) COPYRIGHT 2020 DataExpress ***** END OF FILE **************/

