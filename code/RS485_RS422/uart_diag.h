/**
  ******************************************************************************
  * @file    uart_diag.h
  * @author  Trembach Dmitry
  * @version V2.6.0
  * @date    12-12-2020
  * @brief   Функции диагностики UART в режимах RS485/RS422
  *
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 DataExpress</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART_DIAG_H
#define __UART_DIAG_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "Core.h"
   
#if RS_GATE_ENABLE == 1 

/**
  * @brief Функция формирования пакета диагностики ресурса UART 
  * @param  RS_struct_t* rs_port_box - указатель на структуру контроля UART
  * @param  uint16_t dest_addr - адрес назначения диагностики
  * @retval None
  */
void GenDiagUART(RS_struct_t* rs_port_box, uint16_t dest_addr);  
    
/**
  * @brief Функция обновления данных для диагностики ресурса UART 
  * @param  RS_struct_t* rs_port_box - указатель на структуру контроля UART
  * @param  uint32_t time_update - время с последнего обновления в млсек
  * @retval None
  */
void UpdateDiagUART(RS_struct_t* rs_port_box, uint32_t time_update);

/**
  * @brief Функция обновления данных для диагностики за малый интервал  
  * @param  RS_struct_t* rs_port_box - указатель на структуру контроля UART
  * @retval None
  */
void UpdateLitteDiagUART(RS_struct_t* rs_port_box);

/**
  * @brief Функция отправки аварийного сообщения в ядро системы 
  * @param RS_struct_t* rs_port_box - указатель на структуру контроля UART
  * @param uint8_t CMD_ID - Идентификатор команды 
  * @retval None
  */
void SetRStoCoreCMD( RS_struct_t* rs_port_box, uint8_t CMD_ID );

#endif
#endif /* __UART_CONTROL_H */
/******************* (C)  COPYRIGHT 2020 DataExpress  *****END OF FILE****/
