/**
  ******************************************************************************
  * @file    handler_rs.c
  * @author  Trembach Dmitry
  * @version V1.0.0
  * @date    22-10-2020
  * @brief   Файл функций rs вызываемых по прерыванию и по таймеру
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
#include "handler_rs.h"
#include "loader_settings.h"    
#include "GenerationCRC16.h"

#if RS_GATE_ENABLE == 1 
/*============================================================================*/
/*======================== Прием данных с порта RS ===========================*/
/*============================================================================*/
/**
  * @brief Функция начало приема по RS - принимаем первый байт.
  * @param RS_struct_t* rs_port_box указатель на структуру
  * @retval none
  */ 
void start_receiver_byte( RS_struct_t* rs_port_box )
{
/* Сброс счетчика принятых данных   	    */
  rs_port_box->cnt_data_rx = 0;
  /* Прием 1 байта                                */
  rs_port_box->damp_data_rx[rs_port_box->cnt_data_rx] = (uint8_t)USART_ReceiveData(rs_port_box->base_rs_port);						
  /* Инкрементирование счетчика принятых данных   */
  (rs_port_box->cnt_data_rx)++;        
} 

/**
  * @brief Функция инлайн приема по RS одного байта.
  * @param RS_struct_t* rs_port_box указатель на структуру
  * @retval none
  */ 
void receiver_byte( RS_struct_t* rs_port_box )
{
  /* Прием 1 байта                                */
  rs_port_box->damp_data_rx[rs_port_box->cnt_data_rx] = (uint8_t)USART_ReceiveData(rs_port_box->base_rs_port);						
  /* Инкрементирование счетчика принятых данных   */
  (rs_port_box->cnt_data_rx)++;        
  /* Ограничение на переполнение приемного буфера */
  if (rs_port_box->cnt_data_rx >= sizeof(router_box_t))
  {									
    /* Обнуление счетчика принятыx пакетов        */								
    rs_port_box->cnt_data_rx = 0;
  }   
} 

/**
  * @brief Функция формирования данных статистики.
  * @param router_box_t *rs_box указатель на тестируемый пакет
  * @param RS_struct_t* rs_port_box указатель на структуру
  * @retval bool - true отправить пакет в мультиплексор 
  *                false отбросить
  */ 
bool calc_statistic_box(router_box_t *rs_box, RS_struct_t* rs_port_box )
{
  /* Фильтр для служебных пакетов снифера */
  if ( (rs_box->damp_router_data_box[2] == 0x79) && (rs_box->damp_router_data_box[4] == 0x41) && (rs_box->damp_router_data_box[5] == 0x88) && (rs_box->damp_router_data_box[7] == 0xCD) && (rs_box->damp_router_data_box[8] == 0xAB ) )
  {
    /* Подсчет битой информации (3 байта минимальный интервал между пакетами) + (2 байта преамбула)   */
    rs_port_box->cnt_byte_rx_little = rs_port_box->cnt_byte_rx_little + (uint32_t)(rs_box->damp_router_data_box[2]) + 5;  
    /* Проверка флага получения корректного пакета */
    if (rs_port_box->flag_rx_good_box)
    {
      /* Сброс таймера смены полярности */
      xTimerResetFromISR( rs_port_box->xSoftPolarTimer, NULL );
      /* Установка статуса полярности */
      rs_port_box->status_polar = 1;
      /* Установка статуса - есть подключение */
      rs_port_box->status_port = CON_ON;  
    }  
  }
  else
  {   
  // сохраняем значение последовательности
  rs_port_box->cnt_rx_box = rs_box->cnt;   
  // определяем статус принятого пакет
  rs_port_box->status_rs_box = test_receive_box(rs_box);
  
  // Установка флага принятого пакета (для индикации)  
  switch (rs_port_box->status_rs_box)
  {
  case 0: // - отправляем в мультиплексор
  case 1: // - локальный пакет
  case 2: // - маркерный пакет
    // Принят корректный пакет - установим флаг 
    rs_port_box->flag_rx_good_box = true;
    /* Сброс таймера переключения полярности */
    xTimerResetFromISR(rs_port_box->xSoftPolarTimer, NULL);
    /* Запускаем таймер индикации */
    rs_port_box->cnt_led_rx = 10;    
    /* Обновление адреса к которому подключен */
    rs_port_box->phy_addr_near = rs_port_box->data_rx.src;
    break;     
  case 3: // - ошибка CRC
    // Сброс флага первой проверки последовательности 
    rs_port_box->flag_start_cnt_rx_box = 0;    
  default:    
    break;	
  }   

  /* Проверка флага получения корректного пакета */
  if (rs_port_box->flag_rx_good_box)
  {
    /* Сброс таймера смены полярности */
    xTimerResetFromISR( rs_port_box->xSoftPolarTimer, NULL );
    /* Установка статуса полярности */
    rs_port_box->status_polar = 1;
      /* Установка статуса - есть подключение */
      rs_port_box->status_port = CON_ON;      
  }  
    
  // Тестирование принятого пакета  
  switch (rs_port_box->status_rs_box)
  {
  case 0: // - отправляем в мультиплексор
    // Подсчет всех полученных пакетов  
    (rs_port_box->cnt_box_rx)++;
    // Подсчет принятой информации   (3 байта минимальный интервал между пакетами)  
    rs_port_box->cnt_byte_rx_little = rs_port_box->cnt_byte_rx_little + rs_box->lenght + SIZE_LENGHT + SIZE_PRE + 3; 
    return true;	
  case 1: // - локальный пакет
    // Счетчик локальных пакетов пакетов
    break; 
  case 2: // - маркерный пакет 
    // Счетчик маркерных пакетов пакетов 
    break;     
  case 3: // - ошибка CRC
  default:    
    // Битый пакет считаем как нагрузку
    (rs_port_box->cnt_box_rx)++;   
    // Подсчет битой информации   (3 байта минимальный интервал между пакетами)  
    rs_port_box->cnt_byte_rx_little = rs_port_box->cnt_byte_rx_little + rs_box->lenght + SIZE_LENGHT + SIZE_PRE + 3; 
    // Счётчик ошибок CRC по приёму 
    (rs_port_box->cnt_err_crc)++;
    break;	
  }  
  }
  return false;  
} 

