/**
  ******************************************************************************
  * @file    codec_control.h
  * @author  Trembach Dmitry
  * @version V1.0.0
  * @date    19-04-2020
  * @brief   Инициализация задачи контроля SPI кодека
  *
  ******************************************************************************
  * @attention
  * 
  * 
  *
  * <h2><center>&copy; COPYRIGHT 2020 DataExpress</center></h2>
  ******************************************************************************
  */

#ifndef _CODEC_CONTROL_H
#define _CODEC_CONTROL_H

#include "cmx618_reg.h"  
#include "codec_spi.h"
#include "stm32f4xx.h"
#include "main.h"

/* Буфер для хранения пакета полученного из кодека             */
extern uint8_t   codec_to_data[CMX618_VOICE_LEN]; 
/* Буфер для хранения принятого пакета перед передачей в кодек */
extern uint8_t   data_to_codec[CMX618_VOICE_LEN]; 

extern QueueHandle_t      x_codec_message;  

/* Указатель для уведомления задачи контроля данных CMX PCM */
extern TaskHandle_t       handle_codec_task;

/**
  * @brief  Функция инициализации интерфейса CMX кодека
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @retval none
  * 
  */
void cmx_spi_init( pcm_cmx_t* CODECx);  
 
/**
  * @brief  Функция запуска интерфейса CMX кодека
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @retval none
  * 
  */
void cmx_spi_start( pcm_cmx_t* CODECx);

/**
  * @brief  Задача контроля SPI кодеком
  * @param  pvParameters not used
  * @retval None
  */
void codec_control_task(void* pvParameters);

/**
  * @brief  Функция инициализации порта и запуска задачи сопряжения кодека и мультиплексора 
  * @param  None
  * @retval None
  */
void InitCodecTaskControl( void );

/**
  * @brief  Задача для контроля данных кодека
  * @param  pvParameters not used
  * @retval None
  */
void cmx_pcm_data_task(void* pvParameters);

#endif /* _CODEC_CONTROL_H */