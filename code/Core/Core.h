/**
 ******************************************************************************
 * @file    Core.h
 * @author  Trembach Dmitry
 * @version V1.1.0
 * @date    08-02-2019
 * @brief   Инициализация драйвера для задачи Ядра
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2019 DataExpress</center></h2>
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CORE_H
#define __CORE_H

/* Includes ------------------------------------------------------------------*/
#include "router_streams.h"
#include "stm32f4xx.h"
#include "core_cntrl_diag.h"
#include "router_streams.h"
#include "event_groups.h"

/* Период обновление времени задачи */
#define TIME_CORE_UPDATE        (1000)

#define ADDRES_BROADCASTING     0xFFFF  // Широковещательный адрес
//#define ADDRES_LOG              0xFE02  // логический адрес

// Массив указателей очередей для команд управления ресурсами
extern queue_cntrl_cmd_t BaseQCMD[MAX_NUMBER_PortID]; 

// Определяем группу событий - команд запроса
extern EventGroupHandle_t xCoreOutEvent;
// Определяем группу событий - команд ответа
extern EventGroupHandle_t xCoreInEvent;

// Определяем группу событий - команд ответа
// За каждым ресурсом закрепляем 3 бита
extern EventGroupHandle_t xCoreInEvent;

// Маски ID_RS пакетов трансляции
extern mask_box_id_t                              tabl_mask_box_id[MAX_NUM_MASK_BOX_ID];

