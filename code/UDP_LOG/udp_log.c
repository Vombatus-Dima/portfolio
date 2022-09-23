/**
  ******************************************************************************
  * @file    udp_log.c
  * @author  Trembach D.N.
  * @version V1.2.0
  * @date    08-10-2020
  * @brief   Файл содержит функции UDP логгер потоков  
  ******************************************************************************
  * @attention
  * 
  * 
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "lwip/memp.h"
#include <stdio.h>
#include <string.h>
#include "stm32f4x7_eth.h"
#include "udp_socket.h"
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "event_groups.h"

#if UDP_LOGGER == 1     
#include "udp_log.h"
/* структура для UDP сокета статистики FreeRTOS                             */
udp_socket_t Logger_UDP;

/* временные переменные для отправки и получения команды запроса статистики */
req_logger_t cmd_logger_recv;
req_logger_t cmd_logger_trans;

/* Определяем очередь для запроса статистики FreeRTOS */
QueueHandle_t  xQueueLoggerUDP;
 
/* Объявление флагов логирования */
bool LoggerTxRF = false;
bool LoggerRxRF = false;      
bool LoggerTxRSA = false;      
bool LoggerRxRSA = false;   
bool LoggerTxRSB = false;      
bool LoggerRxRSB = false;

/* Указатель для уведомления задачи */
TaskHandle_t   LoggerHandleTask;    

/* Определяем очередь для лога */
QueueHandle_t xQueueLogger; 

/* Буфер памяти для формирования текстовой строки перед отправкой */
logger_data_box_t log_buf;
 
/* Число адресов запрашивающих лог */
#define MAX_ADDR_DEST_LOGGER   (5)

/* Массив адресов запрашивающих лог                               */
dest_logger_t  dest_addr[MAX_ADDR_DEST_LOGGER];

/**
  * @brief  Функция регистрации адреса назначения лога
  * @param  char *pcInsert указатель на массив pcInsert для формируемой строки
  * @param  req_logger_t* dest_logger_recv - указатель на структуру адреса запроса 
  * @retval None
  */
