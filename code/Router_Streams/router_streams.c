/**
  ******************************************************************************
  * @file    router_streams.c
  * @author  Trembach D.N.
  * @version V1.2.0
  * @date    08-09-2020
  * @brief   Файл содержит функции програмного мультиплексора потоков
  ******************************************************************************
  * @attention
  *
  *     
  *    Дополнения: 08-09-2020 V1.2.0
  *       добавлен програмный таймер для задачи мультиплексора 
  *     
  *    Дополнения: 31-08-2020 V1.1.0
  *       отправка уведомлений порту, которому отправлен пакет 
  *     
  *
  *    Задача мультиплексирования имеет средний приоритет.
  *    Вызывается с точным периодом 1 млсек.
  *    
  *    Для работы мультиплексора резервируются следующие переменные:
  *    1.Счетчик пакетов входящих в мультиплексор (32 бит) используется как идентификатор пакета при отладке
  *    2.Собственный адрес
  *    3.Буфер для хранения принятого пакета, с дополнительными полями(идентификатор пакета, источник пакета, время приема)
  *    4.Переменные для организации циклов по входящим очередям и по исходящим очередям 8 бит.
  *    5.Таблица портов маршрутизации со следующими полями:
  *    		1.Флаг запрос указателя
  *    		2.ID записи в таблице маршрутизации
  *    		3.Статус
  *    		4.Собственный ID порт(тип порта от 0 до 31)
  *    		5.Статус обработки собственного физического адреса.
  *    		6.Номер маски ID портов
  *    		7.Номер маски ID пакетов
  *    		8.Указатель на входящюю очередь
  *    		9.Указатель на исходящюю очередь
  *     	   
  *    Дополнительно прописаны две функции:
  *    Все функции маршрутизации работают только с мьютексами
  *    Выделение указателя (обнуляются все поля структуры).
  *    Удаление указателя (обнуляются все поля структуры).
  * 
  * 
  *    При тестировании загрузка 10%
  *    Параметры тестирования 5 портов 4000 входящих/сек  7000 исходящих
  *    4 независимых задачи с разными приоритетами формируют пакеты 200 данных
  *    и отправляют с периодом 1 млсек на роутер - роутер маршрутизирует
  *    и отправляет 7 пакетов для 5 задач. Задачи принимают пакеты из очереди
  *    но не обрабатывают
  *    STM32F407 Частота CPU 168 МГЦ. (Потерь/пропусков нет)
  *    
  *    Проверить возможность динамического подключения - отключения порта!!!
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "router_streams.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "GenerationCRC16.h"
#include "Core.h"
#include "settings.h"
#include "loader_settings.h"
//#include <cstring>
#include "router_duplicate.h"
#include "Board.h"


SemaphoreHandle_t xRouterMutex;

//==== Переменные задачи маршрутизации ====
/* Указатель на задачу роутера */
TaskHandle_t  HandleTaskRouter = NULL;  
/* Переменная для приема сообщений */
static uint32_t NotifiedValue;
// Таблица портов маршрутизации
port_router_t tabl_router[MAX_ROUTER_PORT];
// Счетчик пакетов - идентификатор
static uint32_t cnt_indent = 0; 
// Буфер для хранения маршрутизируемого пакета
static router_buf_box_t  buf_box;
// Счетчик индекса входящих портов
static uint8_t  index_in_port;
// Счетчик индекса исходящих портов
static uint8_t  index_out_port;
// Флаг запроса проверки входящих сообщений
static bool  flag_read_in_port = true;
 /* указатели на программные таймера для роутера */ 
TimerHandle_t                 xRouterSoftTimer;                                    
TimerHandle_t                 xRouterSoftUptime;
#if  (LOG_STRIME_ENABLE == 1) 
// Если включен режим сохранения шапок маршрутизируемых пакетов
QueueHandle_t     QueueLogRouter;      // Указатель на очередь хранения шапок
log_head_box_t    wr_log_head_box;     // Буфер для формирования шапки для сохранения
log_head_box_t    rd_log_head_box;     // Буфер для приема шапки для сохранения
/**
  * @brief  Функция сохранения шапки входящего пакета роутера.
  * @param  router_buf_box_t* router_buf_box;      - указатель на структуру принятого пакета
  * @param  uint32_t  PortInBitID    - битовый ID порта откуда отправляем пакет
  * @retval none
  */
