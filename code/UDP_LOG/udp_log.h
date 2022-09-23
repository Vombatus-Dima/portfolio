/**
  ******************************************************************************
  * @file    udp_log.h
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
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UDP_LOG_H
#define __UDP_LOG_H

#if UDP_LOGGER == 1     
    
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "lwip/ip_addr.h"
#include "rs_frame_header.h"

/* перечисление кодов команд запроса */

#define CMD_LOG_BREAK      0x30
#define CMD_LOG_RF_TX      0x31
#define CMD_LOG_RF_RX      0x32
#define CMD_LOG_RSA_TX     0x33
#define CMD_LOG_RSA_RX     0x34
#define CMD_LOG_RSB_TX     0x35
#define CMD_LOG_RSB_RX     0x36

/* Определяем очередь для запроса статистики FreeRTOS */
extern QueueHandle_t  xQueueLoggerUDP;
 
/* Объявление флагов логирования */
extern bool LoggerTxRF;
extern bool LoggerRxRF;      
extern bool LoggerTxRSA;      
extern bool LoggerRxRSA;   
extern bool LoggerTxRSB;      
extern bool LoggerRxRSB;     

/* Указатель для уведомления задачи */
extern TaskHandle_t   LoggerHandleTask;  
/* Определяем очередь для лога */
extern QueueHandle_t xQueueLogger; 

/* структура для запроса лога */
typedef struct
{
  ip_addr_t 	ReqIPaddr;	/* IP address источника запроса    */
  uint16_t 	ReqIPport;	/* UDP port источника запроса	   */
  uint8_t     	cmd_cod;	/* код команды запроса статистики  */
}req_logger_t;

/* структура для адреса получателя лога */
typedef struct
{
  ip_addr_t 	DestIPaddr;	/* IP address источника запроса          */
  uint16_t 	DestIPport;	/* UDP port источника запроса	         */
  union
  {
    uint16_t     flag_control;
    struct
    {
      uint16_t   flag_rf_rx  : 1, /*  */
                 flag_rf_tx  : 1, /*  */
                 flag_rsa_rx : 1, /*  */
                 flag_rsa_tx : 1, /*  */
                 flag_rsb_rx : 1, /*  */
                 flag_rsb_tx : 1, /*  */
                 reserv      : 10;/*  */
    };
  }; 
}dest_logger_t;

/* структура для адреса получателя лога */
typedef struct
{
  union  /* Буфер данных          */
  {
    uint8_t            damp_data[300];
    struct
    {
      uint8_t      Logger_data_size;  /* Размер данных                   */       
      uint8_t      Logger_type_code;  /* Тип данных                      */
      uint32_t     Logger_time_cod;   /* Резери данных под временной код */  
      union  /* Буфер данных         */
      {
        uint8_t            damp_data_box[sizeof(router_box_t)];
        router_box_t  data_box;   
      };
    };
  };
}logger_data_box_t;                 
      
/**
  * @brief  Функция создания сокета UDP для логирования потоков
  * @param  None
  * @retval None
  */
void Init_UDP_Logger(void);

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
    
#endif
#endif /* __UDP_LOG_H */
/************************ (C) COPYRIGHT DEX *****END OF FILE****/