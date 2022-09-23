/**
  ******************************************************************************
  * @file    soft_router.h
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
  *     Аппаратный кодек принимает данные из портов 3,4,5,6. 
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
  *
  ******************************************************************************
  */ 

/*--------------------- Define to prevent recursive inclusion ----------------*/
#ifndef __SOFT_ROUTER_H
#define __SOFT_ROUTER_H

/*----------------------------------- Includes -------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"    
    
/*------------------- максимальное число выборок в пакет ---------------------*/
#define MAX_SIZE_BOX_SAMPLE    (480)
#define MAX_NUMBER_BOX         (10) 

#define CODEC_A_SoftPID     (1)   /* Аппаратный кодек A  */
#define CODEC_B_SoftPID     (2)   /* Аппаратный кодек B  */ 
#define CODEC_C_SoftPID     (3)   /* Аппаратный кодек C  */
#define AUDIO_CODEC_SoftPID (4)   /* Аналоговый кодек.   */ 
#define SPICS_CODEC_SoftPID (5)   /* Спикс.              */   
#define UDPGate_SoftPID     (6)   /* Ehternet port.      */   
#define LOOP_SoftPID        (7)   /* Порт петля.         */     
   
#define MAX_SOFT_PORT_INDEX ( LOOP_SoftPID + 1 )

/* Включение ресурсов для тестирования */ 
#define TEST_SOFT_ROUTER_EN (0)    
#if ( TEST_SOFT_ROUTER_EN == 1 )    
  #define TST_A_SoftPID   (8)
  #define TST_B_SoftPID   (9)
#endif      

    
#define BitID(a)      (0x00000001 << a)

#define ChAID         (1)
#define ChBID         (2)
#define ChCID         (3)
#define ChDID         (4)
#define ChEID         (5)
#define ChFID         (6)

#define BitChID(a)    (0x00000001 << (a-1)) 

#define Bit_SoftPID(a)  (0x00000001 << (a-1)) 

#define ChMaskID     ( BitChID(ChAID) | BitChID(ChBID) | BitChID(ChCID) | BitChID(ChDID) | BitChID(ChEID) | BitChID(ChFID) )

#define FIFO_DS     (0)
#define FIFO_EN     (1)      

/*-------------- Описание структуры маршрутизируемого пакета  ----------------*/
typedef __packed struct
{  
  int16_t            data[MAX_SIZE_BOX_SAMPLE];    /* Данные голосового потока               */
  uint8_t            ch_id;                        /* Идентификатор канала                   */
  uint16_t           dest_phy_addr;                /* Физический адрес получателя            */
  uint16_t           src_phy_addr;                 /* Физический адрес источника             */
  uint16_t           codec_phy_addr;               /* Физический адрес шлюза                 */
  uint8_t            priority_box;                 /* Формат данных пакета                   */
  uint8_t            cnt;                          /* Cчетчик неприрывности пакетов 0..255   */
  uint32_t           time_id;                      /* Временной идентификатор пакета         */
} sample_voice_box_t;


/*--------- Описание структуры пакета в буфере програмного роутера  ----------*/
typedef __packed struct
{  
  sample_voice_box_t data_box;                     /* Данные голосового пакет                */ 
  uint8_t            source_port;                  /* Порт источник                          */  
  uint8_t            enable_read;                  /* Флаг разрешения для чтения             */ 
  uint8_t            rezerv;                       /* Выравнивание на 4 байта                */   
} soft_router_box_t;

/*--------- Описание структуры пакета в буфере програмного роутера  ----------*/
typedef __packed struct
{  
  soft_router_box_t  fifo_box[MAX_NUMBER_BOX];   /* Буфер роутера пакетов                       */
  uint8_t            index_wr_box;               /* Индекс для записи пакета в буфер роутера    */
  uint8_t            status_fifo;                /* Статус FIFO                                 */   
  SemaphoreHandle_t  xMutex;                     /* Мьютекс для работы с FIFO                   */
}soft_fifo_t;