void log_in_router(router_buf_box_t* router_buf_box, uint32_t  PortInBitID)
{
  wr_log_head_box.cnt_id = router_buf_box->cnt_id;                 // Cчетчик - идентификатор пакет
  wr_log_head_box.time_entrance = router_buf_box->time_entrance;   // Время получения  
  wr_log_head_box.lenght = router_buf_box->router_box.lenght;      // Длина пакета (включая контрольную сумму, не включая преамбулу и длинну пакета)     
  wr_log_head_box.id = router_buf_box->router_box.id;              // Идентификатор пакета     
  wr_log_head_box.dest = router_buf_box->router_box.dest;          // Физический адрес получателя     
  wr_log_head_box.src = router_buf_box->router_box.src;            // Физический адрес источника   
  wr_log_head_box.cnt = router_buf_box->router_box.cnt;            // Cчетчик неприрывности пакетов 0..255  
  wr_log_head_box.PortInBitID = PortInBitID;                       // Битовый ID порта откуда отправляем пакет    
  // Отправка данных в очередь
  xQueueSend( QueueLogRouter, &wr_log_head_box, ( TickType_t ) 0 );
  wr_log_head_box.PortOutBitID = 0;//обнуляем флаги отправки пакета
}

/**
  * @brief  Функция сохранения номера порта в который отправлен пакет.
  * @param  uint32_t  PortOutBitID    - битовый ID порта куда отправляем пакет
  * @retval none
  */
void log_out_router(uint32_t  PortOutBitID)
{
   wr_log_head_box.PortOutBitID = wr_log_head_box.PortOutBitID | PortOutBitID;  
}

/**
  * @brief  Функция сохранения флага дублирование пакета.
  * @param  uint8_t flag_duplicate 1 - пакет дубликат
  *                                0 - пакет не дубликат
  * @retval none
  */
void log_duplicate_router(uint8_t flag_duplicate)
{
  if (flag_duplicate > 0)
  {
    wr_log_head_box.dublicate = 'D';   
  }
  else
  {
    wr_log_head_box.dublicate = '*';   
  }  
}

/**
  * @brief  Функция формирования текстовой строки для выдачи в UDP.
  * @param  char *pcInsert указатель на массив pcInsert для формируемой строки
  * @retval uint16_t          - размер выдаваемого сообщения
  */
