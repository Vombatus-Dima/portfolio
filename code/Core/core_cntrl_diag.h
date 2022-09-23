/**
  ******************************************************************************
  * @file    core_cntrl_diag.h
  * @author  Trembach Dmitry
  * @version V1.2.0
  * @date    17-03-2019
  * @brief   Файл поддержки диагностики
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2019 DataExpress</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CORE_CNTRL_DIAG_H
#define __CORE_CNTRL_DIAG_H

// Значение аварийно низкого напряжения
#define MIN_VOLT_POWER_ALARM   (900)         //10 В

//      Наименование           Type    N_port (номер порта)  ID_DIAG    Описание        Примечание
//      Ядро
//      CORE_WORK_ALARM         Core    (1)     1       1       Флаги аварий при работе
//      CORE_DAIG                                       2       Диагностика ядра
//      UART    
//      UART_DAIG               UART    (2)     1…255   1       Диагностика UART
//      RF
//      RF_DIAG                 RF      (3)     1       1       Диагностика RF
//      RF_ROUTER_TABLE                                 2       Таблица ближайших роутеров

#define TYPE_CORE_DAIG                (1)
#define ID_CORE_WORK_ALARM            (1)
#define N_PORT_CORE_WORK_ALARM        (1)

#define ID_CORE_DAIG                  (2)
#define N_PORT_CORE_DAIG              (1)
 
#define TYPE_UART_DAIG                (2)
#define ID_UART_DAIG                  (1)
#define N_PORT_UART_DAIG              (1)

#define TYPE_RF_DIAG                  (3)
#define ID_RF_DIAG                    (1)
#define N_PORT_RF_DIAG                (1)
 
#define TYPE_RF_ROUTER_TABLE          (3) 
#define ID_RF_ROUTER_TABLE            (2)
#define N_PORT_RF_ROUTER_TABLE        (1)
    
#define TYPE_ETH_DIAG                 (4)
#define ID_ETH_DIAG                   (1)
#define N_PORT_ETH_DIAG               (1)  

//====================Управление ресурсами с помощью команд=====================

#define MAX_SIZE_QUEUE_CNTRL_CMD      (5) 

// Перечисление режимов работы очереди команд 
typedef enum 
{
  QCMD_NONE = 0,      // очередь отсутствует 
  QCMD_INIT,          // очередь инициализирована	
  QCMD_ENABLE,        // очередь включена
  QCMD_DISABLE,       // очередб отключена 
}Status_QCMD_t;


/* Структура пакета команд */
typedef __packed struct 
{
  uint8_t        PortID;             /* Источник команды          */
  uint8_t        CMD_ID;             /* Идентификатор команды     */
  // Данные команды  
  union
  {
    uint8_t      data_byte[8];       /* Данные в байтовом формате */
    uint16_t     data_word[4];       /* Данные в байтовом формате */
    uint32_t     data_dword[2];      /* Данные в байтовом формате */
  };
}cntrl_cmd_t; 
 
/* Структура очередей команд управления ресурсами */
typedef __packed struct 
{
  Status_QCMD_t  Status_QCMD;
  QueueHandle_t  QueueCMD;
}queue_cntrl_cmd_t;   

/*=============================================================================*/

// Определение зарезервированных команд
#define CMD_REQ_TAB_DIAG                 (0x10)  // Запрос выдачи диагностики 
#define CMD_REQ_TAB_ROUTER               (0x11)  // Запрос выдачи таблицы роутеров
#define CMD_UPDATE_DIAG                  (0x12)  // Команда обновления диагностики
#define CMD_ALARM_PORT                   (0x20)  // Команда аварии порта

#define CMD_RF_ALARM                     (0x21)  // Команда аварии от диспетчера
#define CMD_RF_ALARM_IND                 (0x22)  // Команда аварии индивидуальный от диспетчера
#define CMD_RF_CALL_IND                  (0x23)  // Команда индивидуального вызова от диспетчера