/**
  * @brief Функция обработки принятого пакета.
  * @param RS_struct_t* rs_port_box указатель на структуру
  * @retval none
  */ 
void processing_receiver_box( RS_struct_t* rs_port_box )
{
  /* Проверяем принятый пакет и формируем статистические данные   */
  if (calc_statistic_box(&(rs_port_box->data_rx),rs_port_box))
  {
    /* Пакет корректный отправляем его в маршрутизатор  */
    /* Проверка наличия очереди                         */
    if ( rs_port_box->set_port_router.QueueInRouter != NULL )
    {
      /* Отправляем пакет */
      xQueueSendFromISR( rs_port_box->set_port_router.QueueInRouter, &(rs_port_box->data_rx), NULL );
      /* Если группа событий активна - отправляем флаг заполнения буфера */
      if ( ( rs_port_box->set_port_router.HandleTaskRouter ) != NULL )
      {
        xTaskNotifyFromISR( rs_port_box->set_port_router.HandleTaskRouter,                           /* Указатель на уведомлюемую задачу                         */
                           (rs_port_box->set_port_router.PortBitID)<<OFFSET_NOTE_PORD_ID,         /* Значения уведомления                                     */
                           eSetBits,                                                                 /* Текущее уведомление добавляются к уже прописанному       */
                           NULL );                                                                   /* Внеочередного переключения планировщика не запрашиваем   */
      }
#if UDP_LOGGER == 1 
      /* Если есть указатель на флаг лога  */
      if ( rs_port_box->flag_log_rx != NULL)
      {
        /* Передать пакет в логгер если есть запрос лога */
        if (*(rs_port_box->flag_log_rx))
        {
          /* Не логируем маркеры */
          if ( rs_port_box->data_rx.id != 0x00 )
          {
          /* Проверяем очередь */
          if (xQueueLogger != NULL)
          {
            /* Заполнение данных */
              rs_port_box->Logger_rx_data_size = (uint16_t)(rs_port_box->data_rx.lenght) + 8;
            rs_port_box->Logger_rx_type_code = rs_port_box->code_log_rx;  
            rs_port_box->Logger_rx_time_cod = ulGetRuntimeCounterValue();
            /* Отправка в очередь */      
            xQueueSendFromISR(xQueueLogger,(void*)&(rs_port_box->damp_log_rx_data), 0); 
            
            if ( LoggerHandleTask != NULL )
            {/* Устанавливаем событие срабатывания програмного таймера для периодического уведомления */
              xTaskNotifyFromISR( LoggerHandleTask,              /* Указатель на уведомлюемую задачу                         */
                                 RECEIVING_NOTE,                 /* Значения уведомления                                     */
                                 eSetBits,                       /* Текущее уведомление добавляются к уже прописанному       */
                                 NULL );                         /* Флаг внеочередного переключения планировщика             */    
            }       
          }
        }
      }
      }
#endif
    }
  }
} 