uint16_t send_log_out_router_to_udp( char *pcInsert )
{
  uint8_t temp_cnt_bit;
  uint8_t temp_cnt_poz;
  
  char  temp_str_in_port[]  = "|-|-|-|-|-| |-|-|-|-|-| |-|-|-|-|-| |-|-|-|-|-|";
  char  temp_str_out_port[] = "|-|-|-|-|-| |-|-|-|-|-| |-|-|-|-|-| |-|-|-|-|-|";
  
  if ( xQueueReceive( QueueLogRouter, &rd_log_head_box, ( TickType_t ) 0 ) != pdTRUE ) 
  {
    // Данных в очереди нет - выход
    return 0;
  }
  else
  {
    temp_cnt_poz = 1;
    
    for (temp_cnt_bit = 0;temp_cnt_bit < MAX_NUMBER_PortID; temp_cnt_bit++)
    {
      /* формирование строки входящих портов  */
      if (((rd_log_head_box.PortInBitID)&(0x00000001 << temp_cnt_bit)) > 0)     
      {
        temp_str_in_port[temp_cnt_poz] = '+';
      }       
      else   
      {
        temp_str_in_port[temp_cnt_poz] = '-';
      }
      
      /* формирование строки исходящих портов */
      if (((rd_log_head_box.PortOutBitID)&(0x00000001 << temp_cnt_bit)) > 0) 
      {
        temp_str_out_port[temp_cnt_poz] = '+';
      }
      else
      {
        temp_str_out_port[temp_cnt_poz] = '-';
      }
      
      switch(temp_cnt_bit)
      {
      case 4:   
      case 9:      
      case 14:   
      case 19:         
      case 24:   
      case 29:         
         temp_cnt_poz = temp_cnt_poz + 2;         
        break;          
        
      default:
        break;
      }   
      /* Изменение позиции элемента */
      temp_cnt_poz = temp_cnt_poz + 2;
    }
    
    return sprintf(pcInsert, "| %.9d | %.9d %.47s %.3d | 0x%.2X | 0x%.4X | 0x%.4X | %.3d |%c| %.47s\n",
                   rd_log_head_box.cnt_id,         // Cчетчик - идентификатор пакет       
                   rd_log_head_box.time_entrance,  // Время получения
                   temp_str_in_port,               // Маска входящих    
                   rd_log_head_box.lenght,         // Длина пакета (включая контрольную сумму, не включая преамбулу и длинну пакета)         
                   rd_log_head_box.id,             // Идентификатор пакета
                   rd_log_head_box.dest,           // Физический адрес получателя                  
                   rd_log_head_box.src,            // Физический адрес источника 
                   rd_log_head_box.cnt,            // Физический адрес источника // Cчетчик неприрывности пакетов 0..255 
                   rd_log_head_box.dublicate,      // Флаг пакета - дубликата
                   temp_str_out_port);             // Маска входящих 
  }
}
#endif  /* LOG_STRIME_ENABLE */  

/**
  * @brief  Функция проверки разрешения трансляции в выбраный порт по индексу порта источника.
  * @param  uint32_t     PortBitID;    // Собственный битовый ID порта
  * @param  uint32_t     MaskPortID;   // Маска разрешенных ID портов
  * @retval bool  true - разрешена трансляция
  *               false - запрещена трансляция
  */
bool permit_index_port(uint32_t PortBitID, uint32_t MaskPortID)
{
  // Проверка маски
  if ((MaskPortID & PortBitID) > 0) 
  {
    // Если бит в маске установлен - разрешить трансляцию
    return true;
  }
  else
  {
    // Иначе запретить
    return false;    
  }  
}

/**
  * @brief  Функция проверка разрешения трансляции в выбраный порт по ID пакета
  * @param  uint8_t     id           // Идентификатор пакета
  * @param  uint8_t     umMaskBoxID  // Номер маски разрешенных ID пакетов
  * @retval bool  true - разрешена трансляция
  *               false - запрещена трансляция
  */
bool permit_index_box(uint8_t id, uint8_t umMaskBoxID)
{
  // Если номер маски выходит за границы таблицы масок - выход
  if (umMaskBoxID >= MAX_NUM_MASK_BOX_ID) return false;
  
  // Проверка маски трансляции
  if ((DataSto.Settings.tabl_mask_box_id[umMaskBoxID].mask_id[(id>>4)])&(0x0001<<(id&0x0F)))
  {
    // Если бит в маске установлен - разрешить трансляцию
    return true;
  }
  else
  {
    // Иначе запретить
    return false;    
  }  
}

/**
  * @brief  Функция предназначена для выделени порта в таблице маршрутизации.
  * @param  uint8_t* index_port - указатель на переменную для индекса выделяемого порта
  * @retval bool  true - порт выделен
  *               false - нет доступных портов
  */
