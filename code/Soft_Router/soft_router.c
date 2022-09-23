/**
  ******************************************************************************
  * @file    soft_router.c
  * @author  Trembach D.N.
  * @version V1.0.0
  * @date    13-07-2020
  * @brief   Файл содержит функции програмного мультиплексора потоков
  ******************************************************************************
  * @attention
  *    
  *    
  *    Заложить буфер на 20 пакетов.
  *    
  *    1.Запрос текущей ячейки для записи.
  *      При запросе ячейки обнуляется все маски (канала и порта источника) 
  *      (Индекс записуемой ячейки общий для всего мультиплексора)
  *    2.Заполнение ячейки.
  *    3.Установки маски канала и порта источника.
  *    4.Индекс чтения у каждого порта индивидуальный
  *      Чтение данных из ячейки согласно маски и пока не догоним индекс чтения.
  *    
  *    У коммутатора каждый порт имеет индивидуальный идентификатор.
  *    Маска портов из которых разрешено чтение.
  *    Маска каналов разрешенных для приема.
  *    
  *    К мультиплексору подключается 5 портов:
  *    1.Аппаратный кодек A
  *    2.Аппаратный кодек B
  *    3.Аппаратный кодек C
  *    4.Аналоговый кодек. 
  *    5.Спикс.
  *    6.Ehternet port. 
  *      
  *    Аппаратный кодек принимает данные из портов 3,4,5,6. 
  *    Аналоговый кодек принимает данные из портов 1,2,3,5,6.
  *    Спикс кодек принимает данные из портов 1,2,3,4,6.
  *    Ehternet кодек принимает данные из портов 1,2,3,4,5.
  *    
  *    У каждого пакета есть следующие управляющие поля:
  *    1.Порт источника сигнала.
  *    2.Тип источника сигнала.
  *    
  *            
  *    У мультиплесора общий буфер для всех:
  *    Доступ регламентируется мьютексом.
  *    Для записи порт запрашивает ячейку и если она доступна, получает указатель на ячейку.
  *    При этом индекс записи смещается на следующую ячейку - блокирую ее чтение.
  *    По полученномц указателю записываются данных и порт маски и разрещение для чтения ячейки.  
  *    
  *    Для чтения данных у каждого порта свой указатель на текущую ячейку для чтения.
  *    При чтении осуществдяется поиск ячейки с данными, пока индекс не попадет на ячейку запрещеннйю для чтения.
  *    Выбор пакета для чтения осуществляется по следующим данным:
  *    1.Поиск пока не найдена ячейка, но недоходя до ячейки запрещенным для чтения.
  *    2.После анализа пакета и устанавливается индекс на обработку следующего пакета.
  *      
  *    При тестировании из 5 тестовых задач формировалось по 1 пакету в 1 млсек
  *    Каждая задача принимала пакеты с 4 остальных задач
  *    При загрузке и чтении в буфер производилось полное копирование пакета 
  *    Загрузка системы 8-9% на каждую задачу 
  *    потеря пакетов при такой загрузке 0.004% (?)
  *    При уменьшении нагрузки в 3-4 раза потерь нет вообще 
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "soft_router.h"
#include "hooks.h"
#include "printf_dbg.h" 
#include "semphr.h" 
#include "rs_frame_header.h"     
 
#if ( ROUTER_SOFT_ENABLE == 1 ) 
   
/* Объявляем буфер для FIFO */    
soft_fifo_t fifo_soft;    

/*  Объявляем массив указателей на задачу - используется для отправки сообщений в задачу */
TaskHandle_t  HandleNoteBase[MAX_SOFT_PORT_INDEX];   

#if ( TEST_SOFT_ROUTER_EN == 1 )
/* Объявляем структуры для тестирования */ 
  soft_fifo_test_t st_a_tst;
  soft_fifo_test_t st_b_tst;
#endif

/**
  * @brief  Функция инициализация буфера FIFO
  * @param  None
  * @retval None
  * 
  */
void InitSoftFIFO( void )
{
  /* Обнуление указателей рассылки уведомлений */
  for( uint8_t contik = 0 ; contik < MAX_SOFT_PORT_INDEX ; contik++ )
  {
    HandleNoteBase[contik] = NULL;
  }
  
  fifo_soft.index_wr_box = 0;
  
  fifo_soft.xMutex = xSemaphoreCreateMutex();
 
  fifo_soft.status_fifo = FIFO_EN;
  
  xSemaphoreGive( fifo_soft.xMutex );
}

/**
  * @brief  Функция получения указателя на буффер для чтение
  * @param  uint8_t* index_box  указатель на индекс запращиваемого буффера
  * @param  uint32_t mask_chanel  - маска доступных каналов  
  * @param  uint32_t mask_index_port - маска доступных для приема портов 
  * @retval soft_router_box_t* - указатель на текущий буффер FIFO для чтения 
  */
