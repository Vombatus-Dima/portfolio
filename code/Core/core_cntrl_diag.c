/**
  ******************************************************************************
  * @file    core_cntrl_diag.c
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

#include "main.h"
#include "router_streams.h"
#include "FreeRTOS.h"
#include "task.h"
#include "loader_settings.h"
#include "settings.h"
#include "core_cntrl_diag.h"
#include "driver_adc.h"
#include "Core.h"
#include "udp_voice.h"
#include "printf_dbg.h" 
#include "core_cntrl_dev.h"  
#include "pre_set_rs.h"

// Обявляем очередь для получения значений измерения напряжения
extern QueueHandle_t QueueADC;

// Структура ядра
extern core_struct_t  st_core;

uint32_t   control_diag_time;  // Переменная отсчета периода запроса диагностики
uint32_t   control_led_time;   // Переменная отсчета периода обновления светодиодов
bool       flag_control_led;   // Флаг контроля обновления светодиодов 

// Массив обработанных значений кодов АЦП
adc_value_struct_t  arr_adc_rx[NumberChADC];
// Нормированные значения напряжения в [10mV]
uint16_t  VoltageInputMin = 0;
uint16_t  VoltageInputAverag = 0;
uint16_t  VoltageFiveMin = 0;
uint16_t  VoltageFiveAverag = 0;

/**
  * @brief Функция формирования пакета CORE_WORK_ALARM - Флаги аварий при работе
  * @param none
  * @retval none
  */
void GenDiagCoreWorkAlarm(void)
{
     // Формирование пакета
    st_core.resp_core_work_alarm.rs_head.pre = 0xAA55;     // Преамбула  0x55 0xAA
    st_core.resp_core_work_alarm.rs_head.lenght = sizeof(resp_core_work_alarm_t) - SIZE_LENGHT - SIZE_PRE;  // Длина пакета без данных
    st_core.resp_core_work_alarm.rs_head.id = ID_DIAGNOSTICS_RESP;   // ID ответ команды конфигурации
    st_core.resp_core_work_alarm.rs_head.dest = 0xFFFF;    // Широковещательный  адрес получателя
    st_core.resp_core_work_alarm.rs_head.src = DataLoaderSto.Settings.phy_adr;     // Устанавливаем свой физический адрес источника
    // Физический адрес источника и Cчетчик неприрывности пакетов 0..255 заполняется функцией update_rs_tx_box
    st_core.resp_core_work_alarm.rs_head.reserv = 0x00;     // 
    st_core.resp_core_work_alarm.rs_head.status_box = 0x00; //  
    // Заполнение тела пакета
    st_core.resp_core_work_alarm.Own_PHY_Addr = DataLoaderSto.Settings.phy_adr;    //Собсвенный физический адрес
    st_core.resp_core_work_alarm.Type = TYPE_CORE_DAIG;                            //Тип ресурса
    st_core.resp_core_work_alarm.N_port = N_PORT_CORE_WORK_ALARM;                  //Номер порта
    st_core.resp_core_work_alarm.ID_DIAG = ID_CORE_WORK_ALARM;                     //ID команды диагностики
    st_core.resp_core_work_alarm.VERSION_ID = 0x01;                                //Версия пакета
    st_core.resp_core_work_alarm.Type_Device = DEVICE_TYPE;                        //Тип устройсва  
    st_core.resp_core_work_alarm.alarm_reset_core = st_core.alarm_reset_core;                        // 1.Флаг запуска системы (Перезагрузка)
    
    if (st_core.alarm_reset_core)
    {
      st_core.resp_core_work_alarm.alarm_power = 0;                                  // 2.Авария питание
      st_core.resp_core_work_alarm.alarm_uart_a = 0;                                 // 3.Авария UART 1
      st_core.resp_core_work_alarm.alarm_uart_b = 0;                                 // 4.Авария UART 2
      st_core.resp_core_work_alarm.alarm_uart_c = 0;                                 // 5.Авария UART 3
      st_core.resp_core_work_alarm.alarm_uart_d = 0;                                 // 6.Авария UART 4
      st_core.resp_core_work_alarm.alarm_rf =  0;                                    // 7.Авария RF
      st_core.resp_core_work_alarm.alarm_eth =  0;                                   // 8.Авария ETH
      st_core.resp_core_work_alarm.alarm_boot = 0;                                   // 9.Авария Bootloader   
      st_core.resp_core_work_alarm.alarm_rez = 0x00;                                 // резервные биты    
      
    }
    else
    {
      
    st_core.resp_core_work_alarm.alarm_power = st_core.alarm_power;                                  // 2.Авария питание
    st_core.resp_core_work_alarm.alarm_uart_a = st_core.alarm_uart_a;                                // 3.Авария UART 1
    st_core.resp_core_work_alarm.alarm_uart_b = 0;                                                   // 4.Авария UART 2
    st_core.resp_core_work_alarm.alarm_uart_c = 0;                                                   // 5.Авария UART 3
    st_core.resp_core_work_alarm.alarm_uart_d = 0;                                                   // 6.Авария UART 4
    st_core.resp_core_work_alarm.alarm_rf = st_core.alarm_rf;                                        // 7.Авария RF
    st_core.resp_core_work_alarm.alarm_eth = st_core.alarm_eth;                                      // 8.Авария ETH
    st_core.resp_core_work_alarm.alarm_boot = 0;                                                     // 9.Авария Bootloader   
    st_core.resp_core_work_alarm.alarm_rez = 0x00;                                                   // резервные биты
    }  
    // Обнуление флагов аварийных режимов
    st_core.alarm_flag = 0x0000; 
}

