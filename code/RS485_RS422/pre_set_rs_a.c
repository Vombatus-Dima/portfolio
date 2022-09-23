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
RS_struct_t RSPortA;

/**
  * @brief  Функция отработки програмного таймера 
  * @param  TimerHandle_t pxTimer - указатель на таймер вызвавщий функцию
  * @retval None
  */
void TimCallbackRS_A( TimerHandle_t pxTimer )
{
  /* Функция обработки програмного таймера.*/
  RS_SOFT_TIM_Handler( &RSPortA, pxTimer );
}

/**
  * @brief  Прерывание таймера RS_UART.
  * @param  None
  * @retval None
  */
void TIM8_TRG_COM_TIM14_IRQHandler( void )
{  
  if( TIM_GetITStatus( TIM14, TIM_IT_Update ) == SET)
  {
    /* Сброс запроса на переключение контента по завершению прерывания */
    RSPortA.IrqTimTaskNotifyWoken = pdFALSE;
    /* Функция обработки прерывания таймера RS_UART */
    RS_TIM_Handler( &RSPortA );
    /* Если уведомление было успешно отправлено */ 
    if( RSPortA.IrqTimTaskNotifyWoken != pdFALSE )
    {
      /* Отправим запрос на переключение контента по завершению прерывания */ 
      portYIELD_FROM_ISR( RSPortA.IrqTimTaskNotifyWoken );
    }
    TIM_ClearITPendingBit( TIM14, TIM_IT_Update );
  }
}

/**
  * @brief  Прерывание RS_UART.
  * @param  None
  * @retval None
  */