uint16_t RegDestLogger( char *pcInsert, req_logger_t* dest_logger_recv)
{
  /* Инициализация индекса */
  uint8_t cnt_log_deste = 0;
  
  /* Проверка наличия регистрации подключения */
  for( cnt_log_deste = 0 ; cnt_log_deste < MAX_ADDR_DEST_LOGGER; cnt_log_deste++ )
  {
    if ( ( dest_addr[cnt_log_deste].DestIPaddr.addr == dest_logger_recv->ReqIPaddr.addr ) && ( dest_addr[cnt_log_deste].DestIPport == dest_logger_recv->ReqIPport ) )
    {
      switch( dest_logger_recv->cmd_cod )
      {
      case CMD_LOG_BREAK:
        /* отключить все */
        dest_addr[cnt_log_deste].flag_control = 0; 
        break;
        
      case CMD_LOG_RF_TX:
        /* режим лога передатчика РФ */        
        if ( dest_addr[cnt_log_deste].flag_rf_tx  > 0 )
        {
          dest_addr[cnt_log_deste].flag_rf_tx = 0;
        }
        else
        {
          dest_addr[cnt_log_deste].flag_rf_tx = 1;
        }  
        break;      
        
      case CMD_LOG_RF_RX:
        /* режим лога приемника РФ */ 
        if ( dest_addr[cnt_log_deste].flag_rf_rx  > 0 )
        {
          dest_addr[cnt_log_deste].flag_rf_rx = 0;
        }
        else
        {
          dest_addr[cnt_log_deste].flag_rf_rx = 1;
        }  
        break;
        
      case CMD_LOG_RSA_TX:
        /* режим лога передатчика UART */ 
        if ( dest_addr[cnt_log_deste].flag_rsa_tx  > 0 )
        {
          dest_addr[cnt_log_deste].flag_rsa_tx = 0;
        }
        else
        {
          dest_addr[cnt_log_deste].flag_rsa_tx = 1;
        }  
        break;      
        
      case CMD_LOG_RSA_RX:
        /* режим лога приемника UART */ 
        if ( dest_addr[cnt_log_deste].flag_rsa_rx  > 0 )
        {
          dest_addr[cnt_log_deste].flag_rsa_rx = 0;
        }
        else
        {
          dest_addr[cnt_log_deste].flag_rsa_rx = 1;
        }  
        break;
        
      case CMD_LOG_RSB_TX:
        /* режим лога передатчика UART */ 
        if ( dest_addr[cnt_log_deste].flag_rsb_tx  > 0 )
        {
          dest_addr[cnt_log_deste].flag_rsb_tx = 0;
        }
        else
        {
          dest_addr[cnt_log_deste].flag_rsb_tx = 1;
        }  
        break;      
        
      case CMD_LOG_RSB_RX:
        /* режим лога приемника UART */ 
        if ( dest_addr[cnt_log_deste].flag_rsb_rx  > 0 )
        {
          dest_addr[cnt_log_deste].flag_rsb_rx = 0;
        }
        else
        {
          dest_addr[cnt_log_deste].flag_rsb_rx = 1;
        }  
        break;
        
      default:
        break;
      }
      
      /* Выдаем статус записи */
      return  sprintf(pcInsert,"RF_TX=%d RF_RX=%d RSA_TX=%d RSA_RX=%d RSB_TX=%d RSB_RX=%d\n\n", dest_addr[cnt_log_deste].flag_rf_tx, dest_addr[cnt_log_deste].flag_rf_rx, dest_addr[cnt_log_deste].flag_rsa_tx, dest_addr[cnt_log_deste].flag_rsa_rx, dest_addr[cnt_log_deste].flag_rsb_tx, dest_addr[cnt_log_deste].flag_rsb_rx );   
    }
  }
    
  /* Проверка наличия регистрации подключения */
  for( cnt_log_deste = 0; cnt_log_deste < MAX_ADDR_DEST_LOGGER; cnt_log_deste++ )
  {
    if ( dest_addr[cnt_log_deste].flag_control == 0 )
    {
      dest_addr[cnt_log_deste].DestIPaddr.addr = dest_logger_recv->ReqIPaddr.addr;
      dest_addr[cnt_log_deste].DestIPport = dest_logger_recv->ReqIPport;     

      switch( dest_logger_recv->cmd_cod )
      {
      case CMD_LOG_BREAK:
        /* отключить все */
        dest_addr[cnt_log_deste].flag_control = 0; 
        break;
        
      case CMD_LOG_RF_TX:
        /* режим лога передатчика РФ */        
        dest_addr[cnt_log_deste].flag_rf_tx = 1;
        break;      
        
      case CMD_LOG_RF_RX:
        /* режим лога приемника РФ */ 
        dest_addr[cnt_log_deste].flag_rf_rx = 1;
        break;
        
      case CMD_LOG_RSA_TX:
        /* режим лога передатчика UART */ 
        dest_addr[cnt_log_deste].flag_rsa_tx = 1;
        break;      
        
      case CMD_LOG_RSA_RX:
        /* режим лога приемника UART */ 
        dest_addr[cnt_log_deste].flag_rsa_rx = 1;
        break;
        
      case CMD_LOG_RSB_TX:
        /* режим лога передатчика UART */ 
        dest_addr[cnt_log_deste].flag_rsb_tx = 1;
        break;
        
      case CMD_LOG_RSB_RX:
        /* режим лога приемника UART */ 
        dest_addr[cnt_log_deste].flag_rsb_rx = 1;
        break;        
      default:
        break;
      }
      
      /* Выдаем статус записи */
      return  sprintf(pcInsert,"RF_TX=%d RF_RX=%d RSA_TX=%d RSA_RX=%d RSB_TX=%d RSB_RX=%d\n\n", dest_addr[cnt_log_deste].flag_rf_tx, dest_addr[cnt_log_deste].flag_rf_rx, dest_addr[cnt_log_deste].flag_rsa_tx, dest_addr[cnt_log_deste].flag_rsa_rx, dest_addr[cnt_log_deste].flag_rsb_tx, dest_addr[cnt_log_deste].flag_rsb_rx );   
    }
  }
  /* Запись в таблице не найдена и места нет */
  return  sprintf(pcInsert,"LOG ERROR\n\n");
}


/**
  * @brief  Функция включения флага запроса лога
  * @param  uint8_t code_log - код лога
  * @retval None
  */