/**
  * @brief Функция отправки пакета CORE_WORK_ALARM - Флаги аварий при работе
  * @param none
  * @retval none
  */
void SendDiagCoreWorkAlarm(void)
{
    // Обновление RS пакета перед отправкой
    update_rs_tx_box((void *)&(st_core.resp_core_work_alarm),&(st_core.contic_tx_box));
    
    // Отправляетм пакет в роутер
    if (st_core.xQueue_core_router != NULL)
    {
      // отправить пакет в роутер
      xQueueSend ( st_core.xQueue_core_router, ( void * )&(st_core.resp_core_work_alarm) , ( TickType_t ) 0 );
    }
}

/**
  * @brief Функция формирования CORE_DAIG - Ответ диагностики ядра
  * @param none
  * @retval none
  */
void GenDiagCore(void)
{
    // Формирование пакета
    st_core.resp_core_diag.rs_head.pre = 0xAA55;                             // Преамбула  0x55 0xAA
    st_core.resp_core_diag.rs_head.lenght = sizeof(resp_core_diag_t) - SIZE_LENGHT - SIZE_PRE;  // Длина пакета без данных
    st_core.resp_core_diag.rs_head.id = ID_DIAGNOSTICS_RESP;                 // ID ответ команды конфигурации
    st_core.resp_core_diag.rs_head.dest = 0xFFFF;                            // Широковещательный  адрес получателя
    st_core.resp_core_diag.rs_head.src = DataLoaderSto.Settings.phy_adr;     // Устанавливаем свой физический адрес источника
    // Физический адрес источника и Cчетчик неприрывности пакетов 0..255 заполняется функцией update_rs_tx_box
    st_core.resp_core_diag.rs_head.reserv = 0x00;     // 
    st_core.resp_core_diag.rs_head.status_box = 0x00; //  
    // Заполнение тела пакета
    st_core.resp_core_diag.Own_PHY_Addr = DataLoaderSto.Settings.phy_adr;    //Собсвенный физический адрес
    st_core.resp_core_diag.Type = TYPE_CORE_DAIG;                            //Тип ресурса
    st_core.resp_core_diag.N_port = N_PORT_CORE_DAIG;                        //Номер порта
    st_core.resp_core_diag.ID_DIAG = ID_CORE_DAIG;                           //ID команды диагностики
    st_core.resp_core_diag.VERSION_ID = 0x01;                                //Версия пакета
    st_core.resp_core_diag.Type_Device = DEVICE_TYPE;                        //Тип устройсва  
    st_core.resp_core_diag.MODE = 0x0000;;                                   //Режим работы
    st_core.resp_core_diag.ModeMux = 0x0000;;                                //Режим работы мультиплексора     
    st_core.resp_core_diag.Own_LOG_Addr = DataSto.Settings.mac_adr;                        //Собсвенный логический адрес     
    st_core.resp_core_diag.Value_VCC = VoltageInputAverag;                   //Напряжение питания  (за время *)
    st_core.resp_core_diag.Value_VCC_min =VoltageInputMin;                   //Минимальное напряжение питания (за время *)
    st_core.resp_core_diag.Time_Work = SysWorkTime.all_second;               //Время работы в секундах (после перезапуска или вкл. питания)
    st_core.resp_core_diag.Rezerv_A = 0;                                     //Резерв
    st_core.resp_core_diag.Rezerv_B = 0;                                     //Резерв  
    st_core.resp_core_diag.Rezerv_C = 0;                                     //Резерв 

    if ( VoltageInputMin < MIN_VOLT_POWER_ALARM)
    {
      // Установка флага аварийного режима - низкое напряжение
      st_core.alarm_power = 1;
    }
    
    // Обнуление переменных контроля напряения питания
    //VoltageInputAverag = 0;
    VoltageInputMin = 0;
}