void USART6_IRQHandler( void )
{
  /* Сброс запроса на переключение контента по завершению прерывания */
  RSPortA.IrqRsTaskNotifyWoken = pdFALSE;

  /* Прерывание передатчика - буфер передатчика пуст */
  if (USART_GetITStatus( USART6, USART_IT_TXE ) != RESET)
  {
    RS_TXE_Handler( &RSPortA );
  } 
  /* Прерывание приемника - принят байт */
  if (USART_GetITStatus( USART6, USART_IT_RXNE ) != RESET)
  {
    RS_RXNE_Handler( &RSPortA );
  }  
  /* Прерывание приемника - прием пакета завершен */
  if (USART_GetITStatus( USART6, USART_IT_IDLE ) != RESET)
  {
    RS_IDLE_Handler( &RSPortA );
  }
  /* Событие было успешно отправлено */ 
  if( RSPortA.IrqRsTaskNotifyWoken != pdFALSE )
  {
    /* Отправим запрос на переключение контента по завершению прерывания */ 
    portYIELD_FROM_ISR( RSPortA.IrqRsTaskNotifyWoken );
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
void pre_set_uart_a( uint32_t baudrate, uint8_t n_StopBits, uint8_t mode_Parity_No, type_set_connect_t set_rs_type, uint8_t OwnPortID, uint32_t MaskPortID, uint8_t  NumMaskBoxID )
{
  /*================================================= Настройки пользователя  =============================================*/  
  RSPortA.baudrate = baudrate;                                 /* Бодовая скорость         */  
  RSPortA.set_rs_type = set_rs_type;                           /* Режим работы порта RS    */         
  RSPortA.n_StopBits = n_StopBits;                             /* Число стоповых бит       */
  RSPortA.mode_Parity_No = mode_Parity_No;                     /* Режим контроля четности  */ 
  /*================================================= Апаратные ресурсы ====================================================*/
  /*========================================= Аппаратные ресурсы интерфейса РСМ SPI  =======================================*/           
  RSPortA.base_rs_port = USART6;                                /* порт UART                                                */
  RSPortA.URT_GPIO_AF = GPIO_AF_USART6;                         /*                                                          */
  RSPortA.URT_RCC_APB_Periph = RCC_APB2Periph_USART6;           /*                                                          */ 
  RSPortA.RCC_URT_ClockCmd = RCC_APB2PeriphClockCmd;                 
  RSPortA.URT_IRQn = USART6_IRQn;                               /* прерывание UART                                          */   
  /*========================================================================================================================*/            
  RSPortA.URT_RX_Pin = GPIO_Pin_7;                              /*   ввод RX UART                                           */
  RSPortA.URT_RX_PORT = GPIOC;                                                                              
  RSPortA.URT_RX_CLK_GPIO = RCC_AHB1Periph_GPIOC;                                                                          
  RSPortA.URT_RX_PinSource = GPIO_PinSource7;                                                                         
  /*========================================================================================================================*/            
  RSPortA.URT_TX_Pin = GPIO_Pin_6;                              /*  вывод TX UART                                           */
  RSPortA.URT_TX_PORT = GPIOC;                                                                              
  RSPortA.URT_TX_CLK_GPIO = RCC_AHB1Periph_GPIOC;                                                                          
  RSPortA.URT_TX_PinSource = GPIO_PinSource6;                                                                         
  /*========================================================================================================================*/  
  /*========================================================================================================================*/           
  RSPortA.LED_TX_Pin             = NULL;                        /* вывод светодиода передачи                                */ 
  RSPortA.LED_TX_PORTx           = NULL;                        /* указатель на базовый адреc порта вывода                  */ 
  RSPortA.LED_TX_CLK_GPIO        = RCC_AHB1Periph_GPIOF;        /* включение тактирования вывода                            */   
  /*========================================================================================================================*/ 
  RSPortA.LED_RX_Pin             = NULL;                        /* вывод светодиода приема                                  */ 
  RSPortA.LED_RX_PORTx           = NULL;                        /* указатель на базовый адреc порта вывода                  */ 
  RSPortA.LED_RX_CLK_GPIO        = RCC_AHB1Periph_GPIOF;        /* включение тактирования вывода                            */   
  /*========================================================================================================================*/
  RSPortA.DE_RE_Pin = GPIO_Pin_15;                              /* вывод переключения PHY  прием /передача                  */ 
  RSPortA.DE_RE_PORTx = GPIOE;                                  /* указатель на базовый адреc порта                         */ 
  RSPortA.DE_RE_CLK_GPIO = RCC_AHB1Periph_GPIOE;                /* включение тактирования                                   */ 
  RSPortA.DE_RE_Inversion_Flag = true;                          /* вывод инверсный                                          */   
  /*========================================================================================================================*/   
  RSPortA.RE_Pin = GPIO_Pin_14;                                 /* вывод установка полярности по приему                     */ 
  RSPortA.RE_PORTx = NULL;                                      /* указатель на базовый адреc порта                         */ 
  RSPortA.RE_CLK_GPIO = RCC_AHB1Periph_GPIOE;                   /* включение тактирования                                   */   
  RSPortA.RE_Inversion_Flag = false;                            /* вывод не инверсный                                       */  
  /*========================================================================================================================*/
  RSPortA.POL_RX_Pin = GPIO_Pin_8;                              /* вывод установка полярности по приему                     */ 
  RSPortA.POL_RX_PORTx = GPIOA;                                 /* указатель на базовый адреc порта                         */ 
  RSPortA.POL_RX_CLK_GPIO = RCC_AHB1Periph_GPIOA;               /* включение тактирования                                   */   
  /*========================================================================================================================*/      
  RSPortA.POL_TX_Pin = GPIO_Pin_7;                              /* вывод установка полярности по передачи                   */ 
  RSPortA.POL_TX_PORTx = GPIOH;                                 /* указатель на базовый адреc порта                         */ 
  RSPortA.POL_TX_CLK_GPIO = RCC_AHB1Periph_GPIOH;               /* включение тактирования                                   */   
  /*========================================================================================================================*/
  RSPortA.EN_RS_PHY_Pin = GPIO_Pin_8;                           /* вывод управления питанием PHY                            */ 
  RSPortA.EN_RS_PHY_PORTx = GPIOH;                              /* указатель на базовый адреc порта                         */ 
  RSPortA.EN_RS_PHY_CLK_GPIO = RCC_AHB1Periph_GPIOH;            /* включение тактирования                                   */   
  /*========================================================================================================================*/    
  /*========================================================================================================================*/ 
  RSPortA.MODE_Pin = GPIO_Pin_13;                               /* ввод установки режима работы RS485/RS422                 */ 
  RSPortA.MODE_PORTx = GPIOE;                                   /* указатель на базовый адреc порта                         */ 
  RSPortA.MODE_CLK_GPIO = RCC_AHB1Periph_GPIOE;                 /* включение тактирования                                   */   
  /*========================================================================================================================*/   
  RSPortA.base_rs_timer = TIM14;                                /* Таймер UART                                              */
  RSPortA.TIM_IRQn = TIM8_TRG_COM_TIM14_IRQn;                   /* прерывание таймера для UART                              */ 
  RSPortA.TIM_RCC_APB_Periph = RCC_APB1Periph_TIM14;            /*                                                          */ 
  RSPortA.RCC_TIM_ClockCmd = RCC_APB1PeriphClockCmd;                   
  RSPortA.TIM_clock = 90000000;                  
  /*========================================================================================================================*/    
#if UDP_LOGGER == 1 
  RSPortA.code_log_rx = CMD_LOG_RSA_RX;                         /* код типа логирования принятых данных                     */
  RSPortA.flag_log_rx = &LoggerRxRSA;                           /* указатель на флаг включения логирования принятых данных  */                    
  RSPortA.code_log_tx = CMD_LOG_RSA_TX;                         /* код типа логирования переданных данных                   */  
  RSPortA.flag_log_tx = &LoggerTxRSA;                           /* указатель на флаг включения логирования переданных данных*/   
#endif  
  
#if TEST_PIN_RS_ENABLE
  /*========================================= указатели на тестовые выводы для отладки =====================================*/     
  RSPortA.ArrTestPin[0].test_pin = NULL;
  RSPortA.ArrTestPin[0].p_port   = NULL;  
  RSPortA.ArrTestPin[1].test_pin = NULL;
  RSPortA.ArrTestPin[1].p_port   = NULL;  
  RSPortA.ArrTestPin[2].test_pin = NULL;
  RSPortA.ArrTestPin[2].p_port   = NULL;  
  RSPortA.ArrTestPin[3].test_pin = NULL;
  RSPortA.ArrTestPin[3].p_port   = NULL;  
  RSPortA.ArrTestPin[4].test_pin = NULL;
  RSPortA.ArrTestPin[4].p_port   = NULL;  
  RSPortA.ArrTestPin[5].test_pin = NULL;
  RSPortA.ArrTestPin[5].p_port   = NULL;  
  RSPortA.ArrTestPin[6].test_pin = NULL;
  RSPortA.ArrTestPin[6].p_port   = NULL;  
  RSPortA.ArrTestPin[7].test_pin = NULL;
  RSPortA.ArrTestPin[7].p_port   = NULL;   
#endif   

  /*======================================= Инициализация програмных таймеров ==============================================*/
  /* Открытие таймера периодического уведомления задачи */
  RSPortA.xSoftTimer = xTimerCreate( "TmNoteA",        /* Текстовое имя, не используется в RTOS kernel. */
                                    10,                /* Период таймера в тиках. */
                                    pdTRUE,            /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                    NULL,              /* В функцию параметры не передаем */
                                    TimCallbackRS_A ); /* Указатель на функцию , которую вызывает таймер. */

  if ( RSPortA.set_rs_type == SET_RS485_P )
  {  
    /* Открытие таймера контроля полярности при работе с блоками питания период обновления 60 сек */
    RSPortA.xSoftPolarTimer = xTimerCreate( "TmPLRA",    /* Текстовое имя, не используется в RTOS kernel. */
                                    CONTROL_POLAR_TIME_POWER,/* Период таймера в тиках. */
                                    pdTRUE,            /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                    NULL,              /* В функцию параметры не передаем */
                                    TimCallbackRS_A ); /* Указатель на функцию , которую вызывает таймер. */  
  }
  else
  {
    /* Открытие таймера контроля полярности */
    RSPortA.xSoftPolarTimer = xTimerCreate( "TmPLRA",    /* Текстовое имя, не используется в RTOS kernel. */
                                    CONTROL_POLAR_TIME,/* Период таймера в тиках. */
                                    pdTRUE,            /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                    NULL,              /* В функцию параметры не передаем */
                                    TimCallbackRS_A ); /* Указатель на функцию , которую вызывает таймер. */    
  }  

  /* Открытие таймера контроля максимальной  паузы между передаваемыми пакетами  */
  RSPortA.xSoftWaitTX = xTimerCreate( "TmWtTxA",       /* Текстовое имя, не используется в RTOS kernel. */
                  CONTROL_BOX_TX_MAXTIME_WAIT,         /* Период таймера в тиках. */
                                    pdTRUE,            /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                    NULL,              /* В функцию параметры не передаем */
                                    TimCallbackRS_A ); /* Указатель на функцию , которую вызывает таймер. */  

  /* Открытие таймера запроса подсчета статистики */
  RSPortA.xSoftTimerDiag = xTimerCreate( "TmRsStA",    /* Текстовое имя, не используется в RTOS kernel. */
                                    1000,               /* Период таймера в тиках. */
                                    pdTRUE,            /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                    NULL,              /* В функцию параметры не передаем */
                                    TimCallbackRS_A ); /* Указатель на функцию , которую вызывает таймер. */  
  
  /* Открытие таймера ожидания завершения приема пакета */
  RSPortA.xSoftWaitRxEnd = xTimerCreate( "TmWtRxEA",   /* Текстовое имя, не используется в RTOS kernel. */
                                    160,               /* Период таймера в тиках. установка для скорости 19200*/
                                    pdTRUE,            /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                    NULL,              /* В функцию параметры не передаем */
                                    TimCallbackRS_A ); /* Указатель на функцию , которую вызывает таймер. */    
  
  /* Открытие таймера ожидания завершения передачи пакета */
  RSPortA.xSoftWaitTxEnd = xTimerCreate( "TmWtTxEA",   /* Текстовое имя, не используется в RTOS kernel. */
                                    160,               /* Период таймера в тиках. установка для скорости 19200*/
                                    pdTRUE,            /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                    NULL,              /* В функцию параметры не передаем */
                                    TimCallbackRS_A ); /* Указатель на функцию , которую вызывает таймер. */   
  
  /*======================================= Инициализация подключения к роутеру потоков формата RS ==========================*/
  /* Зададим размер очереди роутеру - rs */
  RSPortA.size_queue_router_rs  = 6;
  /* Зададим размер очереди rs - роутеру */ 
  RSPortA.size_queue_rs_router  = 6;

  /* Проводим инициализацию подключения к роутеру */  
  RSPortA.set_port_router.FlagLockAddr = LOCK_ADDR;                       /* блокирует трансляцию в порт со своим адресом */
  RSPortA.set_port_router.MaskPortID = MaskPortID;                        /* установка маску ID портов                    */
  RSPortA.set_port_router.NumMaskBoxID = NumMaskBoxID;                    /* используем маску ID пакетов                  */
  RSPortA.set_port_router.PortID = OwnPortID;                             /* идентификатор порта порт                     */  
  RSPortA.set_port_router.PortBitID = BitID(OwnPortID);                   /* идентификатор порта порт                     */
  RSPortA.set_port_router.HandleTask = NULL;                              /* Обнуление указателя на задачу                */ 
  
  /*======================================= Инициализация параметров задачи контроля RS =====================================*/
  RSPortA.pcName = "RS_PRTA";
  
  /*====================================== Функция инициализации RS_UART и открытия задачи контроля RS_UART =================*/
  InitRS_UART(&RSPortA);
}

/**
  * @brief  Функция запроса статуса соединения по RS.
  * @param  None
  * @retval bool true - есть соединение
  *                   - нет соединения
  */
bool StatusLinkRSA( void )
{
  /* Проверка статуса полярности */
  if (RSPortA.status_polar == 1)
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
bool GetAlarmRSA( void )
{
  /* Проверка статуса статуса аварий RS */
  if ( ( RSPortA.data_tx_diag.Status_Dev ) > 0 )
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
void ResetEventCountersRSA( void )
{
  RSPortA.cnt_err_crc = 0;   /*  Счётчик ошибок CRC по приёму                               */
  RSPortA.cnt_err_polar = 0; /*  Счётчик поиска полярности, увеличивается, если произошёл   */
                             /*  разрыв связи и начался процесс поиска полярности           */
  RSPortA.cnt_box_tx = 0;    /*  счетчик переданных пакетов                                 */
  RSPortA.cnt_box_rx = 0;    /*  счетчик принятых пакетов                                   */ 
}  

/**
  * @brief  Функция запроса адреса соседа по порту RS.
  * @param  None
  * @retval uint16_t - адреса соседа по порту RS
  *        
  */
uint16_t GetNearPhyAddA( void )
{
  return RSPortA.phy_addr_near;
}  
  
/**
  * @brief  Функция инициализации порта для подключения RS.
  * @param  None
  * @retval None
  */
void Init_Port_RS_A( void )
{
  /* Функция инициализации аппаратных ресурсов и предустановок порта RS */
  pre_set_uart_a( DataSto.Settings.rs_bit_rate_a,                               /* бодовая скорость                          */
                 0,                                             /* число стоповых бит                        */                          
                 0,                                             /* режим контроля четности                   */ 
                  (type_set_connect_t)DataSto.Settings.Type_RS_a,               /* режим работы порта RS                     */ //SET_RS422,//
                  RSAPortID,                                                    /* идентификатор порта                       */
                  DataSto.Settings.mask_inpup_port_id_rs_a,                     /* указатель на маску разрешенных ID портов  */
                  DataSto.Settings.nmask_transl_rs_a );                         /* номер маски разрешенных ID пакетов        */
}
#endif
/******************* (C) COPYRIGHT 2020 DataExpress *****END OF FILE****/
