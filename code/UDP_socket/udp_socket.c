/**
  ******************************************************************************
  * @file    udp_socket.c
  * @author  Trembach D.N.
  * @version V2.5.0
  * @date    21-08-2020
  * @brief   Файл содержит функции контроля UDP  
  ******************************************************************************
  * @attention
  * 
  * 
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "lwip/memp.h"
#include <stdio.h>
#include <string.h>
#include "udp_socket.h"
#include "stm32f4x7_eth.h"

/**
  * @brief  Функция инициализации сокетаSocket
  * @param  udp_socket_t * udp_sock указатель на структуру udp сокета
  * @retval статус инициализации сокета
  */
Status_UDP_t UDP_Init_Socket(udp_socket_t * udp_sock)
{
  /* инициализация UDP сокета  */
  udp_sock->status = INIT_UDP;
  
  /* Открываем новый UDP pcb  */
  udp_sock->pcb = udp_new();
  
  if (udp_sock->pcb!=NULL)
  {
    /* Привязываем это ucpcb к UDP_PORT порту */
    /* Использование IP_ADDR_ANY всегда ucpcb использует адрес локального интерфейса*/
    udp_sock->err = udp_bind(udp_sock->pcb, IP_ADDR_ANY, udp_sock->local_port);
    
    if(udp_sock->err == ERR_OK)
    {
      /* Назначаем функцию по приему */
      udp_recv(udp_sock->pcb, udp_sock->recv, udp_sock);
      
      /* UDP сокет готов к работе    */
      udp_sock->status = READY_UDP;
    }
    else
    {
      /*удаление соединения*/
      udp_remove(udp_sock->pcb);
      
      /* Ошибка при установке UDP сокета на прослушку */
      udp_sock->status = ERROR_BIND_UDP;
    }
  }
  else
  {
    /* Ошибка при создании UDP сокета */
    udp_sock->status = ERROR_PCB_UDP;  
  }  
  /* возвращаем статус сокета */
  return  udp_sock->status;
}

/**
  * @brief Эта функция вызывается когда клиент UDP отправляет сообщение.
  * @param uint8_t *data_box указатель на передаваемый буфер       
  * @param uint16_t size_box размер передаваемых данных
  * @param udp_socket_t * udp_sock указатель на структуру соединения
  * @param uint32_t DestIPaddr ip адресс назначения
  * @param uint16_t DestIPport ip порт назначения
  * @retval 0 - пакет отправлен
  *         1 - пакет не отправлен - отсутствует соединение 
  *         2 - пакет не отправлен - ошибка выделения памяти
  */
uint8_t UDP_Send_Socket(uint8_t *data_box ,uint16_t size_box, udp_socket_t *udp_sock,  uint32_t DestIPaddr ,uint16_t DestIPport)
{
  ip_addr_t temp_ip_addr;
  
  if (link_ethernet() == true)
  {
    /* allocate pbuf from pool*/
    udp_sock->p = pbuf_alloc(PBUF_TRANSPORT, size_box, PBUF_POOL);
    if (udp_sock->p!= NULL)
    {
      /* анализируем тип передаваемого пакета   */
      /* copy data to pbuf                      */
      pbuf_take(udp_sock->p, (char*)data_box, size_box);
      /* преобразование формата ip адреса назначения */
      temp_ip_addr.addr = DestIPaddr;
      /* Send data to a specified address using UDP.*/
      udp_sendto(udp_sock->pcb, /* pcb UDP PCB used to send the data.  */
                 udp_sock->p,  	/* p chain of pbuf's to be sent.       */
                 &temp_ip_addr, /* dst_ip Destination IP address.      */
                 DestIPport);  	/* dst_port Destination UDP port.      */
      /* free pbuf */ 
      pbuf_free(udp_sock->p);
      return 0;/* пакет отправлен */      
    }
    else
    {
      return 2;/* пакет не отправлен - ошибка выделения памяти */
    }  
  }
  else
  {
    return 1;/* пакет не отправлен - отсутствует соединение */
  }  
}

/* Для удобства визуализации в задаче определяем дефайн */
#define UDPSocket  ((udp_socket_t *)pvParameters) 

/**
  * @brief  Задача для контроля сокета UDP
  * @param  *pvParameters - указатель на структуру соединения
  * @retval None
  */
void UDP_Task_Socket(void * pvParameters)
{	
  /* Если функция инициализации ресурсов задачи определена */
  if (UDPSocket->init_task!=NULL)  
  { /* Запуск функции */
    UDPSocket->init_task(UDPSocket);
  }
  /* инициализация сокета только после установки соединения ethernet */
  while(link_ethernet() != true)
  {
    /* Соединение не установлено - ждем 10 тиков RTOS */
    vTaskDelay(10);
  }
  
  /* Инициализация сокета UDP */
  UDP_Init_Socket(UDPSocket);
  
  /* Проверка статуса инициализации */
  while(UDPSocket->status != READY_UDP)
  {
    /* Инициализация сокета UDP прошла некоректно */
    vTaskDelay(10);
  } 
  
  /* Eсли указатель на функцию callback таймера нулевой */  
  if ( UDPSocket->SoftTimerCB == NULL )
  {
    /* запускаем периодический запуск задачи 	                                 */
    /* Инициализация переменной xLastWakeTime на текущее время.                  */
    UDPSocket->xLastWakeTime = xTaskGetTickCount();   
  }
  else
  {
    /* открытие програмного таймера  */
    UDPSocket->SoftTimerForTask = xTimerCreate(  UDPSocket->pcSoftTimerName,      /* Назначим имя таймеру, только для отладки.    */
                                                 UDPSocket->control_time,         /* Период таймера в тиках.                      */                     
                                                 pdTRUE,                          /* Автоперезагрузка включена.                   */
                                                 NULL,                            /* Нет связи с индивидуальным индентификатором. */
                                                 UDPSocket->SoftTimerCB );        /* Функция обратного вызова вызываемая таймером.*/    
 
    /* Запустить таймер                                              */
    xTimerStart( UDPSocket->SoftTimerForTask, 0 );
  }  
  
    
  /* цикл периодического запуска задачи */
  for( ;; )
  {
    /* Eсли указатель на функцию callback таймера нулевой */
    if ( UDPSocket->SoftTimerCB == NULL )
    {
      /* ожидание до следующего цикла.*/
      vTaskDelayUntil( &UDPSocket->xLastWakeTime, UDPSocket->control_time); 
    }
    else
    {
      /* Обнуление сообщения*/
      UDPSocket->NotifiedValue = 0x00000000;
      /* Ожидаем сообщения в задачу */
      xTaskNotifyWait(0x00000000,                          /* Не очищайте биты уведомлений при входе               */
                          0xFFFFFFFF,                      /* Сбросить значение уведомления до 0 при выходе        */
                          &(UDPSocket->NotifiedValue),     /* Значение уведомленное передается в  ulNotifiedValue  */
                      portMAX_DELAY );                 /* Вызываем задачу через 1 сек если не было сообщений   */
    }  
    /* Если функция основного тела задачи определена */
    if (UDPSocket->body_task!=NULL)  
    { /* Запуск функции */
      UDPSocket->body_task(UDPSocket);
    }
  };    
}
/************************ (C) COPYRIGHT DEX *****END OF FILE****/
