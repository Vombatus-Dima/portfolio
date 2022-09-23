/**
  ******************************************************************************
  * @file    core_cntrl_rsn.h
  * @author  Trembach Dmitry
  * @version V1.1.0
  * @date    12-03-2019
  * @brief   файл с описанием функция ядра для контроля РСН в своей области
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2019 DataExpress</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CORE_CNTRL_RSN_H
#define __CORE_CNTRL_RSN_H

#include "stm32f4xx.h"
#include "core.h"
#include "rf_frame_header.h"
#include "rs_frame_header.h"
#include "status_handler.h"     

/* Время жизни записи в таблице регистрации каналов РСН в млсек     */
#define MAX_TTL_TAB_REG_CH_RSN   	(20000/TIME_CORE_UPDATE) // не болеее 0xFFFF

// Период формирования пакета позиционирования 5 сек
#define MAX_TIME_OUT_POSITION  (5000)

// Период период обновления маски каналов для таблиц рассылки 5 сек
#define MAX_UPDATE_TTL_TAB_REG_CH_TAG  (5000)

/* Максимально время регистрации */
#define MAX_TIME_OF_LIFE_TAG ( 20000 / TIME_CORE_UPDATE )

/* Максимально время регистрации активности */
#define MAX_TIME_OF_LIFE_ACTIVE_TAG ( 10000 / TIME_CORE_UPDATE )

/**
  * @brief Функция обнуления таблицы регистраций рсн  
  * @param none
  * @retval none
  */
void ClearRegTagRsn(void);

/**
  * @brief Функция обнуления таблицы каналов рсн
  * @param none
  * @retval none
  */
void ClearRegChanelRsn(void);
    
/**
  * @brief Функция парсинга пакета позиционирования 
  * @param router_box_t* rs_box - указатель на пакета позиционирования  
  *
  * @retval bool true  - пакет обработан
  *              false - пакет не обработан
  */
bool ProcessingNewPositionBox( router_box_t* rs_box );

/**
  * @brief Функция копирования таблицы каналов голосовых тегов
  * @param cntrol_chanel_t*  tab_chanel - указатель на таблицу активных каналов
  * @retval uint32_t - маска зарегистрированных каналов голосовых тегов
  */
uint32_t UpdateTabChanel( cntrol_chanel_t* tab_chanel );

/**
  * @brief Функция обнуления всей таблицы тегов
  * @param none
  * @retval none
  */
void ClearTableTag( void );

/**
  * @brief Функция обновляет время жизни зарегистрированного тега, если время жизни исчерпано
  *        удаляем тег из таблицы
  *
  * @param uint16_t update_time - период обновления
  * @retval uint16_t - число зарегистрированных тегов в таблице
  */
uint16_t UpdateTagTable( uint16_t update_time );

/**
  * @brief Функция обновляет маску каналов прослушки
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  * @param uint16_t   TimeUpdateReg - временной интервал с последнего обновления регистрации
  * @retval none
  */
void UpdateChanelMask( core_struct_t* core_st, uint16_t   TimeUpdateReg );

/**
  * @brief  Функция формирования статуса зарегистрированных тегов
  * @param  char *pcInsert - указатель на массив pcInsert для формируемой строки
  * @param  enum_type_stage_t stage - режим вызова STAT_TITLE - выдача имени страницы
  * 						   STAT_HEAD - выдача заголовка таблицы
  * 						   STAT_LINE - выдача строки таблицы
  *
  *
  * @retval uint16_t длина формируемой строки
  */
uint16_t TabStatusTag(char *pcInsert, enum_type_stage_t stage);

/**
  * @brief  Функция формирования списка зарегистрированных каналов
  * @param  char *pcInsert - указатель на массив pcInsert для формируемой строки
  * @param  enum_type_stage_t stage - режим вызова STAT_TITLE - выдача имени страницы
  * 						   STAT_HEAD - выдача заголовка таблицы
  * 						   STAT_LINE - выдача строки таблицы
  *
  *
  * @retval uint16_t длина формируемой строки
  */
uint16_t TabStatusChanel(char *pcInsert, enum_type_stage_t stage);

/**
  * @brief  Функция формирования статуса зарегистрированных тегов RMA STM
  * @param  char *pcInsert - указатель на массив pcInsert для формируемой строки
  * @param  enum_type_stage_t stage - режим вызова STAT_TITLE - выдача имени страницы
  * 						   STAT_HEAD - выдача заголовка таблицы
  * 						   STAT_LINE - выдача строки таблицы
  *
  *
  * @retval uint16_t длина формируемой строки
  */
uint16_t TabStatusTagRmaSTM(char *pcInsert, enum_type_stage_t stage);

/**
  * @brief  Функция формирования статуса зарегистрированных тегов RMA спутник
  * @param  char *pcInsert - указатель на массив pcInsert для формируемой строки
  * @param  enum_type_stage_t stage - режим вызова STAT_TITLE - выдача имени страницы
  * 						   STAT_HEAD - выдача заголовка таблицы
  * 						   STAT_LINE - выдача строки таблицы
  *
  *
  * @retval uint16_t длина формируемой строки
  */
uint16_t TabStatusTagSputnic(char *pcInsert, enum_type_stage_t stage);

/**
  * @brief  Функция формирования статуса зарегистрированных тегов RMA STM
  * @param  char *pcInsert - указатель на массив pcInsert для формируемой строки
  * @param  enum_type_stage_t stage - режим вызова STAT_TITLE - выдача имени страницы
  * 						   STAT_HEAD - выдача заголовка таблицы
  * 						   STAT_LINE - выдача строки таблицы
  *
  *
  * @retval uint16_t длина формируемой строки
  */
uint16_t TabStatusTagRSN(char *pcInsert, enum_type_stage_t stage);

