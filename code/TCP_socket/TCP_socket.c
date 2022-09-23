/**
  ******************************************************************************
  * @file    TCP_socket.c
  * @author  Trembach D.N.
  * @version V2.6.0
  * @date    24-03-2020
  * @brief   Файл содержит описания  
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
#include "TCP_socket.h"

/* Private typedef -----------------------------------------------------------*/
/* Прописываем указатель на функцию контроля соединение ethernet */
#define link_ethernet()   (StatusLinkETH())   /* флаг LwIP - состояние физического соединения */

//аргумент структура сервера	
#define st_srv ((tcp_server_t*)arg)

//аргумент структура соединения	
#define st_con ((tcp_connect_t*)arg)

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Функция возвращает число соединений сервера
  * @param  tcp_server_t *loader_srv_tcp,  указатель на структуру сервера
  * @retval uint8_t - число соединений сервера
  */
uint8_t get_number_active_link(tcp_server_t *str_srv )
{
  uint8_t cnt_active_link;
  
  cnt_active_link = 0;
  // контроль активности соединений
  for(uint8_t cnt_conn = 0;cnt_conn < str_srv->max_connect;cnt_conn++)
  {
    //проверка на состояния соединения
    if (str_srv->conn[cnt_conn].pcb != NULL)
    {
      //соединение установлено - подсчет
      cnt_active_link++; 
    }
  }
  return cnt_active_link;
}

/**
  * @brief  Функция функция контроля соединения TCP сервера
  * @param  tcp_server_t *loader_srv_tcp,  указатель на структуру сервера
  * @retval None
  */
void eth_control_connect(tcp_server_t *str_srv )
{
  if (link_ethernet())
  {		
#ifdef TCP_Server_Stat	 /* статистика сервера */
    /* подсчет времени без обрывного соединения	 */	
    /* подсчет миллисекунд                       */
    str_srv->info_serv.Time_of_break_ms = str_srv->info_serv.Time_of_break_ms + str_srv->time_control_server;
    /* если насчитано более 1000 миллисекунд */
    if (str_srv->info_serv.Time_of_break_ms > 1000)
    {
      str_srv->info_serv.Time_of_break_ms = str_srv->info_serv.Time_of_break_ms - 1000;
      /* подсчет секунд */
      str_srv->info_serv.Time_of_break++;
    }					
#endif	
  }
  else
  {
#ifdef TCP_Server_Stat	/*статистика сервера*/
    /* подсчет времени без обрывного соединения	 */	
    /* подсчет миллисекунд                       */
    str_srv->info_serv.Time_of_break_ms = 0;
    /* подсчет секунд */
    str_srv->info_serv.Time_of_break = 0;
    
#endif	
  }
  
  /*  контроль активности соединений */
  for(str_srv->cnt_conn = 0;str_srv->cnt_conn < str_srv->max_connect;str_srv->cnt_conn++)
  {
    /* проверка на состояния соединения */
    if (str_srv->conn[str_srv->cnt_conn].pcb != NULL)
    {
      /* соединение установлено                                 */
      /* подсчет времени неактивности соединения в миллесекундах */
      str_srv->conn[str_srv->cnt_conn].time_no_active_ms = str_srv->conn[str_srv->cnt_conn].time_no_active_ms + str_srv->time_control_server; 
      /* подсчет времени неактивности соединения в секундах */
      if (str_srv->conn[str_srv->cnt_conn].time_no_active_ms > 1000) 
      {
        /* Убираем 1000 млсек */
        str_srv->conn[str_srv->cnt_conn].time_no_active_ms = str_srv->conn[str_srv->cnt_conn].time_no_active_ms - 1000;
        /* Прибавим секунду */
        str_srv->conn[str_srv->cnt_conn].time_no_active_sec++;
        
        /* если соединение неактивно в течении заданного интервала - отключаем соединение */
        if (str_srv->conn[str_srv->cnt_conn].time_no_active_sec > str_srv->max_time_no_active_sec)
        {
          /* обнуление времени неактивности соединения */
          str_srv->conn[str_srv->cnt_conn].time_no_active_sec = 0;
          str_srv->conn[str_srv->cnt_conn].time_no_active_ms = 0;								 
          /* отключение соединения  */
          Close_Connect(str_srv->conn[str_srv->cnt_conn].pcb, &str_srv->conn[str_srv->cnt_conn]);
          
        }							 
      }
    }
    else
    {
      /* соединение не установлено                         */
      /* обнуление времени неактивности соединения         */
      str_srv->conn[str_srv->cnt_conn].time_no_active_sec = 0;
      str_srv->conn[str_srv->cnt_conn].time_no_active_ms = 0;							
    }	
  }
}

