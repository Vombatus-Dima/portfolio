/**
  ******************************************************************************
  * @file    eth_rs_tube.c
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

/* Includes ------------------------------------------------------------------*/
#include "eth_rs_tube.h"
#include "GenerationCRC16.h"
#include "rs_frame_header.h"
#include "router_streams.h"
#include "Core.h"
#include "loader_settings.h"

/* максимальное число соединений число соединений сервера */
#define  MAX_NUMBER_CONNECT_TUBE_RS_SERVER              (3)
/* период вызова функции Control_Server 1 млсек           */
#define  TIME_CALL_CONTROL_TUBE_RS_SERVER               (40)
/* максимальное вреня неактивности соединения в секундах  */	 
#define  TIME_MAX_NOACTIVE_CONNECT_TUBE_RS_SERVER       (300) 

/* Private typedef -----------------------------------------------------------*/
//структура соединений сервера
tcp_connect_t eth_rs_conn[MAX_NUMBER_CONNECT_TUBE_RS_SERVER];
//===========================================================
//= обьявление переменных для подключения сервера к роутеру =
//структура подключения к роутеру 
tcp_tube_rs_t   eth_rs;
//===========================================================

/**
  * @brief  Функция диагностики соединения шлюза RS-UDP позиционирования и диагностики.
  * @param uint8_t *NumLink  - указатель на переменную подсчета активных соединений
  * @param uint32_t *NumTxData      - указатель на переменную подсчета переданных пакетов
  * @param uint32_t *NumRxData      - указатель на переменную подсчета полученных пакетов
  * @retval None
  */
void ReqDiagEthRS( uint8_t *NumLink , uint32_t *NumTxData , uint32_t *NumRxData )
{
  *NumLink   = get_number_active_link(&eth_rs.eth_tube_rtr);     /* Подсчет активных соединений        */
  *NumTxData = eth_rs.eth_tube_rtr.info_serv.Box_tx;                      /* Подсчет отправленных пакетов       */
  *NumRxData = eth_rs.eth_tube_rtr.info_serv.Box_rx;                      /* Подсчет полученных пакетов пакетов */
}

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
  #define tcp_tude_st   ((tcp_tube_rs_t*)(eth_conn->tcp_strct))
  #define buf_rs        tcp_tude_st->buf_eth_rtr  
  
  /* принятые данные поместятся в буфер? */
  if (p->tot_len >= sizeof(router_box_t)) return;
  /* копируем принятые данные во временный массив */
  pbuf_copy_partial(p, tcp_tude_st->pdamp_buf_eth_rtr , p->tot_len, 0);
  
  /* проверка пакета на соответствие формату */
  if (buf_rs.pre != 0xAA55) return;                           /* Преамбула  0x55 0xAA */
  
  if (buf_rs.lenght > (sizeof(router_box_t) - SIZE_LENGHT - SIZE_PRE)) return; /* Длина пакета (включая контрольную сумму, не включая преамбулу) */
  /* заполнение полей адреса */
  buf_rs.dest = 0xFFFF;                  /* Физический адрес получателя */
  
   /* если очередь инициализирована */ 
  if (tcp_tude_st->set_port_router.QueueInRouter != NULL)
  { /* отправляем данные для роутера */
    xQueueSend(tcp_tude_st->set_port_router.QueueInRouter,&buf_rs, 0);
    /* Если группа событий активна - отправляем флаг заполнения буфера */
    if ( ( tcp_tude_st->set_port_router.HandleTaskRouter ) != NULL )
    {
      xTaskNotify( tcp_tude_st->set_port_router.HandleTaskRouter,                     /* Указатель на уведомлюемую задачу                         */
                  ( tcp_tude_st->set_port_router.PortBitID ) << OFFSET_NOTE_PORD_ID,  /* Значения уведомления                                     */
                  eSetBits);                                                          /* Текущее уведомление добавляются к уже прописанному       */
    } 
  }    
}		

/**
  * @brief  Функция отправки сообщений от роутера в ETH
  * @param  tcp_tube_rs_t* tcp_tube_rs  указатель на структуру контроля шлюза 
  * @retval None
  */
