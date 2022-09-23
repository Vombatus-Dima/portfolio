/**
  ******************************************************************************
  * @file    core_cntrl_dev.h
  * @author  Trembach Dmitry
  * @version V1.1.0
  * @date    28-12-2020
  * @brief   файл с функциями ядра для контроля устройств в RS
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 DataExpress</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CORE_CNTRL_DEV_H
#define __CORE_CNTRL_DEV_H

#include "stm32f4xx.h"
#include "core.h"
#include "rf_frame_header.h"
#include "rs_frame_header.h"
#include "status_handler.h"     

#define  MAX_TABLE_DEV          (100)
#define  MAX_TABLE_CON          (100)
#define  MAX_TIME_OF_LIFE_DEV   (300)

#define  MAX_NUM_CYCLE_PING_DEV       (4)          /* Максимальное число циклов */
#define  MAX_INTERAL_TEST_TABLE_DEV   ( 20000 )    /* Интервал между тестами устройств по таблице */
#define  MAX_INTERAL_TEST_DEV         ( 400 )      /* Интервал между тестами устройств */   

#define  MAX_PING_REG_TIME           (50000)       /* Ждем пинг не более 5 сек */  

/**
  * @brief Функция парсинга пакета ответ пинга RS 
  * @param router_box_t* rs_box - указатель на пакета диагностики  
  *
  * @retval bool true  - пакет обработан
  *              false - пакет не обработан
  */
bool ProcessinRespPing( router_box_t* rs_box );

/**
  * @brief Функция парсинга пакета диагностики 
  * @param router_box_t* rs_box - указатель на пакета диагностики  
  *
  * @retval bool true  - пакет обработан
  *              false - пакет не обработан
  */
bool ProcessingDiagDev( router_box_t* rs_box );

/**
  * @brief Функция обновляет время жизни зарегистрированного устройства, если время жизни исчерпано
  *        удаляем устройство из таблицы
  *
  * @param none
  * @retval uint16_t - число зарегистрированных устройств в таблице
  */
uint16_t UpdateDevTable( void );

/**
  * @brief Функция обнуления всей таблицы устройств
  * @param none
  * @retval none
  */
void ClearTableDev( void );

/**
  * @brief Функция обнуления динамических параметров в таблице устройств
  * @param none
  * @retval none
  */
void ClearParamTableDev( void );

/**
  * @brief  Функция формирования статуса зарегистрированных устройств в RS
  * @param  char *pcInsert - указатель на массив pcInsert для формируемой строки
  * @param  enum_type_stage_t stage - режим вызова STAT_TITLE - выдача имени страницы
  * 						   STAT_HEAD - выдача заголовка таблицы
  * 						   STAT_LINE - выдача строки таблицы
  *
  *
  * @retval uint16_t длина формируемой строки
  */
uint16_t TabStatusDevRS(char *pcInsert, enum_type_stage_t stage);

/**
  * @brief Функция записи собственных параметров в таблицу устройств
  * @param uint16_t alarm_flag - флаги аварийных режимов                      
  * @param uint16_t Value_VCC - напряжение питания  (за время *)            
  * @param uint16_t Value_VCC_min - минимальное напряжение питания (за время *)  
  * @param uint16_t NearPhyAdd1 - адрес сосед по порту RS 1                       
  * @param uint16_t NearPhyAdd2 - адрес сосед по порту RS 2                       
  * @param uint16_t NearPhyAdd3 - адрес сосед по порту RS 3                       
  * @param uint16_t NearPhyAdd4 - адрес сосед по порту RS 4                       
  * @retval none
  */
void SetOwnDevTable( uint16_t alarm_flag, uint16_t Value_VCC, uint16_t Value_VCC_min, uint16_t NearPhyAdd1, uint16_t NearPhyAdd2, uint16_t NearPhyAdd3, uint16_t NearPhyAdd4 );

/**
  * @brief  Функция формирования статуса соединений устройств в RS
  * @param  char *pcInsert - указатель на массив pcInsert для формируемой строки
  * @param  enum_type_stage_t stage - режим вызова STAT_TITLE - выдача имени страницы
  * 						   STAT_HEAD - выдача заголовка таблицы
  * 						   STAT_LINE - выдача строки таблицы
  *
  *
  * @retval uint16_t длина формируемой строки
  */
uint16_t TabConDevRS(char *pcInsert, enum_type_stage_t stage);

/**
  * @brief  Функция формирования схемы соединений устройств в RS
  * @param  char *pcInsert - указатель на массив pcInsert для формируемой строки
  * @param  enum_type_stage_t stage - режим вызова STAT_TITLE - выдача имени страницы
  * 						   STAT_HEAD - выдача заголовка таблицы
  * 						   STAT_LINE - выдача строки таблицы
  *
  *
  * @retval uint16_t длина формируемой строки
  */
uint16_t TabDiagramConDevRS(char *pcInsert, enum_type_stage_t stage);

/**
  * @brief Функция установки запуска построения сети 
  * @param none
  * @retval none
  */
void SetProcDiagramDev( void );

#endif /* __CORE_CNTRL_DEV_H */
/******************* (C)  COPYRIGHT 2020 DataExpress  *****END OF FILE****/
