/**
  ******************************************************************************
  * @file    eth_core_tube.c
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

/* Includes ------------------------------------------------------------------*/
#include "eth_core_tube.h"
#include "GenerationCRC16.h"
#include "rs_frame_header.h"
#include "Core.h"
#include "settings.h"

/* Прописываем указатель на функцию контроля соединение ethernet */
#define link_ethernet()   (StatusLinkETH())   /* флаг LwIP - состояние физического соединения */

/* максимальное число соединений число соединений сервера */
#define  MAX_NUMBER_CONNECT_TUBE_RS_SERVER              (4)
/* период вызова функции Control_Server 1 млсек           */
#define  TIME_CALL_CONTROL_TUBE_RS_SERVER               (1)
/* максимальное вреня неактивности соединения в секундах  */	 
#define  TIME_MAX_NOACTIVE_CONNECT_TUBE_RS_SERVER       (300) 

/* Private typedef -----------------------------------------------------------*/
//структура соединений сервера
tcp_connect_t eth_rs_conn[MAX_NUMBER_CONNECT_TUBE_RS_SERVER];
//===========================================================
//= обьявление переменных для подключения сервера к роутеру =
//структура подключения к роутеру 
tcp_tube_core_t   eth_rs;
//===========================================================

/**  @brief		Функция обработки принятого сообщения из ETH и отправкa его в роутер.
  *  @param		arg: указатель на аргумент для tcp_pcb соединения
  *  @param		tpcb: указатель на блок контроля соединения tcp_pcb
  *  @param		pbuf: указатель на принятый pbuf
  *  @return	none.
  */
void eth_rs_send(void *arg, struct tcp_pcb *tpcb, struct pbuf *p)
{
   /* аргумент структура контроля соединения */	
  #define eth_conn      ((tcp_connect_t*)arg)  
  #define tcp_tude_st   ((tcp_tube_core_t*)(eth_conn->tcp_strct))
  #define buf_rs        tcp_tude_st->buf_eth_rtr  
  
  /* принятые данные поместятся в буфер? */
  if (p->tot_len >= sizeof(router_data_box_t)) return;
  /* копируем принятые данные во временный массив */
  pbuf_copy_partial(p, tcp_tude_st->pdamp_buf_eth_rtr , p->tot_len, 0);
  
  /* проверка пакета на соответствие формату */
  if (buf_rs.pre != 0xAA55) return;                           /* Преамбула  0x55 0xAA */
  
  if (buf_rs.lenght > (sizeof(router_data_box_t) - SIZE_LENGHT - SIZE_PRE)) return; /* Длина пакета (включая контрольную сумму, не включая преамбулу) */
  /* заполнение полей адреса */
  buf_rs.dest = *((uint16_t*)&(buf_rs.data[0]));    /* Физический адрес получателя */
   /* если очередь инициализирована */ 
  if (tcp_tude_st->QueueInCore != NULL)
  { 	/* отправляем данные для роутера */
    xQueueSend(tcp_tude_st->QueueInCore,&buf_rs, 0);
  }    
}		

/**
  * @brief  Функция отправки сообщений от роутера в ETH
  * @param  tcp_tube_rs_t* tcp_tube_rs  указатель на структуру контроля шлюза 
  * @retval None
  */
