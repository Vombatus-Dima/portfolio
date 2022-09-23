/**
  ******************************************************************************
  * @file    UDA1380.h
  * @author  Trembach Dmitry
  * @date    20-07-2020
  * @brief   Функции инициализации кодека UDA1380
  *
  ******************************************************************************
  * @attention
  * 
  *
  * <h2><center>&copy; COPYRIGHT 2020 DataExpress</center></h2>
  ******************************************************************************
  */

#ifndef __UDA1380_H__
#define __UDA1380_H__

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "stm32f4xx.h"
#include "codec_i2c.h"
#include "codec_i2s.h"

/* Evaluation modes and clock settings*/
#define REG00_EV(x)			  (x << 13)
#define REG00_EN_ADC			  (1 << 11)
#define REG00_EN_DEC			  (1 << 10)
#define REG00_EN_DAC			  (1 << 9)
#define REG00_EN_INT			  (1 << 8)
#define REG00_ADC_CLK			  (1 << 5)
#define REG00_DAC_CLK			  (1 << 4)
#define REG00_SYS_DIV_768Fs		  (3 << 2)
#define REG00_SYS_DIV_512Fs		  (2 << 2)
#define REG00_SYS_DIV_384Fs		  (1 << 2)
#define REG00_SYS_DIV_256Fs		  (0 << 2)
#define REG00_PLL_50_TO_100		  (3)
#define REG00_PLL_25_TO_50		  (2)
#define REG00_PLL_12_5_TO_25		  (1)
#define REG00_PLL_6_25_TO_12_25		  (0)
                                          
/* I2S-bus input and output settings */   
#define REG01_SFORI_I2S_BUS		  (0 << 8)
#define REG01_SFORI_LSB_JUSTIFIED_16BIT	  (1 << 8)
#define REG01_SFORI_LSB_JUSTIFIED_18BIT	  (2 << 8)
#define REG01_SFORI_LSB_JUSTIFIED_20BIT	  (3 << 8)
#define REG01_SFORI_MSB_JUSTIFIED	  (5 << 8)
#define REG01_SFORO_I2S_BUS		  (0)
#define REG01_SFORO_LSB_JUSTIFIED_16BIT	  (1)
#define REG01_SFORO_LSB_JUSTIFIED_18BIT	  (2)
#define REG01_SFORO_LSB_JUSTIFIED_20BIT	  (3)
#define REG01_SFORO_MSB_JUSTIFIED	  (5)
#define REG01_SEL_SOURCE_DOUT_DECIMATOR	  (0 << 6)
#define REG01_SEL_SOURCE_DOUT_MIXER	  (1 << 6)
#define REG01_SIM_INTERFACE_SLAVE	  (0 << 4)
#define REG01_SIM_INTERFACE_MASTER	  (1 << 4)

/* Power control settings */
#define REG02_PON_PLL	                  (1 << 15)
#define REG02_PON_HP	                  (1 << 13)
#define REG02_PON_DAC	                  (1 << 10)
#define REG02_PON_BIAS	                  (1 << 8)
#define REG02_EN_AVC	                  (1 << 7)
#define REG02_PON_AVC	                  (1 << 6)
#define REG02_PON_LNA	                  (1 << 4)
#define REG02_PON_PGAL	                  (1 << 3)
#define REG02_PON_ADCL	                  (1 << 2)
#define REG02_PON_PGAR	                  (1 << 1)
#define REG02_PON_ADCR	                  (1 << 0)

/* ADC settings */
#define REG22_ADC_ADCPOL_INV(value)	  (value << 12)
#define REG22_VGA_CTRL(value)		  ((value & 0xF) << 8)
#define REG22_SEL_LNA_ENABLE		  (1 << 3)
#define REG22_SEL_MIC_ENABLE		  (1 << 2)
#define REG22_SKIP_DCFIL		  (1 << 1)
#define REG22_EN_DCFIL			  (1 << 0)

#define ADC_L_LINE_IN		          (0)
#define ADC_L_SEL_MIC		          (1)
                                          
#define MULTIPLEXER_ADC_R	          (0)
#define MULTIPLEXER_ADC_L	          (1)

#define UDA1380_WRITE_ADDRESS     	  (0x30)
                                          
