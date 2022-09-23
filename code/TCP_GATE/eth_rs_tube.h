/**
  ******************************************************************************
  * @file    eth_rs_tube.h
  * @author  Trembach D.N.
  * @version V1.2.0
  * @date    24-03-2020
  * @brief   Файл содержит описания функций организации моста ETH - RS
  ******************************************************************************
  * @attention
  * 
  * 
  ******************************************************************************
  */ 
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ETH_RS_TUBE_H
#define __ETH_RS_TUBE_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"	 	 
#include <stdio.h>
#include <string.h>
#include "TCP_socket.h"
#include "settings.h"
#include "router_streams.h"   
#include "core_cntrl_diag.h"
#include "timers.h"

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
  uint32_t       max_time_no_active_sec;  
  /*  период контроля сервера в млсек */
  uint32_t       time_control_server;             
  /* IP порт для прослушки */
  uint16_t       ip_port;
  
  /* Индекс порта роутера                                        */ 
  uint8_t        port_index_tab;
  /* Структура для инициализации порта роутера для маршрутизации */
  port_router_t  set_port_router;   
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
 
  /* Определяем размер очереди для пакетов ROUTER_ETH для шлюза */  
  uint16_t              SizeQueueOutRouter;  
  /* Определяем размер очереди для пакетов ETH_ROUTER для шлюза */
  uint16_t              SizeQueueInRouter; 
      
  /* Переменная хранения состояния счетчика неприрывности при передачи */
  uint8_t cnt_tx_box;    
    
  /*  Програмный таймер периодического уведомления задачи                  */
  TimerHandle_t      xSoftTimer;                   
  /* Содержимое сообщения полученного задачей                              */  
  uint32_t           NotifiedValue;                
         
  /* Счетчик формирования периода обновления времени  */
  uint32_t cnt_update_timelife_gate;     
  
}tcp_tube_rs_t;
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
	 
/**
  * @brief  Функция диагностики соединения шлюза RS-UDP позиционирования и диагностики.
  * @param uint8_t *NumLink  - указатель на переменную подсчета активных соединений
  * @param uint32_t *NumTxData      - указатель на переменную подсчета переданных пакетов
  * @param uint32_t *NumRxData      - указатель на переменную подсчета полученных пакетов
  * @retval None
  */
void ReqDiagEthRS( uint8_t *NumLink , uint32_t *NumTxData , uint32_t *NumRxData );

/**
  * @brief  Функция создания сокета TCP для формирование тунеля ETH - RS  
  * @param  None
  * @retval None
  */
void Init_TCP_tube_RS(void);

#endif /* __ETH_RS_TUBE_H */
/************************ (C) COPYRIGHT DEX *****END OF FILE****/

