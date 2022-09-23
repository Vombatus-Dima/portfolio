/**
  ******************************************************************************
  * @file    pre_set_rs.h
  * @author  Trembach Dmitry
  * @version V3.0.0
  * @date    22-10-2020
  * @brief   Файл описания функций предустановки rs uart
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 DataExpress</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PRE_SET_RS_H
#define __PRE_SET_RS_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "uart_control.h"
#include "main.h"

#if (RS_GATE_ENABLE == 1)

/**
  * @brief  Функция обнуления счетчиков событий.
  * @param  None
  * @retval None
  *        
  */
void ResetEventCountersRSA( void );

/**
  * @brief  Функция инициализации порта для подключения RS.
  * @param  None
  * @retval None
  */
void Init_Port_RS_A( void );

/**
  * @brief  Функция запроса статуса соединения по RS.
  * @param  None
  * @retval bool true - есть соединение
  *                   - нет соединения
  */
bool StatusLinkRSA( void );

/**
  * @brief  Функция запроса статуса аварий RS.
  * @param  None
  * @retval bool true - есть авария
  *                   - нет аварии
  */
bool GetAlarmRSA( void );

/**
  * @brief  Функция запроса адреса соседа по порту RS.
  * @param  None
  * @retval uint16_t - адреса соседа по порту RS
  *        
  */
uint16_t GetNearPhyAddA( void );

/**
  * @brief  Функция обнуления счетчиков событий.
  * @param  None
  * @retval None
  *        
  */
void ResetEventCountersRSB( void );

/**
  * @brief  Функция инициализации порта для подключения RS.
  * @param  None
  * @retval None
  */
void Init_Port_RS_B( void );

/**
  * @brief  Функция запроса статуса соединения по RS.
  * @param  None
  * @retval bool true - есть соединение
  *                   - нет соединения
  */
bool StatusLinkRSB( void );

/**
  * @brief  Функция запроса статуса аварий RS.
  * @param  None
  * @retval bool true - есть авария
  *                   - нет аварии
  */
bool GetAlarmRSB( void );

/**
  * @brief  Функция запроса адреса соседа по порту RS.
  * @param  None
  * @retval uint16_t - адреса соседа по порту RS
  *        
  */
uint16_t GetNearPhyAddB( void );

#endif
#endif /* __PRE_SET_RS_H */
/******************* (C)  COPYRIGHT 2020 DataExpress  *****END OF FILE****/