soft_router_box_t* GetRDPointFIFO( uint8_t* index_box ,uint32_t  mask_chanel ,uint32_t mask_index_port )
{
  /* Объявляем переменную для  указателя буфер  */
  soft_router_box_t* point_box;
    
  while(1)
  {
    if ( fifo_soft.fifo_box[(*index_box)].enable_read != 0 )
    { /* Пакет разрешен для чтения */
      /* Указатель на пакет для чтения*/
      point_box = &(fifo_soft.fifo_box[(*index_box)]);
      
      /* Увеличение индекса буфера */  
      if ( (*index_box) < ( MAX_NUMBER_BOX - 1 ) )
      {
        (*index_box)++;
      }
      else
      {
        (*index_box) = 0;
      }  
      /* Проверка пакета на соответствие маске разрешенных каналов */
      if ( ( BitChID(point_box->data_box.ch_id) & mask_chanel ) > 0 )
      {
        /* Проверка пакета на соответствие маске разрешенных портов */
        if ( ( Bit_SoftPID(point_box->source_port) & mask_index_port ) > 0 )
        {
          /* Возвращение указателя на буфер доступный для чтения */
          return point_box;            
        }
      }
    }
    else
    {/* Пакет запрещен для чтения */
      
      /* Обнуляем указатель на пакет для чтения */
      point_box = NULL;   
      /* Возвращение указателя на буфер доступный для чтения */
      return point_box;    
    } 
  }
}

/**
  * @brief  Функция запроса указатель на текущий буфер FIFO для записи 
  * @param  none
  * @retval soft_router_box_t* - указатель на текущий буффер FIFO для записи 
  * 
  */
soft_router_box_t* GetWRPointFIFO( void )
{
  /* Получаем указатель на текущий буфер  */
  soft_router_box_t* point_box = &( fifo_soft.fifo_box[fifo_soft.index_wr_box]); 
  
  /* Получить мьютерс */
  if ( xSemaphoreTake( fifo_soft.xMutex , 0) == pdTRUE ) 
  {
    
    /* Увеличение индекса буфера */  
    if ( fifo_soft.index_wr_box < ( MAX_NUMBER_BOX - 1 ) )
    {
      fifo_soft.index_wr_box++;
    }
    else
    {
      fifo_soft.index_wr_box = 0;
    }  
    
    /* Блокируем чтение буфера */
    fifo_soft.fifo_box[fifo_soft.index_wr_box].enable_read = 0;
    
    /* Отдать мьютекс */
    xSemaphoreGive( fifo_soft.xMutex );
  }
  else
  {
    point_box = NULL;
  }    
  /* Возвращение указателя на текущий буфер */
  return point_box;
}

/**
  * @brief  Функция записи пакета в fifo 
  * @param  soft_fifo_test_t* test_soft_box - указатель на структуру тестирования
  * @retval NONE
  * 
  */
void WR_FIFO_box( soft_fifo_test_t* test_soft_box )
{
  /* Копирование данных */
  for(test_soft_box->cnt_index = 0 ;  test_soft_box->cnt_index < sizeof(soft_router_box_t)/4 ; (test_soft_box->cnt_index)++)
  {
    ((uint32_t*)(test_soft_box->pfifo_box))[test_soft_box->cnt_index] = ((uint32_t*)&(test_soft_box->fifo_gen_box))[test_soft_box->cnt_index];               
  }
  /* Разрешение пакета для чтения */
  test_soft_box->pfifo_box->enable_read = 1;
  /* Подсчет переданных пакетов */
  test_soft_box->cnt_tx_box++;
}

/**
  * @brief  Функция чтения пакета из fifo 
  * @param  soft_fifo_test_t* test_soft_box - указатель на структуру тестирования
  * @retval NONE
  * 
  */
void RD_FIFO_box( soft_fifo_test_t* test_soft_box )
{
  /* Копирование данных */
  for(test_soft_box->cnt_index = 0 ;  test_soft_box->cnt_index < sizeof(soft_router_box_t)/4 ; (test_soft_box->cnt_index)++)
  {
    ((uint32_t*)&(test_soft_box->fifo_test_box))[test_soft_box->cnt_index] = ((uint32_t*)(test_soft_box->pfifo_box))[test_soft_box->cnt_index];               
  }
  /* Подсчет переданных пакетов */
  test_soft_box->cnt_rx_box++;
}

/* Задача для отладки и тестирования мультиплексора мультиплексора  */

/**
 * @brief  Функция формирования пакета.
 * @param  soft_fifo_test_t* test_soft_box - указатель на структуру тестирования
 * @retval None
 */