/**
  * @brief Функция отправки CORE_DAIG - Ответ диагностики ядра
  * @param none
  * @retval none
  */
void SendDiagCore(void)
{
    // Обновление RS пакета перед отправкой
    update_rs_tx_box((void *)&(st_core.resp_core_diag),&(st_core.contic_tx_box));
    
    // Отправляетм пакет в роутер
    if (st_core.xQueue_core_router != NULL)
    {
      // отправить пакет в роутер
      xQueueSend ( st_core.xQueue_core_router, ( void * )&(st_core.resp_core_diag) , ( TickType_t ) 0 );
    }    
}
//=============================== Диагностика ETH ==============================
/**
  * @brief Функция отправки пакета диагностики ресурса ETH
  * @param none
  * @retval none
  */
void SendDiagETH(void)
{
    // Обновление RS пакета перед отправкой
    update_rs_tx_box((void *)&(st_core.resp_eth_diag),&(st_core.contic_tx_box));
    
    // Отправляетм пакет в роутер
    if (st_core.xQueue_core_router != NULL)
    {
      // отправить пакет в роутер
      xQueueSend ( st_core.xQueue_core_router, ( void * )&(st_core.resp_eth_diag) , ( TickType_t ) 0 );
    }
}   

// Функция диагностики соединения ETH.
extern void LinkDiagnostic( uint8_t  *pCntNoLink , uint32_t  *pTimeLink );
// Функция запроса статуса соединения по ETH.
extern bool StatusLinkETH( void );
// Функция запроса статуса соединения по RS.
extern bool StatusLinkRS( void );

/**
  * @brief Функция формирования пакета диагностики ресурса ETH
  * @param None
  * @retval None
  */
void GenRespDiagETH(void)
{
  // Переменные для временного хранения значений счетчиков
  uint32_t       TempVarUI32_a;             
  uint8_t        TempVarUI8;             

  // Формирование пакета
  st_core.resp_eth_diag.rs_head.pre = 0xAA55;                                               // Преамбула  0x55 0xAA
  st_core.resp_eth_diag.rs_head.lenght = sizeof(resp_eth_diag_t) - SIZE_LENGHT - SIZE_PRE;  // Длина пакета без таблицы роутеров
  st_core.resp_eth_diag.rs_head.id = ID_DIAGNOSTICS_RESP;                                   // ID ответ команды конфигурации
  st_core.resp_eth_diag.rs_head.dest = 0x0000;                                              // Широковещательный  адрес получателя
  st_core.resp_eth_diag.rs_head.src = DataLoaderSto.Settings.phy_adr;                       // Устанавливаем свой физический адрес источника
  st_core.resp_eth_diag.rs_head.reserv = 0x00;     //
  st_core.resp_eth_diag.rs_head.status_box = 0x00; //  
  st_core.resp_eth_diag.own_physical_addr = DataLoaderSto.Settings.phy_adr;                 //Собсвенный физический адрес
  st_core.resp_eth_diag.Type = TYPE_ETH_DIAG;                                               //Тип ресурса
  st_core.resp_eth_diag.N_port = N_PORT_ETH_DIAG;                                           //Номер порта
  st_core.resp_eth_diag.ID_DIAG = ID_ETH_DIAG;                                              //ID команды диагностики
  st_core.resp_eth_diag.VERSION_ID = 2;                                                     //Версия пакета
  st_core.resp_eth_diag.Type_Device = DEVICE_TYPE;                                          //Тип устройсва  
  
  // Функция диагностики соединения - запрос счетчика потерь линка и времени работы без потери линка 
  LinkDiagnostic( &TempVarUI8 , &TempVarUI32_a );

  st_core.resp_eth_diag.CntNoLink = TempVarUI8;                                              // Счетчик потерь линка
  st_core.resp_eth_diag.TimeLink = TempVarUI32_a;                                            // Время работы без потери линка  
  /* Заполнение параметров таблиц рассылки по UDP */ 
  UDPSpamDiagnostic(&(st_core.resp_eth_diag));                                                                      
  
  st_core.resp_eth_diag.NumSpamAddrUpload = 0; 
}
//===============================================================================

