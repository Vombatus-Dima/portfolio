/**
  ******************************************************************************
  * @file    uart_control.c
  * @author  Trembach Dmitry
  * @version V2.5.0
  * @date    02-02-2019
  * @brief   Инициализация драйвера для управление UART в режимах RS485/RS422
  *
  *
  *   RS должен работать в двух режимах RS485 и RS422
  *    Контроль полярности
  *    Изменение скорости с диапазоне 1500 - 900 000 бод/сек 
  *
  *    При передаче по RS422 интервал между пакета 30 бод мин
  *    Режим работы - автомат состояний
  *    Переключение режима работы в прерываниях 
  *    Статистика по следующим параметрам:
  *    
  *   Статус подключения
  *   Физический адрес ВРМ к которому подключен
  *   Счётчик ошибок последовательности по приёму
  *   Счётчик ошибок CRC по приёму
  *   Счётчик ошибок переполнения буфера DMA по передаче
  *   Счётчик поиска полярности, увеличивается, если произошёл 
  *   разрыв связи и начался процесс поиска полярности
  *   % загрузки — текущий  (за время *) 
  *   % загрузки — минимальный (за время *)
  *   % загрузки — максимальный  (за время *)
  *
  *    
  *   
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 DataExpress</center></h2>
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "loader_settings.h"
#include "uart_control.h"
#include "board.h"
#include "GenerationCRC16.h"
#include "Core.h"
#include "uart_diag.h"
#include "board.h"   
   
   
#if RS_GATE_ENABLE == 1

/**
  * @brief  Функция инициализации при запуске задачи  
  * @param  RS_struct_t* port_rs - указатель на структуру контроля UART
  * @retval None
  */
void InitRS_Control_Task( RS_struct_t* port_rs )
{
  // Регистрирование очереди команд в таблице
  if (port_rs->xQueueCoreCMD != NULL)
  {
    // Ожидание инициализации указателя в таблице команд
    while(BaseQCMD[port_rs->set_port_router.PortID].Status_QCMD != QCMD_INIT)
    {
      // Ожидание 1 млсек
      vTaskDelay(1);
    }
    // Обнуление укаазтеля
    BaseQCMD[port_rs->set_port_router.PortID].QueueCMD = port_rs->xQueueCoreCMD;
    // Установка статуса
    BaseQCMD[port_rs->set_port_router.PortID].Status_QCMD = QCMD_ENABLE;          
  } 
  // Инициализации ресурсов задачи 
  /*---------- Подключение данного ресурса к мультиплексору ------------------*/ 
  // Запрашиваем ID порта маршрутизации
  while(request_port_pnt(&(port_rs->index_router_port)) != true) vTaskDelay(1);
  // Инициализация порта
  settings_port_pnt(port_rs->index_router_port,&(port_rs->set_port_router));
  // Включаем порт
  enable_port_pnt(port_rs->index_router_port);
  /*--------------------------------------------------------------------------*/

    
  /* Отработка типа соединения                                              */
  switch(port_rs->rs_type)
  {
  case CON_RS422:
    /* Остановка таймера                                                      */
    stop_hard_timer(port_rs->base_rs_timer);
    /* Отключаем прерывание по передаче                                       */
    USART_ITConfig( port_rs->base_rs_port, USART_IT_TXE , DISABLE);      
    /* Предустановка выводов смены полярности */
    if (port_rs->POL_TX_PORTx != NULL ) 
    {
    set_pin(   port_rs->POL_TX_PORTx, port_rs->POL_TX_Pin );
    }
    if (port_rs->POL_RX_PORTx != NULL ) 
    {
    set_pin(   port_rs->POL_RX_PORTx, port_rs->POL_RX_Pin );
    }
    
    /* Предустановка вывода управления передачей */
    if ( ( port_rs->DE_RE_PORTx ) != NULL ) 
    {
      if ( port_rs->DE_RE_Inversion_Flag == false )
      {
        set_pin( port_rs->DE_RE_PORTx, port_rs->DE_RE_Pin );
      }
      else
      {
    reset_pin( port_rs->DE_RE_PORTx,  port_rs->DE_RE_Pin  );
      }
    }          
    
    /* Предустановка вывода управления приемом */
    if ( ( port_rs->RE_PORTx ) != NULL ) 
    {
      if ( port_rs->RE_Inversion_Flag == false )
      {
        set_pin( port_rs->RE_PORTx, port_rs->RE_Pin );
      }
      else
      {
        reset_pin( port_rs->RE_PORTx, port_rs->RE_Pin );
      }
    }
    
    /* Переключаемся на режим                                                 */ 
    port_rs->fst_rs_tx = RS_TX_NONE_STATE; 
    /* Установка состояния по приему                                          */
    port_rs->fst_rs_rx = RS_RX_NONE_STATE;         
    
    /* Запуск таймаута                                                        */      
    set_hard_timer( port_rs->base_rs_timer, port_rs->box_tx_min_time_box_tx);
    start_hard_timer( port_rs->base_rs_timer);  
    break;

    
  case CON_RS485_M:
    
    /* Остановка таймера                                                      */
    stop_hard_timer(port_rs->base_rs_timer);
    /* Включаем прерывание по передаче                                        */
    USART_ITConfig( port_rs->base_rs_port, USART_IT_TXE , ENABLE );  
    
    /* Переключаемся на режим                                                 */ 
    port_rs->fst_rs_tx = RS_TX_WAIT_SEND; ; 
    /* Установка состояния по приему                                          */
    port_rs->fst_rs_rx = RS_RX_WAIT_RECEIVE;         
    
    /* Запуск таймаута                                                        */      
    set_hard_timer( port_rs->base_rs_timer, port_rs->box_tx_time_no_box_rx);
    start_hard_timer( port_rs->base_rs_timer);  
    break;
    
  case CON_RS485_S:
    
    /* Остановка таймера                                                      */
    stop_hard_timer(port_rs->base_rs_timer);
    /* Отключаем прерывание по передаче                                       */
    USART_ITConfig( port_rs->base_rs_port, USART_IT_TXE , DISABLE);
    
    /* Переключаемся на режим                                                 */ 
    port_rs->fst_rs_tx = RS_TX_NONE_STATE; 
    /* Установка состояния по приему                                          */
    port_rs->fst_rs_rx = RS_RX_NONE_STATE;         

    /* Запуск таймаута                                                        */      
    set_hard_timer( port_rs->base_rs_timer, port_rs->box_tx_min_time_box_tx);
    start_hard_timer( port_rs->base_rs_timer);  
    break;  
  }  
  
  /* Запуск програмных таймеров */
  xTimerStart( port_rs->xSoftTimer     , 0 );
  xTimerStart( port_rs->xSoftPolarTimer, 0 );  
  xTimerStart( port_rs->xSoftWaitTX    , 0 );    
  xTimerStart( port_rs->xSoftTimerDiag , 0 );  
 
}  
    
