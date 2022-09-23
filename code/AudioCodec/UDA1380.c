/**
  ******************************************************************************
  * @file    UDA1380.с
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

#include "UDA1380.h"

/*  */
uint8_t UDA1380InitData[][3] =
{
/*	 *Reset to L3 settings	 */
{UDA1380_REG_L3,          0xFF, 0xFF},
/* Включить всё (Enable all power for now)*/
{UDA1380_REG_PWRCTRL,     0x85, 0xDF},
//	{UDA1380_REG_PWRCTRL,     0xA5, 0x1F},  // выкл. микшер
//	{UDA1380_REG_PWRCTRL,     0x25, 0xD6},

/* CODEC ADC and DAC clock from WSPLL, all clocks enabled                               */
/* Кодек АЦП и ЦАП затактированы от WSPLL, тактирование всего включено                  */
//	{UDA1380_REG_EVALCLK,     0x0F, 0x39},
{UDA1380_REG_EVALCLK,     0x0F, 0x32},
//	{UDA1380_REG_EVALCLK,     0x0F, 0x30},

/* I2S bus data I/O formats, use digital mixer for output  *BCKO is slave                */
{UDA1380_REG_I2S,         0x00, 0x00},
//	{UDA1380_REG_I2S,         0x00, 0x40},

/* Full mixer analog input gain	                                                         */
//	{UDA1380_REG_ANAMIX,      0x00, 0x00},
{UDA1380_REG_ANAMIX,      0x3F, 0x3F},

/* Enable headphone short circuit protection                                             */
{UDA1380_REG_HEADAMP,     0x02, 0x02},

/* Full master volume                                                                    */
//{UDA1380_REG_MSTRVOL,     0x55, 0x55},		//commented piskun 18.09.2014
{UDA1380_REG_MSTRVOL,     0x10, 0x10},			//0x28 = -10 dB; 0x04 = -1 dB; 0x00 = 0 dB

/* Enable full mixer volume on both channels                                             */
{UDA1380_REG_MIXVOL,      0x00, 0x00},

/* Bass and treble boost set to flat                                                     */
{UDA1380_REG_MODEBBT,     0x55, 0x15},

/* Disable mute and de-emphasis                                                          */
//	{UDA1380_REG_MSTRMUTE,    0x00, 0x00},
{UDA1380_REG_MSTRMUTE,    0x02, 0x02},

/* Mixer off, other settings off                                                         */
{UDA1380_REG_MIXSDO,      0x00, 0x00},

/* ADC decimator volume to max                                                           */
{UDA1380_REG_DECVOL,      0x00, 0x00},

/* No PGA mute, full gain                                                                */ 
/* усиление линейного входа по двум каналам                                              */
{UDA1380_REG_PGA,         0x00, 0x00},

/* Select line in and MIC, max MIC gain                                                  */
//	{UDA1380_REG_ADC,         0x0F, 0x02},
//для микрофона 0x00, 0x0C, для линейного входа 0x00, 0x00
{UDA1380_REG_ADC,         0x00, 0x08},
/*	 *AGC	 */
{UDA1380_REG_AGC,         0x00, 0x00},

/*	 *Disable clocks to save power
*{UDA1380_REG_EVALCLK,     0x00, 0x32},
*disable power to input to save power
*{UDA1380_REG_PWRCTRL,     0xA5, 0xC0},
*/

/* End of list                                                                           */
{0xFF,                    0xFF, 0xFF}
};



/**
  * @brief  Функция инициализации аудио кодека UDA1380
  * @param  None
  * @retval None
  */
uint8_t UDA1380Init(void)
{
  uint8_t dev_addr = UDA1380_WRITE_ADDRESS;
  uint8_t i = 0;
  uint8_t errorcode;
  CodecI2CInit();
  
  while (UDA1380InitData[i][0] != 0xFF)
  {
    errorcode = CodecI2CWrite(dev_addr, UDA1380InitData[i], 3);
    if (!errorcode)
      i++;
    else
    {
      return ERROR;
    }   
  }
  return SUCCESS;
}


/**
  * @brief  Функции инициализация вывода сброса UDA1380
  * @param  None
  * @retval None
  */
void UDA1380ResetInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* Включаем тактирование вывода */
  RCC_AHB1PeriphClockCmd(UDA1380_RESET_GPIO_CLK, ENABLE);
  /* Заполняем структуру инициализации вывода */
  GPIO_InitStructure.GPIO_Pin = UDA1380_RESET_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  /* Инициализация вывода */
  GPIO_Init(UDA1380_RESET_GPIO_PORT, &GPIO_InitStructure);
}

/**
  * @brief  Функции установки состояния вывода сброса UDA1380
  * @param  FunctionalState state - состояние вывода
  * @retval None
  */
void UDA1380Reset(FunctionalState state)
{
  if (state == DISABLE)
  {
    GPIO_ResetBits(UDA1380_RESET_GPIO_PORT, UDA1380_RESET_PIN);
  }
  else
  {
    GPIO_SetBits(UDA1380_RESET_GPIO_PORT, UDA1380_RESET_PIN);
  }
}

/**
  * @brief  Функции включения режима низкого потребления UDA1380
  * @param  None
  * @retval None
  */
bool UDA1380PowerSaveOn()
{
  uint8_t data[] = {UDA1380_REG_PWRCTRL, 0x01, 0x10};					//01 10
  return (CodecI2CWrite(UDA1380_WRITE_ADDRESS, data, 3) == CODEC_I2C_OK) ? true : false;
  //	return true;
}

/**
  * @brief  Функции отключения режима низкого потребления UDA1380
  * @param  None
  * @retval None
  */
bool UDA1380PowerSaveOff()
{
  //	uint8_t data[] = {UDA1380_REG_PWRCTRL, 0xA5, 0xDF};			//base
  uint8_t data[] = {UDA1380_REG_PWRCTRL, 0x85, 0x1C};			//85 1C
  return (CodecI2CWrite(UDA1380_WRITE_ADDRESS, data, 3) == CODEC_I2C_OK) ? true : false;
  //	return true;
}