/**
* @brief  Функция инициализхации параметров диагностики ядра
* @param  None
* @retval None
*/
void InitDiagnostic( void )
{
  // Переменная отсчета периода запроса диагностики
  control_diag_time = 1000;  // 1 сек
  // Запуск отсчета периода обновления диагностики
  control_diag_time = 10;//(uint32_t)DataSto.Settings.time_update_diagn;   
  // Обнуление флагов аварийных режимов
  st_core.alarm_flag = 0x0000;
  // Установка флага перезапуска системы
  st_core.alarm_reset_core = 1;
    
  // Открытие очереди для получения массива выборок АЦП для контроля напряжения
  if (QueueADC == NULL)
  {
    QueueADC =  xQueueCreate(2 , sizeof(arr_adc_rx));
    // Инициализация и запуск драйвера ADC
    //ADC_Config();
  }
}

/**
  * @brief Функция обработка запросов диагностики
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  *
  * @retval bool true  - пакет обработан
  *              false - пакет не обработан
  */
bool ProcessingDiagnosticBox(core_struct_t* core_st)
{
  // Проверка размера пакета
  if ((core_st->data_rx.lenght) != (sizeof(req_diagnostics_t) - SIZE_LENGHT - SIZE_PRE)) return false;
  
  // Проверка ID RS Запрос команд диагностики
  if ((core_st->data_rx.id) != ID_DIAGNOSTICS_REQ) return false;
  
  // получаем указатель на запрос диагностики
  req_diagnostics_t  *ReqDiag = ((req_diagnostics_t*)&(core_st->data_rx)); 
  
   // Проверка адреса запроса команды диагностики
  if ((ReqDiag->Own_PHY_Addr) != DataLoaderSto.Settings.phy_adr) return true; 
  
  // Анализ типа ресурса диагностики
  switch (ReqDiag->Type)
  {
  case TYPE_CORE_DAIG: // Диагностика ядра
    // Анализ ID команды диагностики
    switch (ReqDiag->ID_DIAG)
    {
    case ID_CORE_WORK_ALARM: // Диагностика ядра
      // Функция отправки пакета CORE_WORK_ALARM - Флаги аварий при работе
      SendDiagCoreWorkAlarm();
      break; 	
    case ID_CORE_DAIG: // Диагностика UART
      // Функция отправки CORE_DAIG - Ответ диагностики ядра
      SendDiagCore();
      break; 
    }  
    break; 
  case TYPE_UART_DAIG: // Диагностика UART
    // Анализ ID команды диагностики
    switch (ReqDiag->ID_DIAG)
    {
//    case ID_UART_DAIG: // Диагностика ядра
//      // Отправка запроса выдачи диагностики UART 
//      if ((BaseQCMD[RSPortID].QueueCMD != NULL)&&(BaseQCMD[RSPortID].Status_QCMD == QCMD_ENABLE))
//      {
//        // Подготовка команды команды
//        core_st->data_core_tx_cmd.CMD_ID = CMD_REQ_TAB_DIAG;     // Тип команды
//        core_st->data_core_tx_cmd.PortID = CorePortID;          // Источник команды
//        core_st->data_core_tx_cmd.data_word[0] = 0xFFFF;        // Отправка диагностики на сервер
//        
//        // Отправка команды
//        xQueueSend ( BaseQCMD[RSPortID].QueueCMD, ( void * )&(core_st->data_core_tx_cmd) , ( TickType_t ) 0 );
//      }
//      break;       
    }	
    break; 
    
  case TYPE_ETH_DIAG: // Диагностика ETH
    // Анализ ID команды диагностики
    switch (ReqDiag->ID_DIAG)
    {
    case ID_ETH_DIAG: 
      // Функция отправки пакета диагностики ресурса ETH
      SendDiagETH();
      break;       
    }  
    break;  
  } 
  /* пакет обработан */
  return true;   
}

