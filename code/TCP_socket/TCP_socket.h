/**
  ******************************************************************************
  * @file    TCP_socket.h
  * @author  Trembach D.N.
  * @version V2.6.0
  * @date    24-03-2020
  * @brief   Файл содержит описания  
  ******************************************************************************
  * @attention
  *   ** 
  * 
  ******************************************************************************
  */ 
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ETH_TCP_H
#define __ETH_TCP_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"	
 
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
	 
   //======================================== TCP состояние ================================================================	 
   /* состояние сервера */
   typedef enum 
   {
     SRV_NONE = 0,
     SRV_NOT_CONN,
     SRV_CONNECTED,	
     SRV_ACCEPTED,
     SRV_RECEIVED,
     SRV_CLOSING,
   }tcp_states_e;
   
   /* состояние сервера */
   typedef enum 
   {
     CON_NONE = 0,
     CON_CONNECTED,	
     CON_ACCEPTED,
     CON_RECEIVED,
     CON_CLOSING,
   }cnt_states_e;	
   //===========================================================================================================================	 
   
#define TCP_Server_Stat  (1)
   
#ifdef TCP_Server_Stat	
	//======================================== структура статуса ================================================================
	//формируем структуру статутса
typedef struct
{
  uint32_t              Time_of_break;          //время работы без потери link сек
  uint32_t              Time_of_break_ms;       //время работы млсек		
  uint32_t              CBreak;                 //число потерь link 
  uint32_t              Cnt_Connect;            //число выполненых соединений за время работы
  uint32_t              Box_rx;                 //число принятых пакетов
  uint32_t              Box_tx;                 //число переданных пакетов
  uint32_t              Data_Rx_HI;             //принято данных 
  uint32_t              Data_Rx_LO;        			
  uint32_t              Data_Tx_HI;             //передано данных 
  uint32_t              Data_Tx_LO;        					
}tcp_stat_t;
//===========================================================================================================================
#endif

//формируем структуру которая в качестве аргумерта передается в функции обратного вызова LwIP
typedef struct
{
  void                  *tcp_strct;             // указатель на структуру сервера  
  struct tcp_pcb        *pcb;                   // указатель на текущий tcp_pcb

  uint8_t               con_state;              // текущее состояние соединения   
  uint8_t               connect_id;             // индекс соединения в массиве соединений
  
  struct pbuf           *p;                     // указатель на принятый или переданный pbuf 
  err_t                 wr_err;                 // статус ошибки при записи
  err_t                 rcv_err;                // статус ошибки при приеме
  err_t                 pol_err;                // статус ошибки при полинге
  
  struct pbuf           *wr_ptr;                // указатель на передаваемый пакет 
  uint16_t              wr_len;                 // длинна переданого пакета   
  uint8_t               contic_box;             // счетчик неприрывности для отправки пакета с шапкой RS   
  uint32_t              time_no_active_ms;      // время отсутствия активности соединения ms		
  uint32_t              time_no_active_sec;     // время отсутствия активности соединения sec 
  
}tcp_connect_t;

//структура контроля соединения передачи данных
typedef struct
{
  void                  *tcp_strct;             //указатель на структуру сервера		
  struct tcp_pcb        *pcb;                   //указатель на блок контроля сервера
  
  tcp_connect_t         *conn;                  //указатель на массив структур соединений
  uint8_t               max_connect;            //максимальное число доступных соединений

  uint32_t              max_time_no_active_sec; //  максимальное время отсутствия активности соединения sec 
  uint32_t              time_control_server;    //  период контроля сервера в млсек     
  
  tcp_states_e          tcp_state;              //состояния соединения 
	   
  uint8_t               cnt_conn;               //счетчик выбора соединения
#ifdef TCP_Server_Stat			
  tcp_stat_t            info_serv;              //статистика сервера
#endif		
  void                  (*recv_cb)(void *arg, struct tcp_pcb *tpcb, struct pbuf *p);    // указатель на обработчик принятого пакета
  err_t                 (*poll_cb)(void *arg, struct tcp_pcb *tpcb);                    // указатель на функцию поллинга
  void                  (*send_poll)(void *arg);                                        // указатель на обработчик передачи пакета
  
  u16_t                 IP_port;                //локальный IP порт
  u32_t                 IP_addr;                //локальный IP адресс		
  struct ip_addr        IP_addr_strct;          //IP в формате ip_addr  
  
  err_t                 init_err;               //ошибка при инициализация			
  err_t                 srv_err;                //ошибка при соединении	
  err_t                 acc_err;                //статус ошибки при соединении (ACCEPTED) 
  
  struct pbuf           *pdata_srv;             //указатель на полученный пакет для размешения в буфер 
  struct pbuf           *p_svr;                 //указатель на буфер передаваемого пакета
}tcp_server_t;

/**
  * @brief  Функция возвращает число соединений сервера
  * @param  tcp_server_t *loader_srv_tcp,  указатель на структуру сервера
  * @retval uint8_t - число соединений сервера
  */