#define CMD_REQ_SAMPLE_ALARM             (0x31)  // Команда выдачи голосового сообщения авария
#define CMD_REQ_SAMPLE_RESP_DISP         (0x32)  // Команда выдачи подтверждения диспетчера
#define CMD_REQ_SAMPLE_DISP_CALL         (0x33)  // Команда выдачи сигнала приема вызова диспетчером

#define MAX_PERIOD_REQ_SAMPLE_ALARM      (15000)  // Период выдачи голосового сообщения авария 
#define MAX_PERIOD_REQ_SAMPLE_DISP_CALL  (15000)  // Период выдачи голосового сообщения вызов диспетчера



#include "router_streams.h"
    
// Пакет запроса диагности DIAGNOSTICS_REQ
typedef __packed struct 
{
  head_box_t     rs_head;        // Шапка RS пакета   
  
  uint16_t       Own_PHY_Addr;   // Собсвенный физический адрес  
  uint8_t        Type;           // Тип ресурса
  uint8_t        N_port;         // Номер порта
  uint8_t        ID_DIAG;        // ID команды диагностики
  uint16_t    crc;               // Контрольная сумма пакета
}req_diagnostics_t;

// Шапка запроса диагности DIAGNOSTICS_RESP
typedef __packed struct 
{
  head_box_t     rs_head;        /*  Шапка RS пакета              */
  uint16_t       Own_PHY_Addr;   /* Собсвенный физический адрес   */ 
  uint8_t        Type;           /* Тип ресурса                   */         
  uint8_t        N_port;         /* Номер порта                   */
  uint8_t        ID_DIAG;        /* ID команды диагностики        */  
  uint8_t        VERSION_ID;     /* Версия пакета                 */ 
  uint8_t        Type_Device;    /* Тип устройсва                 */  
}resp_diag_head_t;

// Пакет CORE_WORK_ALARM - Флаги аварий при работе
typedef __packed struct 
{
  head_box_t     rs_head;        // Шапка RS пакета  
  
  uint16_t       Own_PHY_Addr;   //Собсвенный физический адрес
  uint8_t        Type;           //Тип ресурса
  uint8_t        N_port;         //Номер порта
  uint8_t        ID_DIAG;        //ID команды диагностики
  uint8_t        VERSION_ID;     //Версия пакета
  uint8_t        Type_Device;    //Тип устройсва     
  union
  {
    uint16_t    alarm_flag; //Флаги аварийных режимов ВРМ 
    __packed struct
    {
      uint16_t  alarm_reset_core: 1, // 1.Флаг запуска системы (Перезагрузка)   
                alarm_power     : 1, // 2.Авария питание 
                alarm_uart_a    : 1, // 3.Авария UART 1  
                alarm_uart_b    : 1, // 4.Авария UART 2  
                alarm_uart_c    : 1, // 5.Авария UART 3 
                alarm_uart_d    : 1, // 6.Авария UART 4 
                alarm_rf        : 1, // 7.Авария RF
                alarm_eth       : 1, // 8.Авария ETH 
                alarm_boot      : 1, // 9.Авария Bootloader                 
                alarm_rez       : 7; // резервные биты                  
    };                     
  }; 
 
  uint16_t    crc;               // Контрольная сумма пакета
}resp_core_work_alarm_t;

// Пакет CORE_DAIG - Ответ диагностики ядра
typedef __packed struct 
{
  head_box_t     rs_head;        // Шапка RS пакета  
  
  uint16_t       Own_PHY_Addr;   //Собсвенный физический адрес
  uint8_t        Type;           //Тип ресурса
  uint8_t        N_port;         //Номер порта
  uint8_t        ID_DIAG;        //ID команды диагностики
  uint8_t        VERSION_ID;     //Версия пакета
  uint8_t        Type_Device;    //Тип устройсва   
  uint16_t       MODE;           //Режим работы
  uint16_t       ModeMux;        //Режим работы мультиплексора 
  uint16_t       Own_LOG_Addr;   //Собсвенный логический адрес 
  uint16_t       Value_VCC;      //Напряжение питания  (за время *)
  uint16_t       Value_VCC_min;  //Минимальное напряжение питания (за время *)
  uint32_t       Time_Work;      //Время работы в секундах (после перезапуска или вкл. питания)
  uint32_t       Rezerv_A;       //Резерв
  uint32_t       Rezerv_B;       //Резерв  
  uint32_t       Rezerv_C;       //Резерв
 
  uint16_t    crc;               // Контрольная сумма пакета  
}resp_core_diag_t;