void gen_soft_box(soft_fifo_test_t* test_soft_box)
{
  for(test_soft_box->cnt_index = 0 ;  test_soft_box->cnt_index < MAX_SIZE_BOX_SAMPLE ; (test_soft_box->cnt_index)++)
  {
    test_soft_box->fifo_gen_box.data_box.data[test_soft_box->cnt_index] = test_soft_box->cnt_index;             /* Данные голосового потока               */  
  }
  
  test_soft_box->fifo_gen_box.data_box.ch_id =                     test_soft_box->ch_id;                        /* Идентификатор канала                   */
  test_soft_box->fifo_gen_box.data_box.dest_phy_addr =             test_soft_box->dest_phy_addr;                /* Физический адрес получателя            */
  test_soft_box->fifo_gen_box.data_box.src_phy_addr =              test_soft_box->src_phy_addr;                 /* Физический адрес источника             */
  test_soft_box->fifo_gen_box.data_box.codec_phy_addr =            test_soft_box->gate_phy_addr;                /* Физический адрес шлюза                 */
  test_soft_box->fifo_gen_box.data_box.priority_box =              test_soft_box->priority_box;                 /* Приоритет данных пакета                */
  test_soft_box->fifo_gen_box.data_box.cnt = 0;                                                                 /* Cчетчик неприрывности пакетов 0..255   */
  test_soft_box->fifo_gen_box.source_port = test_soft_box->index_port;                                          /* Собственный индекс порта               */
  test_soft_box->fifo_gen_box.enable_read = 0;                                                                  /* Сброс флага чтения                     */
}

/**
 * @brief  Функция обновления пакета.
 * @param  soft_fifo_test_t* test_soft_box - указатель на структуру тестирования
 * @retval None
 */
void upd_soft_box(soft_fifo_test_t* test_soft_box)
{
  /* Установка временного маркера пакета */
  test_soft_box->fifo_gen_box.data_box.time_id = ulGetRuntimeCounterValue();
  
  /* обновление счетчика неприрывности пакетов */
  test_soft_box->fifo_gen_box.data_box.cnt++;      
}

/**
 * @brief  Функция регистрации на рассылку.
 * @param  uint8_t index_src - индекс порта 
 * @param  TaskHandle_t  HandleNote - указатель на задачу 
 * @retval None
 */
void reg_notify_port( uint8_t index_src, TaskHandle_t  HandleNote )
{
  /* Ждем инициализации програмного роутера */
  while( fifo_soft.status_fifo != FIFO_EN )
  {
    vTaskDelay(1);
  }
  
  /* Проверка индекса */
  if ( ( index_src > 0 ) && ( index_src < MAX_SOFT_PORT_INDEX ) )
  {
    /* Проверка указателя */
    if ( HandleNote != NULL )
    { /* Сохранения указателя */
      HandleNoteBase[index_src] = HandleNote;
    }
  }
}

/**
 * @brief  Функция оповешения зарегистрировавшихся на рассылку.
 * @param  uint8_t index_src - индекс порта инициатора рассылки
 * @retval None
 */
void notify_soft_port( uint8_t index_src )
{
  /* Обнуление указателей рассылки уведомлений */
  for( uint8_t contik = 0 ; contik < MAX_SOFT_PORT_INDEX ; contik++ )
  {
    if ( index_src != contik )
    {
      /* Если есть запрос на рассылку - рассылаем уведомления */
      if ( HandleNoteBase[contik] != NULL)
      {
        {/* Устанавливаем событие срабатывания програмного таймера для периодического уведомления */
          xTaskNotify( HandleNoteBase[contik],   /* Указатель на уведомлюемую задачу                         */
                      SOFT_ROUTER_NOTE,          /* Значения уведомления                                     */
                      eSetBits );                /* Текущее уведомление добавляются к уже прописанному       */
        }
      }
    }
  }
}

#define soft_test_box ((soft_fifo_test_t *)arg)
/**
 * @brief  Тестовая задача формирования и приема пакетов системы.
 * @param  None
 * @retval None
 */
