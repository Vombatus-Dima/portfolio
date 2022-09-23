/**
  ******************************************************************************
  * @file    settings.h
  * @author  Trembach Dmitriy
  * @version V1.2.0
  * @date    21-09-2015
  * @brief   This file contains all the functions prototypes for settings.c 
  *          file.
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SETTINGS_H
#define __SETTINGS_H

         
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "main.h"
#include "router_streams.h"

/* Версия насстроек */
#define VER_SET       (0x00001)  /* Инкрементирует в случаем изменений в структуре настроек */
/* Формирование идентификатора структуры настроек */
#define ID_SETTINGS   (( DEVICE_TYPE )|( VER_SET<<8 ) ) 

/* Exported types ------------------------------------------------------------*/
#define  MAX_NUMBER_KEY   (32)  // максимальное число конфигурируемых кнопок	 
   
/* Коды для формирования маски трансляции */   
#define CIDX0 (0x0001) 
#define CIDX1 (0x0002) 
#define CIDX2 (0x0004)   
#define CIDX3 (0x0008)   
#define CIDX4 (0x0010) 
#define CIDX5 (0x0020) 
#define CIDX6 (0x0040)   
#define CIDX7 (0x0080)   
#define CIDX8 (0x0100) 
#define CIDX9 (0x0200) 
#define CIDXA (0x0400)   
#define CIDXB (0x0800)   
#define CIDXC (0x1000) 
#define CIDXD (0x2000) 
#define CIDXE (0x4000)   
#define CIDXF (0x8000)    
      