//базовая структура ядра
typedef struct 
{
  uint8_t         size_queue_router_core; // Зададим размер очереди маршрутизатор - rs
  uint8_t         size_queue_core_router; // Зададим размер очереди rs - маршрутизатор
  
  QueueHandle_t   xQueue_router_core;     // Очереди  для передачи данных router to core
  QueueHandle_t   xQueue_core_router;     // Очереди  для передачи данных core to router
  
  QueueHandle_t   xQueueCoreCMD;          // Открытие очереди для приема MAX_SIZE_QUEUE_CNTRL_CMD комманд
  cntrl_cmd_t     data_core_rx_cmd;       // Буфер приема данных комманды 
  cntrl_cmd_t     data_core_tx_cmd;       // Буфер подготовки для передачи данных комманды  
  
  TimerHandle_t   xSoftTimer;             /*  Програмный таймер периодического уведомления задачи     */
  TimerHandle_t   xSoftTimerDiag;         /*  Програмный таймер запроса контроля диагностики          */     
  TimerHandle_t   xSoftTimerPosition;     /*  Програмный таймер формирования отчета позиционирования  */   
  TimerHandle_t   xSoftTimerPing;         /*  Програмный таймер запроса пинга устройств RS            */ 
  
  uint32_t        NotifiedValue;          /* Содержимое сообщения полученного задачей                 */
  
  //------------------------- Буфер приема данных ----------------------------
  union
  {
    uint8_t                   damp_data_rx[sizeof(router_box_t)]; // Буфер приема данных как байтовый массив
    router_box_t 	      data_rx; 	                               // Буфер приема данных
    req_diagnostics_t         req_diagnostics_rx;                      // Пакет запроса диагности DIAGNOSTICS_REQ
    resp_core_work_alarm_t    resp_core_work_alarm_rx;                 // Пакет CORE_WORK_ALARM - Флаги аварий при работе
    resp_core_diag_t          resp_core_diag_rx;                       // Пакет CORE_DAIG - Ответ диагностики ядра
    resp_uart_diag_v2_t       resp_uart_diag_rx;                       // Пакет UART_DAIG - Ответ диагностики для UART
    resp_rf_diag_v2_t         resp_rf_diag_rx;                         // Пакет RF_DIAG  - Ответ диагностики для RF
    resp_rf_near_diag_t       resp_rf_near_diag_rx;                    // Пакет RF_ROUTER_TABLE - Ответ диагностики, таблица соседних ВРМ
    resp_eth_diag_t           resp_eth_diag_rx;                        // Пакет ETH_DAIG - Ответ диагностики для ETH
    RSFrameMODBUS_t           req_modbus_rx;                           // Пакет modbus - запрос
    RSFrameSETUP_t            req_nwk_setup_rx;                        // Пакет nwk_setup - запрос
    setup_box_t               req_setup_rx;                            // Пакет setup - запрос
    RSFrameUPLOAD_t           req_nwk_upload_rx;                       // Пакет nwk_upload - запрос
    rs_upload_box_t           req_upload_rx;                           // Пакет upload - запрос
  };
  
  uint8_t 		      contic_rx_box;            // Переменная для хранения состояния счетчика неприрывности принятых пакетов
  uint8_t 		      flag_start_cnt_rx_box;    // Переменная для хранения состояния счетчика неприрывности принятых пакетов
  //--------------------------------------------------------------------------
  //------------------------ Буфер передачи данных ---------------------------
  union
  {
    uint8_t                   damp_data_tx[sizeof(router_box_t)]; // Буфер приема данных как байтовый массив
    router_box_t 	      data_tx; 	                               // Буфер передачи данных
    req_diagnostics_t         req_diagnostics_tx;                      // Пакет запроса диагности DIAGNOSTICS_REQ
    resp_core_diag_t          resp_core_diag_tx;                       // Пакет CORE_DAIG - Ответ диагностики ядра
    resp_core_work_alarm_t    resp_core_work_alarm_tx;                 // Пакет CORE_WORK_ALARM - Флаги аварий при работе      
    resp_uart_diag_v2_t       resp_uart_diag_tx;                       // Пакет UART_DAIG - Ответ диагностики для UART
    resp_rf_diag_v2_t         resp_rf_diag_tx;                         // Пакет  RF_DIAG  - Ответ диагностики для RF
    resp_rf_near_diag_t       resp_rf_near_diag_tx;                    // Пакет RF_ROUTER_TABLE - Ответ диагностики, таблица соседних ВРМ
    resp_eth_diag_t           resp_eth_diag_tx;                        // Пакет ETH_DAIG - Ответ диагностики для ETH
    RSFrameMODBUS_t           resp_modbus_tx;                          // Пакет modbus - ответ    
    RSFrameSETUP_t            req_nwk_setup_tx;                        // Пакет nwk_setup - ответ
    setup_box_t               req_setup_tx;                            // Пакет setup - ответ 
    RSFrameUPLOAD_t           req_nwk_upload_tx;                       // Пакет nwk_upload - ответ
    rs_upload_box_t           req_upload_tx;                           // Пакет upload - ответ 
  };
  
  uint8_t 		      contic_tx_box;    // Переменная для хранения состояния счетчика неприрывности передаваемых пакетов
  //--------------------------------------------------------------------------
  //-----------------  Флаги аварийных статусов  -----------------------------  
  union
  {
    uint16_t    alarm_flag; //Флаги аварийных режимов ВРМ 
    struct
    {
      uint16_t  alarm_reset_core: 1, // 1.Флаг запуска системы (Перезагрузка)   
                alarm_power     : 1, // 2.Авария питание 
                alarm_uart_a    : 1, // 3.Авария UART 1  
                alarm_uart_b    : 1, // 4.Авария UART 2  
                alarm_uart_c    : 1, // 5.Авария UART 3 
                alarm_uart_d    : 1, // 6.Авария UART 4 
                alarm_rf        : 1, // 7.Авария RF
                alarm_eth       : 1, // 8.Авария ETH                
                alarm_rez       : 8; // резервные биты                  
    };                      
  }; 
  //----------------------------------------------------------------------------
  /* Структура статистики по тегам и каналам по конкректному порту */
  tag_chanel_stat_t        tag_ch_stat[MAX_TAG_CH_TAB];
  
  //-------  Буфер с данными о текущем состоянии диагностики ядра  -------------  
  req_diagnostics_t         req_diag;                             /* Пакет запрос диагностики                        */ 
  resp_core_diag_t          resp_core_diag;                       /* Пакет CORE_DAIG - Ответ диагностики ядра        */  
  resp_core_work_alarm_t    resp_core_work_alarm;                 /* Пакет CORE_WORK_ALARM - Флаги аварий при работе */  
  resp_eth_diag_t           resp_eth_diag;                        /* Пакет Ответ диагностики для ETH                 */ 
  //----------------------------------------------------------------------------  
    
  // переменная для указателя на порт роутера для маршрутизации
  uint8_t                     index_router_port;
  // структура для инициализации порта роутера для маршрутизации
  port_router_t               set_port_core_router;
  
  /* Актуальная маска каналов */
  uint32_t                    MaskCodeChReg;
  /* Переменная для временного хранения обновленной маски */
  uint32_t                    TempMaskCodeChReg;
  
  const char * pcName;        //текстовое имя задачи (для удобства отладки)
  uint16_t   control_time;    //период активности задачи в тиках системы
}core_struct_t;

// Структура ядра
extern core_struct_t  st_core;

/**
 * @brief  Функция предназначена тестовых таблиц маршрутизации по ID пакета.
 * @param  none
 * @retval none
 */
void init_mask_id(void);

/**
 * @brief  Функция инициализации задачи ядра
 * @param  None
 * @retval None
 */
void InitCore( void );
#endif /* __CORE_H */
/******************* (C)  COPYRIGHT 2019 DataExpress  *****END OF FILE****/
