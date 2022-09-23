 /**
  ******************************************************************************
  * @file    config_var.h
  * @author  Trembach D.N.
  * @version V1.0.2
  * @date    03-05-2015
  * @brief   Файл содержащий описание конфигурируемых переменных
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CONFIG_VAR_H
#define __CONFIG_VAR_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "settings.h"

//перечисление для типа данных
typedef enum
{ 
  UINT8 = 0,
  INT8,
  UINT16,
  INT16,	
  UINT32,
  INT32,
  FLOAT,
  BOOL, 	
  CHARS,
  STRING,
  MAC_TYPE,
  IP_TYPE,
  MASK_TYPE  
}enum_type_tag_t;

//перечисление для типа данных
typedef enum
{ 
  CODEHEX = 0,
  CODEDEC,
  CODEBIN
}enum_type_print_t;

//Определение для разрешения доступа к данным

//Ethernet
#define ETH_R      0x01
#define ETH_W      0x02
#define ETH_MASK   0x03

//UART
#define UART_R     0x04
#define UART_W     0x08
#define UART_MASK  0x0C


//перечисление для типа секций
typedef enum
{ 
	NO_SECT = 0,
	//------- добавлять новые секции только внутри выделенного блока --------
        RF_SECT,
	ETH_SECT,
        ETH_STREAM_SECT,
	CODEC_SECT,  
	ANALOG_SECT,         
	PARAM_SECT,
        LOADER_SECT,        
	//-----------------------------------------------------------------------	
	STOP_SECT	
}section_t;

//структура описания секции 
typedef struct
{
	  section_t   				        number_section; 		//номер переменный
	  char const*					name_section;			//имя секции
	  char const*					descrip_section;		//описание секции	
}Section_typedef_t;

//обьявим массив для секций редактирования переменных
extern Section_typedef_t SectionArray[];

//номер старшей секции переменных
#define MaxNumSection  (7) //(sizeof(SectionArray[]) / sizeof(Section_typedef_t))

#define VarID(a,b,c,d)   ( ( ( ((uint32_t)a) & 0x000000FF )<<24) | ( ( ((uint32_t)b) & 0x000000FF )<<16) | ( ( ((uint32_t)c) & 0x000000FE )<<8) | ( ((uint32_t)d) & 0x000000FF) )   // Type(Тип ресурса),N_port(Номер порта), SETUP_ID(Идентификатор настроеки, Num(номер параметра внутри переменной))

typedef struct
{
	section_t   				number_section;	    //номер секции
        uint32_t                                id_var;             //идентификатор переменной   
	uint8_t   				access_var; 	    //описание доступа к переменным
	enum_type_tag_t				type_var;           //тип переменной;
	enum_type_print_t			type_print;	    //тип вывода на экран;
	void*   				store_var;          //сохраненное значение переменной;
	void*   				new_var;            //новое значение переменных;
	void*   				default_var;	    //значение переменной по умолчанию;
	void*   				flash_var;	    //значение переменной во flash;        
	char const*				description_var;    //описание переменных
}VAR_typedef_t;

//обьявим массив для редактирования переменных
extern VAR_typedef_t SetVarArray[];
	
//номер текущей секции
extern uint8_t NumSect;

//номер текущей переменной
extern uint16_t NumVar;

//номер текущей команды главного меню
extern uint16_t CmdMain;

/**
  * @brief  функция подсчета переменных в массиве SetVarArray.
  * @param  None
  * @retval uint16_t - число переменных.
  */
uint16_t SizeSetVarArray(void);

/**
  * @brief  Функция для восстанавливления значение по умолчанию в области редактирования для всех переменных текущей секции.
  * @param  section_t num_sect - номер секции для которой необходимо установить значение по умолчанию
  * @retval none
  */
void SecDefaultToNew(section_t num_sect);

/**
  * @brief  функция копирования значения по умолчанию в зону редактирования  
  * @param  uint16_t порядковый номер переменной
  * @retval none 
  *
  */
void DefaultToNew(uint16_t NumberVar);

#endif /* __CONFIG_VAR_H */

/************************ (C) COPYRIGHT DEX *****END OF FILE****/