/**
  * @brief  Эта функция закрывает tcp соединение
  * @param  tcp_pcb: указатель на блок контроля соединения
  * @param  es: указатель на структуру соединения
  * @retval None
  */
void Close_Connect(struct tcp_pcb *tpcb, tcp_connect_t *str_con)
{
  /* удаляем все указатели на функции обратного вызова */
  tcp_arg(tpcb, NULL);
  tcp_sent(tpcb, NULL);
  tcp_recv(tpcb, NULL);
  tcp_err(tpcb, NULL);
  tcp_poll(tpcb, NULL, 2);
  
  /* удаляем указатель на блок контроля из структуры соединения */
  if (str_con->pcb != NULL)
  {
    str_con->pcb = NULL;
  }  
  //обновляем статус
  str_con->con_state = CON_NONE;
  /* закрываем блок контроля соединения */
  tcp_close(tpcb);
}

/**
  * @brief  Функция обработки обратного вызова tcp_err callback (вызывается 
  *         когда произошла фатальная ошибка соединения. 
  * @param  arg: указатель на аргумент 
  * @param  err: not used
  * @retval None
  */
void Error_Server(void *arg, err_t err)
{
  // ошибку не анализируем (пока!)
  LWIP_UNUSED_ARG(err);
  // закрываем соединение
  Close_Connect(st_con->pcb,st_con);
}

/**
  * @brief  Эта функция используется для передачи данных в tcp connection
  * @param  tpcb: указатель на блок управления tcp_pcb соединения
  * @param  es: указатель на структуру соединения
  * @retval None
  */
void Send_Server(struct tcp_pcb *tpcb, tcp_connect_t *str_con)
{
  str_con->wr_err = ERR_OK;
  
  while ((str_con->wr_err == ERR_OK) && (str_con->p != NULL))
  {
    if (str_con->p->len > tcp_sndbuf(tpcb)) 
    {
      /* освобождаем буфер */ 
      pbuf_free(str_con->p);
    }
    else
    {  
      str_con->wr_ptr = str_con->p;
      
      /* enqueue data for transmission */
      str_con->wr_err = tcp_write(tpcb, str_con->wr_ptr->payload, str_con->wr_ptr->len, 1);
      
      str_con->wr_err = tcp_output(tpcb);
      
      if (str_con->wr_err == ERR_OK)
      {
        str_con->wr_len = str_con->wr_ptr->len;
        /* continue with next pbuf in chain (if any) */
        str_con->p = str_con->wr_ptr->next;
        
        if(str_con->p != NULL)
        {
          /* new reference! */
          pbuf_ref(str_con->p);
        }
        /* chop first pbuf from chain */
        pbuf_free(str_con->wr_ptr);
        /* we can read more data now */
        tcp_recved(tpcb, str_con->wr_len);
      }
      else 
      {  
        /* освобождаем буфер */ 
        pbuf_free(str_con->p);
      }
    }
  }
}  

/**
  * @brief  Функция обработки обратного вызова tcp_sent LwIP callback (вызывается 
  *         когда было подтверждения полученных данных от удаленного хоста) 
  * @param  None
  * @retval None
  */
err_t Sent_Server(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
  LWIP_UNUSED_ARG(len);
  
  if(st_con->p != NULL)
  {
    /* не все данные отправлены отправляем еще */
    Send_Server(tpcb, st_con);
  }
  else
  {
    /* нет больше данных для отправки и клиент закрыл соединение */
    if(st_con->con_state == SRV_CLOSING)
    {
      Close_Connect(tpcb, st_con);
    }	
  }
  return ERR_OK;
}