bool GetFlagLogger( uint8_t code_log )
{
  /* Инициализация индекса */
  for( uint8_t cnt_log_deste = 0; cnt_log_deste < MAX_ADDR_DEST_LOGGER; cnt_log_deste++ )
  {
    switch( code_log )
    {
    case CMD_LOG_RF_TX:
      /* режим лога передатчика РФ */        
      if ( dest_addr[cnt_log_deste].flag_rf_tx  > 0 )  return true;
    case CMD_LOG_RF_RX:
      /* режим лога приемника РФ */ 
      if ( dest_addr[cnt_log_deste].flag_rf_rx  > 0 )  return true;
    case CMD_LOG_RSA_TX:
      /* режим лога передатчика UART */ 
      if ( dest_addr[cnt_log_deste].flag_rsa_tx  > 0 )  return true;
    case CMD_LOG_RSA_RX:
      /* режим лога приемника UART */ 
      if ( dest_addr[cnt_log_deste].flag_rsa_rx  > 0 )   return true;
    case CMD_LOG_RSB_TX:
      /* режим лога передатчика UART */ 
      if ( dest_addr[cnt_log_deste].flag_rsb_tx  > 0 )  return true;
    case CMD_LOG_RSB_RX:
      /* режим лога приемника UART */ 
      if ( dest_addr[cnt_log_deste].flag_rsb_rx  > 0 )   return true;      
      
      
    default:
      break;
    }
  }
  
  /* Запись в таблице не найдена и места нет */
  return false;
}

/**
  * @brief Эта функция приема пакета на порт UDP.
  * @param arg аргумент пользователя ((udp_socket_t *)arg)
  * @param pcb udp_pcb который принял данные 
  * @param p буфер пакета который был принят
  * @param addr удаленное IP address с которого пакет был принят
  * @param port удаленный порт с которого пакет был принят
  * @retval None
  */
void UDP_Receive_Logger(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
  /* обработка данных */
  cmd_logger_recv.cmd_cod = ((uint8_t *)(p->payload))[0];
  
  cmd_logger_recv.ReqIPaddr = *addr;
  
  cmd_logger_recv.ReqIPport = port;	
  
  if (xQueueLoggerUDP != NULL)
  {
    xQueueSend ( xQueueLoggerUDP, ( void * )&cmd_logger_recv , ( TickType_t ) 0 );
  }  
  /* освобождаем принятый буфер */
  pbuf_free(p);
}

#define UDP_Logger ((udp_socket_t *)arg)	
/**
  * @brief Эта функция обработки основного тела задачи.
  * @param arg аргумент пользователя ((udp_socket_t *)arg)
  * @retval None
  */
void UDP_Body_Task_Logger(void *arg)
{
  uint16_t temp_size = 0;

  /* ожидание команд запроса статистики */
  if (xQueueLoggerUDP != NULL) 
  {
    /* ожидание флагов событий */
    if (xQueueReceive(xQueueLoggerUDP , ( void * )&cmd_logger_trans , ( TickType_t ) 0) == pdTRUE)
    {
      /* подготавливаем массив с значение свободного стека */
      temp_size = RegDestLogger( (char *)(log_buf.damp_data), &cmd_logger_trans );
      /* отправляем данные в указанный порт                */
      UDP_Send_Socket((log_buf.damp_data), temp_size, &Logger_UDP, cmd_logger_trans.ReqIPaddr.addr, cmd_logger_trans.ReqIPport);
    
      /* Обновление флагов логирования */
      LoggerTxRF = GetFlagLogger(CMD_LOG_RF_TX);
      LoggerRxRF = GetFlagLogger(CMD_LOG_RF_RX);      
      LoggerTxRSA = GetFlagLogger(CMD_LOG_RSA_TX);      
      LoggerRxRSA = GetFlagLogger(CMD_LOG_RSA_RX);  
      LoggerTxRSB = GetFlagLogger(CMD_LOG_RSB_TX);      
      LoggerRxRSB = GetFlagLogger(CMD_LOG_RSB_RX);        
    }
  }	
  /* Проверка очереди лога */
  if (xQueueLogger != NULL)
  {
    /* ожидание флагов событий */
    if (xQueueReceive(xQueueLogger , ( void * )&log_buf , ( TickType_t ) 0) == pdTRUE)
    {
      /* Поиск запросов на полученный лог */
      for ( uint8_t cntc_dest = 0; cntc_dest < MAX_ADDR_DEST_LOGGER ; cntc_dest++ )
      {
        /* Если зарегистрирован запрос */
        if ( dest_addr[cntc_dest].flag_control > 0 )
        {
          /* Проверка соответствия запроса */
          switch( log_buf.Logger_type_code )
          {
          case CMD_LOG_RF_TX:
            /* режим лога передатчика РФ */        
            if ( dest_addr[cntc_dest].flag_rf_tx  > 0 )  
            {
              /* отправляем данные в указанный порт                */
              UDP_Send_Socket((log_buf.damp_data), log_buf.Logger_data_size, &Logger_UDP, dest_addr[cntc_dest].DestIPaddr.addr, dest_addr[cntc_dest].DestIPport);
            }
            break;
          case CMD_LOG_RF_RX:
            /* режим лога приемника РФ */ 
            if ( dest_addr[cntc_dest].flag_rf_rx  > 0 ) 
            {
              /* отправляем данные в указанный порт                */
              UDP_Send_Socket((log_buf.damp_data), log_buf.Logger_data_size, &Logger_UDP, dest_addr[cntc_dest].DestIPaddr.addr, dest_addr[cntc_dest].DestIPport);
            }
            break;
          case CMD_LOG_RSA_TX:
            /* режим лога передатчика UART */ 
            if ( dest_addr[cntc_dest].flag_rsa_tx  > 0 )  
            {
              /* отправляем данные в указанный порт                */
              UDP_Send_Socket((log_buf.damp_data), log_buf.Logger_data_size, &Logger_UDP, dest_addr[cntc_dest].DestIPaddr.addr, dest_addr[cntc_dest].DestIPport);
            }
            break;
          case CMD_LOG_RSA_RX:
            /* режим лога приемника UART */ 
            if ( dest_addr[cntc_dest].flag_rsa_rx  > 0 )   
            {
              /* отправляем данные в указанный порт                */
              UDP_Send_Socket((log_buf.damp_data), log_buf.Logger_data_size, &Logger_UDP, dest_addr[cntc_dest].DestIPaddr.addr, dest_addr[cntc_dest].DestIPport);
            }
            break;
          case CMD_LOG_RSB_TX:
            /* режим лога передатчика UART */ 
            if ( dest_addr[cntc_dest].flag_rsb_tx  > 0 )  
            {
              /* отправляем данные в указанный порт                */
              UDP_Send_Socket((log_buf.damp_data), log_buf.Logger_data_size, &Logger_UDP, dest_addr[cntc_dest].DestIPaddr.addr, dest_addr[cntc_dest].DestIPport);
            }
            break;
          case CMD_LOG_RSB_RX:
            /* режим лога приемника UART */ 
            if ( dest_addr[cntc_dest].flag_rsb_rx  > 0 )   
            {
              /* отправляем данные в указанный порт                */
              UDP_Send_Socket((log_buf.damp_data), log_buf.Logger_data_size, &Logger_UDP, dest_addr[cntc_dest].DestIPaddr.addr, dest_addr[cntc_dest].DestIPport);
            }
            break;
          default:
            break;
          }
        }
      }
    }
  }
}