// структура все переменные которые будем хранить в EEPROM FLASH
// если добавляем переменную - то обязательно вносим ее в таблицу значений по умолчанию	!!! 
   typedef  struct  {
//==============================================================================
//      Настройки для Ядра
//==============================================================================     
//#define SET_CORE_PHY_ADDR                       (3) //(uint16_t)// Установить физический адрес
     uint16_t                                   mac_adr;                        // Логический адрес блока
    
     uint16_t                                   time_update_diagn;              // Время (период) обновления диагностики
     mask_box_id_t                              tabl_mask_box_id[MAX_NUM_MASK_BOX_ID];// Маски ID_RS пакетов трансляции
   
     uint8_t                                    nmask_transl_core;              // Hомер текущей маски ID_RS пакетов ретрансляции
     uint32_t                                   mask_inpup_port_id_core;        // Маска ID портов разрешенных для приема в данный порт  
     
     uint8_t                                    group_ring;                      /* Номер группы кольца (0 - без группы)                             */      
     uint8_t                                    priority_ring;                   /* Приоритет в группе кольца                                        */
     uint8_t                                    number_position_vrm;            // Число лучших врм при позиционировании тегов 
       
//==============================================================================
//      Настройки для ETH
//============================================================================== 
     uint16_t 					ip_port_voice; 			// Порт приема передачи голосовых сообщений
 
     uint8_t 					ip_table_main_srv_ad0; 		// Адрес IPv4 сервера раздающего  таблицу рассылки
     uint8_t 					ip_table_main_srv_ad1; 		// Адрес IPv4 сервера раздающего  таблицу рассылки
     uint8_t 					ip_table_main_srv_ad2; 		// Адрес IPv4 сервера раздающего  таблицу рассылки
     uint8_t 					ip_table_main_srv_ad3; 		// Адрес IPv4 сервера раздающего  таблицу рассылки
     uint16_t 					ip_table_main_srv_port; 	// Порт сервера раздающего  таблицу рассылки
     
     uint8_t 					ip_table_rezv_srv_ad0; 		// Адрес IPv4 сервера раздающего  таблицу рассылки
     uint8_t 					ip_table_rezv_srv_ad1; 		// Адрес IPv4 сервера раздающего  таблицу рассылки
     uint8_t 					ip_table_rezv_srv_ad2; 		// Адрес IPv4 сервера раздающего  таблицу рассылки
     uint8_t 					ip_table_rezv_srv_ad3; 		// Адрес IPv4 сервера раздающего  таблицу рассылки
     uint16_t 					ip_table_rezv_srv_port; 	// Порт сервера раздающего  таблицу рассылки
 
     uint16_t 					phy_addr_disp_1; 		/* Физический адрес диспетчера 1                                    */ 
     uint16_t 					phy_addr_disp_2; 		/* Физический адрес диспетчера 2                                    */ 
  
     uint8_t                                    nmask_transl_voice;             // Hомер текущей маски ID_RS пакетов трансляции для голосового порта рассылки
     uint8_t                                    nmask_transl_config;            // Hомер текущей маски ID_RS пакетов трансляции конфигурирования и загрузки
      
     uint32_t                                   mask_inpup_port_id_voice;       // Маска ID портов разрешенных для приема в данный порт трансляции для голосового порта рассылки
     uint32_t                                   mask_inpup_port_id_config;      // Маска ID портов разрешенных для приема в данный порт трансляции конфигурирования и загрузки
     
     //==============================================================================
     //      Настройки для RS A
     //==============================================================================   
     uint8_t 					Type_RS_a; 			// Значениe адреса устр-ва по UART
     uint32_t 					rs_bit_rate_a; 			// Скорость интерфейса RS
     uint8_t                                    nmask_transl_rs_a;              // Номер текущей маски ID_RS пакетов трансляции
     uint32_t                                   mask_inpup_port_id_rs_a;        // Mаска ID портов разрешенных для приема в данный порт
     //==============================================================================     
      
     //==============================================================================
     //      Настройки для RS B
     //==============================================================================   
     uint8_t 					Type_RS_b; 			// Значениe адреса устр-ва по UART
     uint32_t 					rs_bit_rate_b; 			// Скорость интерфейса RS
     uint8_t                                    nmask_transl_rs_b;              // Номер текущей маски ID_RS пакетов трансляции
     uint32_t                                   mask_inpup_port_id_rs_b;        // Mаска ID портов разрешенных для приема в данный порт
     //==============================================================================   
     
     /*============================================================================*/
     /*     Настройки для CODEC                                                    */
     /*============================================================================*/
     /* Настройки кодека канала А        */
     uint8_t 				       codec_a_chanel;                  /* Номер канала закреленный за кодеком */     
     uint8_t 				       codec_a_priority_ch_pcm;         /* Приоритет голосового канала pcm по умолчанию (1..5) */ 
     uint8_t 				       codec_a_priority_ch_cmx;         /* Приоритет голосового канала cmx по умолчанию   (1..5) */ 
     uint32_t                                  codec_a_mask_source_soft_port;   /* Маска портов разрешенных для приема в данный порт из програмного роутера */ 
     uint32_t                                  codec_a_mask_chanel_soft_port;   /* Маска каналов разрешенных для приема в данный порт из програмного роутера */  
     /* Настройки кодека канала B        */                                      
     uint8_t 				       codec_b_chanel;                  /* Номер канала закреленный за кодеком */     
     uint8_t 				       codec_b_priority_ch_pcm;         /* Приоритет голосового канала pcm по умолчанию (1..5) */ 
     uint8_t 				       codec_b_priority_ch_cmx;         /* Приоритет голосового канала cmx по умолчанию   (1..5) */ 
     uint32_t                                  codec_b_mask_source_soft_port;   /* Маска портов разрешенных для приема в данный порт из програмного роутера */ 
     uint32_t                                  codec_b_mask_chanel_soft_port;   /* Маска каналов разрешенных для приема в данный порт из програмного роутера */  
     /* Настройки кодека канала C        */                                      
     uint8_t 				       codec_c_chanel;                  /* Номер канала закреленный за кодеком */     
     uint8_t 				       codec_c_priority_ch_pcm;         /* Приоритет голосового канала pcm по умолчанию (1..5) */ 
     uint8_t 				       codec_c_priority_ch_cmx;         /* Приоритет голосового канала cmx по умолчанию   (1..5) */ 
     uint32_t                                  codec_c_mask_source_soft_port;   /* Маска портов разрешенных для приема в данный порт из програмного роутера */ 
     uint32_t                                  codec_c_mask_chanel_soft_port;   /* Маска каналов разрешенных для приема в данный порт из програмного роутера */  
     /* Настройки общие для всех кодеков */                                      
     uint8_t                                   codec_nmask_transl_codec;        /* Hомер текущей маски ID_RS пакетов трансляции в порт кодека  */
     uint32_t                                  codec_mask_inpup_port_id_codec;  /* Маска ID портов разрешенных для приема в порт кодека        */   
     /* Настройки аналогового кодека     */      
     uint8_t                                   analog_codec_mode;                  /* Режим работы аналогового кодека     */
     uint8_t 				       analog_codec_chanel;                /* Номер канала закреленный за кодеком */     
     uint8_t 				       analog_codec_priority_discrt;       /* Приоритет голосового дискретного канала по умолчанию   (1..5) */ 
     uint8_t 				       analog_codec_priority_analog;       /* Приоритет голосового аналогового канала по умолчанию   (1..5) */ 
     uint32_t                                  analog_codec_mask_source_soft_port; /* Маска портов разрешенных для приема в данный порт из програмного роутера */ 
     uint32_t                                  analog_codec_mask_chanel_soft_port; /* Маска каналов разрешенных для приема в данный порт из програмного роутера */  
     /* Настройки порта ETH несжатых потов    */
     uint32_t                                  udp_mask_source_soft_port;       /* Маска портов разрешенных для приема в UDP порт из програмного роутера */ 
     uint32_t                                  udp_mask_chanel_soft_port;       /* Маска каналов разрешенных для приема в UDP порт из програмного роутера */ 
   }Settings_t;	