bool request_port_pnt(uint8_t* index_table)
{
  uint8_t temp_index_cnt;
  // Устанавливаем предварительный статус команды
  bool tmp_val_bool = false;

  /* Если задача роутера не запущена - выход */
  if (HandleTaskRouter == NULL) return false;

  // Если мютекс обьявлен
  if (xRouterMutex != NULL)
  {
    // Заняли мьютех
    xSemaphoreTake( xRouterMutex, portMAX_DELAY );
    // поиск свободного порта маршрутизации
    for (temp_index_cnt = 0;temp_index_cnt < MAX_ROUTER_PORT;temp_index_cnt++) 
    {
      if ( tabl_router[temp_index_cnt].FlagReqPnt == FREE_PNT )
      {
        // Найден свободный ресурс для порта - занимаем
        tabl_router[temp_index_cnt].FlagReqPnt = BUSY_PNT; 
        // Возвращаем выделенный индекс индек
        *index_table = temp_index_cnt;
        // Команда выполнена
        tmp_val_bool = true;
        // выход из цикла
        break;
      }
    }
    // Освободили мьютех
    xSemaphoreGive( xRouterMutex );
  }
  return tmp_val_bool;
}

/**
  * @brief  Функция включение порта маршрутизации.
  * @param  uint8_t index_port - индекс порта маршрутизации
  * @retval bool  true - команда выполнена
  *               false - команда не выполнена
  */ 
bool enable_port_pnt(uint8_t index_port)
{
  // Устанавливаем предварительный статус команды
  bool tmp_val_bool = false;
  // Если мютекс обьявлен
  if (xRouterMutex != NULL)
  {
    // Заняли мьютех
    xSemaphoreTake( xRouterMutex, portMAX_DELAY );
    // Если индекс не выходит за границы таблицы маршрутизации,
    if (index_port < MAX_ROUTER_PORT)
    { // указатель на порт выделен и порт инициализирован
      if((tabl_router[index_port].PortStatus != NOT_INSTALL_PORT)&&(tabl_router[index_port].FlagReqPnt == BUSY_PNT))
      {
        // Включаем порт для маршрутизации
        tabl_router[index_port].PortStatus = ENABLE_PORT;   
        // Команда выполнена
        tmp_val_bool = true;     
      }
    }
    // Освободили мьютех
    xSemaphoreGive( xRouterMutex );
  }
  return tmp_val_bool;
}


/**
  * @brief  Функция выключение порта маршрутизации.
  * @param  uint8_t index_port - индекс порта маршрутизации
  * @retval bool  true - команда выполнена
  *               false - команда не выполнена
  */
bool disable_port_pnt(uint8_t index_port)
{
  // Устанавливаем предварительный статус команды
  bool tmp_val_bool = false;
  // Если мютекс обьявлен
  if (xRouterMutex != NULL)
  {
    // Заняли мьютех
    xSemaphoreTake( xRouterMutex, portMAX_DELAY );
    // Если индекс не выходит за границы таблицы маршрутизации,
    if (index_port < MAX_ROUTER_PORT)
    { // указатель на порт выделен и порт инициализирован
      if((tabl_router[index_port].PortStatus != NOT_INSTALL_PORT)&&(tabl_router[index_port].FlagReqPnt == BUSY_PNT))
      {
        // Выключаем порт для маршрутизации
        tabl_router[index_port].PortStatus = DISABLE_PORT;   
        // Команда выполнена
        tmp_val_bool = true;     
      }
    }
    // Освободили мьютех
    xSemaphoreGive( xRouterMutex );
  }
  return tmp_val_bool;
}

/**
  * @brief  Функция обнуления настроек порта маршрутизации.
  * @param  uint8_t index_port - индекс порта маршрутизации
  * @retval bool  true - команда выполнена
  *               false - команда не выполнена
  */
bool reset_port_pnt(uint8_t index_port)
{
  // Устанавливаем предварительный статус команды
  bool tmp_val_bool = false;
  // Если мютекс обьявлен
  if (xRouterMutex != NULL)
  {
    // Заняли мьютех
    xSemaphoreTake( xRouterMutex, portMAX_DELAY );
    // Если индекс не выходит за границы таблицы маршрутизации
    if (index_port < MAX_ROUTER_PORT) 
    {
      //(обнуляются все поля структуры)
      tabl_router[index_port].FlagLockAddr = LOCK_ADDR;
      tabl_router[index_port].FlagReqPnt = FREE_PNT;  
      tabl_router[index_port].MaskPortID = 0x0000;   
      tabl_router[index_port].NumMaskBoxID = 0x00;      
      tabl_router[index_port].PortID = 0x00;      
      tabl_router[index_port].PortBitID = 0x00000000; 
      tabl_router[index_port].PortStatus = NOT_INSTALL_PORT;   
      tabl_router[index_port].QueueOutRouter = NULL;      
      tabl_router[index_port].QueueInRouter = NULL;
      tabl_router[index_port].HandleTask = NULL;      
      // Команда выполнена
      tmp_val_bool = true;     
    }
    // Освободили мьютех
    xSemaphoreGive( xRouterMutex );
  }
  return tmp_val_bool;
}

