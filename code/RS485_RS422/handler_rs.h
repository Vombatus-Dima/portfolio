/**
  ******************************************************************************
  * @file    handler_rs.h
  * @author  Trembach Dmitry
  * @version V1.0.0
  * @date    22-10-2020
  * @brief   Файл описания функций rs вызываемых по прерыванию и по таймеру
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 DataExpress</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HANDLER_RS_H
#define __HANDLER_RS_H


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "uart_control.h"
#include "main.h"

#if RS_GATE_ENABLE == 1
/**
  * @brief  функция обработки прерывания передатчика - буфер передатчика пуст.
  * @param  RS_struct_t* PortRS указатель на структуру порта
  * @retval None
  */
void RS_TXE_Handler(RS_struct_t* PortRS);

/**
  * @brief  функция обработки прерывания приемника - принят байт.
  * @param  RS_struct_t* PortRS указатель на структуру порта
  * @retval None
  */
void RS_RXNE_Handler(RS_struct_t* PortRS);

/**
  * @brief  функция обработки прерывания приемника - прием пакета завершен.
  * @param  RS_struct_t* PortRS указатель на структуру порта
  * @retval None
  */
void RS_IDLE_Handler(RS_struct_t* PortRS);

/**
  * @brief  функция обработки прерывания таймера RS_UART.
  * @param  RS_struct_t* PortRS указатель на структуру порта
  * @retval None
  */
void RS_TIM_Handler(RS_struct_t* PortRS);

/**
  * @brief  функция обработки програмного таймера.
  * @param  RS_struct_t* PortRS указатель на структуру порта
  * @retval None
  */
void RS_SOFT_TIM_Handler(RS_struct_t* PortRS , TimerHandle_t pxTimer );
#endif 
#endif /* __HANDLER_RS_H */
/******************* (C)  COPYRIGHT 2020 DataExpress  *****END OF FILE****/