void rs_eth_send(tcp_tube_core_t* tcp_tube_rs)
{	
  /* если очередь инициализирована */ 
  if (tcp_tube_rs->QueueOutCore != NULL)
  { 
    /* проверяем наличие данных */
    while(xQueueReceive(tcp_tube_rs->QueueOutCore,&tcp_tube_rs->buf_rtr_eth, 0 ) == pdTRUE)
    {
      /* Устанавливаем свой физический адрес источника  */
      tcp_tube_rs->buf_rtr_eth.src = DataLoaderSto.Settings.phy_adr;
      /* Обновление RS пакета перед отправкой           */
      update_rs_tx_box(&(tcp_tube_rs->buf_rtr_eth),&(tcp_tube_rs->cnt_tx_box));
      
      /* данные приняты - производим обработку   */
      /* Цикл по всем активным соединениям       */
      for(tcp_tube_rs->eth_tube_rtr.cnt_conn = 0;tcp_tube_rs->eth_tube_rtr.cnt_conn < tcp_tube_rs->eth_tube_rtr.max_connect;tcp_tube_rs->eth_tube_rtr.cnt_conn++)
      {
        /* проверка на состояния соединения */
        if (tcp_tube_rs->eth_tube_rtr.conn[tcp_tube_rs->eth_tube_rtr.cnt_conn].pcb != NULL)
        {
          /* если соединение активно - отправляем пакет */
          tcp_tube_rs->eth_tube_rtr.conn[tcp_tube_rs->eth_tube_rtr.cnt_conn].p = pbuf_alloc(PBUF_TRANSPORT, tcp_tube_rs->buf_rtr_eth.lenght + SIZE_PRE + SIZE_LENGHT, PBUF_POOL);
          /* copy data to pbuf */
          pbuf_take(tcp_tube_rs->eth_tube_rtr.conn[tcp_tube_rs->eth_tube_rtr.cnt_conn].p, &tcp_tube_rs->buf_rtr_eth, tcp_tube_rs->buf_rtr_eth.lenght + SIZE_PRE + SIZE_LENGHT);
          /* отправить данные  */
          Send_Server(tcp_tube_rs->eth_tube_rtr.conn[tcp_tube_rs->eth_tube_rtr.cnt_conn].pcb, &tcp_tube_rs->eth_tube_rtr.conn[tcp_tube_rs->eth_tube_rtr.cnt_conn]);
        }
      }
    }	
  }    
}	

/**
  * @brief  Задача контроля работы сервера моста ETH RS
  * @param  pvParameters not used
  * @retval None
  */
void eth_rs_task(void * pvParameters)
{
  /* ожидаем соединения по ETH */
  while(link_ethernet() == false)
  {
    vTaskDelay(1);
  } 

  /* инициализация сервера для моста ETH RS */
  Create_Server(&eth_rs.eth_tube_rtr, 
                eth_rs.conn, 
                eth_rs.max_connect,   
                eth_rs.ip_port,
                eth_rs.recv_cb,
                eth_rs.poll_cb,
                NULL);  

  /* Инициализация переменной xLastWakeTime на текущее время.                 */
  eth_rs.xLastWakeTime = xTaskGetTickCount();
  /* Цикл периодического запуска задачи                                       */ 
  while (1) 
  {
    /*  контроль открытых ТСР соединений                                      */
    eth_control_connect(&eth_rs.eth_tube_rtr);
    /*  контроль отправки полученных данных                                   */
    rs_eth_send(&eth_rs);
    /*  Ожидание до следующего цикла                                          */
    vTaskDelayUntil( &(eth_rs.xLastWakeTime), eth_rs.time_control_server );
  }
}

/**
  * @brief  Функция создания сокета TCP для формирование тунеля ETH - CORE  
  * @param  QueueHandle_t QueueInCore    - указатель на очередь в CORE
  * @param  QueueHandle_t QueueOutCore   - указатель на очередь из CORE 
  * @retval None
  */
void Init_TCP_tube_CORE( QueueHandle_t QueueInCore, QueueHandle_t QueueOutCore )
{
  /* Пропишим порт для прослушки                                              */
  eth_rs.ip_port = DataLoaderSto.Settings.ip_port_loader,
  /* Пропишем указатель на структуру соединений                               */
  eth_rs.conn = eth_rs_conn; 
  /* Максимальное число доступных соединений                                  */
  eth_rs.max_connect = MAX_NUMBER_CONNECT_TUBE_RS_SERVER; 
  /* максимальное время отсутствия активности соединения sec                  */
  eth_rs.max_time_no_active_sec = TIME_MAX_NOACTIVE_CONNECT_TUBE_RS_SERVER;  
  /* Период контроля сервера в млсек                                          */
  eth_rs.time_control_server = TIME_CALL_CONTROL_TUBE_RS_SERVER;      
  
  /* Функция обработки принятого сообщения из ETH и отправкa его в роутер     */
  eth_rs.recv_cb = eth_rs_send;
  /* Функция обработки обратного вызова  the tcp_poll LwIP                    */
  eth_rs.poll_cb = Polling_Server; 

  /* Прописать указатели на очередь        */
  eth_rs.QueueInCore = QueueInCore;
  eth_rs.QueueOutCore = QueueOutCore;  
     
  /* запуск задачи управления мостом ETH Radio */
  xTaskCreate(eth_rs_task, "EthRS", configMINIMAL_STACK_SIZE * 2, &eth_rs ,_ETH_THREAD_PRIO_, &(eth_rs.HandleTask) );    
}
