/**
  ******************************************************************************
  * @file    cmx_gate.h
  * @author  Trembach Dmitry
  * @date    19-06-2020
  * @brief   Инициализация задачи сопряжения кодеков и мультиплексора
  *
  ******************************************************************************
  * @attention
  * 
  * Задача контролирующая подключение голосового кодека к мультиплексору.
  * 
  * 1.Получение и парсинг голосовых пакетов из мультиплексора.
  * 2.Подготовка, групировка и отправка голосовых пакетов в мультиплексор.
  * 3.Отправка в кодек пакетов только заданного канала и привязка к источнику сообщения.
  * 4.Отправка в кодек пакетов только с заданного массива.
  * 5.Получение из кодека пакетов и формирование из них групп, формирование шапки 
  * для задданного канала(каналов).
  * 6.Получение и парсинг команд вызова диспетчера 
  * 7.Отправка команды вызова диспетчера 
  * 8.Получение команды вызова диспетчера.
  * 9.Формирование шапки для команды вызова диспетчера и отправки ее в мультиплексор.
  * 
  *
  * <h2><center>&copy; COPYRIGHT 2020 DataExpress</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CMX_GATE_H
#define __CMX_GATE_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "main.h"
#include "loader_settings.h"
#include "board.h"
#include "Core.h"
#include "router_streams.h"
#include "rs_frame_header.h"
#include "codec_cmx.h"    

/**
  * @brief  Функция инициализации порта сопряжения кодека и мультиплексора 
  * @param  None
  * @retval None
  */
void InitPortCodecRouterStreams( void );

/**
  * @brief  Функция подключения порта сопряжения кодека к мультиплексору 
  * @param  None
  * @retval None
  */
void StartPortCodecRouterStreams( void );
/**
  * @brief  Функция приема голосового пакета из stream роутера потоков захват потока канала и отправка в CMX 
  * @param  none
  * @retval bool - true пакет обработан как голосовой
  *                false пакет не обработан или не получен
  */
bool VoiceFrameToCodecCMX( void );

/**
  * @brief  Функция обновления статуса захвата потока каналов
  * @param  uint16_t time_update - период обновления 
  * @retval none
  */
void UpdateVoiceFrameToCodecCMX( uint16_t time_update );

/**
  * @brief  Функция приема голосового пакета от CMX, формирования фрейма и отправки его в роутер 
  * @param  None
  * @retval None
  */
void VoiceFrameToRouterCMX( void );


/**
  * @brief  Функция обновления статуса захвата канала отправки данных с CMX в потоковый роутер 
  * @param  uint16_t time_update - период обновления 
  * @retval None
  */
void UpdateStatusVoiceFrameToRouterCMX( uint16_t time_update );

/**
  * @brief  Функция контроля интерфейсов CMX кодека
  * @param  None
  * @retval None
  */
void ProcessingCMX(void);

#endif /* __CMX_GATE_H */
/******************* (C)  COPYRIGHT 2020 DataExpress  *****END OF FILE****/