uint8_t get_number_active_link(tcp_server_t *str_srv );

/**
  * @brief  Эта функция закрывает tcp соединение
  * @param  tcp_pcb: указатель на блок контроля соединения
  * @param  es: указатель на структуру соединения
  * @retval None
  */
void Close_Connect(struct tcp_pcb *tpcb, tcp_connect_t *str_con);

/**
  * @brief  Функция обработки обратного вызова tcp_err callback (вызывается 
  *         когда произошла фатальная ошибка соединения. 
  * @param  arg: указатель на аргумент 
  * @param  err: not used
  * @retval None
  */
void Error_Server(void *arg, err_t err);

/**
  * @brief  Эта функция используется для передачи данных в tcp connection
  * @param  tpcb: указатель на блок управления tcp_pcb соединения
  * @param  es: указатель на структуру соединения
  * @retval None
  */
void Send_Server(struct tcp_pcb *tpcb, tcp_connect_t *str_con);

/**
  * @brief  Функция обработки обратного вызова tcp_sent LwIP callback (вызывается 
  *         когда было подтверждения полученных данных от удаленного хоста) 
  * @param  None
  * @retval None
  */
err_t Sent_Server(void *arg, struct tcp_pcb *tpcb, u16_t len);

/**
  * @brief  Функция обработки обратного вызова tcp_recv LwIP
  * @param  arg: указатель на аргумент для tcp_pcb соединения
  * @param  tpcb: указатель на блок контроля соединения tcp_pcb
  * @param  pbuf: указатель на принятый pbuf
  * @param  err: error информация относительно принятого pbuf
  * @retval err_t: error code
  */
err_t Recv_Server(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

/**
  * @brief  Функция обработки обратного вызова  the tcp_poll LwIP 
  * @param  arg: указатель на аргумент для tcp_pcb соединения
  * @param  tpcb: указатель на блок контроля соединения tcp_pcb
  * @retval err_t: error code
  */
err_t Polling_Server(void *arg, struct tcp_pcb *tpcb);

/**
  * @brief  This function is the implementation of tcp_accept LwIP callback
  * @param  arg: not used
  * @param  newpcb: указатель на структуру  tcp_pcb для этого открытого tcp соединения
  * @param  err: not used 
  * @retval err_t: error status
  */
err_t Accept_Server(void *arg, struct tcp_pcb *newpcb, err_t err);

/**
  * @brief  Инициализация tcp сервера
  * @param  tcp_server_t *loader_srv_tcp  указатель на структуру сервера
  * @retval uint8_t 0 - сервер проинициализирован успешно
  *								  1 - не удалось открыть pcb 
  *									2 - не удалось назначить номер для IP pcb, pcb - удален
  */
uint8_t Init_Server(tcp_server_t *str_srv);
	
/**
  * @brief  Функция создания сервера TCP
  * @param  tcp_server_t *loader_srv_tcp,  указатель на структуру сервера
  * @param  tcp_connect_t *conn_serv,  указатель на массив структур соединений
  * @param  uint8_t size_conn_serv,    число соединений сервера
  * @param  uint16_t IP_port,					 локальный IP порт
  * @param  void  (*recv_cb)(void *arg, struct tcp_pcb *tpcb, struct pbuf *p)	указатель на обработчик принятого пакета
  * @param  err_t (*poll_cb)(void *arg, struct tcp_pcb *tpcb)	                указатель на функцию поллинга
  * @param  void  (*send_poll)(void *arg)	                                указатель на обработчик передачи пакета
  * @retval uint8_t 0 - сервер проинициализирован успешно
  *								  1 - не удалось открыть pcb 
  *									2 - не удалось назначить номер для IP pcb, pcb - удален
  */
uint8_t Create_Server( tcp_server_t *str_srv, 
                      tcp_connect_t *conn_serv, 
                      uint8_t size_conn_serv,   
                      uint16_t IP_port,	
                      void	(*recv_cb)(void *arg, struct tcp_pcb *tpcb, struct pbuf *p),
                      err_t     (*poll_cb)(void *arg, struct tcp_pcb *tpcb),
                      void      (*send_poll)(void *arg));

/**
  * @brief  Функция функция контроля соединения TCP сервера
  * @param  tcp_server_t *loader_srv_tcp,  указатель на структуру сервера
  * @retval None
  */
void eth_control_connect(tcp_server_t *str_srv );

// Создать структура для сервисной команды
typedef struct
{
  union
  {
    struct
    {
      uint8_t     local_cmd;                // Локальная команда
      uint8_t     local_data;               // Данные локальной команды
    };
    uint16_t    res;                          // Резерв 2 байта
  }; 
  ip_addr_t 	ReqIPaddr;	        // IP address рассылки
  uint16_t 	ReqIPport;	        // UDP port рассылки
}service_cmd_t; 

#endif /* __ETH_TCP_H */

/************************ (C) COPYRIGHT DEX *****END OF FILE****/

