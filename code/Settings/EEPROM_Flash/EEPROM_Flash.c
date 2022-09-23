 /**
  ******************************************************************************
  * <h2><center>&copy; COPYRIGHT 2015 DEX DONETSK. Co., Ltd.</center></h2>
  * @file    EEPROM_Flash.c
  * @author  Trembach Dmitriy
  * @version V2.0.0
  * @date    29-01-2015
  * @brief   This file contains all the functions prototypes for the EEPROM_Flash.c 
  *          file.    
  *          Modified to support the STM32F407
  ******************************************************************************
  * @attention   !!! Необходимо добавить действие в случае невозможности стирания флеш
  *												невозможности стирания флеш !!!
  ******************************************************************************
  */ 
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_flash.h"
#include "EEPROM_Flash.h"
 
/**
  * @brief  Определение по адресу номера сектора 
  * @param  None
  * @retval Номер сектора соответствующий адресу
  * @brief  Работает только в диапазоне флеш памяти программ
  */
static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_Sector_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_Sector_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_Sector_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_Sector_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_Sector_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_Sector_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_Sector_6;  
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_Sector_7;  
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_Sector_8;  
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_Sector_9;  
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_Sector_10;  
  }  
  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/     
  {
    sector = FLASH_Sector_11;  
  }
  return sector;
}

/**
  * @brief  Функция записи массива в флеш память
  * @param  uint16_t* BuffWR  указатель на буфер данных
  * @param  uint16_t BuffWRSize  размер данных
  * @param  uint32_t FlashStartAdr адресс памяти флеш с которой начинаем запись
  * @retval None
  */
uint8_t Write_Setup_Flash(uint32_t* BuffWR, uint16_t BuffWRSize, uint32_t FlashStartAdr)
{
  uint16_t TempCntFlash; 
	/* Unlock the Flash *********************************************************/
  /* Enable the flash control register access */
  FLASH_Unlock();
    
  /* Erase the user Flash area ************************************************/
  /* area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR */
  
  /* Clear pending flags (if any) */  
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
  /* Device voltage range supposed to be [2.7V to 3.6V], the operation will be done by word */ 
  if (FLASH_EraseSector(GetSector(FlashStartAdr), VoltageRange_3) != FLASH_COMPLETE)
   { 
    /* Error occurred while sector erase. 
    User can add here some code to deal with this error  */
		while (1);
	 }
  
		//цикл загрузки данных
		for(TempCntFlash = 0;TempCntFlash < (BuffWRSize>>2);TempCntFlash++)
		 {
			//функция записи 
			FLASH_ProgramWord(FlashStartAdr + (((uint32_t)TempCntFlash)<<2),*(BuffWR + TempCntFlash));
		 }
  /* Lock the Flash to disable the flash control register access (recommended
  to protect the FLASH memory against possible unwanted operation) */
  FLASH_Lock(); 
	return 0;	
}

/**
  * @brief  Функция чтения параметров из FLASH 
  * @param  uint16_t* BuffWR  указатель на буфер данных
  * @param  uint16_t BuffWRSize  размер данных
  * @param  uint32_t FlashStartAdr адресс памяти флеш с которой начинаем чтение
  * @retval None
  */
void Read_Setup_Flash(uint32_t* BuffRD, uint16_t BuffRDSize, uint32_t FlashStartAdr)
{
  uint16_t TempCntFlash; 
  //цикл чтения данных
  for(TempCntFlash = 0;TempCntFlash < (BuffRDSize>>2);TempCntFlash++)
  {
    //чтение непосредственно из флеш памяти
    *(BuffRD+TempCntFlash) = (*(__IO uint32_t*) (FlashStartAdr + (((uint32_t)TempCntFlash)<<2)));
  }
}
/*********** Portions COPYRIGHT 2013 DEX DONETSK. Co., Ltd.*****END OF FILE****/