/**
  * @brief  Функция вызываемая периодически задачей контроля UART 
  * @param  RS_struct_t* rs_port_box - указатель на структуру контроля UART
  * @retval None
  */
void PeriodicFuncRS_Control_Task( RS_struct_t* rs_port_box )
{
  /* Если очередь команд открыта                  */
  if (rs_port_box->xQueueCoreCMD != NULL)
  {
    /* Проверка очереди команд                    */
    if ( xQueueReceive( rs_port_box->xQueueCoreCMD, &(rs_port_box->data_core_rx_cmd), ( TickType_t ) 0 ) == pdTRUE )
    {
      /* Получена команда - запуск обработки      */
      switch(rs_port_box->data_core_rx_cmd.CMD_ID)
      {
      case CMD_REQ_TAB_DIAG:
        /* Запрос диагностики на ядро             */
        GenDiagUART(rs_port_box,rs_port_box->data_core_rx_cmd.data_word[0]);
        break;
        
      case CMD_UPDATE_DIAG:
        /* Запрос обновления таблицы диагностики  */
        UpdateDiagUART(rs_port_box,rs_port_box->data_core_rx_cmd.data_dword[0]);      
        break;
      }      
    }
  } 
}

// Переопределение указателя аргуметра задачи
#define port_rs ((RS_struct_t*)pvParameters)

/**
  * @brief  Задача для контроля RS_UART
  * @param  pvParameters not used
  * @retval None
  */