/**
  * @brief  Функция обработки обратного вызова tcp_recv LwIP
  * @param  arg: указатель на аргумент для tcp_pcb соединения
  * @param  tpcb: указатель на блок контроля соединения tcp_pcb
  * @param  pbuf: указатель на принятый pbuf
  * @param  err: error информация относительно принятого pbuf
  * @retval err_t: error code
  */
err_t Recv_Server(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
  LWIP_ASSERT("arg != NULL",arg != NULL);
  
  /* если принятый пакет пустой */
  if (p == NULL)
  {
    /* устанавливаем статус закрытого соединения */
    st_con->con_state = CON_CLOSING;
    
    /* проверка наличия неотправленых сообщений*/
    if(st_con->p == NULL)
    {
      /* мы закончили отправку - закрыть соединение */
      Close_Connect(tpcb, st_con);
    }
    else
    {
      /* мы еще не закончили отправку */
      /* отправляем оставшееся сообщение*/
      Send_Server(tpcb, st_con);
    }
    st_con->rcv_err = ERR_OK;
  }  
  else /* иначе : если получен непустой кадр но с ошибкой по какой либо причине != ERR_OK */
  {	
    if(err != ERR_OK)
    {
      /* освобождаем принятый буфер*/
      st_con->p = NULL;
      pbuf_free(p);
      st_con->rcv_err = err;
    }
    else 
    {	
#ifdef TCP_Server_Stat	//статистика сервера		
      //подсчет принятых пакетов
      ((tcp_server_t*)(st_con->tcp_strct))->info_serv.Box_rx++;
      
      // подсчет отправленой информации
      if( (((tcp_server_t*)(st_con->tcp_strct))->info_serv.Data_Rx_LO) > (((tcp_server_t*)(st_con->tcp_strct))->info_serv.Data_Rx_LO +  p->tot_len))
      {
        //посчет старшего значения
        ((tcp_server_t*)(st_con->tcp_strct))->info_serv.Data_Rx_HI++;
      }
      //подсчет младшего значения  
      ((tcp_server_t*)(st_con->tcp_strct))->info_serv.Data_Rx_LO = ((tcp_server_t*)(st_con->tcp_strct))->info_serv.Data_Rx_LO +   p->tot_len;
#endif	
      
      //обнуление таймера неактивности							
      st_con->time_no_active_ms = 0;
      st_con->time_no_active_sec = 0;							
      
      
      if(st_con->con_state == CON_ACCEPTED)
      {
        /* Первый блок данных соединения */
        st_con->con_state = CON_RECEIVED;
        
        /* Вызываем обработчик принятого пакета*/
        ((tcp_server_t*)(st_con->tcp_strct))->recv_cb(arg, tpcb, p);//Recv_Packet(arg, tpcb, p);//
        
        /* восстанавливаем актуальный размер окна */
        tcp_recved(tpcb, p->tot_len);	
        
        /* освобождаем принятый блок*/
        pbuf_free(p);
        
        st_con->rcv_err = ERR_OK;
      }
      else
      {
        if (st_con->con_state == CON_RECEIVED)
        {
          
          /* Вызываем обработчик принятого пакета*/
          ((tcp_server_t*)(st_con->tcp_strct))->recv_cb(arg, tpcb, p);//Recv_Packet(arg, tpcb, p);//
          
          /* восстанавливаем актуальный размер окна */
          tcp_recved(tpcb, p->tot_len);	
          
          /* освобождаем принятый блок*/ 								
          pbuf_free(p);
          
          st_con->rcv_err = ERR_OK;
        }
        
        /* данные получены при уже закрытом соединении */
        else
        {
          /* восстанавливаем актуальный размер окна */
          tcp_recved(tpcb, p->tot_len);
          
          /* освобождаем буффер и ничего не делаем */
          st_con->p = NULL;
          pbuf_free(p);
          st_con->rcv_err = ERR_OK;
        }
      }
    } 	
  }			
  return st_con->rcv_err;
}

