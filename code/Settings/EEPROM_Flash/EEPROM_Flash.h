 /**
  ******************************************************************************
  * <h2><center>&copy; COPYRIGHT 2015 DEX DONETSK. Co., Ltd.</center></h2>
  * @file    EEPROM_Flash.h
  * @author  Trembach Dmitriy
  * @version V2.0.0
  * @date    29-01-2015
  * @brief   This file contains all the functions prototypes for the EEPROM_Flash.c 
  *          file.    
  *          Modified to support the STM32F407
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __EEPROM_FLASH_H
#define __EEPROM_FLASH_H

/* Base address of the Flash sectors */ 
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base address of Sector 0, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base address of Sector 1, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base address of Sector 2, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base address of Sector 3, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base address of Sector 4, 64 Kbytes   */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base address of Sector 5, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base address of Sector 6, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base address of Sector 7, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base address of Sector 8, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base address of Sector 9, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base address of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base address of Sector 11, 128 Kbytes */


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Exported functions ------------------------------------------------------- */ 

/**
  * @brief  Функция записи массива в флеш память
  * @param  uint16_t* BuffWR  указатель на буфер данных
  * @param  uint16_t BuffWRSize  размер данных
  * @param  uint32_t FlashStartAdr адресс памяти флеш с которой начинаем запись
  * @retval None
  */
uint8_t Write_Setup_Flash(uint32_t* BuffWR, uint16_t BuffWRSize, uint32_t FlashStartAdr);

/**
  * @brief  Функция чтения параметров из FLASH 
  * @param  uint16_t* BuffWR  указатель на буфер данных
  * @param  uint16_t BuffWRSize  размер данных
  * @param  uint32_t FlashStartAdr адресс памяти флеш с которой начинаем чтение
  * @retval None
  */
void Read_Setup_Flash(uint32_t* BuffRD, uint16_t BuffRDSize, uint32_t FlashStartAdr);


#endif /* __EEPROM_FLASH_H */
/*********** Portions COPYRIGHT 2015 DEX DONETSK. Co., Ltd.*****END OF FILE****/


