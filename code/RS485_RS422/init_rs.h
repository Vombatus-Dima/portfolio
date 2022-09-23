/**
  ******************************************************************************
  * @file    init_rs.h
  * @author  Trembach Dmitry
  * @version V1.0.0
  * @date    22-10-2020
  * @brief   Файл описания функций инициализации rs по заданным предустановкам 
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 DataExpress</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __INIT_RS_H
#define __INIT_RS_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "main.h"
#include "uart_control.h"
#if RS_GATE_ENABLE == 1
/**
  * @brief  Функция инициализации RS_UART и открытия задачи контроля RS_UART 
  * @param  RS_struct_t* PortRS указатель на структуру порта
  * @retval None
  */
void InitRS_UART( RS_struct_t* PortRS );
#endif
#endif /* __INIT_RS_H */
/******************* (C)  COPYRIGHT 2020 DataExpress  *****END OF FILE****/