/**
  * @brief  Функция обновления состояния светодиодов
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  * @retval None
  */
void UpdateStatusLED(core_struct_t*  core_st)
{
  // Отсчет периода отработки режимов светодиода индикации
  if ( (control_led_time) > 0)
  {
    // Отсчет периода
    if ((control_led_time) > (core_st->control_time))
    {
      // Уменьшение периода диагностики на период контроля задачи
      control_led_time = control_led_time - core_st->control_time;
    }
    else
    {
      // Период исчерпан
      control_led_time = 0;
    }
  }
  else
  {     
//    if (flag_control_led)
//    {
//      flag_control_led = false;
//      // Функция запроса статуса соединения по ETH.
//      if (StatusLinkETH())
//      {
//        LED_GREEN_ON;
//      }
//      else
//      {
//        LED_GREEN_OFF;        
//      }  
//      // Функция запроса статуса соединения по RS.
//      if (StatusLinkRS())
//      {
//        LED_RED_ON;
//      }
//      else
//      {
//        LED_RED_OFF;        
//      } 
//    }
//    else
//    {
//      flag_control_led = true;
//      LED_GREEN_ON;
//      LED_RED_ON;      
//    }  
    // Запуск отсчета периода
    control_led_time = 1000; 
  }
}

#if (VER_BOARD == 101)  
#define INDEX_POWER_CONTROL  (0)  
#define MULT_KOFF  (1.1) 
#define ADD_COFF   (100.0) 
#endif  

#if (VER_BOARD == 170)  
#define INDEX_POWER_CONTROL  (0)  
#define MULT_KOFF  (1.1) 
#define ADD_COFF   (100.0) 
#endif  

#if (VER_BOARD == 200)  
#define INDEX_POWER_CONTROL  (0)	
#define MULT_KOFF  (1.1) 
#define ADD_COFF   (100.0) 
#endif  

#if (VER_BOARD == 510)  
#define INDEX_POWER_CONTROL  (1)
#define MULT_KOFF  (0.6) 
#define ADD_COFF   (100.0) 
#endif  

/**
  * @brief  Функция получения и обработки данных ADC
* @param  core_struct_t*  core_st - указатель на структуру ядра
* @retval None
*/
void Core_ADC_Control(core_struct_t*  core_st)
{
  uint16_t temp_val_min;
  uint16_t temp_val_averag;  
  
  //======================= Работа с напряжением ядра ==========================
  
  // Получаем из очереди ADC
  if (xQueueReceive(QueueADC,(void *)&arr_adc_rx[0], 0 ))
  {

    temp_val_averag =    (uint16_t)((((float)arr_adc_rx[INDEX_POWER_CONTROL].average_value)*(3300.0/4096.0)*MULT_KOFF) + ADD_COFF); // 100 - падение напряжения на входном диоде = 1В
    temp_val_min = (uint16_t)((((float)arr_adc_rx[INDEX_POWER_CONTROL].min_value)*(3300.0/4096.0)*MULT_KOFF) + ADD_COFF);     // 100 - падение напряжения на входном диоде = 1В
    
    // Нормирование и усреднение  значений
    
    // Для минимального значения выбираем минимум
    if ((VoltageInputMin > temp_val_min)||(VoltageInputMin == 0))
    {
      VoltageInputMin = temp_val_min;
    }
    
    // Для среднего проводим усреднение
    if (VoltageInputAverag == 0)
    {
      VoltageInputAverag = temp_val_averag;
    }
    else
    {  
      VoltageInputAverag = VoltageInputAverag - (VoltageInputAverag>>1) + (temp_val_averag>>1);
    }
    
    // Для минимального значения выбираем минимум       
    if ((VoltageFiveMin > temp_val_min)||(VoltageFiveMin == 0))
    {
      VoltageFiveMin = temp_val_min;
    }    
    
    // Для среднего проводим усреднение    
    if (VoltageInputAverag == 0)
    {
      VoltageFiveAverag = temp_val_averag;
    }
    else
    {
      VoltageFiveAverag = VoltageFiveAverag - (VoltageFiveAverag>>1) + (temp_val_averag>>1);
    }  
  }
  //============================================================================

  // Функция обновления состояния светодиодов
  UpdateStatusLED(core_st);
  }
  