// Пакет UART_DAIG - Ответ диагностики для UART
typedef __packed struct 
{
  head_box_t     rs_head;        // Шапка RS пакета  
  
  uint16_t       Own_PHY_Addr;                  //Собсвенный физический адрес
  uint8_t        Type;                          //Тип ресурса
  uint8_t        N_port;                        //Номер порта
  uint8_t        ID_DIAG;                       //ID команды диагностики
  uint8_t        VERSION_ID;                    //Версия пакета
  uint8_t        Type_Device;                   //Тип устройсва   
  uint16_t       MODE;                          //Режим работы
  uint16_t       ModeMux;                       //Режим работы мультиплексора 
  uint8_t        Connect_Type;                  //Тип соединения 0 - slave; 1 - master
  uint8_t        Status_Dev;                    //Статус подключения  
  uint16_t       Connect_PHY_Addr;              //Физический адрес ВРМ к которому подключен
  uint8_t        rx_buff_max;                   //Макс. загрузка буфера приёмника
  uint32_t       err_rx_seq_cnt;                //Счётчик ошибок последовательности по приёму 
  uint32_t       err_rx_buff_size_cnt;          //Счетчик ошибок размера данных при приёме, (ув. если новая посылка не помещается в текущий свободный буфер)   
  uint32_t       err_rx_crc_cnt;                //Счётчик ошибок CRC по приёму 
  uint8_t        tx_buff_max;                   //Макс. загрузка буфера передатчика 
  uint32_t       err_tx_over_buff_dma_cnt;      //Счётчик ошибок переполнения буфера DMA по передаче       
  uint32_t       polarity_find_cnt;             //Счётчик поиска полярности, увеличивается, если произошёл 
                                                //разрыв связи и начался процесс поиска полярности     
  uint8_t        RS_Load_Cur;                   // % загрузки - текущий  (за время *)   
  uint8_t        RS_Load_Min;                   // % загрузки - минимальный (за время *)      
  uint8_t        RS_Load_Max;                   // % загрузки - максимальный  (за время *)     
  uint32_t       Rezerv;                        //Резерв
 
  uint16_t    crc;               // Контрольная сумма пакета  
}resp_uart_diag_t;

// Пакет UART_DAIG - Ответ диагностики для UART
typedef __packed struct 
{
  head_box_t     rs_head;        // Шапка RS пакета  
  
  uint16_t       Own_PHY_Addr;                  //Собсвенный физический адрес
  uint8_t        Type;                          //Тип ресурса
  uint8_t        N_port;                        //Номер порта
  uint8_t        ID_DIAG;                       //ID команды диагностики
  uint8_t        VERSION_ID;                    //Версия пакета
  uint8_t        Type_Device;                   //Тип устройсва   
  uint8_t        Hard_Mode;                     //Аппаратная предустановка режима
  uint8_t        Connect_Type;                  //Тип соединения 0 - slave; 1 - master
  uint8_t        Status_Dev;                    //Статус подключения  
  uint16_t       Connect_PHY_Addr;              //Физический адрес ВРМ к которому подключен
  uint32_t       err_rx_seq_cnt;                //Счётчик ошибок последовательности по приёму 
  uint32_t       err_rx_buff_size_cnt;          //Счетчик ошибок размера данных при приёме, (ув. если новая посылка не помещается в текущий свободный буфер)   
  uint32_t       err_rx_crc_cnt;                //Счётчик ошибок CRC по приёму 
  uint32_t       polarity_find_cnt;             //Счётчик поиска полярности, увеличивается, если произошёл 
                                                //разрыв связи и начался процесс поиска полярности     
  uint8_t        num_ch_list;                   /* Число прослушиваемых каналов */
  uint8_t        num_tag_rsn;                   /* Число радиостанций           */ 
  uint8_t        num_tag_all;                   /* Число тегов                  */    
  
  uint8_t        RS_Load_Cur;                   // % загрузки - текущий  (за время *)   
  uint8_t        RS_Load_Max;                   // % загрузки - максимальный  (за время *)     
  uint16_t    crc;               // Контрольная сумма пакета  
}resp_uart_diag_v2_t;