/**
  * @brief  Функция формирования статуса зарегистрированных тегов в RF
  * @param  char *pcInsert - указатель на массив pcInsert для формируемой строки
  * @param  enum_type_stage_t stage - режим вызова STAT_TITLE - выдача имени страницы
  * 						   STAT_HEAD - выдача заголовка таблицы
  * 						   STAT_LINE - выдача строки таблицы
  *
  *
  * @retval uint16_t длина формируемой строки
  */
uint16_t TabStatusTagRF(char *pcInsert, enum_type_stage_t stage);

/**
  * @brief  Функция формирования списка число зарегистрированных тегов по типам
  * @param  char *pcInsert - указатель на массив pcInsert для формируемой строки
  * @param  enum_type_stage_t stage - режим вызова STAT_TITLE - выдача имени страницы
  * 						   STAT_HEAD - выдача заголовка таблицы
  * 						   STAT_LINE - выдача строки таблицы
  *
  *
  * @retval uint16_t длина формируемой строки
  */
uint16_t TabStatusRegTag(char *pcInsert, enum_type_stage_t stage);

/**
  * @brief Функция формирования пакета отчета о позиционировании
  * @param core_struct_t*  core_st - указатель на структуру ядра
  * @retval None
  */
void ProcessingPositionReport( core_struct_t* core_st );

/**
  * @brief Функция управления запуска построения сети 
  * @param none
  * @retval none
  */
void UpdateProcDiagramDev( void );

/**
  * @brief Функция проверяет таблицу на наличие голосовых тегов на канале 
  * @param uint8_t Chanel - номер канала
  * @param uint8_t src_port_ID - порт ID источника
  * @retval bool - true - есть зарегистрированные теги
  *		 false  - нет зарегистрированныз тегов 
  */
bool GetRegVoiceTagChanel( uint8_t Chanel, uint8_t src_port_ID );

/**
  * @brief Функция проверяет таблицу на наличие голосовых тегов на канале 
  * @param uint8_t Chanel - номер канала
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - колличество тегов  
  */
uint8_t GetNumVoiceTagChanel( uint8_t Chanel, uint8_t src_port_ID );

/**
  * @brief Функция возвращает число зарегистрированных каналов  
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - колличество каналов 
  */
uint8_t GetRegChanel( uint8_t src_port_ID );

/**
  * @brief Функция возвращает число зарегистрированных/видимых тегов светильников  
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - колличество тегов 
  */
uint8_t GetRegTagRMA( uint8_t src_port_ID );

/**
  * @brief Функция возвращает число зарегистрированных/видимых тегов радиостанций  
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - колличество тегов 
  */
uint8_t GetRegTagRSN( uint8_t src_port_ID );

/**
  * @brief Функция возвращает число зарегистрированных/видимых тегов газоанализаторов  
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - колличество тегов 
  */
uint8_t GetRegTagGas( uint8_t src_port_ID );

/**
  * @brief Функция возвращает число зарегистрированных/видимых тегов   
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - колличество тегов 
  */
uint8_t GetRegTagAll( uint8_t src_port_ID );

/**
  * @brief Функция проверяет таблицу на наличие голосовых тегов 
  * @param uint8_t src_port_ID - порт ID источника
  * @retval bool - true - есть зарегистрированные теги
  *		 false  - нет зарегистрированныз тегов 
  */
bool GetRegVoiceTag( uint8_t src_port_ID );

/**
  * @brief Функция возвращает число голосовых тегов зарегистрированных на заданном канале  
  * @param uint8_t Chanel  - канал регистрации РСН
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t  число голосовых тегов зарегистрированных на заданном канале
  */
uint8_t GetNumRegVoiceTagChanel( uint8_t Chanel , uint8_t src_port_ID);

/**
  * @brief Функция проверки регистрации тега эфире на  
  * @param uint16_t ADDR_Tag - адрес тега
  * @param uint8_t src_port_ID - порт ID источника
  * @retval bool - true зарегистрирован
  *                false не зарегистрирован
  */
bool CheckRegTag( uint16_t ADDR_Tag, uint8_t src_port_ID );    

#define UDP_RX_POS_DEBUG  (0)
#define UDP_TX_POS_DEBUG  (0)

#if (UDP_RX_POS_DEBUG==1)
/**
  * @brief Функция для формирования диагностического сообщения по рассылке пакета позиционирования.
  * @param const char *header_mess - шапка сообщения 
  * @param uint16_t addr_src - адрес источника врм 
  * @param uint8_t type_tag - тип позиционируемого тега   
  * @param uint16_t addr_tag - адрес позиционируемого тега 
  * @param uint16_t addr_phy - адрес наилучшего врм 
  * @retval None
  */
void diag_reg_rs_pos( const char *header_mess, uint16_t addr_src, uint8_t type_tag, uint16_t addr_tag, uint16_t addr_phy );
#else
#define diag_reg_rs_pos( a, b, c, d, e );
#endif  

#if (UDP_TX_POS_DEBUG==1)
/**
  * @brief Функция для формирования диагностического сообщения по рассылке пакета позиционирования.
  * @param const char *header_mess - шапка сообщения 
  * @param uint8_t type_tag - тип позиционируемого тега   
  * @param uint16_t addr_tag - адрес позиционируемого тега 
  * @param uint16_t addr_phy - адрес наилучшего врм 
  * @retval None
  */
void diag_tx_mes_pos( const char *header_mess, uint8_t type_tag, uint16_t addr_tag, uint16_t addr_phy );
#else
#define diag_tx_mes_pos( a, b, c, d );
#endif  

#endif /* __CORE_CNTRL_RSN_H */
/******************* (C)  COPYRIGHT 2019 DataExpress  *****END OF FILE****/