void RS_Control_Task(void* pvParameters)
{	
  /* Функция инициализации при запуске задачи */
  InitRS_Control_Task( port_rs );
  
  /* Если есть контроль полярности */
  if ( ( ( port_rs->POL_RX_PORTx ) != NULL ) || ( ( port_rs->POL_TX_PORTx ) != NULL ) ) 
  {
  /* Установка стартовой полярности подключения приемника */ 
  Reversal_Polar_RX( port_rs );
  }
  /* Цикл периодического запуска задачи                */
  /* Запуск задачи по событию или по истечению 1 млсек */
  for( ;; )
  {
    /* Обнуляем сообщение */
    port_rs->NotifiedValue = 0;
    /*================================== Проверка наличия сообщений ========================================*/
    xTaskNotifyWait(0x00000000,                 /* Не очищайте биты уведомлений при входе               */
                    0xFFFFFFFF,                 /* Сбросить значение уведомления до 0 при выходе        */
                    &(port_rs->NotifiedValue),  /* Значение уведомленное передается в  NotifiedValue    */
                    portMAX_DELAY  );           /* Блокировка задачи до появления уведомления           */
    /* Получено уведомление. Проверяем, какие биты были установлены. */
    /*=========================================================================*/
    /*=========================================================================*/
    /*=========================================================================*/ 
    if( ( ( port_rs->NotifiedValue ) & ROUTER_NOTE ) != 0 )
    {  
      if ( ( port_rs->rs_type ) == CON_RS422 )
      {
        /* Включение прерывания передачи */
        USART_ITConfig( port_rs->base_rs_port, USART_IT_TXE , ENABLE);
        /* Сброс таймера ожидания передачи */
        xTimerReset( port_rs->xSoftWaitTX    , 0 );  
      }
    }        
    /*=========================================================================*/
    /*=========================================================================*/      
    if( ( ( port_rs->NotifiedValue ) & TIMER_NOTE ) != 0 )
    { /* Периодическое уведомление по таймеру */
      /* Проверяем очередь */
      /* Отправляем данные если они есть */        
      /* Функция вызываемая периодически задачей контроля UART */
      PeriodicFuncRS_Control_Task( port_rs );      
      /* Управления индикацией приемника */
      if (port_rs->cnt_led_rx != 0)
      {
        if ( ( port_rs->cnt_led_rx > 10 ) || ( port_rs->cnt_led_rx == 1 ) )
        {
          /* Отключение индикации приема пакета */
          if ( ( port_rs->LED_RX_PORTx ) != NULL ) 
          {
            reset_pin( port_rs->LED_RX_PORTx, port_rs->LED_RX_Pin ); 
          }
          /* Отключения таймера отсчета отключения индикации */
          port_rs->cnt_led_rx = 0;
        }
        else
        {
          /* Включение индикации приема пакета */
          if ( ( port_rs->LED_RX_PORTx ) != NULL ) 
          {
            /* Включаем светодиод */
            set_pin( port_rs->LED_RX_PORTx, port_rs->LED_RX_Pin ); 
          }
          /* Отсчет таймера отключения индикации */
          (port_rs->cnt_led_rx)--;
        }  
      }      
      /* Управления индикацией приемника */
      if (port_rs->cnt_led_tx != 0)
      {
        if ( ( port_rs->cnt_led_tx > 10 ) || ( port_rs->cnt_led_tx == 1 ) )
        {
          /* Отключение индикации приема пакета */
          if ( ( port_rs->LED_TX_PORTx ) != NULL ) 
          {
            reset_pin( port_rs->LED_TX_PORTx, port_rs->LED_TX_Pin ); 
          }
          /* Отключения таймера отсчета отключения индикации */
          port_rs->cnt_led_tx = 0;
        }
        else
        {
          if ( ( port_rs->LED_TX_PORTx ) != NULL ) 
          {
            /* Включаем светодиод */
            set_pin( port_rs->LED_TX_PORTx, port_rs->LED_TX_Pin ); 
          }    
          /* Отсчет таймера отключения индикации */
          (port_rs->cnt_led_tx)--;
        }  
      }
    }
    /*=========================================================================*/
    /*=========================================================================*/ 
    if( ( ( port_rs->NotifiedValue ) & REQ_DIAG_NOTE ) != 0 )
    {
      /* Вызов функции обновления диагностики малого интервала */
      UpdateLitteDiagUART(port_rs);
    }  
    /*=========================================================================*/
    /*=========================================================================*/ 
    if( ( ( port_rs->NotifiedValue ) & RS_POLAR_NOTE ) != 0 )
    {/* Отработка события таймера смены полярности */
      /* Если есть контроль полярности */
      if ( ( ( port_rs->POL_RX_PORTx ) != NULL ) || ( ( port_rs->POL_TX_PORTx ) != NULL ) ) 
      {  
      /* Смена полярности подключения приемника */ 
      Reversal_Polar_RX( port_rs );
    }  
      /* Сброс адреса к которому подключен */
      port_rs->phy_addr_near = 0x0000;
      /* Установка статуса полярности */
      port_rs->status_polar = 0;
      /*  */
      if (port_rs->status_port == CON_ON)
      {
        /* Установка статуса - нет подключения */
        port_rs->status_port = CON_OFF;    
        /* Подсчет включений режима поиска полярности */ 
        port_rs->cnt_err_polar++; 
        /* Отправка команды авария в ядро */
        SetRStoCoreCMD( port_rs, CMD_ALARM_PORT );
      }
      /* Запуск таймера смены полярности */
      xTimerReset( port_rs->xSoftPolarTimer, 0 );
    }  
    /*=========================================================================*/
    /*=========================================================================*/ 
    if( ( ( port_rs->NotifiedValue ) & RS_WAIT_TX_NOTE ) != 0 )
    {/* Отработка события таймера запроса передачи */
      /* Сброс таймера запроса передачи */
      xTimerReset( port_rs->xSoftWaitTX, 0 );  
      /* Анализ режима передатчика UART */
      if ( (  port_rs->fst_rs_tx == RS_TX_WAIT_SEND ) || ( port_rs->fst_rs_tx == RS_TX_NONE_STATE ) ) 
      {
        /* Анализ автомата состояний передачи UART    */
        switch ( port_rs->rs_type )
        {
        case CON_RS485_M: 
          if (  port_rs->fst_rs_rx == RS_RX_WAIT_RECEIVE )
          {
            /* Установка флага запроса маркера */
            port_rs->flag_marker = true;
            /* Включение прерывания передачи */
            USART_ITConfig( port_rs->base_rs_port, USART_IT_TXE , ENABLE);            
          }
          break; 
        case CON_RS422:  
          /* Установка флага запроса маркера */
          port_rs->flag_marker = true; 
          /* Включение прерывания передачи */
          USART_ITConfig( port_rs->base_rs_port, USART_IT_TXE , ENABLE);      
          break; 
        default:    
          break;	
        }  
      }
    }  
    /*=========================================================================*/
    /*=========================================================================*/     
    if( ( ( port_rs->NotifiedValue ) & RX_TIME_ERROR ) != 0 )
    {/* Событие превышения времени ожидания завершения приема пакета */
     /* Отключение таймера */
     xTimerStop( port_rs->xSoftWaitRxEnd, 0 );
     /* Аварийное завершение передачи */ 
     
     /*  Переключаемся на режим  */
     port_rs->fst_rs_rx = RS_RX_WAIT_RECEIVE;  
     /* Если режим RS485 отправка запроса передачи */
     if ( ( port_rs->rs_type == CON_RS485_M ) || ( port_rs->rs_type == CON_RS485_S ) )
     {
       /* Обновление таймера   - ответ не ранее чем через 400 мкс           */
       update_hard_timer( port_rs->base_rs_timer, port_rs->box_rx_time_box_tx );
     }
    }  
    /*=========================================================================*/
    /*=========================================================================*/ 
    if( ( ( port_rs->NotifiedValue ) & TX_TIME_ERROR ) != 0 )
    {/* Событие превышения времени ожидания завершения передачи пакета */
     /* Отключение таймера */
     xTimerStop( port_rs->xSoftWaitTxEnd, 0 );
     /* Аварийное завершение передачи */ 
     /* Отключаем прерывание по передаче    */
     USART_ITConfig( port_rs->base_rs_port, USART_IT_TXE , DISABLE);
     /* Переключаемся на режим              */ 
     port_rs->fst_rs_tx = RS_TX_WAIT_SEND_COMPLT;  
     /* Обновление таймера - ждем пока завершится передача 10 бод ******/
     update_hard_timer( port_rs->base_rs_timer, 10 );
    }  
    /*=========================================================================*/
    /*=========================================================================*/ 
  }    
}

