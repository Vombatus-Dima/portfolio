/**
  ******************************************************************************
  * @file    uart_diag.c
  * @author  Trembach Dmitry
  * @version V2.6.0
  * @date    12-12-2020
  * @brief   Функции диагностики UART в режимах RS485/RS422
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
#include "core_cntrl_rsn.h"

#if RS_GATE_ENABLE == 1  

/**
  * @brief Функция формирования пакета диагностики ресурса UART 
  * @param  RS_struct_t* rs_port_box - указатель на структуру контроля UART
  * @param  uint16_t dest_addr - адрес назначения диагностики
  * @retval None
  */
void GenDiagUART(RS_struct_t* rs_port_box, uint16_t dest_addr)
{
    /* Получение указателя на буфер для подготовки пакета диагностики  */
    resp_uart_diag_v2_t *diag_box = &(rs_port_box->data_tx_diag);
  
    /* Адрес получателя                     */
    diag_box->rs_head.dest = dest_addr;                       
    
    /* Обновление RS пакета перед отправкой */
    update_rs_tx_box((void *)&(rs_port_box->data_tx_diag),&(rs_port_box->contic_tx_box));
    
    /* Отправляетм пакет в роутер */
    if (rs_port_box->set_port_router.QueueInRouter != NULL)
    {
      /* отправить пакет в роутер */
      xQueueSend ( rs_port_box->set_port_router.QueueInRouter, &(rs_port_box->data_tx_diag) , ( TickType_t ) 0 );
      
      if ( ( rs_port_box->set_port_router.HandleTaskRouter ) != NULL )
      {
        xTaskNotify( rs_port_box->set_port_router.HandleTaskRouter,                    /* Указатель на уведомлюемую задачу                         */
                    ( rs_port_box->set_port_router.PortBitID ) << OFFSET_NOTE_PORD_ID, /* Значения уведомления                                     */
                    eSetBits );                                                           /* Текущее уведомление добавляются к уже прописанному       */
      } 
    }    
    
    /* Отправляетм пакет в rs порт */
    if (rs_port_box->set_port_router.QueueOutRouter != NULL)
    {
      /* отправить пакет в rs порт */
      xQueueSend ( rs_port_box->set_port_router.QueueOutRouter, &(rs_port_box->data_tx_diag) , ( TickType_t ) 0 );
    }   
}    
    
/**
  * @brief Функция обновления данных для диагностики ресурса UART 
  * @param  RS_struct_t* rs_port_box - указатель на структуру контроля UART
  * @param  uint32_t time_update - время с последнего обновления в млсек
  * @retval None
  */
void UpdateDiagUART(RS_struct_t* rs_port_box, uint32_t time_update)
{
  /* Получение указателя на буфер для подготовки пакета диагностики */
  resp_uart_diag_v2_t *diag_box = &(rs_port_box->data_tx_diag);
  
  /* Подготовить данные диагностики */

  /* Обновление загрузки за нагрузку принимаем принятые данные  */
  if ( rs_port_box->cnt_interval > 0 )
  {
    rs_port_box->load_corrent = (rs_port_box->load_average_corrent_little_rx)/(rs_port_box->cnt_interval);    
  }
  rs_port_box->load_max = rs_port_box->load_max_little_rx; 
  
  /* Инициализация */
  rs_port_box->load_max_little_rx = 0;
  rs_port_box->load_average_corrent_little_rx = 0;
  
  rs_port_box->load_max_little_tx = 0;
  rs_port_box->load_average_corrent_little_tx = 0;
  rs_port_box->cnt_interval = 0;
  
  /* Формирование пакета   */
  diag_box->rs_head.pre = 0xAA55;                                                /* Преамбула  0x55 0xAA                                                                                    */                                                                                                            
  diag_box->rs_head.lenght = sizeof(resp_uart_diag_v2_t) - SIZE_LENGHT - SIZE_PRE;  /* Длина пакета                                                                                            */
  diag_box->rs_head.id = ID_DIAGNOSTICS_RESP;                                    /* ID ответ команды конфигурации                                                                           */
                                                                                 /* Физический адрес источника и Cчетчик неприрывности пакетов 0..255 заполняется функцией update_rs_tx_box */
  diag_box->rs_head.status_box = 0x00;                                           /* Резерв                                                                                                  */
  diag_box->rs_head.reserv = 0x00;                                               /* статус                                                                                                  */  
  diag_box->rs_head.src = DataLoaderSto.Settings.phy_adr;                        /* Устанавливаем свой физический адрес источника                                                           */
  
  /* Заполнение тела пакета */
  diag_box->Own_PHY_Addr = DataLoaderSto.Settings.phy_adr;                      /*Собсвенный физический адрес                              */
  diag_box->Type = TYPE_UART_DAIG;                                              /*Тип ресурса                                              */
  diag_box->N_port = N_PORT_UART_DAIG;                                          /*Номер порта                                              */
  diag_box->ID_DIAG = ID_UART_DAIG;                                             /*ID команды диагностики                                   */
  diag_box->VERSION_ID = 2;                                                     /*Версия пакета                                            */
  diag_box->Type_Device = DEVICE_TYPE;                                          /*Тип устройсва                                            */
  diag_box->Hard_Mode = rs_port_box->hard_setting_mode;                         /*Режим работы                                             */
  diag_box->Connect_Type = rs_port_box->rs_type;                                /*Тип соединения                                           */
  diag_box->Status_Dev =  rs_port_box->status_port;                             /*Статус подключения                                       */
  
  diag_box->Connect_PHY_Addr = rs_port_box->phy_addr_near;                      /*Физический адрес ВРМ к которому подключен                */
  
  diag_box->err_rx_crc_cnt = rs_port_box->cnt_err_crc;                          /*Счётчик ошибок CRC по приёму                             */
  diag_box->polarity_find_cnt = rs_port_box->cnt_err_polar;                     /*Счётчик поиска полярности, увеличивается, если произошёл */
                                                                                /*разрыв связи и начался процесс поиска полярности         */
  
  diag_box->RS_Load_Cur = rs_port_box->load_corrent;                            /* % загрузки - текущий  (за время *)                      */
  diag_box->RS_Load_Max = rs_port_box->load_max;                                /* % загрузки - максимальный  (за время *)                 */
  
  diag_box->num_ch_list = GetRegChanel( rs_port_box->set_port_router.PortID );  /*  uint8_t Число каналов в наблюдаемых радиостанций       */
  diag_box->num_tag_rsn = GetRegTagRSN( rs_port_box->set_port_router.PortID );  /*  uint8_t Число наблюдаемых радиостанций                 */ 
  diag_box->num_tag_all = GetRegTagAll( rs_port_box->set_port_router.PortID );  /*  uint8_t Число наблюдаемых тегов                        */   
}