/**
  * @brief Функция предназначена для инициализации порта в таблице маршрутизации.
  * @param uint8_t index_port - индекс порта маршрутизации
  * @param port_router_t* settings_port - указатель на структуру с настройками порта
  * ( !!! параметры FlagReqPnt,PortStatus, в структуре settings_port не задействованы )
  * @retval bool  true - порт прошел инициализацию успешно
  *               false - порт не прошел инициализацию
  */
bool settings_port_pnt(uint8_t index_port , port_router_t* settings_port)
{
  // Устанавливаем предварительный статус команды
  bool tmp_val_bool = false;
  // Если мютекс обьявлен
  if (xRouterMutex != NULL)
  {
    // Заняли мьютех
    xSemaphoreTake( xRouterMutex, portMAX_DELAY );
    // Если индекс не выходит за границы таблицы маршрутизации,
    if (index_port < MAX_ROUTER_PORT)
    { // указатель на порт выделен и порт не инициализирован
      if((tabl_router[index_port].PortStatus == NOT_INSTALL_PORT)&&(tabl_router[index_port].FlagReqPnt == BUSY_PNT))
      {
        /* Сохраняем указатель на задачу роутера  */
        settings_port->HandleTaskRouter = HandleTaskRouter;   
        // Производим инициализацию
        tabl_router[index_port].FlagLockAddr = settings_port->FlagLockAddr;
        tabl_router[index_port].MaskPortID = settings_port->MaskPortID;   
        tabl_router[index_port].NumMaskBoxID = settings_port->NumMaskBoxID;      
        tabl_router[index_port].PortID = settings_port->PortID;         
        tabl_router[index_port].PortBitID = settings_port->PortBitID; 
        tabl_router[index_port].QueueOutRouter = settings_port->QueueOutRouter;      
        tabl_router[index_port].QueueInRouter = settings_port->QueueInRouter;
        tabl_router[index_port].HandleTask = settings_port->HandleTask;  
        tabl_router[index_port].HandleTaskRouter = settings_port->HandleTaskRouter;         
        // Обновляем статус порта
        tabl_router[index_port].PortStatus = DISABLE_PORT;        
        // Команда выполнена
        tmp_val_bool = true;     
      }
    }
    // Освободили мьютех
    xSemaphoreGive( xRouterMutex );
  }
  return tmp_val_bool;
}

/*
  * @brief  функция обработки програмного таймера для задачи роутера.
  * @param  TimerHandle_t pxTimer - указатель на таймер вызвавщий функцию
  * @retval None
  */
void SoftTimerRouterCallback( TimerHandle_t pxTimer )
{
  if ( ( HandleTaskRouter ) != NULL  )
  {
    if ( xRouterSoftTimer == pxTimer )
    {/* Устанавливаем событие срабатывания програмного таймера для периодического уведомления */
      xTaskNotify( HandleTaskRouter,       /* Указатель на уведомлюемую задачу                         */
                  TIMER_NOTE,                     /* Значения уведомления                                     */
                  eSetBits );                     /* Текущее уведомление добавляются к уже прописанному       */
    }
    if ( xRouterSoftUptime == pxTimer )
    {/* Устанавливаем событие срабатывания програмного таймера для периодического уведомления */
      xTaskNotify( HandleTaskRouter,       /* Указатель на уведомлюемую задачу                         */
                  UPTIME_NOTE,                    /* Значения уведомления                                     */
                  eSetBits );                     /* Текущее уведомление добавляются к уже прописанному       */
    } 
  }  
}

