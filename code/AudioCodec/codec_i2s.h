/**
  ******************************************************************************
  * @file    codec_i2s.h
  * @author  Trembach Dmitry
  * @date    20-07-2020
  * @brief   Функции инициализации интерфейса i2s управления кодеком UDA1380
  *
  ******************************************************************************
  * @attention
  * 
  *
  * <h2><center>&copy; COPYRIGHT 2020 DataExpress</center></h2>
  ******************************************************************************
  */

#ifndef __CODEC_I2S_H__
#define __CODEC_I2S_H__

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "stm32f4xx.h"
#include "stm32f4xx_spi.h"

/* Устанавливаем формат данных для I2S */
#define I2S_STANDARD_PHILLIPS

/* Описание выводов интерфейса I2S3            */
/*	I2S3_WS	        PA15	138	WS     */
#define CODEC_I2S_WS_PIN               GPIO_Pin_15
#define CODEC_I2S_WS_PINSRC            GPIO_PinSource15
#define CODEC_I2S_WS_GPIO              GPIOA
#define CODEC_I2S_WS_CLOCK             RCC_AHB1Periph_GPIOA
#define CODEC_I2S_WS_AF                GPIO_AF_SPI3

/*	I2S3_CK	        PC10	139	BCK    */
#define CODEC_I2S_SCK_PIN              GPIO_Pin_10
#define CODEC_I2S_SCK_PINSRC           GPIO_PinSource10
#define CODEC_I2S_SCK_GPIO             GPIOC
#define CODEC_I2S_SCK_CLOCK            RCC_AHB1Periph_GPIOC
#define CODEC_I2S_SCK_AF               GPIO_AF_SPI3

/*  	I2S3_SD	        PC12	141	DATAI  */		
#define CODEC_I2S_SD_PIN               GPIO_Pin_12
#define CODEC_I2S_SD_PINSRC            GPIO_PinSource12
#define CODEC_I2S_SD_GPIO              GPIOC
#define CODEC_I2S_SD_CLOCK             RCC_AHB1Periph_GPIOC
#define CODEC_I2S_SD_AF                GPIO_AF_SPI3

/*	I2S3_SD_ext	PC11	140	DATAO  */		
#define CODEC_I2S_EXT_SD_PIN           GPIO_Pin_11
#define CODEC_I2S_EXT_SD_SOURCE        GPIO_PinSource11
#define CODEC_I2S_EXT_SD_GPIO_PORT     GPIOC
#define CODEC_I2S_EXT_CLOCK            RCC_AHB1Periph_GPIOC
#define CODEC_I2S_EXT_AF               GPIO_AF_SPI2

/* Описания порта интерфейса I2S3              */
#define CODEC_I2S                      SPI3
#define CODEC_I2S_EXT                  I2S3ext
#define CODEC_I2S_CLK                  RCC_APB1Periph_SPI3
#define CODEC_I2S_CLK_CMD              RCC_APB1PeriphClockCmd

#define CODEC_I2S_ADDRESS              ((uint32_t)&(SPI3->DR))
#define CODEC_I2S_EXT_ADDRESS          ((uint32_t)&(I2S3ext->DR))

/* Описание ресурсов DMA */
/* Тактирование */
#define AUDIO_DMA_CLOCK               RCC_AHB1Periph_DMA1
#define AUDIO_DMA_CLOCK_CMD           RCC_AHB1PeriphClockCmd

/* Определения для I2S DMA TX */
/* TX  DMA1	Ch 0	Str 5 */
#define AUDIO_TX_DMA_STREAM           DMA1_Stream5
#define AUDIO_TX_DMA_CHANNEL          DMA_Channel_0
#define AUDIO_TX_DMA_IRQ              DMA1_Stream5_IRQn

#define AUDIO_TX_DMA_IT_FLAG_FEIF     DMA_IT_FEIF5
#define AUDIO_TX_DMA_IT_FLAG_DMEIF    DMA_IT_DMEIF5
#define AUDIO_TX_DMA_IT_FLAG_TEIF     DMA_IT_TEIF5
#define AUDIO_TX_DMA_IT_FLAG_HTIF     DMA_IT_HTIF5
#define AUDIO_TX_DMA_IT_FLAG_TCIF     DMA_IT_TCIF5

/* Переопределение ф-ции прерывания */
#define Audio_TX_IRQHandler           DMA1_Stream5_IRQHandler     

/* Определения для I2S DMA RX */
/* RX  DMA1	Ch 2	Str 2 */   /*!!! в доке ошибка,  DMA1 Stream3 Ch3 может быть только для I2S2_EXT_RX */
#define AUDIO_RX_DMA_CLOCK            RCC_AHB1Periph_DMA1
#define AUDIO_RX_DMA_STREAM           DMA1_Stream2
#define AUDIO_RX_DMA_CHANNEL          DMA_Channel_2
#define AUDIO_RX_DMA_IRQ              DMA1_Stream2_IRQn

#define AUDIO_RX_DMA_IT_FLAG_FEIF     DMA_IT_FEIF2
#define AUDIO_RX_DMA_IT_FLAG_DMEIF    DMA_IT_DMEIF2
#define AUDIO_RX_DMA_IT_FLAG_TEIF     DMA_IT_TEIF2
#define AUDIO_RX_DMA_IT_FLAG_HTIF     DMA_IT_HTIF2
#define AUDIO_RX_DMA_IT_FLAG_TCIF     DMA_IT_TCIF2
/* Переопределение ф-ции прерывания */
#define Audio_RX_IRQHandler           DMA1_Stream2_IRQHandler

/**
  * @brief  Инициализация интерфейса PCM аудио кодека
  * @param  None
  * @retval None
  */
void CODEC_I2S_Configuration(void);

#endif  /* __CODEC_I2S_H__ */