//// Пакет  RF_DIAG  - Ответ диагностики для RF
//typedef __packed struct 
//{
//  head_box_t     rs_head;        // Шапка RS пакета  
//  
//  uint16_t       Own_PHY_Addr;                  //Собсвенный физический адрес
//  uint8_t        Type;                          //Тип ресурса
//  uint8_t        N_port;                        //Номер порта
//  uint8_t        ID_DIAG;                       //ID команды диагностики
//  uint8_t        VERSION_ID;                    //Версия пакета
//  uint8_t        Type_Device;                   //Тип устройсва   
//  uint16_t       MODE;                          //Режим работы
//  uint16_t       ModeMux;                       //Режим работы мультиплексора 
//  
// //Для приёмника
//  uint8_t        rx_load_max;                   //Макс. загрузка RF по приёму в % за время UpTime
//  uint8_t        rx_buff_count_max;             //Макс. загрузка буфера приёмника за время UpTime
//  uint32_t       rx_start_cnt;                  //Счётчик принятых признаков начала пакета за всё время
//  uint32_t       rx_start_delta;                //Кол-во принятых признаков начала пакета за время UpTime
//  uint32_t       rx_cnt;                        //Счётчик принятых пакетов всё время
//  uint32_t       rx_delta;                      //Кол-во принятых пакетов за время UpTime 
//  uint16_t       rx_crc_error_cnt;              //Счётчик ошибок CRC по приёму за всё время 
//  uint8_t        rx_error_percent_curr;         //% ошибок по приёму за время UpTime
//  uint8_t        rx_error_percent_avg;          //% ошибок по приёму за время UpTime
//  uint8_t        rx_error_percent_max;          //% ошибок по приёму за время UpTime
//  uint8_t        rx_error_percent;              //% ошибок по приёму за всё время 
//  uint8_t        dublicate_max;                 //Макс. кол-во записей в таблице дубликата за время UpTime
//  uint32_t       Rezerv_Rx;                        //Резерв  
////Для передатчика
//  uint8_t        tx_load_max;                   //Макс. загрузка RF по передаче в % за время UpTime
//  uint8_t        tx_buff_count_max;             //Макс. загрузка буфера передатчика за время UpTime  
//  uint32_t       tx_cnt;                        //Счётчик переданных пакетов за всё время  
//  uint32_t       tx_delta;                      // Кол-во  переданных пакета за время UpTime 
//  uint32_t       tx_acces_fail_cnt;             //Счётчик занятости канала по передаче за всё время  
//  uint32_t       tx_acces_fail_delta;           //Кол-во занятости канала по передаче за время UpTime  
//  uint16_t       tx_time_retransmit_min;        //Мин. время ретрансляции в мсек
//  uint16_t       tx_time_retransmit_avg;        //Среднее время ретрансляции в мсек
//  uint16_t       tx_time_retransmit_max;        //Макс. время ретрансляции в мсек
//  uint8_t        tx_acces_fail_percent_curr;    //Текущий % занятости канала по передаче за время за время UpTime
//  uint8_t        tx_acces_fail_percent_avg;     //Средний % занятости канала по передаче за время за время UpTime
//  uint8_t        tx_acces_fail_percent_max;     //Макс. % занятости канала по передаче за время за время UpTime
//  uint32_t       Rezerv_Tx;                     //Резерв
// 
//  uint16_t    crc;               // Контрольная сумма пакета  
//}resp_rf_diag_t;