/**
  * @brief  Функция обработки обратного вызова  the tcp_poll LwIP 
  * @param  arg: указатель на аргумент для tcp_pcb соединения
  * @param  tpcb: указатель на блок контроля соединения tcp_pcb
  * @retval err_t: error code
  */
err_t Polling_Server(void *arg, struct tcp_pcb *tpcb)
{
  if (st_con != NULL)
  {
    if (st_con->p != NULL)
    {
      /* есть неотправленный pbuf (chain) , отправить данные */
      Send_Server(st_con->pcb, st_con);
    }
    else
    {
      /* нет ожидающих pbuf (chain)  */
      if(st_con->con_state == CON_CLOSING)
      {
        /*  закрыть tcp соединение */
        Close_Connect(st_con->pcb, st_con);
      }
    }
    st_con->pol_err = ERR_OK;
  }
  else
  {
    /* ничего не поделаешь */
    tcp_abort(st_con->pcb);
    st_con->pol_err = ERR_ABRT;
  }
  return st_con->pol_err;
}

/**
  * @brief  This function is the implementation of tcp_accept LwIP callback
  * @param  arg: not used
  * @param  newpcb: указатель на структуру  tcp_pcb для этого открытого tcp соединения
  * @param  err: not used 
  * @retval err_t: error status
  */
err_t Accept_Server(void *arg, struct tcp_pcb *newpcb, err_t err)
{
  tcp_connect_t *str_connect;
  
  LWIP_UNUSED_ARG(err);
  
  /* установить приоритет для вновь принятого tcp соединения pcb */
  tcp_setprio(newpcb, TCP_PRIO_MIN);
  
  //обнуляем указатель
  str_connect = NULL;
  
  //поиск свободной структуры для соединения 	
  for(st_srv->cnt_conn = 0;st_srv->cnt_conn < st_srv->max_connect;st_srv->cnt_conn++)
  {
    // если указатель на pcb нулевой 
    if ((st_srv->conn[st_srv->cnt_conn].pcb) == NULL)   
    {
      //будем использовать данную структуру для соединения 
      str_connect = &(st_srv->conn[st_srv->cnt_conn]);
      break; 
    }
  }
  
  //проверяем указатель на структуру соединения  
  if (str_connect != NULL)
  {	//если указатель не нулевой - заносим в структуру параметры соединения
    str_connect->con_state = CON_ACCEPTED;
    str_connect->pcb = newpcb;
    str_connect->p = NULL;
    //обнуление таймера неактивности							
    str_connect->time_no_active_ms = 0;
    str_connect->time_no_active_sec = 0;		
    
    /* указать вновь выделенную структуру в качестве аргумента*/
    tcp_arg(newpcb, str_connect);
    
    /* инициализации lwip tcp_recv callback функции для этого соединения*/ 
    tcp_recv(newpcb, Recv_Server);
    
    /* инициализации lwip tcp_err callback функции для этого соединения*/
    tcp_err(newpcb, Error_Server);
    
    /* инициализации lwip tcp_poll callback функции для этого соединения*/
    tcp_poll(newpcb, st_srv->poll_cb, 2);
    
    /* нициализации lwip tcp_sent_poll callback функции для этого соединения*/
    tcp_sent(newpcb, Sent_Server);
    
#ifdef TCP_Server_Stat	//статистика сервера		
    /* подсчет числа соединений */
    st_srv->info_serv.Cnt_Connect++;				
#endif	
    
    st_srv->acc_err = ERR_OK;
  }
  else
  {	//нет доступных структур для соединения
    /*  закрыть pcb соединенние */
    Close_Connect(newpcb, str_connect);
    /* возвратить ошибку выделения памяти */
    st_srv->acc_err = ERR_MEM;
  }
  return st_srv->acc_err;  
}

/**
  * @brief  Инициализация tcp сервера
  * @param  tcp_server_t *loader_srv_tcp  указатель на структуру сервера
  * @retval uint8_t 0 - сервер проинициализирован успешно
  *								  1 - не удалось открыть pcb 
  *									2 - не удалось назначить номер для IP pcb, pcb - удален
  */