/**
  * @brief  функция обработки прерывания приемника - принят байт.
  * @param  RS_struct_t* PortRS указатель на структуру порта
  * @retval None
  */
void RS_RXNE_Handler(RS_struct_t* PortRS)
{
  if (USART_GetITStatus( PortRS->base_rs_port, USART_IT_IDLE ) != RESET)
  {
    /* Если прием завершен принимем последний байт */
    /* Прием байта            */
    receiver_byte( PortRS ); 
    /* Анализ автомата состояний приема UART */
    switch (PortRS->fst_rs_rx)
    { 
    case RS_RX_RECEIVE:         /* прием           */
      /* Прием окончен - обработка пакета          */
      processing_receiver_box( PortRS );
    case RS_RX_WAIT_RECEIVE:    /* ожидание приема */     
    case RS_RX_NONE_STATE:      /* выключено       */
    default:
      /*  Переключаемся на режим  */
      PortRS->fst_rs_rx = RS_RX_WAIT_RECEIVE;  
      /* Если режим RS485 отправка запроса передачи */
      if ( ( PortRS->rs_type == CON_RS485_M ) || ( PortRS->rs_type == CON_RS485_S ) )
      {
        /* Обновление таймера   - ответ не ранее чем через 400 мкс           */
        update_hard_timer( PortRS->base_rs_timer, PortRS->box_rx_time_box_tx );
      }
      break;	
    }  
    /* Отключаем програмный таймер контроля приема пакета */
    xTimerStopFromISR( PortRS->xSoftWaitRxEnd, NULL );
  }
  else
  {
  /* Анализ автомата состояний приема UART */
  switch (PortRS->fst_rs_rx)
  {
  case RS_RX_WAIT_RECEIVE:    /* ожидание приема */ 
    /* Начало приема данных    */
    start_receiver_byte( PortRS );
    /* Переключаемся на режим  */
    PortRS->fst_rs_rx = RS_RX_RECEIVE;          
      /* Включаем програмный таймер контроля приема пакета */
      xTimerResetFromISR( PortRS->xSoftWaitRxEnd, NULL );
      
    break;						
    
  case RS_RX_RECEIVE:         /* прием          */	
    /* Прием байта            */
    receiver_byte( PortRS ); 
    break;
  case RS_RX_NONE_STATE:      /* выключено       */
  default:
    /* Начало приема данных   */
    start_receiver_byte( PortRS );
    /* Переключаемся на режим */
    PortRS->fst_rs_rx = RS_RX_RECEIVE;      
      /* Включаем програмный таймер контроля приема пакета */
      xTimerResetFromISR( PortRS->xSoftWaitRxEnd, NULL );
    break;	
  }  
}
}

/**
  * @brief  функция обработки прерывания приемника - прием пакета завершен.
  * @param  RS_struct_t* PortRS указатель на структуру порта
  * @retval None
  */
void RS_IDLE_Handler(RS_struct_t* PortRS)
{
  /* принимаем байт но не обрабатываем */
  USART_ReceiveData( PortRS->base_rs_port );
  
  /* Анализ автомата состояний приема UART */
  switch (PortRS->fst_rs_rx)
  { 
  case RS_RX_RECEIVE:         /* прием           */
    /* Прием окончен - обработка пакета          */
    processing_receiver_box( PortRS );
  case RS_RX_WAIT_RECEIVE:    /* ожидание приема */     
  case RS_RX_NONE_STATE:      /* выключено       */
  default:
    /*  Переключаемся на режим  */
    PortRS->fst_rs_rx = RS_RX_WAIT_RECEIVE;  
    /* Если режим RS485 отправка запроса передачи */
    if ( ( PortRS->rs_type == CON_RS485_M ) || ( PortRS->rs_type == CON_RS485_S ) )
    {
      /* Обновление таймера   - ответ не ранее чем через 400 мкс           */
      update_hard_timer( PortRS->base_rs_timer, PortRS->box_rx_time_box_tx );
    }
    break;	
  }  
  /* Отключаем програмный таймер контроля приема пакета */
  xTimerStopFromISR( PortRS->xSoftWaitRxEnd, NULL );
}

/*============================================================================*/
/*======================== Передача данных в порт RS =========================*/
/*============================================================================*/

/**
  * @brief Функция запуска передачи пакета.
  * @param RS_struct_t* rs_port_box указатель на структуру
  * @retval uint8_t 0 - передачи не было
  *                 1 - передан пакет данных
  *                 2 - передан маркерный пакет
  */ 
