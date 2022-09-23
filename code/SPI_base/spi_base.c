/**
  ******************************************************************************
  * @file    spi_base.c
  * @author  Trembach D.N.
  * @version V1.0.0
  * @date    11-11-2014
  * @brief   Файл содержит расширенные функции SPI 
  ******************************************************************************
  * @attention
  *   
  * 
  ******************************************************************************
  */ 
#include "stm32f4xx.h"
#include "stm32f4xx_spi.h"
#include "spi_base.h"

// *****************************************************************************
// Прочитать байт из SPI (Этого посылается байт и зачет читается регист данных)
// [SPIx] - выбранный SPI
// [data] - записать банные для передачи
// [return] - полученные данные
uint16_t SPI_I2S_SendReceive_Data(SPI_TypeDef* SPIx, uint16_t data)
{
  // Ожидание пока не освободится регистр DR
  while((SPIx->SR & SPI_I2S_FLAG_TXE) == RESET);
  
  // Записать данные в регистр DR для передачи
  SPIx->DR = data;
  
  // Ожидание принятие данных
  while((SPIx->SR & SPI_I2S_FLAG_RXNE) == RESET);
  
  // Вернуть полученные данные, прочитанные из регистр DR
  return SPIx->DR;
}

void SPI_SendMulti(SPI_TypeDef* SPIx, uint8_t* dataOut, uint8_t* dataIn, uint32_t count) {
 
  for (uint32_t i = 0; i < count; i++) 
  {
    dataIn[i] = (uint8_t)(SPI_I2S_SendReceive_Data( SPIx , (uint16_t)(dataOut[i])));
  }
}

void SPI_WriteMulti(SPI_TypeDef* SPIx, uint8_t* dataOut, uint32_t count) {
 
  for (uint32_t i = 0; i < count; i++) 
  {
    SPI_I2S_SendReceive_Data( SPIx , (uint16_t)(dataOut[i]));
  }
}

void SPI_ReadMulti(SPI_TypeDef* SPIx, uint8_t* dataIn, uint8_t dummy, uint32_t count) {
 
  for (uint32_t i = 0; i < count; i++) 
  {
    dataIn[i] = (uint8_t)(SPI_I2S_SendReceive_Data( SPIx , (uint16_t)dummy ));
  }
}

void SPI_SendMulti16(SPI_TypeDef* SPIx, uint16_t* dataOut, uint16_t* dataIn, uint32_t count) {
 
  for (uint32_t i = 0; i < count; i++) 
  {
    dataIn[i] = SPI_I2S_SendReceive_Data( SPIx , dataOut[i] );
  }
}

void SPI_WriteMulti16(SPI_TypeDef* SPIx, uint16_t* dataOut, uint32_t count) {
  for (uint32_t i = 0; i < count; i++) 
  {
    SPI_I2S_SendReceive_Data( SPIx , dataOut[i] );
  }
}

void SPI_ReadMulti16(SPI_TypeDef* SPIx, uint16_t* dataIn, uint16_t dummy, uint32_t count) {
  
  for (uint32_t i = 0; i < count; i++) 
  {
    dataIn[i] = SPI_I2S_SendReceive_Data( SPIx , dummy );
  }
}

// *****************************************************************************
// Определение мин. предделителя для SPI
static const uint16_t spi_prescaler[] = {2, 4, 8, 16, 32, 64, 128, 256};
static const uint16_t spi_prescaler_valreg[] = 
  { SPI_BaudRatePrescaler_2,   SPI_BaudRatePrescaler_4,  SPI_BaudRatePrescaler_8, 
    SPI_BaudRatePrescaler_16,  SPI_BaudRatePrescaler_32, SPI_BaudRatePrescaler_64, 
    SPI_BaudRatePrescaler_128, SPI_BaudRatePrescaler_256 };
// [freq]       - желаемая частота
// [*freq_real] - реальная частота
uint16_t SPI_GetPrescalerMin(SPI_TypeDef* SPIx, uint32_t freq, uint32_t* freq_real)
{
  // Получить частоты
  RCC_ClocksTypeDef RCC_ClocksStatus;
  uint32_t ApbClock = 0x00;
  RCC_GetClocksFreq(&RCC_ClocksStatus);
  // Определение частоты шины в зависимости от SPI
  if (SPIx == SPI1)
    ApbClock = RCC_ClocksStatus.PCLK2_Frequency;
  else
    ApbClock = RCC_ClocksStatus.PCLK1_Frequency;
  // Определение желаемого делителя
  uint32_t divider = ApbClock/freq;
  
  // Цикл определения предделителя
  for(uint32_t i = 0; i < COUNTOF(spi_prescaler) - 1; i++)
  {
    // Проверка теоретического предделителя с возможными вариантами
    if(divider >= spi_prescaler[i] && divider < spi_prescaler[i + 1])
    { // если диапазон найден, то ...
      // Записать реальную частоту SPI
      *freq_real = ApbClock/spi_prescaler[i + 1];
      // Вернуть предделитель (значение для регистра)
      return spi_prescaler_valreg[i + 1];
    } 
  }
  // если предделитель вышел за диапазон возможных, то ...
  // Записать реальную частоту SPI
  *freq_real = ApbClock/spi_prescaler[COUNTOF(spi_prescaler) - 1];
  // Вернуть предделитель (значение для регистра)
  return spi_prescaler_valreg[COUNTOF(spi_prescaler) - 1];
}


