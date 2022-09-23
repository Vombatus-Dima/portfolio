/**
  ******************************************************************************
  * @file    eth_core_tube.h
  * @author  Trembach D.N.
  * @version V1.0.0
  * @date    30-03-2018
  * @brief   Файл содержит описания функций организации моста ETH - CORE
  ******************************************************************************
  * @attention
  * 
  * 
  ******************************************************************************
  */ 
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ETH_CORE_TUBE_H
#define __ETH_CORE_TUBE_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"	 	 
#include <stdio.h>
#include <string.h>
#include "TCP_socket.h"
#include "settings.h"
#include "rs_frame_header.h"

/* Exported types ------------------------------------------------------------*/
/* структура контроля соединения c роутером */
typedef struct
{
  /*= Нельзя смещать tcp_server_t eth_tube_rtr;- положение только в шапке структуры!! =*/
  /*============ Для корректной работы с функциями в TCP_socket.c ================     */
  /*структура сервера                                                                  */
  tcp_server_t eth_tube_rtr;
  /* указатель на массив структур соединений  */
  tcp_connect_t         *conn; 
  /* максимальное число доступных соединений  */
  uint8_t               max_connect;  
  /*==============================================================================     */  
  /* указатель на обработчик принятого пакета */
  void                  (*recv_cb)(void *arg, struct tcp_pcb *tpcb, struct pbuf *p);    
  /* указатель на функцию поллинга           */
  err_t                 (*poll_cb)(void *arg, struct tcp_pcb *tpcb);  
  /* указатель на обработчик передачи пакета */
  void                  (*send_poll)(void *arg);  
  
  /*  максимальное время отсутствия активности соединения sec */
  uint32_t              max_time_no_active_sec;  
  /*  период контроля сервера в млсек */
  uint32_t              time_control_server;             
  /* IP порт для прослушки */
  uint16_t              ip_port;

  union
  { /* Дамп памяти буфера для временного хранения данных для передачи в ETH  */
    uint8_t             pdamp_buf_rtr_eth[sizeof(router_box_t)] ;
    /* Буфер для временного хранения данных формата RS для передачи в ETH    */
    router_box_t   buf_rtr_eth;    
  };
  
  union
  { /* Дамп памяти буфера для временного хранения данных принятых из ETH */
    uint8_t             pdamp_buf_eth_rtr[sizeof(router_box_t)];
    /* Буфер для временного хранения данных формата RS принятых из ETH   */
    router_box_t   buf_eth_rtr;    
  };    
 
  /* Указатель на очередь в CORE */  
  QueueHandle_t         QueueInCore;  
  /* Указатель на очередь из CORE */  
  QueueHandle_t         QueueOutCore; 
      
  /* Переменная хранения состояния счетчика неприрывности при передачи */
  uint8_t cnt_tx_box;    
  
  TaskHandle_t              HandleTask;                           // 
  
  /* вспомогательная переменная для формирования периодического вызова задачи контроля соединения */
  TickType_t            xLastWakeTime;          
  /* Счетчик формирования периода обновления времени  */
  uint32_t              cnt_update_timelife_gate;     
  
}tcp_tube_core_t;
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/**
  * @brief  Функция создания сокета TCP для формирование тунеля ETH - CORE  
  * @param  QueueHandle_t QueueInCore    - указатель на очередь в CORE
  * @param  QueueHandle_t QueueOutCore   - указатель на очередь из CORE 
  * @retval None
  */
void Init_TCP_tube_CORE( QueueHandle_t QueueInCore, QueueHandle_t QueueOutCore );

#endif /* __ETH_CORE_TUBE_H */
/************************ (C) COPYRIGHT DEX *****END OF FILE****/