/*----------------------- структура тестирование пакета ----------------------*/  
typedef __packed struct
{  
  soft_router_box_t  fifo_gen_box;               /* Буфер для формирования пакета                 */ 
  soft_router_box_t  fifo_test_box;              /* Буфер для приема пакета                       */   
  uint8_t            index_rd_box;               /* Индекс для приема из буфер роутера            */   
  
  soft_router_box_t* pfifo_box;                  /* Переменная хранения указателя */  
  
  uint32_t           cnt_tx_box;                 /* Счетчик переданных пакетов                    */
  uint32_t           cnt_rx_box;                 /* Счетчик принятых пакетов                      */  
  uint32_t           cnt_rx_err;                 /* Счетчик принятых пакетов c ошибкой            */   
    
  uint16_t           task_period;                /* Период вызова задачи в тиках                  */
  uint16_t           cnt_period_tx_box;          /* Счетчик периода формирования пакетов          */  
  uint16_t           max_period_tx_box;          /* Период формирования пакетов                   */  
    
  /* Предустановка переменых для формирования шапки пакета */
  uint8_t            ch_id;                      /* Идентификатор канала                          */
  uint16_t           dest_phy_addr;              /* Физический адрес получателя                   */
  uint16_t           src_phy_addr;               /* Физический адрес источника                    */
  uint16_t           gate_phy_addr;              /* Физический адрес шлюза                        */
  uint8_t            priority_box;               /* Приоритет данных пакета                       */
  uint8_t            cnt;                        /* Cчетчик неприрывности пакетов 0..255          */
                    
  /* Собственные переменные идентификации порта            */
  uint8_t            index_port;                 /* Собственный индекс порта                      */
  uint32_t           mask_index_port;            /* Маска доступных для приема портов             */
  uint32_t           mask_chanel;                /* Маска доступных каналов                       */   
  
  uint16_t           cnt_index;                  /* Счетчик заполнения данных                     */   
  
}soft_fifo_test_t;

#if ( ROUTER_SOFT_ENABLE == 1 ) 

/* Объявляем буфер для FIFO */ 
extern soft_fifo_t fifo_soft;    

/**
  * @brief  Функция инициализация буфера FIFO
  * @param  None
  * @retval None
  * 
  */
void InitSoftFIFO( void );

#if ( TEST_SOFT_ROUTER_EN == 1 )  
  /**
   * @brief  Запуск задач тестирования передачи и приема пакетов системы.
   * @param  None
   * @retval None
   */
  void start_test_soft_router(void );
#endif

/**
  * @brief  Функция получения указателя на буффер для чтение
  * @param  uint8_t* index_box  указатель на индекс запращиваемого буффера
  * @param  uint32_t mask_chanel  - маска доступных каналов  
  * @param  uint32_t mask_index_port - маска доступных для приема портов 
  * @retval soft_router_box_t* - указатель на текущий буффер FIFO для чтения 
  */
soft_router_box_t* GetRDPointFIFO( uint8_t* index_box ,uint32_t  mask_chanel ,uint32_t mask_index_port );

/**
  * @brief  Функция запроса указатель на текущий буфер FIFO для записи 
  * @param  none
  * @retval soft_router_box_t* - указатель на текущий буффер FIFO для записи 
  * 
  */
soft_router_box_t* GetWRPointFIFO( void );

/**
 * @brief  Функция оповешения зарегистрировавшихся на рассылку.
 * @param  uint8_t index_src - индекс порта инициатора рассылки
 * @retval None
 */
void notify_soft_port( uint8_t index_src );

/**
 * @brief  Функция регистрации на рассылку.
 * @param  uint8_t index_src - индекс порта 
 * @param  TaskHandle_t  HandleNote - указатель на задачу 
 * @retval None
 */
void reg_notify_port( uint8_t index_src, TaskHandle_t  HandleNote );

#endif /*  ROUTER_SOFT_ENABLE == 1 */
#endif /* __SOFT_ROUTER_H */  
/************************ (C) COPYRIGHT DEX 2020 *****END OF FILE**************/