// Пакет  RF_DIAG  - Ответ диагностики для RF
typedef __packed struct 
{
  head_box_t     rs_head;        // Шапка RS пакета  
  
  uint16_t       Own_PHY_Addr;                  //Собсвенный физический адрес
  uint8_t        Type;                          //Тип ресурса
  uint8_t        N_port;                        //Номер порта
  uint8_t        ID_DIAG;                       //ID команды диагностики
  uint8_t        VERSION_ID;                    //Версия пакета
  uint8_t        Type_Device;                   //Тип устройсва   
  uint16_t       MODE;                          //Режим работы
  
 //Для приёмника
  uint8_t        rx_load_max;                   //Макс. загрузка RF по приёму в % за время UpTime
  uint32_t       rx_delta;                      //Кол-во принятых пакетов за время UpTime 
  uint8_t        rx_error_percent_curr;         //% ошибок по приёму за время UpTime
  uint8_t        rx_error_percent;              //% ошибок по приёму за всё время 
  //Для передатчика
  uint8_t        tx_load_max;                   //Макс. загрузка RF по передаче в % за время UpTime
  uint32_t       tx_delta;                      // Кол-во  переданных пакета за время UpTime 
  
  uint8_t        num_pos_ch_list;               /* Число каналов в позиционируемых радиостанций */
  uint8_t        num_pos_tag_rsn;               /* Число позиционируемых радиостанций           */ 
  uint8_t        num_pos_tag_all;               /* Число позиционируемых тегов                  */ 
  
  uint8_t        num_ch_list;                   /* Число каналов в наблюдаемых радиостанций */
  uint8_t        num_tag_rsn;                   /* Число наблюдаемых радиостанций           */ 
  uint8_t        num_tag_all;                   /* Число наблюдаемых тегов                  */   
 
  uint16_t    crc;               // Контрольная сумма пакета  
}resp_rf_diag_v2_t;

#define  MAX_SIZE_TAB_NEAR_ROUTER  10 

// Пакет RF_ROUTER_TABLE - Ответ диагностики, таблица соседних ВРМ
typedef __packed struct 
{
  head_box_t     rs_head;        // Шапка RS пакета  
  
  uint16_t       Own_PHY_Addr;   //Собсвенный физический адрес
  uint8_t        Type;           //Тип ресурса
  uint8_t        N_port;         //Номер порта
  uint8_t        ID_DIAG;        //ID команды диагностики
  uint8_t        VERSION_ID;     //Версия пакета
  uint8_t        Type_Device;    //Тип устройсва 
  
  diagn_near_router_t  router_near[MAX_SIZE_TAB_NEAR_ROUTER];  // Таблица соседних роутеров
 
  uint16_t    crc;               // Контрольная сумма пакета
}resp_rf_near_diag_t;

/**
  * @brief Функция формирования пакета CORE_WORK_ALARM - Флаги аварий при работе
  * @param none
  * @retval none
  */
void GenDiagCoreWorkAlarm(void);

/**
  * @brief Функция отправки пакета CORE_WORK_ALARM - Флаги аварий при работе
  * @param none
  * @retval none
  */
void SendDiagCoreWorkAlarm(void);

/**
  * @brief Функция формирования CORE_DAIG - Ответ диагностики ядра
  * @param none
  * @retval none
  */
void GenDiagCore(void);

/**
  * @brief Функция отправки CORE_DAIG - Ответ диагностики ядра
  * @param none
  * @retval none
  */
void SendDiagCore(void);

/**
  * @brief Функция формирования пакета диагностики ресурса ETH
  * @param None
  * @retval None
  */
void GenRespDiagETH(void);

#endif /* __CORE_CNTRL_DIAG_H */
/******************* (C)  COPYRIGHT 2019 DataExpress  *****END OF FILE****/