/**
  * @brief Функция обновления данных для диагностики за малый интервал  
  * @param  RS_struct_t* rs_port_box - указатель на структуру контроля UART
  * @retval None
  */
void UpdateLitteDiagUART(RS_struct_t* rs_port_box)
{
  if (rs_port_box->MaxDataTxRxValue > 0)
  {
    /* Расчет загрузки uart в % * 100  */ 
    /* Загрузка для RX                 */
    rs_port_box->load_corrent_little_rx = (uint8_t)((((float)rs_port_box->cnt_byte_rx_little)*(100.0))/(rs_port_box->MaxDataTxRxValue));
    /* Ограничение загрузки 100%       */
    if (rs_port_box->load_corrent_little_rx > 100) rs_port_box->load_corrent_little_rx = 100;
    /* Загрузка для TX                 */
    rs_port_box->load_corrent_little_tx = (uint8_t)((((float)rs_port_box->cnt_byte_tx_little)*(100.0))/(rs_port_box->MaxDataTxRxValue)); 
    /* Ограничение загрузки 100%       */
    if (rs_port_box->load_corrent_little_tx > 100) rs_port_box->load_corrent_little_tx = 100;    
    
    /* Обнуление регистров подсчета данных */
    rs_port_box->cnt_byte_rx_little = 0;
    rs_port_box->cnt_byte_tx_little = 0;        
        
    /* Отработка типа соединения           */
    if ( ( rs_port_box->rs_type == CON_RS485_M ) || ( rs_port_box->rs_type == CON_RS485_S )  )
    {
      /* Суммарную загрузку считаем для RS485 и сохраняем для rx и tx */
      rs_port_box->load_corrent_little_rx = rs_port_box->load_corrent_little_tx + rs_port_box->load_corrent_little_rx;
      rs_port_box->load_corrent_little_tx = rs_port_box->load_corrent_little_rx;
    }
    /* Накопление текущих значений для усреднения загрузки приема */
    rs_port_box->load_average_corrent_little_tx = rs_port_box->load_average_corrent_little_tx + (uint16_t)(rs_port_box->load_corrent_little_tx); 
    
    /* Накопление текущих значений для усреднения загрузки передачи */
    rs_port_box->load_average_corrent_little_rx = rs_port_box->load_average_corrent_little_rx + (uint16_t)(rs_port_box->load_corrent_little_rx);
  
    /* Подсчет малых интервалов диагностики */
    rs_port_box->cnt_interval++;    
        
    /* Ищем максимум */
    if ((rs_port_box->load_corrent_little_rx > rs_port_box->load_max_little_rx)||(rs_port_box->load_max_little_rx == 0))
    {
      /* Обновляем максимум */
      rs_port_box->load_max_little_rx = rs_port_box->load_corrent_little_rx;
    }
    
    if ((rs_port_box->load_corrent_little_tx > rs_port_box->load_max_little_tx)||(rs_port_box->load_max_little_tx == 0))
    {
      /* Обновляем максимум */
      rs_port_box->load_max_little_tx = rs_port_box->load_corrent_little_tx;
    }      
  }
}

/**
  * @brief Функция отправки аварийного сообщения в ядро системы 
  * @param RS_struct_t* rs_port_box - указатель на структуру контроля UART
  * @param uint8_t CMD_ID - Идентификатор команды 
  * @retval None
  */
void SetRStoCoreCMD( RS_struct_t* rs_port_box, uint8_t CMD_ID )
{
  /* Отправка актуальной маски каналов прослушки                                                                */
  if ((BaseQCMD[CorePortID].QueueCMD != NULL)&&(BaseQCMD[CorePortID].Status_QCMD == QCMD_ENABLE))
    {
    /* Подготовка команды команды */
    rs_port_box->data_core_tx_cmd.CMD_ID = CMD_ID;                               /* Тип команды                     */
    rs_port_box->data_core_tx_cmd.PortID = rs_port_box->set_port_router.PortID;  /* Источник команды                */
    rs_port_box->data_core_tx_cmd.data_dword[0] = 0;                             /* Отправка диагностики на сервер  */
    rs_port_box->data_core_tx_cmd.data_dword[1] = 0;                             /* Отправка диагностики на сервер  */   
    
    /* Отправка команды */
    xQueueSend ( BaseQCMD[CorePortID].QueueCMD, ( void * )&(rs_port_box->data_core_tx_cmd) , ( TickType_t ) 0 );
  }
}
#endif
/******************* (C) COPYRIGHT 2020 DataExpress *****END OF FILE****/