uint8_t start_send_box( RS_struct_t* rs_port_box )
{
  /* проверяем наличие очереди */              
  if (rs_port_box->set_port_router.QueueOutRouter != NULL) 
  {  
    /* Есть в очереди пакет */
    if ( ( uxQueueMessagesWaitingFromISR( rs_port_box->set_port_router.QueueOutRouter ) ) > 0 )
    {
      /* Подготовка к отправке данных                    */
      /* Проверяем наличие входящих пакетов              */
      if(xQueueReceiveFromISR(rs_port_box->set_port_router.QueueOutRouter , &(rs_port_box->damp_data_tx) , NULL ) == pdTRUE)
      {
        /* Сброс запроса маркера */
        rs_port_box->flag_marker = false;
        /* Есть данные для передачи                      */
        /* Проверит размер пакета                        */
        if ((rs_port_box->len_data_tx + SIZE_LENGHT + SIZE_PRE) >= sizeof(router_box_t))
        {
          /* Выходит за границу - корректируем           */
          rs_port_box->len_data_tx = sizeof(router_box_t) - SIZE_LENGHT - SIZE_PRE;
        }
        /* Устанавливаем свой физический адрес источника */
        rs_port_box->data_tx.src = DataLoaderSto.Settings.phy_adr;   
        /* Обновляем счетчик неприрывности */
        rs_port_box->data_tx.cnt = (rs_port_box->cnt_tx_box)++;  
        /* Вычисление контрольной суммы */
        CalcCRC16FromISR((uint8_t*)(&(rs_port_box->data_tx.lenght)),
                         rs_port_box->data_tx.lenght,
                         &(rs_port_box->data_tx.data[(rs_port_box->data_tx.lenght) + SIZE_LENGHT + SIZE_CRC - sizeof(marker_box_t)]),
                         &(rs_port_box->index_calc_crc));
        /* Cкорректировать длинну с учетом приамбулы и длинны пакета */
        rs_port_box->len_data_tx = rs_port_box->data_tx.lenght + SIZE_LENGHT + SIZE_CRC;	
        /* обнуление счетчика переданных байт */
        rs_port_box->cnt_data_tx = 0;         
        
        /* Подсчет переданных пакетов с данными          */
        (rs_port_box->cnt_box_tx)++;
        /* Подсчет переданной информации   (3 байта минимальный интервал между пакетами) */ 
        rs_port_box->cnt_byte_tx_little = rs_port_box->cnt_byte_tx_little + rs_port_box->len_data_tx + SIZE_LENGHT + SIZE_PRE + 3; 
        /* передан пакет данных                          */
#if UDP_LOGGER == 1         
        /* Если есть указатель на флаг лога  */
        if ( rs_port_box->flag_log_tx != NULL)
        {
          /* Передать пакет в логгер если есть запрос лога */
          if (*(rs_port_box->flag_log_tx))
          {
            /* Не логируем маркеры */
            if ( rs_port_box->data_tx.id != 0x00 )
            {            
            /* Проверяем очередь */
            if (xQueueLogger != NULL)
            {
              /* Заполнение данных */
                rs_port_box->Logger_tx_data_size = (uint16_t)(rs_port_box->data_tx.lenght) + 8;
              rs_port_box->Logger_tx_type_code = rs_port_box->code_log_tx;  
              rs_port_box->Logger_tx_time_cod = ulGetRuntimeCounterValue();
              /* Отправка в очередь */      
              xQueueSendFromISR(xQueueLogger,(void*)&(rs_port_box->damp_log_tx_data), 0); 
              
              if ( LoggerHandleTask != NULL )
              {/* Устанавливаем событие срабатывания програмного таймера для периодического уведомления */
                xTaskNotifyFromISR( LoggerHandleTask,              /* Указатель на уведомлюемую задачу                         */
                                   RECEIVING_NOTE,                 /* Значения уведомления                                     */
                                   eSetBits,                       /* Текущее уведомление добавляются к уже прописанному       */
                                   NULL );                         /* Флаг внеочередного переключения планировщика             */    
              }       
            }
          }
        }
        }
#endif
        return 1;
      }
    }
  }  
  /* Если режим 485 */ /* Если установлен флаг маркера */
  if ( ( ( rs_port_box->rs_type == CON_RS485_M ) || ( rs_port_box->rs_type == CON_RS485_S ) || ( rs_port_box->flag_marker == true ) ) && ( rs_port_box->marker_mode == MARKER_ENA_TRANS ) )
  {
    /* Сброс запроса маркера */
    rs_port_box->flag_marker = false;
    /* Очередь пуста или неоткрыта - формируем маркер */
    /* Формирование маркерного пакета */
    rs_port_box->data_tx.pre = 0xAA55;                         /* Преамбула  0x55 0xAA                           */
    rs_port_box->data_tx.lenght = 0x000A;                      /* Длина маркерного пакета                        */
    rs_port_box->data_tx.id = 0x00;                            /* Маркерный пакет                                */
    rs_port_box->data_tx.dest = 0x0000;                        /* Локальный адрес получателя                     */
    rs_port_box->data_tx.src = DataLoaderSto.Settings.phy_adr; /* Устанавливаем свой физический адрес источника  */  
    /* Обновляем счетчик неприрывности */
    rs_port_box->data_tx.cnt = (rs_port_box->cnt_tx_box)++;
    /* Вычисление контрольной суммы */
    CalcCRC16FromISR((uint8_t*)(&(rs_port_box->data_tx.lenght)),
                     rs_port_box->data_tx.lenght,
                     &(rs_port_box->data_tx.data[(rs_port_box->data_tx.lenght) + SIZE_LENGHT + SIZE_CRC - sizeof(marker_box_t)]),
                     &(rs_port_box->index_calc_crc));
    /* Cкорректировать длинну с учетом приамбулы и длинны пакета */
    rs_port_box->len_data_tx = rs_port_box->data_tx.lenght + SIZE_LENGHT + SIZE_CRC;	
    /* обнуление счетчика переданных байт */
    rs_port_box->cnt_data_tx = 0;    
    /* Подсчет переданных маркерных пакетов */    
    /* передан маркерный пакет */
#if UDP_LOGGER == 1     
    /* Если есть указатель на флаг лога  */
    if ( rs_port_box->flag_log_tx != NULL)
    {
      /* Передать пакет в логгер если есть запрос лога */
      if (*(rs_port_box->flag_log_tx))
      {
        /* Не логируем маркеры */
        if ( rs_port_box->data_tx.id != 0x00 )
        
        /* Проверяем очередь */
        if (xQueueLogger != NULL)
        {
          /* Заполнение данных */
          rs_port_box->Logger_tx_data_size = (uint16_t)(rs_port_box->data_tx.lenght) + 8;
          rs_port_box->Logger_tx_type_code = rs_port_box->code_log_tx;  
          rs_port_box->Logger_tx_time_cod = ulGetRuntimeCounterValue();
          /* Отправка в очередь */      
          xQueueSendFromISR(xQueueLogger,(void*)&(rs_port_box->damp_log_tx_data), 0); 
          
          if ( LoggerHandleTask != NULL )
          {/* Устанавливаем событие срабатывания програмного таймера для периодического уведомления */
            xTaskNotifyFromISR( LoggerHandleTask,              /* Указатель на уведомлюемую задачу                         */
                               RECEIVING_NOTE,                 /* Значения уведомления                                     */
                               eSetBits,                       /* Текущее уведомление добавляются к уже прописанному       */
                               NULL );                         /* Флаг внеочередного переключения планировщика             */    
          }       
        }
      }
    }    
#endif    
    return 2; 
  }
  else
  {
    /* Сброс запроса маркера */
    rs_port_box->flag_marker = false;
  }  
  
  /* отправки нет */
  return 0;
} 