uint8_t Init_Server(tcp_server_t *str_srv)
{
  
  /* сохраняем указатель на структуру сервера */
  str_srv->tcp_strct = str_srv;
  
  /* открытие блока контроля tcp */
  str_srv->pcb = tcp_new();
  
  /* если блок контроля открыт */
  if (str_srv->pcb != NULL)
  {
    /* назначение номера IP порта блоку контроля  */
    str_srv->init_err = tcp_bind(str_srv->pcb, IP_ADDR_ANY, str_srv->IP_port);
    
    /* номер установлен успешно */
    if (str_srv->init_err == ERR_OK)
    {
      
      /* запуск прослушки порта */
      str_srv->pcb = tcp_listen(str_srv->pcb);
      
      /* указать структуру в качестве аргумента*/
      tcp_arg(str_srv->pcb, str_srv);
      
      /* инициализация функции обратного вызова */
      tcp_accept(str_srv->pcb, Accept_Server);

      /* 0 - сервер проинициализирован успешно */
      return 0;					
    }
    else 
    {
      /* освобождение и удаление блока контроля pcb */
      memp_free(MEMP_TCP_PCB, str_srv->pcb);
      /* не удалось назначить номер для IP pcb, pcb - удален */
      return 2;
    }
  }
  else
  {
    /*блок контроля создать не удалось*/
    return 1;
  }
}

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
  *								        1 - не удалось открыть pcb 
  *									2 - не удалось назначить номер для IP pcb, pcb - удален
  */
uint8_t Create_Server( tcp_server_t *str_srv, 
                      tcp_connect_t *conn_serv, 
                      uint8_t size_conn_serv,   
                      uint16_t IP_port,	
                      void	(*recv_cb)(void *arg, struct tcp_pcb *tpcb, struct pbuf *p),
                      err_t     (*poll_cb)(void *arg, struct tcp_pcb *tpcb),
                      void      (*send_poll)(void *arg)) 
{
  // инициализация указателя на структуру соединений
  str_srv->conn = conn_serv;
  // установка максимального значения соединений 
  str_srv->max_connect = size_conn_serv;

  // инициализация структур соединений
  for(str_srv->cnt_conn = 0;str_srv->cnt_conn < str_srv->max_connect;str_srv->cnt_conn++)
  {
    //иниализация указателя на структуру сервера
    str_srv->conn[str_srv->cnt_conn].tcp_strct = str_srv;
    /* инициализация индексов соединений */
    str_srv->conn[str_srv->cnt_conn].connect_id = ((str_srv->cnt_conn) + 1);      
    /* инициализация указателей соединений */
    str_srv->conn[str_srv->cnt_conn].pcb = NULL;
  }
  
  //удаленный порт
  str_srv->IP_port = IP_port;      
  
  //установка обработчика принятых пакетов
  str_srv->recv_cb = recv_cb;
  
  //установка обработчика поллинга
  str_srv->poll_cb = poll_cb;		
  
  //установка на обработчик передачи пакета
  str_srv->send_poll = send_poll;	  
  
#ifdef TCP_Server_Stat	//статистика сервера		
  //инициализация статистики
  str_srv->info_serv.Time_of_break = 0;          //время работы без потери link сек
  str_srv->info_serv.Time_of_break_ms = 0;  	 //время работы без потери link млсек		
  str_srv->info_serv.CBreak = 0;           	 //число обрывов 
  str_srv->info_serv.Cnt_Connect = 0;       	 //число выполненых соединений за время работы
  str_srv->info_serv.Box_rx = 0;            	 //число принятых пакетов
  str_srv->info_serv.Box_tx = 0;        	 //число переданных пакетов
  str_srv->info_serv.Data_Rx_HI = 0;           	 //принято данных 
  str_srv->info_serv.Data_Rx_LO = 0;        			
  str_srv->info_serv.Data_Tx_HI = 0;           	 //передано данных 
  str_srv->info_serv.Data_Tx_LO = 0; 
#endif
  //инициализация сервера
  return Init_Server(str_srv);
}