/**
  * @brief Эта функция инициализации задачи.
  * @param arg аргумент пользователя ((udp_socket_t *)arg)
  * @retval None
  */
void UDP_Init_Task_Logger(void *arg)
{
  /* ожидаем соединения по ETH */
  while( StatusLinkETH() == false)
  {
    vTaskDelay(10);
  } 	
}

/**
  * @brief  Функция создания сокета UDP для логирования потоков
  * @param  None
  * @retval None
  */
void Init_UDP_Logger(void)
{
  /* период активности задачи в тиках системы */			
  Logger_UDP.control_time = 1;		
  /* имя для задачи                           */
  Logger_UDP.pcName = "LOGGER"; 
  /* установка функции инициализации задачи   */
  Logger_UDP.init_task = UDP_Init_Task_Logger;
  /* установка функции тела задачи            */
  Logger_UDP.body_task = UDP_Body_Task_Logger;
  /* установка обработчика принятых пакетов   */
  Logger_UDP.recv = UDP_Receive_Logger;
  /* состояния соединения                     */  
  Logger_UDP.status = NONE_UDP;          
  /* указатель на блок контроля соединения    */
  Logger_UDP.pcb = NULL;
  /* локальный порт сервера                   */
  Logger_UDP.local_port = 7750;
  
  /* открытие очереди для запроса лога        */   
  if (xQueueLogger == NULL)
  {
    /* открытие очереди для 3 команд */
    xQueueLogger = xQueueCreate( 3 , sizeof(logger_data_box_t)); 
  }	 
  
  /* открытие очереди для запроса лога        */   
  if (xQueueLoggerUDP == NULL)
  {
    /* открытие очереди для 5 команд */
    xQueueLoggerUDP = xQueueCreate( 5 , sizeof(req_logger_t)); 
  }		 
  // =============== очереди для голосовых поток ========================================
  xTaskCreate(UDP_Task_Socket, Logger_UDP.pcName, configMINIMAL_STACK_SIZE * 2, (void *)&Logger_UDP, _ETH_THREAD_PRIO_,  &(Logger_UDP.HandleTask));
}
#endif
/************************ (C) COPYRIGHT DEX *****END OF FILE****/
