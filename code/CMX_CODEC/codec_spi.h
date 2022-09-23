/**
  ******************************************************************************
  * @file    codec_spi.h
  * @author  Trembach Dmitry
  * @version V1.0.0
  * @date    12-04-2020
  * @brief   Инициализация интерфейса SPI кодека
  *
  ******************************************************************************
  * @attention
  * 
  * 
  *
  * <h2><center>&copy; COPYRIGHT 2020 DataExpress</center></h2>
  ******************************************************************************
  */

#ifndef _CODEC_SPI_H
#define _CODEC_SPI_H

#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"
#include "main.h"
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "codec_cmx.h"

#define CODEC_SPI                       SPI6
#define CODEC_SPI_GPIO_AF               GPIO_AF_SPI6  
#define CODEC_SPI_RCC                   RCC_APB2Periph_SPI6
#define CODEC_RCC_APBPeriphClockCmd     RCC_APB2PeriphClockCmd
   
#define CODEC_MOSI_PIN                  GPIO_Pin_14            /* PG.14  */
#define CODEC_MOSI_GPIO_PORT            GPIOG                  /* GPIOG  */
#define CODEC_MOSI_SOURCE               GPIO_PinSource14
#define CODEC_MOSI_GPIO_CLK             RCC_AHB1Periph_GPIOG

#define CODEC_MISO_PIN                  GPIO_Pin_12            /* PG.12  */
#define CODEC_MISO_GPIO_PORT            GPIOG                  /* GPIOG  */
#define CODEC_MISO_SOURCE               GPIO_PinSource12   
#define CODEC_MISO_GPIO_CLK             RCC_AHB1Periph_GPIOG

#define CODEC_CLK_PIN                   GPIO_Pin_13            /* PG.13  */
#define CODEC_CLK_GPIO_PORT             GPIOG                  /* GPIOG  */
#define CODEC_CLK_SOURCE                GPIO_PinSource13
#define CODEC_CLK_GPIO_CLK              RCC_AHB1Periph_GPIOG

#define CODEC_PORTs_RCC                 ( CODEC_MOSI_GPIO_CLK | CODEC_MISO_GPIO_CLK | CODEC_CLK_GPIO_CLK )

/* ================= Definition for TIMER GEN SYNC  ==================  */
#define CODEC_TIM			TIM4
#define CODEC_TIM_IRQHandler	        TIM4_IRQHandler
#define CODEC_TIM_IRQn		        TIM4_IRQn	
#define CODEC_TIM_CLK                   RCC_APB1Periph_TIM4
#define CODEC_TIM_CLK_CMD               RCC_APB1PeriphClockCmd
#define CODEC_TIME_CLOCK	        90000000 

#define CODEC_TIMER_Prescaler	        (uint16_t)(90)
#define CODEC_TIME_UPDATE_SYNC 	        (uint16_t)(60000) 	        

//* Фиктивное значение, для записи в SPDR, чтобы получить данные по SPI *
#define SPI_DUMMY_8                     (0x00)
//* Фиктивное значение, для записи в SPDR, чтобы получить данные по SPI *
#define SPI_DUMMY_16                    (0x0000)

extern TaskHandle_t xHandlingTask;

/**
 * @brief  Основная задача контроля трансивера. 
 * @param  uint16_t time_val
 * @retval None
  ******************************************************************************
  * @attention
  * результаты измерения
  *  time_val=3     - 0.29 мкс 
  *  time_val=100   - 10.1 мкс
  *     ошибка ~ 1%
  ******************************************************************************
 */
void ns100_delay(uint16_t time_val);

/**
  * @brief  Инициализация интерфейса spi. 
  * @param  uint32_t spi_bitrate  желаемая (макс.) скорость SPI
  * @retval uint32_t - установленная скорость
  * 
  */
uint32_t spi_interface_init(uint32_t spi_bitrate);

/**
  * @brief  Запись/чтение байта по spi
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @param  uint8_t data - байт для записи
  * @retval uint8_t прочитаный байт 
  * 
  */
uint8_t spi_wr_rd_byte(pcm_cmx_t* CODECx , uint8_t wr_data);
 
/**
  * @brief  Запись/чтение слова по spi
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @param  uint16_t data - слово для записи
  * @retval uint16_t прочитанное слово 
  * 
  */
uint16_t spi_wr_rd_word( pcm_cmx_t* CODECx , uint16_t wr_data);

/**
  * @brief  Запись/чтение регистра байта по заданному адресу
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @param  uint8_t addr - адрес для записи
  * @param  uint8_t data - байт для записи
  * @retval uint8_t прочитанный байт 
  * 
  */
uint8_t spi_wr_rd_reg_byte( pcm_cmx_t*  CODECx , uint8_t addr, uint8_t data);

/**
  * @brief  Запись/чтение регистра 16 бит по заданному адресу
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @param  uint8_t addr - адрес для записи
  * @param  uint16_t data - слово для записи
  * @retval uint16_t прочитанный байт 
  * 
  */
uint16_t spi_wr_rd_reg_word( pcm_cmx_t*  CODECx , uint8_t addr, uint16_t data);

/**
  * @brief  Вкл. режим микрофона
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @retval None 
  * 
  */
void Set_Mic( pcm_cmx_t*  CODECx );

/**
  * @brief  Вкл. режим спикера
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @retval None 
  * 
  */
void Set_Speaker( pcm_cmx_t*  CODECx );

/**
  * @brief  Чтения данных голоса из кодека CMX используя DMA
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @retval None 
  */
void Read_Voice_DMA(pcm_cmx_t*  CODECx);

/**
  * @brief  Запись данных голоса в кодек CMX используя DMA
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @retval None 
  */
void Send_Voice_DMA(pcm_cmx_t* CODECx);


/**
  * @brief  Запись данных голоса в кодек CMX
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @retval None 
  */
void Send_Voice(pcm_cmx_t* CODECx);

/**
  * @brief  Чтения данных голоса из кодека CMX
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @retval None 
  */
void Read_Voice(pcm_cmx_t*  CODECx);

#endif /* _CODEC_SPI_H */