/**
  * @brief  функция обработки прерывания таймера RS_UART.
  * @param  RS_struct_t* PortRS указатель на структуру порта
  * @retval None
  */
void RS_TIM_Handler(RS_struct_t* PortRS)
{
  /* Остановка таймера                          */
  stop_hard_timer( PortRS->base_rs_timer ); 

  /* Анализ автомата состояний передачи UART    */
  switch ( PortRS->fst_rs_tx )
  {
  case RS_TX_BLOK_SEND:             /* Блокировка передачи */  
    /* Переключаемся на режим                   */ 
    PortRS->fst_rs_tx = RS_TX_WAIT_SEND;    
   
  case RS_TX_WAIT_SEND:             /* Ожидание передачи   */
    /* Если режим передачи разрешен */
    if ( ( ( ( PortRS->rs_type == CON_RS485_M ) || ( PortRS->rs_type == CON_RS485_S ) ) && ( PortRS->fst_rs_rx == RS_RX_WAIT_RECEIVE ) ) || ( PortRS->rs_type == CON_RS422 ) )
    {
      /* Функция запуска передачи пакета.       */
      if  ( start_send_box( PortRS ) == 0 )
      {
        /* 0 - передачи не было                 */
        /* Выключаем прерывание по передаче     */
        USART_ITConfig( PortRS->base_rs_port, USART_IT_TXE , DISABLE );	 
        /* Запуск таймаута на ожидание ответа   */   
        /* Обновление таймера контроля максимальной паузы между передаваемыми пакетами *****/
        update_hard_timer( PortRS->base_rs_timer, 20 * sizeof(router_box_t) );
      }
      else
      {
        /* 1 - передача пакет данных            */
        /* 2 - передача маркерный пакет         */
        /* Переключаемся на режим передачи      */        
        PortRS->fst_rs_tx = RS_TX_SEND; 
        /* Управление PHY RS485                 */
        if ( ( PortRS->rs_type == CON_RS485_M ) || ( PortRS->rs_type == CON_RS485_S )  )
        {
          /* Если вывод управления указан */
          if ( ( PortRS->DE_RE_PORTx ) != NULL ) 
          {
            if ( PortRS->DE_RE_Inversion_Flag == false )
        {
          set_pin( PortRS->DE_RE_PORTx, PortRS->DE_RE_Pin ); 
            }
            else
            {
              reset_pin( PortRS->DE_RE_PORTx, PortRS->DE_RE_Pin );
            }
          }          
          
          /* Если вывод управления указан */
          if ( ( PortRS->RE_PORTx ) != NULL ) 
          {
            if ( PortRS->RE_Inversion_Flag == false )
            {
              reset_pin( PortRS->RE_PORTx, PortRS->RE_Pin );
          }
            else
            {
              set_pin( PortRS->RE_PORTx, PortRS->RE_Pin );
            }
          }
        }
        /* Запускаем таймер индикации */
        PortRS->cnt_led_tx = 2;        
        /* Включаем прерывание по передаче      */
        USART_ITConfig( PortRS->base_rs_port, USART_IT_TXE , ENABLE);	  
        /* Обновление таймера - время передачи не должно превысить две длинны максимально пакета *****/
        update_hard_timer( PortRS->base_rs_timer, 20 * sizeof(router_box_t) );
        /* Включаем програмный таймер контроля передачи пакета */
        xTimerResetFromISR( PortRS->xSoftWaitTxEnd, NULL );
      }  
    }
    else
    {  
      /* Выключаем прерывание по передаче      */
      USART_ITConfig( PortRS->base_rs_port, USART_IT_TXE , DISABLE );	
      /* Обновление таймера                          *****/
      update_hard_timer( PortRS->base_rs_timer, 10 * sizeof(router_box_t) );
    }
    break;	

  case RS_TX_WAIT_SEND_COMPLT:      /* Ожидание завершения передачи */
    /* Управление PHY RS485                 */
    if ( ( PortRS->rs_type == CON_RS485_M ) || ( PortRS->rs_type == CON_RS485_S ) )
    {
        /* Переключение PHY RS485 в режим приема */
      /* Если вывод управления указан */
      if ( ( PortRS->DE_RE_PORTx ) != NULL )
        {
        if ( PortRS->DE_RE_Inversion_Flag == false )
          {
        reset_pin( PortRS->DE_RE_PORTx, PortRS->DE_RE_Pin );       
        }
          else
          {
          set_pin( PortRS->DE_RE_PORTx, PortRS->DE_RE_Pin );
        }
      }          
            
      /* Если вывод управления указан */
        if ( ( PortRS->RE_PORTx ) != NULL ) 
        {
        if ( PortRS->RE_Inversion_Flag == false )
          {
          set_pin( PortRS->RE_PORTx, PortRS->RE_Pin );
        }        
          else
          {
          reset_pin( PortRS->RE_PORTx, PortRS->RE_Pin );
        }        
      }
      
      if ( PortRS->rs_type == CON_RS485_S )
      {
        /* Остановка таймера                                                      */
        stop_hard_timer( PortRS->base_rs_timer );
      }
      else
      {
      /* Обновление таймера                       ******/
      update_hard_timer( PortRS->base_rs_timer, PortRS->box_tx_time_no_box_rx ); 
    }
    }
    else
    {
      /* Обновление таймера                       ******/
      update_hard_timer( PortRS->base_rs_timer, PortRS->box_tx_min_time_box_tx );    
    }  
    
    /* Переключаемся на режим                   */ 
    PortRS->fst_rs_tx = RS_TX_BLOK_SEND;    

    /* Включаем програмный таймер контроля передачи пакета */
    xTimerStopFromISR( PortRS->xSoftWaitTxEnd, NULL );
    break;				
	        
  case RS_TX_NONE_STATE:            /* Передача отключена  */
  case RS_TX_SEND:                  /* Передача            */
  default:
    /* Отключаем прерывание по передаче    */
    USART_ITConfig( PortRS->base_rs_port, USART_IT_TXE , DISABLE);
    /* Переключаемся на режим              */ 
    PortRS->fst_rs_tx = RS_TX_WAIT_SEND_COMPLT;  
    /* Обновление таймера - ждем пока завершится передача 10 бод ******/
    update_hard_timer( PortRS->base_rs_timer, 10 );
    break;	
  }
}

