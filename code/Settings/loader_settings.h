/**
  ******************************************************************************
  * @file    loader_settings.h
  * @author  Trembach Dmitriy
  * @version V1.0.0
  * @date    09-09-2015
  * @brief   This file contains all the functions prototypes for loader_settings
  *          file.
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LOADER_SETTINGS_H
#define __LOADER_SETTINGS_H


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
/* Exported types ------------------------------------------------------------*/

// структура все переменные которые будем хранить в EEPROM FLASH
// если добавляем переменную - то обязательно вносим ее в таблицу значений по умолчанию	!!! 
typedef struct  {
  uint16_t      phy_adr;                // физический адрес блока	
        
  uint8_t       ip_ad0;                 // Адрес IPv4 	
  uint8_t       ip_ad1;                 // Адрес IPv4 	
  uint8_t       ip_ad2;                 // Адрес IPv4 	
  uint8_t       ip_ad3;                 // Адрес IPv4 
        
  uint8_t       ip_gt0;                 // Шлюз 
  uint8_t       ip_gt1;                 // Шлюз 
  uint8_t       ip_gt2;                 // Шлюз 
  uint8_t       ip_gt3;                 // Шлюз 
        
  uint8_t       ip_mask0;               // Маска подсети 	
  uint8_t       ip_mask1;               // Маска подсети 	
  uint8_t       ip_mask2;               // Маска подсети  	
  uint8_t       ip_mask3;               // Маска подсети 	
        
  uint8_t       mac_ad0;                // MAC адрес 	
  uint8_t       mac_ad1;                // MAC адрес 	
  uint8_t       mac_ad2;                // MAC адрес  	
  uint8_t       mac_ad3;                // MAC адрес 
  uint8_t       mac_ad4;                // MAC адрес  	
  uint8_t       mac_ad5;                // MAC адрес
    
  uint16_t      ip_port_loader;         // Порт загрузчика
  
  uint16_t      code_pass;              // Код доступа 
      
  uint32_t      addr_start; 	        // Адрес программы пользователя  	
  uint32_t      soft_size; 	        // Размер программы пользователя 
  uint32_t      soft_crc; 	        // Контрольная сумма программы  
  
}Loader_Settings_t;	

/** 
  * @brief структура данных для записи параметров в енергонезависимую память  
  */ 
typedef struct{
  uint16_t  		 	Cnt_WRFlash; 					/* счетчик числа стираний FLASH */	
  Loader_Settings_t		Settings;  					/* структуры параметров */
  uint16_t  		 	CRC_Array;  					/* контрольная сумма структуры параметров и счетчика числа стираний FLASH*/		
}FlashLoaderSettingsTypeDef;
//================================================================================
//================================================================================	
#if VECT_TAB_OFFSET == 0x20000
/* Exported constants --------------------------------------------------------*/
//определение расположения данных во FLASH памяти
//Адрес начала
#define FLASH_Loader_Setup_START_ADDR	ADDR_FLASH_SECTOR_1    /* Base address of Sector 3, 16 Kbytes   */
//Адрес конца
#define FLASH_Loader_Setup_END_ADDR	(ADDR_FLASH_SECTOR_2 - (uint32_t)1)	

/**
  * @brief  Функция чтения структуры параметров 
  * @param  FlashSettingsTypeDef *DataSettings указатель на массив для прочитанных данныx
  * @retval None
  */
void Read_Loader_Settings(FlashLoaderSettingsTypeDef *DataLoadSettings);

// структура переменных которые прочитали из EEPROM FLASH при запуске программы
extern FlashLoaderSettingsTypeDef DataLoaderSto;	
#else
// структура переменных которые прочитали из EEPROM FLASH при запуске программы
extern const FlashLoaderSettingsTypeDef DataLoaderSto;	
#endif
/* Exported functions ------------------------------------------------------- */
#endif /* __LOADER_SETTINGS_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

