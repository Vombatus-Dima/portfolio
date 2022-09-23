/**
  ******************************************************************************
  * @file    init_rs.c
  * @author  Trembach Dmitry
  * @version V1.0.0
  * @date    22-10-2020
  * @brief   Файл функций инициализации rs по заданным предустановкам 
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 DataExpress</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "pre_set_rs.h"
#include "init_rs.h"
#if RS_GATE_ENABLE == 1

/**
  * @brief  Функция вычисления констант временных интервалов
  * @param  RS_struct_t* PortRS указатель на структуру порта
  * @retval None
  */
void CalcTimeDelayUART(RS_struct_t* PortRS)
{
  //   При скорости  1 000 000 бод/сек - 1 бод = 1 мкс
  //   При скорости  600 бод/сек       - 1 бод = 1667 мкс
  //
  //   Пересчитаем требуемые интервалы времени в боды
  //   
  //   Маркер/данные RS422 не реже 300 ms
  //   Между макетами RS422 не менее 30 бод, не менее 100 мкс
  //   RS485 ответ не ранее чем через 400 мкс        
  //   RS485 время ожидания ответа 400 байт, не менее 50 мс 
  //    
  //   Доотправка пакета 12 бод    

  // Вычисление делителей для таймера пауз
  // допустимая минимальная скорость 1200 бит/сек 
  
  // Устанавливаем предварительный делитель тактовой на 1
  PortRS->TIM_RS_ClockDivision = TIM_CKD_DIV1; 
  // Вычисляем прескалер таймера
  PortRS->TIM_RS_Prescaler = (uint16_t)(((float)(PortRS->TIM_clock))/((float)(PortRS->baudrate)))+1;
  // Вычисляем время одного тика в микросекундах
  PortRS->time_one_tic = 1000000.0/(((float)(PortRS->TIM_clock))/((float)PortRS->TIM_RS_Prescaler));

  // Вычисление констант
  //==========================================================================
  // Вычисляем число бод на 400 мкс
  PortRS->temp_time_var = 400.0/(PortRS->time_one_tic);     
  // RS485 ответ не ранее чем через 400 мкс (не ранее 30 бод)
  if (PortRS->temp_time_var > 30.0)
  {
    // Если полученное значение больше 30 бод - принимаем интервал 400 мкс 
    // в течении 20 бод приемник ждет байт до запуска прерывания  
    PortRS->box_rx_time_box_tx = ((uint16_t)(PortRS->temp_time_var)) - 20;
  }
  else
  {
    // Если нет принимаем 10 бод
    // в течении 20 бод приемник ждет байт до запуска прерывания 
    PortRS->box_rx_time_box_tx = 10;      
  }
  //==========================================================================    
  // Вычисляем число бод на 50 мс
  PortRS->temp_time_var = 50000.0/(PortRS->time_one_tic);     
  // RS485 время ожидания ответа 400 байт, не менее 50 мс 
  if (PortRS->temp_time_var > 4000.0)
  {
    // Если полученное значение больше 4000 бод - принимаем интервал 50 мс 
    PortRS->box_tx_time_no_box_rx = (uint16_t)(PortRS->temp_time_var);
  }
  else
  {
    // Если полученное значение меньше 4000 бод 
    PortRS->box_tx_time_no_box_rx = 4000;      
  }  
  //========================================================================== 
  // Вычисляем число бод на 100 мкс
  PortRS->temp_time_var = 100.0/(PortRS->time_one_tic);     
  // RS422 между макетами не менее 30 бод, не менее 100 мкс 
  if (PortRS->temp_time_var > 30.0)
  {
    // Если полученное значение больше 30 бод - принимаем интервал 100 мкс 
    PortRS->box_tx_min_time_box_tx = (uint16_t)(PortRS->temp_time_var);
  }
  else
  {
    // Если полученное значение меньше 30 бод 
    PortRS->box_tx_min_time_box_tx = 30;      
  }  
  //==========================================================================    
  // Вычисляем число бод на 10 мс
  // RS422 маркер/данные не реже 300 ms  
  // Интервал box_tx_max_time_box_tx отсчитываем частями
  PortRS->box_tx_max_time_box_tx = (uint16_t)(10000.0/(PortRS->time_one_tic));
  //========================================================================== 
  //  Завершени отправки данных из буфера 12 бод 
  PortRS->box_tx_completion = 10;//12;
  // Период таймера.
  PortRS->TIM_RS_Period = 0xffff; 
  // максимально возможный объем передавемых данных
  PortRS->MaxDataTxRxValue = ((((float)(PortRS->baudrate))/(10000.0))*((float)SIZE_LITTLE_TIME));
  //==========================================================================
  // Вычисляем максимальную продолжительность операции приема - передачи пакета
  // Значение в мс для програмного таймера FREERTOS
  // За максимальную длительность операции принимаем время приема передачи пакета
  // максимальной длинный умноженный на 1.5
  PortRS->max_soft_time_box_rx_tx = (uint32_t)((float)1000.0/(((float)(PortRS->baudrate))/((float)1.5*(float)8.0*((float)(sizeof(router_box_t))))))+1;

  xTimerChangePeriod(PortRS->xSoftWaitRxEnd ,PortRS->max_soft_time_box_rx_tx, 0 ); 
  xTimerChangePeriod(PortRS->xSoftWaitTxEnd ,PortRS->max_soft_time_box_rx_tx, 0 );  
  
}

