/**
  ******************************************************************************
  * @file    pre_set_rs_a.c
  * @author  Trembach Dmitry
  * @version V3.0.0
  * @date    22-10-2020
  * @brief   Файл функций предустановки rs uart
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 DataExpress</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "uart_control.h"
#include "pre_set_rs.h"
#include "handler_rs.h"
#include "init_rs.h"
#include "settings.h"    

#if RS_GATE_ENABLE == 1

/* Обьявление основной структуры RSPORT */
RS_struct_t RSPortB;

/**
  * @brief  Функция отработки програмного таймера 
  * @param  TimerHandle_t pxTimer - указатель на таймер вызвавщий функцию
  * @retval None
  */
void TimCallbackRS_B( TimerHandle_t pxTimer )
{
  /* Функция обработки програмного таймера.*/
  RS_SOFT_TIM_Handler( &RSPortB, pxTimer );
}

/**
  * @brief  Прерывание таймера RS_UART.
  * @param  None
  * @retval None
  */
void TIM8_UP_TIM13_IRQHandler( void )
{  
  if( TIM_GetITStatus( TIM13, TIM_IT_Update ) == SET)
  {
    /* Сброс запроса на переключение контента по завершению прерывания */
    RSPortB.IrqTimTaskNotifyWoken = pdFALSE;
    /* Функция обработки прерывания таймера RS_UART */
    RS_TIM_Handler( &RSPortB );
    /* Если уведомление было успешно отправлено */ 
    if( RSPortB.IrqTimTaskNotifyWoken != pdFALSE )
    {
      /* Отправим запрос на переключение контента по завершению прерывания */ 
      portYIELD_FROM_ISR( RSPortB.IrqTimTaskNotifyWoken );
    }
    TIM_ClearITPendingBit( TIM13, TIM_IT_Update );
  }
}

/**
  * @brief  Прерывание RS_UART.
  * @param  None
  * @retval None
  */
void UART7_IRQHandler( void )
{
  /* Сброс запроса на переключение контента по завершению прерывания */
  RSPortB.IrqRsTaskNotifyWoken = pdFALSE;

  /* Прерывание передатчика - буфер передатчика пуст */
  if (USART_GetITStatus( UART7, USART_IT_TXE ) != RESET)
  {
    RS_TXE_Handler( &RSPortB );
  } 
  /* Прерывание приемника - принят байт */
  if (USART_GetITStatus( UART7, USART_IT_RXNE ) != RESET)
  {
    RS_RXNE_Handler( &RSPortB );
  }  
  /* Прерывание приемника - прием пакета завершен */
  if (USART_GetITStatus( UART7, USART_IT_IDLE ) != RESET)
  {
    RS_IDLE_Handler( &RSPortB );
  }
  /* Событие было успешно отправлено */ 
  if( RSPortB.IrqRsTaskNotifyWoken != pdFALSE )
  {
    /* Отправим запрос на переключение контента по завершению прерывания */ 
    portYIELD_FROM_ISR( RSPortB.IrqRsTaskNotifyWoken );
  }    
}

/**
  * @brief Функция инициализации аппаратных ресурсов и предустановок порта RS
  * @param uint32_t baudrate - бодовая скорость                                       
  * @param uint8_t n_StopBits - число стоповых бит                                       
  * @param uint8_t mode_Parity_No - режим контроля четности      
  * @param type_set_connect_t set_rs_type - Установка для режима работы порта RS  
  * @param uint8_t OwnPortID - идентификатор порта
  * @param uint32_t MaskPortID - указатель на маску разрешенных ID портов
  * @param uint8_t  NumMaskBoxID - номер маски разрешенных ID пакетов
  * @retval None
  */