#define UDA1380_REG_EVALCLK	      	  (0x00)  /* Evaluation modes, WSPLL settings, clock divider and clock selectors    */
#define UDA1380_REG_I2S		          (0x01)  /* I2S-bus I/O settings                                                   */
#define UDA1380_REG_PWRCTRL	      	  (0x02)  /* Power control settings                                                 */
#define UDA1380_REG_ANAMIX	      	  (0x03)  /* Analog mixer settings                                                  */
#define UDA1380_REG_HEADAMP	      	  (0x04)  /* Headphone amplifier settings                                           */
#define UDA1380_REG_MSTRVOL	      	  (0x10)  /* Master volume control                                                  */
#define UDA1380_REG_MIXVOL	      	  (0x11)  /* Mixer volume control                                                   */
#define UDA1380_REG_MODEBBT	      	  (0x12)  /* Mode selection, left and right bass boost, and treble settings         */
#define UDA1380_REG_MSTRMUTE      	  (0x13)  /* Master mute, channel 1 and channel 2 de-emphasis and channel mute      */
#define UDA1380_REG_MIXSDO	      	  (0x14)  /* Mixer, silence detector and interpolation filter oversampling settings */
#define UDA1380_REG_DECVOL	      	  (0x20)  /* Decimator volume control                                               */
#define UDA1380_REG_PGA		          (0x21)  /* PGA settings and mute                                                  */
#define UDA1380_REG_ADC		          (0x22)  /* ADC settings                                                           */
#define UDA1380_REG_AGC		          (0x23)  /* AGC settings                                                           */
                                          
#define UDA1380_REG_L3		          (0x7f)  /* Restore L3-default values                                              */

/* Register map of status bits (read-out) */
#define UDA1380_REG_HEADPHONE     	  (0x18)  /* Interpolation filter status                                            */
#define UDA1380_REG_DEC		          (0x28)  /* Decimator status                                                       */


typedef enum
{
	UDA1380_REG00_EVALCLK = 0x00,             /* Evaluation modes, WSPLL settings, clock divider and clock selectors    */
	UDA1380_REG01_I2S,                        /* I2S-bus I/O settings                                                   */
	UDA1380_REG02_PWRCTRL,                    /* Power control settings                                                 */
	UDA1380_REG03_ANAMIX,                     /* Analog mixer settings                                                  */
	UDA1380_REG04_HEADAMP,                    /* Headphone amplifier settings                                           */
	UDA1380_REG10_MSTRVOL = 0x10,             /* Master volume control                                                  */
	UDA1380_REG11_MIXVOL,                     /* Mixer volume control                                                   */
	UDA1380_REG13_MODEBBT,                    /* Mode selection, left and right bass boost, and treble settings         */
	UDA1380_REG13_MSTRMUTE,                   /* Master mute, channel 1 and channel 2 de-emphasis and channel mute      */
	UDA1380_REG14_MIXSDO,                     /* Mixer, silence detector and interpolation filter oversampling settings */
	UDA1380_REG20_DECVOL =  0x20,             /* Decimator volume control                                               */
	UDA1380_REG21_PGA,                        /* PGA settings and mute                                                  */
	UDA1380_REG22_ADC,                        /* ADC settings                                                           */
	UDA1380_REG23_AGC,                        /* AGC settings                                                           */
	UDA1380_REG7F_SW_RESET = 0x7F             /* Restore L3-default values                                              */
} UDA1380_Registers;

/*  Аудио контроль размеров */
//#define LOUD				0x00
//#define SOFT				0x0f
//#define DUMB				0x3f

#define UDA1380_RESET_PIN                 GPIO_Pin_1
#define UDA1380_RESET_GPIO_PORT           GPIOD
#define UDA1380_RESET_GPIO_CLK            RCC_AHB1Periph_GPIOD

/**
  * @brief  Функция инициализации аудио кодека UDA1380
  * @param  None
  * @retval None
  */
uint8_t UDA1380Init(void);

/**
  * @brief  Функции инициализация вывода сброса UDA1380
  * @param  None
  * @retval None
  */
void UDA1380ResetInit();

/**
  * @brief  Функции установки состояния вывода сброса UDA1380
  * @param  FunctionalState state - состояние вывода
  * @retval None
  */
void UDA1380Reset(FunctionalState state);

/**
  * @brief  Функции включения режима низкого потребления UDA1380
  * @param  None
  * @retval None
  */
bool UDA1380PowerSaveOn();

/**
  * @brief  Функции отключения режима низкого потребления UDA1380
  * @param  None
  * @retval None
  */
bool UDA1380PowerSaveOff();
#endif  /*__UDA1380_H___*/







