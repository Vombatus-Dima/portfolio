/**
  ******************************************************************************
  * @file    router_streams.h
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ROUTER_STREAMS_H
#define __ROUTER_STREAMS_H

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "rs_frame_header.h"
#include "settings.h"
#include "loader_settings.h"

// Максимальное число портов для мультиплексирования
#define MAX_ROUTER_PORT  14

#define BitID(a)                (0x00000001 << a)

// Максимальное значение ID порта маршрутизации пакетов
#define MAX_PORD_ID          15 + 1  //(от 0 до 15)
    
// Максимальное значение ID порта маршрутизации пакетов
#define OFFSET_NOTE_PORD_ID  (16)    

// Период вызова задачи мультиплексирования по таймеру 1 тик = 1 млсек
#define TIME_PERIOD_WORK_ROUTER_THREAD    10    
    // Период вызова функции обновления таблицы дубликатов 100 млсек
#define TIME_PERIOD_UPDATE_TBL_DUBLICATE  100  
    
#if  (LOG_STRIME_ENABLE == 1) 
#define SIZE_QUEUE_LOG_ROUTER  20  // Размер очереди хранения шапок пакетов
    extern QueueHandle_t     QueueLogRouter;      // Указатель на очередь хранения шапок
#endif
  

// Описание структуры буфера для маршрутизируемого пакета
typedef __packed struct 
{
  uint32_t  cnt_id;             // счетчик - идентификатор пакет
  uint32_t  time_entrance;      // время получения
  uint8_t   source_port;        // порт - источник пакета
  router_box_t router_box;      // мультиплексируемый пакет
} router_buf_box_t;    
    
/** 
  * @brief  определение типа для Флага запроса указателя
  */ 
typedef enum
{ 
  FREE_PNT   = 0x0F,
  BUSY_PNT   = 0xF0
}FlagReqPnt_TypeDef;

/** 
  * @brief  определение типа статуса порта маршрутизации
  */ 
typedef enum
{ 
  NOT_INSTALL_PORT = 0x00,
  DISABLE_PORT,  
  ENABLE_PORT 
}PortStatus_TypeDef;

/** 
  * @brief  определение типа статуса обработки собственного физического адреса.
  */ 
typedef enum
{ 
  LOCK_ADDR = 0x00, // Трансляция в порт пакета только с собственны адресом запрещена
  UNLOCK_ADDR,     // Трансляция в порт пакета только с собственны адресом или широковещательного
}FlagLockAddr_TypeDef;
   

// Описание структуры порта для мультиплексирования
typedef struct
{
  FlagReqPnt_TypeDef     FlagReqPnt;          // Флаг запрос указателя
  PortStatus_TypeDef     PortStatus;          // Статус
  uint8_t                PortID;              // Собственный ID порт  
  uint32_t               PortBitID;           // Собственный битовый ID порт
  FlagLockAddr_TypeDef   FlagLockAddr;        // Статус обработки собственного физического адреса.(Для порта CORE установить 1)
  uint32_t               MaskPortID;          // Указатель на маску разрешенных ID портов
  uint8_t                NumMaskBoxID;        // Номер маски разрешенных ID пакетов
  QueueHandle_t          QueueInRouter;	      // Указатель на входящюю очередь
  QueueHandle_t          QueueOutRouter;      // Указатель на исходящюю очередь
  TaskHandle_t           HandleTask;          // Указатель на задачу - используется для отправки сообщений в задачу 
  TaskHandle_t           HandleTaskRouter;    // Указатель на задачу роутера - используется для отправки сообщений в задачу роутера   
}port_router_t;


/* Указатель на задачу роутера */
extern TaskHandle_t  HandleTaskRouter;   

/**
  * @brief  Функция проверки разрешения трансляции в выбраный порт по индексу порта источника.
  * @param  uint32_t     PortBitID;    // Собственный битовый ID порта
  * @param  uint32_t     MaskPortID;   // Маска разрешенных ID портов
  * @retval bool  true - разрешена трансляция
  *               false - запрещена трансляция
  */
bool permit_index_port(uint32_t PortBitID, uint32_t MaskPortID);

/**
  * @brief  Функция проверка разрешения трансляции в выбраный порт по ID пакета
  * @param  uint8_t     id           // Идентификатор пакета
  * @param  uint8_t     umMaskBoxID  // Номер маски разрешенных ID пакетов
  * @retval bool  true - разрешена трансляция
  *               false - запрещена трансляция
  */
bool permit_index_box(uint8_t id, uint8_t umMaskBoxID);

/**
  * @brief  Функция предназначена для выделени порта в таблице маршрутизации.
  * @param  uint8_t* index_port - указатель на переменную для индекса выделяемого порта
  * @retval bool  true - порт выделен
  *               false - нет доступных портов
  */
bool request_port_pnt(uint8_t* index_table);

/**
  * @brief  Функция включение порта маршрутизации.
  * @param  uint8_t index_port - индекс порта маршрутизации
  * @retval bool  true - команда выполнена
  *               false - команда не выполнена
  */ 
bool enable_port_pnt(uint8_t index_port);

/**
  * @brief  Функция выключение порта маршрутизации.
  * @param  uint8_t index_port - индекс порта маршрутизации
  * @retval bool  true - команда выполнена
  *               false - команда не выполнена
  */
bool disable_port_pnt(uint8_t index_port);

/**
  * @brief  Функция обнуления настроек порта маршрутизации.
  * @param  uint8_t index_port - индекс порта маршрутизации
  * @retval bool  true - команда выполнена
  *               false - команда не выполнена
  */
bool reset_port_pnt(uint8_t index_port);

/**
  * @brief Функция предназначена для инициализации порта в таблице маршрутизации.
  * @param uint8_t index_port - индекс порта маршрутизации
  * @param port_router_t* settings_port - указатель на структуру с настройками порта
  * ( !!! параметры FlagReqPnt,PortStatus, в структуре settings_port не задействованы )
  * @retval bool  true - порт прошел инициализацию успешно
  *               false - порт не прошел инициализацию
  */
bool settings_port_pnt(uint8_t index_port , port_router_t* settings_port);

/**
  * @brief  Задача мультиплексирования потоков.
  * @param  None
  * @retval None
  */
void router_thread(void *arg);

/**
  * @brief Эта функция формирования маркетного пакета.
  * @param router_box_t* marker_buf указатель на буфер для маркерного пакета
  * @param uint8_t local_cmd локальная команда
  * @param uint8_t local_data данные локальной команды
  * @retval none	
  */
void CreateMarkerBox(router_box_t* marker_buf, uint8_t local_cmd, uint8_t local_data );

/**
  * @brief Функция корректирует параметры пакета RS перед отправкой.
  * @param router_box_t* rs_box указатель на буфер пакета
  * @param uint8_t* contic указатель на счетчик неприрывности
  * @retval none 
  */
void update_rs_tx_box(router_box_t* rs_box,uint8_t* contic);

/**
  * @brief Функция тестирования пакета.
  * @param router_box_t* box_test указатель на тестируемый пакет
  * @retval uint8_t  0 - отправляем в мультиплексор
  *                  1 - локальный пакет
  *                  2 - маркерный пакет
  *                  3 - ошибка CRC
  *                  4 - неккоректная шапка пакета
  */
uint8_t test_receive_box(router_box_t* box_test);
#endif /* __ROUTER_STREAMS_H */  
/************************ (C) COPYRIGHT DEX 2019 *****END OF FILE**************/