void pre_set_uart_b( uint32_t baudrate, uint8_t n_StopBits, uint8_t mode_Parity_No, type_set_connect_t set_rs_type, uint8_t OwnPortID, uint32_t MaskPortID, uint8_t  NumMaskBoxID )
{
  /*================================================= Настройки пользователя  =============================================*/  
  RSPortB.baudrate = baudrate;                                 /* Бодовая скорость         */  
  RSPortB.set_rs_type = set_rs_type;                           /* Режим работы порта RS    */         
  RSPortB.n_StopBits = n_StopBits;                             /* Число стоповых бит       */
  RSPortB.mode_Parity_No = mode_Parity_No;                     /* Режим контроля четности  */ 
  /*================================================= Апаратные ресурсы ====================================================*/
  /*========================================= Аппаратные ресурсы интерфейса РСМ SPI  =======================================*/           
  RSPortB.base_rs_port = UART7;                                 /* порт UART                                                */
  RSPortB.URT_GPIO_AF = GPIO_AF_UART7;                          /*                                                          */
  RSPortB.URT_RCC_APB_Periph = RCC_APB1Periph_UART7;            /*                                                          */ 
  RSPortB.RCC_URT_ClockCmd = RCC_APB1PeriphClockCmd;                 
  RSPortB.URT_IRQn = UART7_IRQn;                                /* прерывание UART                                          */   
  /*========================================================================================================================*/            
  RSPortB.URT_RX_Pin = GPIO_Pin_7;                              /*   ввод RX UART                                           */
  RSPortB.URT_RX_PORT = GPIOE;                                                                              
  RSPortB.URT_RX_CLK_GPIO = RCC_AHB1Periph_GPIOE;                                                                          
  RSPortB.URT_RX_PinSource = GPIO_PinSource7;                                                                         
  /*========================================================================================================================*/            
  RSPortB.URT_TX_Pin = GPIO_Pin_8;                              /*  вывод TX UART                                           */
  RSPortB.URT_TX_PORT = GPIOE;                                                                              
  RSPortB.URT_TX_CLK_GPIO = RCC_AHB1Periph_GPIOE;                                                                          
  RSPortB.URT_TX_PinSource = GPIO_PinSource8;                                                                         
  /*========================================================================================================================*/  
  /*========================================================================================================================*/           
  RSPortB.LED_TX_Pin            = GPIO_Pin_3;             /* вывод светодиода передачи                                */ 
  RSPortB.LED_TX_PORTx          = NULL;                   /* указатель на базовый адреc порта вывода                  */
  RSPortB.LED_TX_CLK_GPIO       = RCC_AHB1Periph_GPIOF;   /* включение тактирования вывода                            */   
  /*==================================================================================================================*/ 
  RSPortB.LED_RX_Pin            = GPIO_Pin_2;             /* вывод светодиода приема                                  */ 
  RSPortB.LED_TX_PORTx          = NULL;                   /* указатель на базовый адреc порта вывода                  */ 
  RSPortB.LED_RX_CLK_GPIO       = RCC_AHB1Periph_GPIOF;   /* включение тактирования вывода                            */    
  /*========================================================================================================================*/
  RSPortB.DE_RE_Pin = GPIO_Pin_10;                              /* вывод переключения PHY  прием /передача                  */ 
  RSPortB.DE_RE_PORTx = GPIOE;                                  /* указатель на базовый адреc порта                         */ 
  RSPortB.DE_RE_CLK_GPIO = RCC_AHB1Periph_GPIOE;                /* включение тактирования                                   */ 
  RSPortB.DE_RE_Inversion_Flag = true;                          /* вывод не инверсный                                       */   
  /*========================================================================================================================*/   
  RSPortB.RE_Pin = GPIO_Pin_2;                                  /* вывод переключения PHY  прием                            */ 
  RSPortB.RE_PORTx = NULL;                                      /* указатель на базовый адреc порта                         */ 
  RSPortB.RE_CLK_GPIO = RCC_AHB1Periph_GPIOE;                   /* включение тактирования                                   */ 
  RSPortB.RE_Inversion_Flag = false;                            /* вывод не инверсный                                       */ 
  /*========================================================================================================================*/   
  RSPortB.POL_RX_Pin = GPIO_Pin_9;                              /* вывод установка полярности по приему                     */ 
  RSPortB.POL_RX_PORTx = GPIOE;                                 /* указатель на базовый адреc порта                         */ 
  RSPortB.POL_RX_CLK_GPIO = RCC_AHB1Periph_GPIOE;               /* включение тактирования                                   */   
  /*========================================================================================================================*/     
  RSPortB.POL_TX_Pin = GPIO_Pin_11;                             /* вывод установка полярности по передачи                   */ 
  RSPortB.POL_TX_PORTx = GPIOE;                                 /* указатель на базовый адреc порта                         */ 
  RSPortB.POL_TX_CLK_GPIO = RCC_AHB1Periph_GPIOE;               /* включение тактирования                                   */   
  /*========================================================================================================================*/ 
  RSPortB.EN_RS_PHY_Pin = GPIO_Pin_12;                          /* вывод управления питанием PHY                            */ 
  RSPortB.EN_RS_PHY_PORTx = GPIOE;                              /* указатель на базовый адреc порта                         */ 
  RSPortB.EN_RS_PHY_CLK_GPIO = RCC_AHB1Periph_GPIOE;            /* включение тактирования                                   */   
  /*========================================================================================================================*/   
  /*========================================================================================================================*/     
  RSPortB.MODE_Pin = GPIO_Pin_15;                               /* ввод установки режима работы RS485/RS422                 */ 
  RSPortB.MODE_PORTx = GPIOF;                                   /* указатель на базовый адреc порта                         */ 
  RSPortB.MODE_CLK_GPIO = RCC_AHB1Periph_GPIOF;                 /* включение тактирования                                   */   
  /*========================================================================================================================*/ 
  RSPortB.base_rs_timer = TIM13;                                /* Таймер UART                                              */
  RSPortB.TIM_IRQn = TIM8_UP_TIM13_IRQn;                        /* прерывание таймера для UART                              */ 
  RSPortB.TIM_RCC_APB_Periph = RCC_APB1Periph_TIM13;            /*                                                          */ 
  RSPortB.RCC_TIM_ClockCmd = RCC_APB1PeriphClockCmd;                   
  RSPortB.TIM_clock = 90000000;                  
  /*========================================================================================================================*/    
#if UDP_LOGGER == 1 
  RSPortB.code_log_rx = CMD_LOG_RSB_RX;                         /* код типа логирования принятых данных                     */
  RSPortB.flag_log_rx = &LoggerRxRSB;                           /* указатель на флаг включения логирования принятых данных  */                    
  RSPortB.code_log_tx = CMD_LOG_RSB_TX;                         /* код типа логирования переданных данных                   */  
  RSPortB.flag_log_tx = &LoggerTxRSB;                           /* указатель на флаг включения логирования переданных данных*/   
#endif  

#if TEST_PIN_RS_ENABLE
  /*========================================= указатели на тестовые выводы для отладки =====================================*/     
  RSPortB.ArrTestPin[0].test_pin = NULL; 
  RSPortB.ArrTestPin[0].p_port   = NULL;  
  RSPortB.ArrTestPin[1].test_pin = NULL; 
  RSPortB.ArrTestPin[1].p_port   = NULL;  
  RSPortB.ArrTestPin[2].test_pin = NULL; 
  RSPortB.ArrTestPin[2].p_port   = NULL;  
  RSPortB.ArrTestPin[3].test_pin = NULL; 
  RSPortB.ArrTestPin[3].p_port   = NULL;
  RSPortB.ArrTestPin[4].test_pin = NULL; 
  RSPortB.ArrTestPin[4].p_port   = NULL;  
  RSPortB.ArrTestPin[5].test_pin = NULL; 
  RSPortB.ArrTestPin[5].p_port   = NULL;  
  RSPortB.ArrTestPin[6].test_pin = NULL; 
  RSPortB.ArrTestPin[6].p_port   = NULL;  
  RSPortB.ArrTestPin[7].test_pin = NULL; 
  RSPortB.ArrTestPin[7].p_port   = NULL;
#endif    
  
  /*======================================= Инициализация програмных таймеров ==============================================*/
  /* Открытие таймера периодического уведомления задачи */
  RSPortB.xSoftTimer = xTimerCreate( "TmNoteB",        /* Текстовое имя, не используется в RTOS kernel. */
                                    10,                /* Период таймера в тиках. */
                                    pdTRUE,            /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                    NULL,              /* В функцию параметры не передаем */
                                    TimCallbackRS_B ); /* Указатель на функцию , которую вызывает таймер. */
 
  if ( RSPortB.set_rs_type == SET_RS485_P )
  {  
    /* Открытие таймера контроля полярности при работе с блоками питания период обновления 60 сек */
    RSPortB.xSoftPolarTimer = xTimerCreate( "TmPLRB",    /* Текстовое имя, не используется в RTOS kernel. */
                                    CONTROL_POLAR_TIME_POWER,/* Период таймера в тиках. */
                                    pdTRUE,            /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                    NULL,              /* В функцию параметры не передаем */
                                    TimCallbackRS_B ); /* Указатель на функцию , которую вызывает таймер. */     
  }
  else
  {
    /* Открытие таймера контроля полярности */
    RSPortB.xSoftPolarTimer = xTimerCreate( "TmPLRB",    /* Текстовое имя, не используется в RTOS kernel. */
                                    CONTROL_POLAR_TIME,/* Период таймера в тиках. */
                                    pdTRUE,            /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                    NULL,              /* В функцию параметры не передаем */
                                    TimCallbackRS_B ); /* Указатель на функцию , которую вызывает таймер. */ 
  }   

  /* Открытие таймера контроля максимальной  паузы между передаваемыми пакетами  */
  RSPortB.xSoftWaitTX = xTimerCreate( "TmWtTxB",       /* Текстовое имя, не используется в RTOS kernel. */
                  CONTROL_BOX_TX_MAXTIME_WAIT,         /* Период таймера в тиках. */
                                    pdTRUE,            /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                    NULL,              /* В функцию параметры не передаем */
                                    TimCallbackRS_B ); /* Указатель на функцию , которую вызывает таймер. */  

  /* Открытие таймера запроса подсчета статистики */
  RSPortB.xSoftTimerDiag = xTimerCreate( "TmRsStB",    /* Текстовое имя, не используется в RTOS kernel. */
                                    1000,              /* Период таймера в тиках. */
                                    pdTRUE,            /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                    NULL,              /* В функцию параметры не передаем */
                                    TimCallbackRS_B ); /* Указатель на функцию , которую вызывает таймер. */  
  
  /* Открытие таймера ожидания завершения приема пакета */
  RSPortB.xSoftWaitRxEnd = xTimerCreate( "TmWtRxEB",   /* Текстовое имя, не используется в RTOS kernel. */
                                    160,               /* Период таймера в тиках. установка для скорости 19200*/
                                    pdTRUE,            /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                    NULL,              /* В функцию параметры не передаем */
                                    TimCallbackRS_B ); /* Указатель на функцию , которую вызывает таймер. */    

  /* Открытие таймера ожидания завершения передачи пакета */
  RSPortB.xSoftWaitTxEnd = xTimerCreate( "TmWtTxEB",   /* Текстовое имя, не используется в RTOS kernel. */
                                    160,               /* Период таймера в тиках. установка для скорости 19200*/
                                    pdTRUE,            /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                    NULL,              /* В функцию параметры не передаем */
                                    TimCallbackRS_B ); /* Указатель на функцию , которую вызывает таймер. */
   
  /*======================================= Инициализация подключения к роутеру потоков формата RS ==========================*/
  /* Зададим размер очереди роутеру - rs */
  RSPortB.size_queue_router_rs  = 6;
  /* Зададим размер очереди rs - роутеру */ 
  RSPortB.size_queue_rs_router  = 6;

  /* Проводим инициализацию подключения к роутеру */  
  RSPortB.set_port_router.FlagLockAddr = LOCK_ADDR;                       /* блокирует трансляцию в порт со своим адресом */
  RSPortB.set_port_router.MaskPortID = MaskPortID;                        /* установка маску ID портов                    */
  RSPortB.set_port_router.NumMaskBoxID = NumMaskBoxID;                    /* используем маску ID пакетов                  */
  RSPortB.set_port_router.PortID = OwnPortID;                             /* идентификатор порта порт                     */  
  RSPortB.set_port_router.PortBitID = BitID(OwnPortID);           /* идентификатор порта порт                     */
  RSPortB.set_port_router.HandleTask = NULL;                              /* Обнуление указателя на задачу                */ 
  
  /*======================================= Инициализация параметров задачи контроля RS ==================================*/
  RSPortB.pcName = "RS_PRTB";
  /*====================================== Функция инициализации RS_UART и открытия задачи контроля RS_UART ==============*/
  InitRS_UART(&RSPortB);
}