/**
  * @brief  Инициализация аппаратной части .
  * @param  RS_struct_t* PortRS указатель на структуру порта
  * @retval None
  */
void RS_Hardware_Setup(RS_struct_t* PortRS)
{
  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;	
  
  /*------- Включение тактирования и инициализациия вводов -выводов ----------*/
  /* Включение тактирования Tx и Rx */
  RCC_AHB1PeriphClockCmd(PortRS->URT_TX_CLK_GPIO | PortRS->URT_RX_CLK_GPIO, ENABLE); 
  
  /* Включение альтернативной функции ввода -вывода UART */
  GPIO_PinAFConfig(PortRS->URT_TX_PORT, PortRS->URT_TX_PinSource, PortRS->URT_GPIO_AF);
  GPIO_PinAFConfig(PortRS->URT_RX_PORT, PortRS->URT_RX_PinSource, PortRS->URT_GPIO_AF);
  
  /* Конфигурирование Tx и Rx выводов как альтернативный push-pull */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  
  GPIO_InitStructure.GPIO_Pin = PortRS->URT_TX_Pin;
  GPIO_Init(PortRS->URT_TX_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = PortRS->URT_RX_Pin;
  GPIO_Init( PortRS->URT_RX_PORT, &GPIO_InitStructure);  

  /* Вспомогательный вывод POLAR_TX */  
  if ( PortRS->POL_TX_PORTx != NULL )
  {
    /* Конфигурирование POLAR_TX_PIN вывода */
    RCC_AHB1PeriphClockCmd( PortRS->POL_TX_CLK_GPIO , ENABLE );  
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    
    GPIO_InitStructure.GPIO_Pin = PortRS->POL_TX_Pin;    
    GPIO_Init( PortRS->POL_TX_PORTx, &GPIO_InitStructure );   
    
    set_pin( PortRS->POL_TX_PORTx, PortRS->POL_TX_Pin );
  }
  
  /* Вспомогательный вывод POLAR_RX */  
  if ( PortRS->POL_RX_PORTx != NULL )
  {
    /* Конфигурирование POLAR_RX_PIN вывода */
    RCC_AHB1PeriphClockCmd( PortRS->POL_RX_CLK_GPIO , ENABLE );  
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    
    GPIO_InitStructure.GPIO_Pin = PortRS->POL_RX_Pin;    
    GPIO_Init( PortRS->POL_RX_PORTx, &GPIO_InitStructure );   
    
    set_pin( PortRS->POL_RX_PORTx, PortRS->POL_RX_Pin );
  }  
  
  /* Вспомогательный вывод DE_RE_RS pin */  
  if ( PortRS->DE_RE_PORTx != NULL )
  {
    /* Конфигурирование POLAR_TX_PIN вывода */
    RCC_AHB1PeriphClockCmd( PortRS->DE_RE_CLK_GPIO , ENABLE );  
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    
    GPIO_InitStructure.GPIO_Pin = PortRS->DE_RE_Pin;    
    GPIO_Init( PortRS->DE_RE_PORTx, &GPIO_InitStructure );   
    
  }    
  
  /* Вспомогательный вывод RE_RS pin */  
  if ( PortRS->RE_PORTx != NULL )
  {
    /* Конфигурирование POLAR_TX_PIN вывода */
    RCC_AHB1PeriphClockCmd( PortRS->RE_CLK_GPIO , ENABLE );  
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    
    GPIO_InitStructure.GPIO_Pin = PortRS->RE_Pin;    
    GPIO_Init( PortRS->RE_PORTx, &GPIO_InitStructure );   
    
  }    
  
  /* Конфигурирование LED_RX вывода */ 
  if ( PortRS->LED_RX_PORTx != NULL )
  {
      /* Включение тактирования вывода LED_RX */
    RCC_AHB1PeriphClockCmd( PortRS->LED_RX_CLK_GPIO , ENABLE );  
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    
    GPIO_InitStructure.GPIO_Pin = PortRS->LED_RX_Pin;    
    GPIO_Init( PortRS->LED_RX_PORTx, &GPIO_InitStructure );  
    
    reset_pin( PortRS->LED_RX_PORTx, PortRS->LED_RX_Pin );
  }  
  
  /* Конфигурирование LED_TX вывода */ 
  if ( PortRS->LED_TX_PORTx != NULL )
  {
    
    RCC_AHB1PeriphClockCmd( PortRS->LED_TX_CLK_GPIO , ENABLE );  
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    
    GPIO_InitStructure.GPIO_Pin = PortRS->LED_TX_Pin;    
    GPIO_Init( PortRS->LED_TX_PORTx, &GPIO_InitStructure );   
    
    reset_pin( PortRS->LED_TX_PORTx, PortRS->LED_TX_Pin );
  }      

  /* Конфигурирование вывода управления питанием PHY */ 
  if ( PortRS->EN_RS_PHY_PORTx != NULL )
  {
    
    RCC_AHB1PeriphClockCmd( PortRS->EN_RS_PHY_CLK_GPIO , ENABLE );  
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    
    GPIO_InitStructure.GPIO_Pin = PortRS->EN_RS_PHY_Pin;    
    GPIO_Init( PortRS->EN_RS_PHY_PORTx, &GPIO_InitStructure );   
    
    reset_pin( PortRS->EN_RS_PHY_PORTx, PortRS->EN_RS_PHY_Pin );
  }   
    
  /* Конфигурирование ввода MODE*/ 
  if ( PortRS->MODE_PORTx != NULL )
  {
    /* Включение тактирования ввода MODE */
    RCC_AHB1PeriphClockCmd( PortRS->MODE_CLK_GPIO , ENABLE );
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   
    
    GPIO_InitStructure.GPIO_Pin =  PortRS->MODE_Pin;
    GPIO_Init(PortRS->MODE_PORTx, &GPIO_InitStructure);  
  }    
  
  /* Предустановка флага полярности выводов */
  PortRS->flag_polar = POLAR_LO;  
  
  /* Предустановка статуса полярности выводов */
  PortRS->status_polar = 0;
  
  /*--------------------------------------------------------------------------*/
  /*----------------- Включение тактирования блока RS_UART -------------------*/
  PortRS->RCC_URT_ClockCmd( PortRS->URT_RCC_APB_Periph, ENABLE );  
  /*---------------------- Конфирурирование блока UART------------------------*/
  
  if (PortRS->baudrate > 4800)
  {
    USART_OverSampling8Cmd( PortRS->base_rs_port, ENABLE );   
  }
  else
  {
    USART_OverSampling8Cmd( PortRS->base_rs_port, DISABLE );   
  }  
  
  USART_InitStructure.USART_BaudRate = PortRS->baudrate;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  
  switch(PortRS->n_StopBits)
  {
  case 1:  USART_InitStructure.USART_StopBits = USART_StopBits_0_5;  break;
  case 2:  USART_InitStructure.USART_StopBits = USART_StopBits_2;    break;
  case 3:  USART_InitStructure.USART_StopBits = USART_StopBits_1_5;  break;
  case 0:   
  default: USART_InitStructure.USART_StopBits = USART_StopBits_1;    break;
  }

  switch(PortRS->n_StopBits)
  {
  case 1:  USART_InitStructure.USART_Parity = USART_Parity_Even;     break;
  case 2:  USART_InitStructure.USART_Parity = USART_Parity_Odd;      break;
  case 0:                                                            
  default: USART_InitStructure.USART_Parity = USART_Parity_No;       break;
  }  

  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init( PortRS->base_rs_port, &USART_InitStructure );
  /*-------------------- Включаем прерывание блока UART-----------------------*/ 
  NVIC_InitStructure.NVIC_IRQChannel = PortRS->URT_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;//при Priority <= 4 не работает RTOS
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init( &NVIC_InitStructure );
  USART_ITConfig( PortRS->base_rs_port, USART_IT_RXNE , ENABLE );
  USART_ITConfig( PortRS->base_rs_port, USART_IT_IDLE , ENABLE );			
  /*------------------------- Включаем блока UART-----------------------------*/ 
  USART_Cmd(PortRS->base_rs_port, ENABLE);
    
  /*--------------------------- Инициализация таймера ------------------------*/
  /*--------------------- Включаем тактирование таймера ----------------------*/
  PortRS->RCC_TIM_ClockCmd( PortRS->TIM_RCC_APB_Periph, ENABLE );
  /*------------------------- Деинициализация таймера ------------------------*/  
  TIM_DeInit( PortRS->base_rs_timer );
  /*--------------- Вычисление констант временных интервалов -----------------*/
  CalcTimeDelayUART( PortRS );
  /*---------------------- Конфирурирование блока таймера --------------------*/
  TIM_TimeBaseStructure.TIM_Period = PortRS->TIM_RS_Period;
  TIM_TimeBaseStructure.TIM_Prescaler = PortRS->TIM_RS_Prescaler;
  TIM_TimeBaseStructure.TIM_ClockDivision = PortRS->TIM_RS_ClockDivision;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit( PortRS->base_rs_timer, &TIM_TimeBaseStructure );
  /*---------------------- Конфирурирование блока таймера --------------------*/
  TIM_SetCounter( PortRS->base_rs_timer, 0 );
  /*------------- Конфирурирование прерывания блока таймера ------------------*/
  TIM_ITConfig( PortRS->base_rs_timer, TIM_IT_Update, ENABLE );
  NVIC_InitStructure.NVIC_IRQChannel = PortRS->TIM_IRQn;
  /* при Priority <= 4 не работает RTOS */
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;
  /* Not used as 4 bits are used for the pre-emption priority. */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00; 
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  /*----------------- Сброс флага прерывания блока таймера -------------------*/
  TIM_ClearITPendingBit( PortRS->base_rs_timer, TIM_IT_Update );
  /*------------------------ Отлючение таймера -------------------------------*/
  TIM_Cmd( PortRS->base_rs_timer, DISABLE );
}

/**
  * @brief  Функция инициализации RS_UART и открытия задачи контроля RS_UART 
  * @param  RS_struct_t* PortRS указатель на структуру порта
  * @retval None
  */
void InitRS_UART( RS_struct_t* PortRS )
{
  // Открытие очереди для пакетов router to rs
  if ((PortRS->set_port_router.QueueOutRouter == NULL)&&( PortRS->size_queue_router_rs > 0 ))
  {
    //открытие очереди для size_queue_rs_router пакетов
    PortRS->set_port_router.QueueOutRouter = xQueueCreate( PortRS->size_queue_router_rs , sizeof(router_box_t)); 
  }	
  // Открытие очереди для пакетов rs to router   
  if ((PortRS->set_port_router.QueueInRouter == NULL)&&( PortRS->size_queue_rs_router > 0 ))
  {
    //открытие очереди для size_queue_rs_router пакетов
    PortRS->set_port_router.QueueInRouter = xQueueCreate( PortRS->size_queue_rs_router , sizeof(router_box_t)); 
  }	
  // Открытие очереди для приема комманд
  if (PortRS->xQueueCoreCMD == NULL )
  {
    //открытие очереди для приема MAX_SIZE_QUEUE_CNTRL_CMD комманд
    PortRS->xQueueCoreCMD = xQueueCreate( MAX_SIZE_QUEUE_CNTRL_CMD , sizeof(cntrl_cmd_t));
  }
  // Инициализация аппаратной части.
  RS_Hardware_Setup( PortRS );
  
  /* Анализ установленого режима - по вводу MODE */
  if ( PortRS->MODE_PORTx != NULL )
  {
    if ( ( get_pin( PortRS->MODE_PORTx, PortRS->MODE_Pin ) ) != 0 )
    {
      /* Предустановлен режим RS485 */
      PortRS->hard_setting_mode = HARD_MODE_RS485;
      switch ( PortRS->set_rs_type )
      {
      case SET_RS485_M:          
        PortRS->rs_type = CON_RS485_M;
        PortRS->marker_mode = MARKER_ENA_TRANS;     
        break;       
      case SET_RS485_S:          
        PortRS->rs_type = CON_RS485_S; 
        PortRS->marker_mode = MARKER_ENA_TRANS;
        break;
      case SET_RS485_P:          
        PortRS->rs_type = CON_RS485_M; 
        PortRS->marker_mode = MARKER_DIS_TRANS;        
        break;
      case SET_RS422:          
      default:
      PortRS->rs_type = CON_RS422;
      PortRS->marker_mode = MARKER_ENA_TRANS;        
        break;
      }
    }
    else
    {
      PortRS->hard_setting_mode = HARD_MODE_RS422;
      /* Предустановлен режим RS422 */
      PortRS->rs_type = CON_RS422;
      PortRS->marker_mode = MARKER_ENA_TRANS;      
    }  
  }
  else
  {
    PortRS->hard_setting_mode = HARD_MODE_EMPTY;
    
    /* Если ввод не определен работаем по настройкам Flash */
    switch ( PortRS->set_rs_type )
    {
    case SET_RS422:         
      PortRS->rs_type = CON_RS422;
      PortRS->marker_mode = MARKER_ENA_TRANS;     
      break;    
    case SET_RS485_S:          
      PortRS->rs_type = CON_RS485_S;  
      PortRS->marker_mode = MARKER_ENA_TRANS;
      break;
    case SET_RS485_P:          
      PortRS->rs_type = CON_RS485_M; 
      PortRS->marker_mode = MARKER_DIS_TRANS;   
      break;
    case SET_RS485_M:          
    default:
      PortRS->rs_type = CON_RS485_M;
      PortRS->marker_mode = MARKER_ENA_TRANS;
      break;
    }  
  }    

  // Открытие задачи контроля RS_UART
  xTaskCreate( RS_Control_Task, PortRS->pcName, configMINIMAL_STACK_SIZE*1.5, (void *)PortRS, CNTRL_RS_TASK_PRIO, &(PortRS->set_port_router.HandleTask) );
}
#endif
/******************* (C) COPYRIGHT 2020 DataExpress *****END OF FILE****/