void rs_eth_send(tcp_tube_rs_t* tcp_tube_rs)
{	
  /* если очередь инициализирована */ 
  if (tcp_tube_rs->set_port_router.QueueOutRouter != NULL)
  { 
    /* проверяем наличие данных */
    while(xQueueReceive(tcp_tube_rs->set_port_router.QueueOutRouter,&tcp_tube_rs->buf_rtr_eth, 0 ) == pdTRUE)
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

/*============================================================================*/
/*======================== Программный таймер порт RS ========================*/
/*============================================================================*/
/*
  * @brief  функция обработки програмного таймера.
  * @param  TimerHandle_t pxTimer указатель на таймер
  * @retval None
  */
void SOFT_TIM_EthRS_Handler( TimerHandle_t pxTimer )
{
  if ( ( eth_rs.set_port_router.HandleTask ) != NULL  )
  {
    if ( eth_rs.xSoftTimer == pxTimer )
    {/* Устанавливаем событие срабатывания програмного таймера для периодического уведомления */
      xTaskNotify( eth_rs.set_port_router.HandleTask, /* Указатель на уведомлюемую задачу                         */
                  TIMER_NOTE,                                  /* Значения уведомления                                     */
                  eSetBits );                                  /* Текущее уведомление добавляются к уже прописанному       */
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
  /* Открытие очереди для пакетов ROUTER_ETH для шлюза */
  if (eth_rs.set_port_router.QueueOutRouter == NULL)
  { /* открытие очереди */
    eth_rs.set_port_router.QueueOutRouter = xQueueCreate( eth_rs.SizeQueueOutRouter , sizeof(router_box_t));
  }	
  
  /* Открытие очереди для пакетов ETH_ROUTER для шлюза  */ 
  if (eth_rs.set_port_router.QueueInRouter == NULL)
  { /* открытие очереди */
    eth_rs.set_port_router.QueueInRouter = xQueueCreate( eth_rs.SizeQueueOutRouter , sizeof(router_box_t));
  }  
 
  /*===================== Подключение к порту роутера =========================*/
  /* Запрашиваем ID порта маршрутизации */
  while(request_port_pnt(&(eth_rs.port_index_tab)) != true)
  {
    vTaskDelay(1);
  }
  /* Инициализация порта */
  settings_port_pnt(eth_rs.port_index_tab,&(eth_rs.set_port_router));

  /* ожидаем соединения по ETH */
  while( StatusLinkETH() == false)
  {
    vTaskDelay(1);
  } 
  // Включаем порт
  enable_port_pnt(eth_rs.port_index_tab);
  /*===========================================================================*/

  /* инициализация сервера для моста ETH RS */
  Create_Server(&eth_rs.eth_tube_rtr, 
                eth_rs.conn, 
                eth_rs.max_connect,   
                eth_rs.ip_port,
                eth_rs.recv_cb,
                eth_rs.poll_cb,
                NULL);  

  /* Запуск програмного таймера */
  xTimerStart( eth_rs.xSoftTimer , 0 );
  
  /* Цикл периодического запуска задачи                                       */ 
  while (1) 
  {
    /* Обнуляем сообщение */
    eth_rs.NotifiedValue = 0;
    /*================================== Проверка наличия сообщений ========================================*/
    xTaskNotifyWait(0x00000000,                         /* Не очищайте биты уведомлений при входе               */
                    0xFFFFFFFF,                         /* Сбросить значение уведомления до 0 при выходе        */
                    &(eth_rs.NotifiedValue),           /* Значение уведомленное передается в  NotifiedValue    */
                    portMAX_DELAY  );                   /* Блокировка задачи до появления уведомления           */
    /* Получено уведомление. Проверяем, какие биты были установлены. */
    /*=========================================================================*/
    /*=========================================================================*/
    /*=========================================================================*/ 
    if( ( ( eth_rs.NotifiedValue ) & ROUTER_NOTE ) != 0 )
    {  
      /*  контроль отправки полученных данных                                   */
      rs_eth_send(&eth_rs);
    }        
    /*=========================================================================*/
    /*=========================================================================*/      
    if( ( ( eth_rs.NotifiedValue ) & TIMER_NOTE ) != 0 )
    { /* Периодическое уведомление по таймеру */
      /*  контроль открытых ТСР соединений                                      */
      eth_control_connect(&eth_rs.eth_tube_rtr);
      /*  контроль отправки полученных данных                                   */
      rs_eth_send(&eth_rs);
    }
    /*=========================================================================*/
    /*=========================================================================*/
    /*=========================================================================*/     
  }  
}

/**
  * @brief  Функция создания сокета TCP для формирование тунеля ETH - RS  
  * @param  None
  * @retval None
  */
void Init_TCP_tube_RS(void)
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
  /* пропишем подключение к внутреннему маршрутизатору                        */
  /*===================================================*/
  /* Проводим инициализацию */
  eth_rs.set_port_router.FlagLockAddr = LOCK_ADDR;                            	   /* блокирует трансляцию в порт со своим адресом    */
  eth_rs.set_port_router.MaskPortID = DataSto.Settings.mask_inpup_port_id_config;  /* установка маску ID портов                       */
  eth_rs.set_port_router.NumMaskBoxID =  DataSto.Settings.nmask_transl_config;     /* используем маску ID пакетов                     */
  eth_rs.set_port_router.PortID = ETHtubeRSPortID;                                 /* идентификатор порта                             */
  eth_rs.set_port_router.PortBitID = BitID(ETHtubeRSPortID);                       /* идентификатор порта                             */
  eth_rs.set_port_router.HandleTaskRouter = NULL;                                  /* Обнуление указателя на задачу роутера           */
  /*===================================================*/  
  /* Устанавливаем размер очереди для пакетов ROUTER_ETH для шлюза */  
  eth_rs.SizeQueueOutRouter = 4;  
  /* Устанавливаем размер очереди для пакетов ETH_ROUTER для шлюза */
  eth_rs.SizeQueueInRouter = 2; 
    
  /* Открытие таймера периодического уведомления задачи */
  eth_rs.xSoftTimer = xTimerCreate( "TmEthRS",                 /* Текстовое имя, не используется в RTOS kernel. */
                                   eth_rs.time_control_server, /* Период таймера в тиках. */
                                   pdTRUE,                     /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                   NULL,                       /* В функцию параметры не передаем */
                                   SOFT_TIM_EthRS_Handler );   /* Указатель на функцию , которую вызывает таймер. */
  
  /* запуск задачи управления мостом ETH Radio */
  xTaskCreate(eth_rs_task, "EthRS", configMINIMAL_STACK_SIZE * 2.5, &eth_rs ,_ETH_THREAD_PRIO_, &(eth_rs.set_port_router.HandleTask) );    
}
