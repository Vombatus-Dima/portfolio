/**
  ******************************************************************************
  * @file    spi_base.h
  * @author  Trembach D.N.
  * @version V1.0.0
  * @date    11-11-2014
  * @brief   Файл содержит описания расширенных функций SPI 
  ******************************************************************************
  * @attention
  *   
  * 
  ******************************************************************************
  */ 

#ifndef __SPI_BASE_H
#define __SPI_BASE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "stm32f4xx_spi.h"
#include "stm32f4xx_rcc.h"

// Определение кол-во элементов массива независимо от его размера
#define COUNTOF(mass)          (sizeof(mass)/sizeof(mass[0]))

// *****************************************************************************
// Прочитать байт из SPI (Этого посылается байт и зачет читается регист данных)
// [SPIx] - выбранный SPI
// [data] - записать банные для передачи
// [return] - полученные данные
uint16_t SPI_I2S_SendReceive_Data(SPI_TypeDef* SPIx, uint16_t data);


void SPI_SendMulti(SPI_TypeDef* SPIx, uint8_t* dataOut, uint8_t* dataIn, uint32_t count);
void SPI_WriteMulti(SPI_TypeDef* SPIx, uint8_t* dataOut, uint32_t count);
void SPI_ReadMulti(SPI_TypeDef* SPIx, uint8_t* dataIn, uint8_t dummy, uint32_t count);

void SPI_SendMulti16(SPI_TypeDef* SPIx, uint16_t* dataOut, uint16_t* dataIn, uint32_t count);
void SPI_WriteMulti16(SPI_TypeDef* SPIx, uint16_t* dataOut, uint32_t count);
void SPI_ReadMulti16(SPI_TypeDef* SPIx, uint16_t* dataIn, uint16_t dummy, uint32_t count);

// *****************************************************************************
// Определение мин. предделителя для SPI
// [freq]       - желаемая частота
// [*freq_real] - реальная частота
uint16_t SPI_GetPrescalerMin(SPI_TypeDef* SPIx, uint32_t freq, uint32_t* freq_real);

#endif /* __SPI_BASE_H */
