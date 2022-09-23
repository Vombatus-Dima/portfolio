/**
  ******************************************************************************
  * @file    codec_pcm.h
  * @author  Trembach Dmitry
  * @version V1.2.0
  * @date    17-07-2020
* @brief   Функции контроля интерфейса PCM кодека CMX
  *
  ******************************************************************************
  * @attention
  * 
  * 
  *
  * <h2><center>&copy; COPYRIGHT 2020 DataExpress</center></h2>
  ******************************************************************************
  */

#ifndef _CODEC_PCM_H
#define _CODEC_PCM_H

#include "cmx618_reg.h"  
#include "codec_spi.h"
#include "stm32f4xx.h"

/**
  * @brief  Инициализация интерфейса PCM голосового кодека
  * @param  pcm_cmx_t* pcm_spi - указатель на порт подключения
  * @retval None
  */
void pcm_spi_init(pcm_cmx_t* pcm_spi);

/**
  * @brief  Функция функция загрузки пакетов из soft_router и контроля захвата потока канала
  * @param  pcm_cmx_t* pcm_cmx - указатель на структуру контроля кодека
  * @retval None
  */
void ProcessingReadBox( pcm_cmx_t* pcm_cmx );

/**
  * @brief  Функция функция обработки уведомления из прерывания DMA PCM
  * @param  pcm_cmx_t* cmx_pcm - указатель на структуру контроля кодека
  * @param  uint8_t n_buffer - номер обрабатываемого буфера
  * @retval None
  */
void ProcessingNotifyDMA( pcm_cmx_t* pcm_cmx , uint8_t n_buffer );

/**
  * @brief  Функция контроля интерфейсов PCM кодека
  * @param  None
  * @retval None
  */
void ProcessingPCM( void );

#endif /* _CODEC_PCM_H */
/******************* (C) COPYRIGHT 2020 DataExpress *****END OF FILE****/