/* Exported constants --------------------------------------------------------*/
//определение расположения данных во FLASH памяти
//Адрес начала
#define FLASH_Setup_START_ADDR	ADDR_FLASH_SECTOR_2    /* Base address of Sector 3, 16 Kbytes   */
//Адрес конца
#define FLASH_Setup_END_ADDR		(ADDR_FLASH_SECTOR_3 - (uint32_t)1)	


//================================================================================
//================================================================================	 
/** 
  * @brief структура данных для записи параметров в енергонезависимую память  
  */ 
typedef struct  {
		uint16_t  		 		Cnt_WRFlash; 					/* счетчик числа стираний FLASH */
		uint16_t  		 		ID_Settings; 					/* id структуры параметров */                
		Settings_t				Settings;  					/* структуры параметров */
		uint32_t                                buff_protect;                                   /* защитный буфер контрольной суммы */
		uint32_t  		 		CRC_Array;  					/* контрольная сумма структуры параметров и счетчика числа стираний FLASH*/		
}FlashSettingsTypeDef;

// структура содержит значения по умолчанию для всех переменных которые будем хранить в EEPROM FLASH
// если добавляем переменную - то обязательно вносим ее в таблицу значений по умолчанию	!!!
extern const	FlashSettingsTypeDef DataDef;

// структура для редактирования переменных которые будем хранить в EEPROM FLASH
// при запуске программы заполняется хранимыми в EEPROM значениями 	
extern FlashSettingsTypeDef DataNew;	

// структура переменных которые прочитали из EEPROM FLASH при запуске программы
extern FlashSettingsTypeDef DataSto;	


/* Exported functions ------------------------------------------------------- */

/**
  * @brief  Функция записи структуры параметров 
  * @param  FlashSettingsTypeDef *DataSto указатель на записываемые данные
  * @retval None
  */
void Write_Settings(FlashSettingsTypeDef *DataSto);

/**
  * @brief  Функция чтения структуры параметров 
  * @param  FlashSettingsTypeDef *DataSto указатель на массив для прочитанных данныx
  * @retval None
  */
void Read_Settings(FlashSettingsTypeDef *DataSto);

/**
  * @brief  Функция проверки контрольной суммы структуры параметров 
  * @param  FlashSettingsTypeDef *DataSto указатель на структуру параметров
  * @retval uint8_t 0 - контрольная сумма совпала
  *                 1 - контрольная сумма не совпала
  */
uint8_t Test_CRC_Settings(FlashSettingsTypeDef *DataSto);

/**
  * @brief  Функция обновления контрольной суммы структуры параметров 
  * @param  FlashSettingsTypeDef *DataSto указатель на структуру параметров
  * @retval None
  */
void Set_CRC_Settings(FlashSettingsTypeDef *DataSto);

/**
  * @brief  Функция обновления структуры параметров из флеш памяти 
  * @param  None
  * @retval uint8_t 0 - контрольная сумма совпала
  *                 1 - контрольная сумма не совпала
  *                 2 - контрольная сумма не совпала установлены параметры по умолчанию
  */
uint8_t Load_Settings(void);

/**
  * @brief  Функция проверка коррекности структуры параметров в оперативной памяти
  * @param  None
  * @retval None
  */
void Control_Settings(void);

/**
  * @brief  Функция копирования структур параметров
  * @param  FlashSettingsTypeDef *DataSource - структура источник
  * @param  FlashSettingsTypeDef *DataDestination - структура назначение
  * @retval None
  */
void Copy_Settings(FlashSettingsTypeDef *DataSource,FlashSettingsTypeDef *DataDestination);

/**
  * @brief  Функция записи отредактированных параметров в Flash память 
  * @param  None
  * @retval None
  */
void Write_Edit_Settings(void);


#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