/**
  * @brief  функция обработки прерывания передатчика - буфер передатчика пуст.
  * @param  RS_struct_t* PortRS указатель на структуру порта
  * @retval None
  */
void RS_TXE_Handler(RS_struct_t* PortRS)
{
  /* Анализ автомата состояний передачи UART */
  switch (PortRS->fst_rs_tx)
  {
  case RS_TX_SEND:                  /* Передача            */
    /* Проверяем есть ли данные для передачи */   
    if (PortRS->cnt_data_tx < PortRS->len_data_tx)
    {
      /* Передача сообщения                  */
      USART_SendData( PortRS->base_rs_port,PortRS->damp_data_tx[PortRS->cnt_data_tx]);
      /* Инкремент счетчика передачи         */
      PortRS->cnt_data_tx++;
    }
    else
    {
      /* Отключаем прерывание по передаче    */
      USART_ITConfig( PortRS->base_rs_port, USART_IT_TXE , DISABLE);
      /* Переключаемся на режим              */ 
      PortRS->fst_rs_tx = RS_TX_WAIT_SEND_COMPLT;  
      /* Обновление таймера - ждем пока завершится передача 10 бод ******/
      update_hard_timer( PortRS->base_rs_timer, 10 );
    }	
    break;	
    
  case RS_TX_NONE_STATE:            /* Передача отключена  */
  case RS_TX_WAIT_SEND:             /* Ожидание передачи   */
    /* Если режим передачи разрешен */
    if ( ( ( ( PortRS->rs_type == CON_RS485_M ) || ( PortRS->rs_type == CON_RS485_S ) ) && ( PortRS->fst_rs_rx == RS_RX_WAIT_RECEIVE ) ) || ( PortRS->rs_type == CON_RS422 ) )
    {
      /* Функция запуска передачи пакета.       */
      if  ( start_send_box( PortRS ) == 0 )
      {
        /* 0 - передачи не было                 */
        /* Выключаем прерывание по передаче     */
        USART_ITConfig( PortRS->base_rs_port, USART_IT_TXE , DISABLE );	 
        /* Запуск таймаута на ожидание ответа   */   
       
        /* Обновление таймера контроля максимальной паузы между передаваемыми пакетами *****/
        update_hard_timer( PortRS->base_rs_timer, 20 * sizeof(router_box_t) );
      }
      else
      {
        /* 1 - передача пакет данных            */
        /* 2 - передача маркерный пакет         */
        /* Переключаемся на режим               */ 
        PortRS->fst_rs_tx = RS_TX_SEND;   
          /* Переключение PHY RS485 в режим передачи */
        if ( ( PortRS->rs_type == CON_RS485_M ) || ( PortRS->rs_type == CON_RS485_S )  )
        {
          /* Если вывод управления передачи указан */
          if ( ( PortRS->DE_RE_PORTx ) != NULL ) 
          {
            if ( PortRS->DE_RE_Inversion_Flag == false )
        {
          set_pin( PortRS->DE_RE_PORTx, PortRS->DE_RE_Pin ); 
            }
            else
            {
              reset_pin( PortRS->DE_RE_PORTx, PortRS->DE_RE_Pin );
            }
          }          
          
          /* Если вывод управления приема указан */
          if ( ( PortRS->RE_PORTx ) != NULL ) 
          {
            if ( PortRS->RE_Inversion_Flag == false )
            {
              reset_pin( PortRS->RE_PORTx, PortRS->RE_Pin );
          }
            else
            {
              set_pin( PortRS->RE_PORTx, PortRS->RE_Pin );
            }
          }
        }
        /* Запускаем таймер индикации */
        PortRS->cnt_led_tx = 2;
        /* Включаем прерывание по передаче      */
        USART_ITConfig( PortRS->base_rs_port, USART_IT_TXE , ENABLE);	  
        /* Обновление таймера - время передачи не должно превысить две длинны максимально пакета *****/
        update_hard_timer( PortRS->base_rs_timer, 20 * sizeof(router_box_t) );
        /* Включаем програмный таймер контроля передачи пакета */
        xTimerResetFromISR( PortRS->xSoftWaitTxEnd, NULL );
      }  
    }
    else
    {  
      /* Выключаем прерывание по передаче      */
      USART_ITConfig( PortRS->base_rs_port, USART_IT_TXE , DISABLE );	
      /* Обновление таймера                          *****/
      update_hard_timer( PortRS->base_rs_timer, 10 * sizeof(router_box_t) );
    }
    break;	
  case RS_TX_WAIT_SEND_COMPLT:      /* Ожидание завершения передачи */
  case RS_TX_BLOK_SEND:             /* Блокировка передачи */    
  default:
    /* Отключаем прерывание по передаче    */
    USART_ITConfig( PortRS->base_rs_port, USART_IT_TXE , DISABLE);
    /* Переключаемся на режим              */ 
    PortRS->fst_rs_tx = RS_TX_WAIT_SEND_COMPLT;  
    /* Обновление таймера - ждем пока завершится передача 10 бод ******/
    update_hard_timer( PortRS->base_rs_timer, 10 );
    break;	
  }   
}

