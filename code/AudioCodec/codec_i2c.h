/**
  ******************************************************************************
  * @file    codec_i2c.h
  * @author  Trembach Dmitry
  * @date    20-07-2020
  * @brief   ‘ункции инициализации интерфейса i2c управлени¤ кодеком UDA1380
  *
  ******************************************************************************
  * @attention
  * 
  *
  * <h2><center>&copy; COPYRIGHT 2020 DataExpress</center></h2>
  ******************************************************************************
  */

#ifndef CODEC_I2C_H_
#define CODEC_I2C_H_

#include "stm32f4xx.h"
/* ќписание статусов линии i2c                  */
#define CODEC_I2C_MASTER_ERR			0xF2
#define CODEC_I2C_TRANS_MODE_ERR		0xF3
#define CODEC_I2C_TRANS_ERR			0xF4
#define CODEC_I2C_ERROR				1
#define CODEC_I2C_OK				0

#define CODEC_I2C_TIMEOUT			10000

/* ”становка скорости и адреса шины             */
#define CODEC_I2C_SPEED               		40000//30000
#define CODEC_I2C_OWN_ADDRESS7      		0x33
 
/* ќписание ресурсов порта i2c и тактировани¤   */
#define CODEC_I2C                       	I2C3
#define CODEC_CLK_INIT				RCC_APB1PeriphClockCmd
#define CODEC_CLK_RESET				RCC_APB1PeriphResetCmd
#define CODEC_I2C_CLK                    	RCC_APB1Periph_I2C3
/* ќписание ввода/вывода SDA                    */
#define CODEC_I2C_SDA_PIN                 	GPIO_Pin_9
#define CODEC_I2C_SDA_GPIO_PORT           	GPIOC
#define CODEC_I2C_SDA_GPIO_CLK            	RCC_AHB1Periph_GPIOC
#define CODEC_I2C_SDA_SOURCE              	GPIO_PinSource9
#define CODEC_I2C_SDA_AF                  	GPIO_AF_I2C3
/* ќписание вывода SCL                          */
#define CODEC_I2C_SCL_PIN                 	GPIO_Pin_8
#define CODEC_I2C_SCL_GPIO_PORT           	GPIOA
#define CODEC_I2C_SCL_GPIO_CLK            	RCC_AHB1Periph_GPIOA
#define CODEC_I2C_SCL_SOURCE              	GPIO_PinSource8
#define CODEC_I2C_SCL_AF                  	GPIO_AF_I2C3

/**
  * @brief  ‘ункци¤ инициализации порта I2C 
  * @param  None
  * @retval None
  */
void CodecI2CInit(void);

/**
  * @brief  ‘ункци¤ записи данных в порта I2C 
  * @param  uint8_t addr    - адрес
  * @param  uint8_t *p_data - указатель на нассив данных
  * @param  uint8_t len     - длинна данных
  * @retval uint8_t - результат операции
  */
uint8_t CodecI2CWrite(uint8_t addr, uint8_t *p_data, uint8_t len);

#endif /* CODEC_I2C_H_ */