/**
  * @brief  Задача мультиплексирования потоков.
  * @param  None
  * @retval None
  */
void router_thread(void *arg)
{
  // Инициализация таблицы масок ID пакетов
  //init_mask_id(); 

  // Если включен режим сохранения шапок маршрутизируемых пакетов  
#if  (LOG_STRIME_ENABLE == 1) 
  if (QueueLogRouter == NULL)
  {
    //открытие очереди хранения шапок маршрутизируемых пакетов
    QueueLogRouter = xQueueCreate( SIZE_QUEUE_LOG_ROUTER , sizeof(log_head_box_t)); 
  }
#endif  /* LOG_STRIME_ENABLE */  
  
  // Инициализация мьютекса маршрутизации
  xRouterMutex = xSemaphoreCreateMutex();
  // Обнуляем таблицу портов маршрутизации
  for( uint8_t temp_index_var = 0; temp_index_var <  MAX_ROUTER_PORT ; temp_index_var++ )
  {
    // установка параметров по умолчанию для выбраного порта
    reset_port_pnt(temp_index_var);
  }
 
  /* открытие програмного таймера  */
  xRouterSoftTimer = xTimerCreate( "RTR_TIM",                       /* Назначим имя таймеру, только для отладки.    */
                                 TIME_PERIOD_WORK_ROUTER_THREAD,  /* Период таймера в тиках.                      */                     
                                 pdTRUE,                          /* Автоперезагрузка включена.                   */
                                 NULL,                            /* Нет связи с индивидуальным индентификатором. */
                                 SoftTimerRouterCallback );        /* Функция обратного вызова вызываемая таймером.*/ 
  
  /* открытие програмного таймера  */
  xRouterSoftUptime = xTimerCreate( "RTR_UPD",                      /* Назначим имя таймеру, только для отладки.    */
                                  TIME_PERIOD_UPDATE_TBL_DUBLICATE,/* Период таймера в тиках.                      */                     
                                  pdTRUE,                          /* Автоперезагрузка включена.                   */
                                  NULL,                            /* Нет связи с индивидуальным индентификатором. */
                                  SoftTimerRouterCallback );       /* Функция обратного вызова вызываемая таймером.*/    
  
  
  /* Запустить програмные таймеры */
  xTimerStart( xRouterSoftTimer, 0 );
  xTimerStart( xRouterSoftUptime, 0 );
  
  // Тело задачи
  for( ;; )
  {
    /* Обнуление сообщения */
    NotifiedValue = 0x00000000;
    /* Ожидаем сообщения в задачу */
    xTaskNotifyWait(0x00000000,       /* Не очищайте биты уведомлений при входе               */
                    0xFFFFFFFF,       /* Сбросить значение уведомления до 0 при выходе        */
                    &(NotifiedValue), /* Значение уведомленное передается в  ulNotifiedValue  */
                    portMAX_DELAY );  /* Блокировка задачи до появления уведомления           */    
    /*=========================================================================*/      
    /* Если собщение обновления времени жизни дубликатов - обновляем время */
    /*=========================================================================*/    
    if( ( ( NotifiedValue ) & UPTIME_NOTE ) != 0 )
    {  
      /* Производим обновление времени жизни меток фреймов (обновляем каждые TIME_PERIOD_UPDATE_TBL_DUBLICATE = 100) */
      Refresh_Duplicate_Table(TIME_PERIOD_UPDATE_TBL_DUBLICATE);
    }      
    
    
    /*=========================================================================*/     
    /* Если запущен периодический контроль - проводим периодический контроль TIME_PERIOD_WORK_ROUTER_THREAD = 10*/
    /*=========================================================================*/
    if( ( ( NotifiedValue ) & TIMER_NOTE ) != 0 )
    {  
      /* Пока не используется  */  
    } 
    
    /*=========================================================================*/       
    /* Если сообщени о получении пакета и во всех остальных случаях - проверяем очереди роутеров */
    /*=========================================================================*/         
    // Заняли мьтех
    xSemaphoreTake( xRouterMutex, portMAX_DELAY );
    // Установка флага проверки входящих сообщений
    flag_read_in_port = true;
    // Цикл вычитывания входящих сообщений
    while(flag_read_in_port)
    {  
      // Запущена проверка входящих - флаг сброшен
      flag_read_in_port = false;
      
      // Цикл по активным входящим портам
      for (index_in_port = 0 ;index_in_port <  MAX_ROUTER_PORT; index_in_port++)
      {
        // Проверка выделения порта
        if ( tabl_router[index_in_port].FlagReqPnt != BUSY_PNT ) continue;
        // Проверка активности порта
        if ( tabl_router[index_in_port].PortStatus != ENABLE_PORT ) continue;
        // Проверка наличия очереди входных данных порта
        if ( tabl_router[index_in_port].QueueInRouter == NULL ) continue;
        // Проверка наличия данных в очереди
        if ( xQueueReceive( tabl_router[index_in_port].QueueInRouter, &buf_box.router_box, ( TickType_t ) 0 ) != pdTRUE ) continue;
        
        // Получен пакет для маршрутизатора
        // Есть входящие - включаем флаг повторной проверки
        flag_read_in_port = true;
        // Задаем идентификатор пакету
        buf_box.cnt_id = cnt_indent++;
        // Сохраняем индекса источника пакета
        buf_box.source_port = index_in_port;
        /* Маркируем пакет идентификатором порта источника */
        buf_box.router_box.SrcPortID = tabl_router[index_in_port].PortID;
        // Время получения пакета
        buf_box.time_entrance = xTaskGetTickCount();
        // Если пакет дублирован - не транслируем.  
        if (ControlDuplicateFrame(&buf_box.router_box))
        {   
          // Принят пакет дубликат - не маршрутизируем
#if  (LOG_STRIME_ENABLE == 1)  // Если включен режим сохранения шапок маршрутизируемых пакетов
          // Функция сохранения флага дублирование пакета.
          log_duplicate_router(1);
#endif  
        }
        else
        { // Принят пакет не дубликат - начинаем обработку
#if  (LOG_STRIME_ENABLE == 1)  // Если включен режим сохранения шапок маршрутизируемых пакетов
          // Функция сохранения флага дублирование пакета.
          log_duplicate_router(0);
#endif  
          // Цикл по активным исходящим портам
          for (index_out_port = 0 ;index_out_port <  MAX_ROUTER_PORT; index_out_port++)
          {
            // Проверка выделения порта
            if ( tabl_router[index_out_port].FlagReqPnt != BUSY_PNT ) continue;
            // Проверка активности порта
            if ( tabl_router[index_out_port].PortStatus != ENABLE_PORT ) continue;
            // Проверка порт является источником обрабатываемого пакета
            if ( buf_box.source_port == index_out_port ) continue;
            // Проверка наличия очереди выходных данных порта
            if ( tabl_router[index_out_port].QueueOutRouter == NULL ) continue;
            // Проверка соответствия адреса назначения пакета - собственному адресу
            if (tabl_router[index_out_port].FlagLockAddr == UNLOCK_ADDR)
            {
              // Трансляция в порт пакета только с собственны адресом или широковещательного иначе переключаемся на следующий порт
              if ( ( buf_box.router_box.dest != DataLoaderSto.Settings.phy_adr ) && ( buf_box.router_box.dest != ADDRES_BROADCASTING ) ) continue;
            }
            else
            {
              // Трансляция в порт пакета только с собственны адресом запрещена - переключаемся на следующий порт
              if ( buf_box.router_box.dest == DataLoaderSto.Settings.phy_adr ) continue;
            } 
            // Проверка разрешения трансляции в выбраный порт по индексу порта источника
            if (permit_index_port(tabl_router[buf_box.source_port].PortBitID,tabl_router[index_out_port].MaskPortID) == false) continue;
            // Проверка разрешения трансляции в выбраный порт по ID пакета
            if (permit_index_box(buf_box.router_box.id,tabl_router[index_out_port].NumMaskBoxID) == false) continue;
            
            // Транслируем пакет
            xQueueSend( tabl_router[index_out_port].QueueOutRouter, &buf_box.router_box, ( TickType_t ) 0 );
            
            /* Если указатель на задачу не нулевой отправляем сообщение задаче что пакет отправлен*/
            if (tabl_router[index_out_port].HandleTask != NULL)
            {
              /* Отправляем сообщение для задачи контроля шлюза */
              xTaskNotify( tabl_router[index_out_port].HandleTask,    /* Указатель на уведомлюемую задачу                         */
                          ROUTER_NOTE,                                /* Значения уведомления                                     */
                          eSetBits );                                 /* Текущее уведомление добавляются к уже прописанному       */
            }
            
#if  (LOG_STRIME_ENABLE == 1)  // Если включен режим сохранения шапок маршрутизируемых пакетов
            // Функция сохранения номера порта в который отправлен пакет.
            log_out_router(tabl_router[index_out_port].PortBitID);
#endif     
          } 
        } 
#if  (LOG_STRIME_ENABLE == 1) // Если включен режим сохранения шапок маршрутизируемых пакетов
        //  Функция сохранения шапки входящего пакета роутера.
        log_in_router( &buf_box, tabl_router[index_in_port].PortBitID );
#endif  
      }
    }
    // Освободили мьютех
    xSemaphoreGive( xRouterMutex );
  }	
}