void test_soft_router(void *arg)
{
  /* Инициализация генератора пакетов */ 
  gen_soft_box(soft_test_box);
  
  for( ;; )
  {
    /* Отсчет периода отправки пакета */
    if( soft_test_box->cnt_period_tx_box < soft_test_box->max_period_tx_box) 
    {
      soft_test_box->cnt_period_tx_box++;
    }  
    else
    {
      /*  */
      soft_test_box->cnt_period_tx_box = 0;
      /* Функция обновления пакета */
      upd_soft_box(soft_test_box);
      /* Запрос указателя на пакет */
      soft_test_box->pfifo_box = GetWRPointFIFO();
      /* Если указатель корректный */
      if ( soft_test_box->pfifo_box != NULL)
      {
        /* Отправка пакет по указателю */
        WR_FIFO_box(soft_test_box);
        /* Рассылка оповешения зарегистрировавшихся на рассылку.*/
        notify_soft_port(soft_test_box->ch_id);
      }
    }  
    do
    {
      /* Функция приема пакета   */
      soft_test_box->pfifo_box = GetRDPointFIFO ( &(soft_test_box->index_rd_box), soft_test_box->mask_chanel, soft_test_box->mask_index_port );
      /* Если указатель корректный */      
      if ( (soft_test_box->pfifo_box) != NULL )
      {
        /* Чтение пакета по указателю */
        RD_FIFO_box(soft_test_box);
      }
    }while((soft_test_box->pfifo_box) != NULL);
    
    /* Ожидание до следующего цикла. */
    vTaskDelay( soft_test_box->task_period);
  }
}

#if ( TEST_SOFT_ROUTER_EN == 1 )  
  /**
   * @brief  Запуск задач тестирования передачи и приема пакетов системы.
   * @param  None
   * @retval None
   */
  void start_test_soft_router(void )
  {
      
    /* Инициализация структуры тестирования */
    st_a_tst.dest_phy_addr = 0x2012;                                  /* Физический адрес получателя                   */
    st_a_tst.src_phy_addr = 0x2012;                                   /* Физический адрес источника                    */
    st_a_tst.gate_phy_addr = 0x2012;                                  /* Физический адрес шлюза                        */
    st_a_tst.type_box = 1;                                            /* Формат данных пакета                          */
    st_a_tst.cnt_tx_box = 0;                                          /* Счетчик переданных пакетов                    */
    st_a_tst.cnt_rx_box = 0;                                          /* Счетчик принятых пакетов                      */  
    st_a_tst.cnt_rx_err = 0;                                          /* Счетчик принятых пакетов c ошибкой            */   
    st_a_tst.task_period = 1;                                         /* Период вызова задачи в тиках                  */
    st_a_tst.cnt_period_tx_box = 0;                                   /* Счетчик периода формирования пакетов          */  
    st_a_tst.max_period_tx_box = 1000;                                /* Период формирования пакетов                   */
    st_a_tst.ch_id = ChAID;                                           /* Идентификатор канала                          */  
    st_a_tst.index_port = TST_A_SoftPID;                              /* Собственный индекс порта                      */
    st_a_tst.mask_chanel = BitChID(ChAID);                            /* Маска доступных каналов                       */  
    st_a_tst.mask_index_port =  Bit_SoftPID(TST_B_SoftPID);                 /* Маска доступных для приема портов             */
    /* Запуск задачи структуры тестирования */
    xTaskCreate(test_soft_router, (const char*)"SFaTST", configMINIMAL_STACK_SIZE, (void*)&st_a_tst, tskIDLE_PRIORITY + 3, NULL);  

    /* Инициализация структуры тестирования */
    st_b_tst.dest_phy_addr = 0x2011;                                  /* Физический адрес получателя                   */
    st_b_tst.src_phy_addr = 0x2011;                                   /* Физический адрес источника                    */
    st_b_tst.gate_phy_addr = 0x2011;                                  /* Физический адрес шлюза                        */
    st_b_tst.type_box = 1;                                            /* Формат данных пакета                          */
    st_b_tst.cnt_tx_box = 0;                                          /* Счетчик переданных пакетов                    */
    st_b_tst.cnt_rx_box = 0;                                          /* Счетчик принятых пакетов                      */
    st_b_tst.cnt_rx_err = 0;                                          /* Счетчик принятых пакетов c ошибкой            */
    st_b_tst.task_period = 1;                                         /* Период вызова задачи в тиках                  */
    st_b_tst.cnt_period_tx_box = 0;                                   /* Счетчик периода формирования пакетов          */
    st_b_tst.max_period_tx_box = 1000;                                /* Период формирования пакетов                   */
    st_b_tst.ch_id = ChAID;                                           /* Идентификатор канала                          */
    st_b_tst.index_port = TST_B_SoftPID;                              /* Собственный индекс порта                      */
    st_b_tst.mask_chanel = BitChID(ChAID);                            /* Маска доступных каналов                       */
    st_b_tst.mask_index_port = Bit_SoftPID(TST_A_SoftPID);                  /* Маска доступных для приема портов                 */
    /* Запуск задачи структуры тестирования */
    xTaskCreate(test_soft_router, (const char*)"SFbTST", configMINIMAL_STACK_SIZE, (void*)&st_b_tst, tskIDLE_PRIORITY + 3, NULL);  
  }
#endif /* TEST_SOFT_ROUTER_EN == 1 */
#endif /*  ROUTER_SOFT_ENABLE == 1 */
/************************ (C) COPYRIGHT DEX 2020 *****END OF FILE**************/
