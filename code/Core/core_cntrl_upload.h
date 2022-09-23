/**
  ******************************************************************************
  * @file    core_cntrl_upload.h
  * @author  Trembach Dmitry
  * @version V1.2.0
  * @date    21-06-2019
  * @brief   файл с функциями ядра для обновления програмного обеспечения
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2019 DataExpress</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CORE_CNTRL_UPLOAD_H
#define __CORE_CNTRL_UPLOAD_H

#include "stm32f4xx.h"
#include "Core.h"
#include "rf_frame_header.h"
#include "rs_frame_header.h"
#include "FreeRTOS.h"
#include "task.h"


/**
  * @brief Функция формирования шапки ответа на запрос обновления програмного обеспечения
  * @param uint8_t data_size - размер передаваемых данных
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  *
  * @retval none
  */
void GenRespUploadBox(core_struct_t* core_st, uint8_t data_lenght);

/**
  * @brief Функция обработка запросов обновления програмного обеспечения
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  *
  * @retval bool true  - пакет обработан
  *              false - пакет не обработан
  */
bool ProcessingUploadBox(core_struct_t* core_st);


#endif /* __CORE_CNTRL_UPLOAD_H */
/******************* (C)  COPYRIGHT 2019 DataExpress  *****END OF FILE****/