/**
  * @brief  Функция запроса статуса соединения по RS.
  * @param  None
  * @retval bool true - есть соединение
  *                   - нет соединения
  */
bool StatusLinkRSB( void )
{
  /* Проверка статуса полярности */
  if (RSPortB.status_polar == 1)
  {
    return true;
  }
  else
  {
    return false;
  }
}

/**
  * @brief  Функция запроса статуса аварий RS.
  * @param  None
  * @retval bool true - есть авария
  *                   - нет аварии
  */
bool GetAlarmRSB( void )
{
  /* Проверка статуса статуса аварий RS */
  if ( ( RSPortB.data_tx_diag.Status_Dev ) > 0 )
  {
    return true;
  }
  else
  {
    return false;
  }
}

/**
  * @brief  Функция обнуления счетчиков событий.
  * @param  None
  * @retval None
  *        
  */
void ResetEventCountersRSB( void )
{
  RSPortB.cnt_err_crc = 0;   /*  Счётчик ошибок CRC по приёму                               */
  RSPortB.cnt_err_polar = 0; /*  Счётчик поиска полярности, увеличивается, если произошёл   */
                             /*  разрыв связи и начался процесс поиска полярности           */
  RSPortB.cnt_box_tx = 0;    /*  счетчик переданных пакетов                                 */
  RSPortB.cnt_box_rx = 0;    /*  счетчик принятых пакетов                                   */ 
}  

/**
  * @brief  Функция запроса адреса соседа по порту RS.
  * @param  None
  * @retval uint16_t - адреса соседа по порту RS
  *        
  */
uint16_t GetNearPhyAddB( void )
{
  return RSPortB.phy_addr_near;
}  
  
/**
  * @brief  Функция инициализации порта для подключения RS.
  * @param  None
  * @retval None
  */
void Init_Port_RS_B( void )
{
  /* Функция инициализации аппаратных ресурсов и предустановок порта RS */
  pre_set_uart_b( DataSto.Settings.rs_bit_rate_b,                               /* бодовая скорость                          */
                  0,               /* число стоповых бит                        */                                      
                  0,               /* режим контроля четности                   */   
                 (type_set_connect_t)DataSto.Settings.Type_RS_b,  /* режим работы порта RS                     */ 
                  RSBPortID,                                                    /* идентификатор порта                       */
                  DataSto.Settings.mask_inpup_port_id_rs_b,                     /* указатель на маску разрешенных ID портов  */
                  DataSto.Settings.nmask_transl_rs_b );                         /* номер маски разрешенных ID пакетов        */          
}
#endif
/******************* (C) COPYRIGHT 2020 DataExpress *****END OF FILE****/