/**
  * @brief  Функция обновления параметров диагностики ядра
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  * @retval None
  */
void UpdateDiagnostic(core_struct_t*  core_st)
{
    // Отправляем всем ресурсам команду - обновить диагностику
    // Подготовка команды команды
    core_st->data_core_tx_cmd.CMD_ID = CMD_UPDATE_DIAG;         // Тип команды
    core_st->data_core_tx_cmd.PortID = CorePortID;              // Источник команды
    core_st->data_core_tx_cmd.data_dword[0] = control_diag_time; // Интервал обновления диагностики 

    // Цикл по всем доступным ресурсам
    for( uint8_t cnt_queue_cmd = 0; cnt_queue_cmd < MAX_NUMBER_PortID ;cnt_queue_cmd++)
    {
      if ((BaseQCMD[cnt_queue_cmd].QueueCMD != NULL)&&(BaseQCMD[cnt_queue_cmd].Status_QCMD == QCMD_ENABLE))
      {
        // Отправка команды
        xQueueSend ( BaseQCMD[cnt_queue_cmd].QueueCMD, ( void * )&(core_st->data_core_tx_cmd) , ( TickType_t ) 0 );
      }  
    }  
    // Подготовка и отправка диагностического собщения
    // Функция формирования пакета диагностики ресурса ETH
    GenRespDiagETH();
    // Функция формирования CORE_DAIG - Ответ диагностики ядра
    GenDiagCore();    
    // Функция формирования пакета CORE_WORK_ALARM - Флаги аварий при работе
    GenDiagCoreWorkAlarm();
    // Функция отправки пакета CORE_WORK_ALARM - Флаги аварий при работе
    SendDiagCoreWorkAlarm();
    
    /* Функция записи собственных параметров в таблицу устройств* */
    SetOwnDevTable(  core_st->resp_core_work_alarm.alarm_flag,/*uint16_t alarm_flag - флаги аварийных режимов                        */
                    core_st->resp_core_diag.Value_VCC,        /*uint16_t Value_VCC - напряжение питания  (за время *)                */
                    core_st->resp_core_diag.Value_VCC_min,    /*uint16_t Value_VCC_min - минимальное напряжение питания (за время *) */
                    
#if (DEVICE_TYPE == PDH_B )                     
                    0,      /*uint16_t NearPhyAdd1 - адрес сосед по порту RS 1                     */
                    0,      /*uint16_t NearPhyAdd2 - адрес сосед по порту RS 2                     */
#endif                    
                    
#if (DEVICE_TYPE == VRM_V )                     
                    GetNearPhyAddA(),      /*uint16_t NearPhyAdd1 - адрес сосед по порту RS 1                     */
                    0,      /*uint16_t NearPhyAdd2 - адрес сосед по порту RS 2                     */
#endif 
                    
#if (DEVICE_TYPE == PI_ETH ) || (DEVICE_TYPE == CODEC )                     
                    GetNearPhyAddA(),  /*uint16_t NearPhyAdd1 - адрес сосед по порту RS 1                     */
                    GetNearPhyAddB(),  /*uint16_t NearPhyAdd2 - адрес сосед по порту RS 2                     */
#endif                     
                    
                    0,      /*uint16_t NearPhyAdd3 - адрес сосед по порту RS 3                     */
                    0 );    /*uint16_t NearPhyAdd4 - адрес сосед по порту RS 4                     */
  }
/******************* (C) COPYRIGHT 2019 DataExpress *****END OF FILE****/