/**
  * @brief Функция корректирует параметры пакета RS перед отправкой.
  * @param router_box_t* rs_box указатель на буфер пакета
  * @param uint8_t* contic указатель на счетчик неприрывности
  * @retval none 
  */
void update_rs_tx_box(router_box_t* rs_box,uint8_t* contic)
{
  // Загрузка счетчика неприрывности
  rs_box->cnt = *contic;  
  
  // Обновляем счетчик неприрывности
  if ((*contic) > 254) (*contic) = 0;
  else (*contic)++;
 
  // Вычисление контрольной суммы
  *((uint16_t*)&(rs_box->data[(rs_box->lenght) + SIZE_LENGHT + SIZE_CRC - sizeof(marker_box_t)])) = CalcCRC16((uint8_t*)(&(rs_box->lenght)), rs_box->lenght);
} 

/**
  * @brief Функция тестирования пакета.
  * @param router_box_t* box_test указатель на тестируемый пакет
  * @retval uint8_t  0 - отправляем в мультиплексор
  *                  1 - локальный пакет
  *                  2 - маркерный пакет
  *                  3 - ошибка CRC
  *                  4 - неккоректная шапка пакета
  */
uint8_t test_receive_box(router_box_t* box_test)
{
  //проверка преамбулы
  if (box_test->pre != 0xAA55) return 4;
  //проверка длинны сообщения
  if ((box_test->lenght) >= (sizeof(router_box_t) - SIZE_LENGHT - SIZE_PRE)) return 4;
  //если длинна пакета меньше шапки
  if ((box_test->lenght) < (sizeof(marker_box_t) - SIZE_LENGHT - SIZE_PRE)) return 4;  
  //контрольная сумма не совпадает
  if ((*((uint16_t*)&(box_test->data[(box_test->lenght) + SIZE_LENGHT + SIZE_PRE - sizeof(marker_box_t)])))\
    != (CalcCRC16((uint8_t*)(&(box_test->lenght)), box_test->lenght))) return 3;
  //если пакет маркер
  if (box_test->id == 0) return 2;
  //если пакет локальный или закольцованый 
  if ( ( (box_test->dest) == 0x0000 ) || ( (box_test->src ) == DataLoaderSto.Settings.phy_adr ) )  return 1;
  // Отправить пакет в коммутатор
  return 0;  
}

/************************ (C) COPYRIGHT DEX 2019 *****END OF FILE**************/