/**
  * @brief Функция смены полярности приемника.
  * @param RS_struct_t* rs_port_box указатель на структуру
  * @retval none 
  */
void Reversal_Polar_RX( RS_struct_t* rs_port_box )
{ 
  /* Смена полярности подключения по RS */ 
  if (rs_port_box->flag_polar == POLAR_LO)
  {
    rs_port_box->flag_polar = POLAR_HI;
    if ( rs_port_box->POL_RX_PORTx != NULL ) set_pin( rs_port_box->POL_RX_PORTx, rs_port_box->POL_RX_Pin );
    if ( ( rs_port_box->rs_type == CON_RS485_M ) || ( rs_port_box->rs_type == CON_RS485_S ) )
    {
      if ( rs_port_box->POL_TX_PORTx != NULL ) set_pin( rs_port_box->POL_TX_PORTx, rs_port_box->POL_TX_Pin );
    }
  }
  else
  {
    rs_port_box->flag_polar = POLAR_LO;
    if ( rs_port_box->POL_RX_PORTx != NULL ) reset_pin( rs_port_box->POL_RX_PORTx, rs_port_box->POL_RX_Pin );
    if ( ( rs_port_box->rs_type == CON_RS485_M ) || ( rs_port_box->rs_type == CON_RS485_S ) )
    {
      if ( rs_port_box->POL_TX_PORTx != NULL ) reset_pin( rs_port_box->POL_TX_PORTx, rs_port_box->POL_TX_Pin );
    }   
  } 
}

#endif
/******************* (C) COPYRIGHT 2015 DataExpress *****END OF FILE****/