/*============================================================================*/
/*======================== Программный таймер порт RS ========================*/
/*============================================================================*/
/*
  * @brief  функция обработки програмного таймера.
  * @param  RS_struct_t* PortRS указатель на структуру порта
  * @retval None
  */
void RS_SOFT_TIM_Handler(RS_struct_t* PortRS , TimerHandle_t pxTimer )
{
  if ( ( PortRS->set_port_router.HandleTask ) != NULL  )
  {
    if ( PortRS->xSoftTimer == pxTimer )
    {/* Устанавливаем событие срабатывания програмного таймера для периодического уведомления */
      xTaskNotify( PortRS->set_port_router.HandleTask,       /* Указатель на уведомлюемую задачу                         */
                  TIMER_NOTE,                                /* Значения уведомления                                     */
                  eSetBits );                                /* Текущее уведомление добавляются к уже прописанному       */
    }
    if ( PortRS->xSoftPolarTimer == pxTimer )
    {/* Устанавливаем событие срабатывания програмного таймера для периодического уведомления */
      xTaskNotify( PortRS->set_port_router.HandleTask,       /* Указатель на уведомлюемую задачу                         */
                  RS_POLAR_NOTE,                             /* Значения уведомления                                     */
                  eSetBits );                                /* Текущее уведомление добавляются к уже прописанному       */
    }    
    if ( PortRS->xSoftWaitTX == pxTimer )
    {/* Устанавливаем событие срабатывания програмного таймера для периодического уведомления */
      xTaskNotify( PortRS->set_port_router.HandleTask,       /* Указатель на уведомлюемую задачу                         */
                  RS_WAIT_TX_NOTE,                           /* Значения уведомления                                     */
                  eSetBits );                                /* Текущее уведомление добавляются к уже прописанному       */
    }       
    if ( PortRS->xSoftTimerDiag == pxTimer )
    {/* Устанавливаем событие срабатывания програмного таймера для периодического уведомления */
      xTaskNotify( PortRS->set_port_router.HandleTask,       /* Указатель на уведомлюемую задачу                         */
                  REQ_DIAG_NOTE,                             /* Значения уведомления                                     */
                  eSetBits );                                /* Текущее уведомление добавляются к уже прописанному       */
    }   
    if ( PortRS->xSoftWaitRxEnd == pxTimer )
    {/* Устанавливаем событие превышения времени ожидания завершения приема пакета */
      xTaskNotify( PortRS->set_port_router.HandleTask,       /* Указатель на уведомлюемую задачу                         */
                  RX_TIME_ERROR,                             /* Значения уведомления                                     */
                  eSetBits );                                /* Текущее уведомление добавляются к уже прописанному       */
    }       
    if ( PortRS->xSoftWaitTxEnd == pxTimer )
    {/* Устанавливаем событие превышения времени ожидания завершения передачи пакета */
      xTaskNotify( PortRS->set_port_router.HandleTask,       /* Указатель на уведомлюемую задачу                         */
                  TX_TIME_ERROR,                             /* Значения уведомления                                     */
                  eSetBits );                                /* Текущее уведомление добавляются к уже прописанному       */
    }  
  }  
}
#endif
/******************* (C) COPYRIGHT 2020 DataExpress *****END OF FILE****/
