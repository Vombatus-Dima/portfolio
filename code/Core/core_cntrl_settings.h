/**
  ******************************************************************************
  * @file    core_cntrl_settings.h
  * @author  Trembach Dmitry
  * @version V1.1.0
  * @date    18-04-2019
  * @brief   файл с функциями ядра для контроля настроек
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2019 DataExpress</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CORE_CNTRL_SETTINGS_H
#define __CORE_CNTRL_SETTINGS_H

#include "stm32f4xx.h"
#include "Core.h"
#include "rf_frame_header.h"
#include "rs_frame_header.h"
#include "FreeRTOS.h"
#include "task.h"
#include "config_var.h"

#define CLEAR_EVENTS_COUNTER                   (0xEE) // Сброс счетчиков событий для ресурса

//==============================================================================
//      Настройки для Ядра
//==============================================================================
#define Core_Type                               (1) // Тип ресурса
#define Core_N_port                             (1) // Номер порта
//      SETUP_ID настроек для пакетов Ядра      
#define GET_CORE_VERSION                        (1) // Прочитать версию устройства и его ID
          /*  Перечень подтип GET_CORE_VERSION  */
                #define ID_VER                  (0x01) // Чтение версии проекта   
                #define ID_DATE                 (0x02) // Чтение Даты компиляц
                #define ID_TIME                 (0x03) // Чтение Времени компи
                #define ID_MCU_SIGNATURE        (0x04) // Чтение сигнатуры
                #define ID_MCU_REVISION         (0x05) // Чтение Ревизии мк
                #define ID_MCU_FLASH_SIZE       (0x06) // Чтение flash памяти в килобайтах
                #define ID_MCU_UNIQUE           (0x07) // Чтение 96-битного ID 


#define PING_CORE_PHY_ADDR                      (2) // Пинг по физическому адресу
#define GET_CORE_LOG_ADDR                       (4) //(uint16_t)// Прочитать логический адрес
#define SET_CORE_LOG_ADDR                       (5) //(uint16_t)// Установить логический адрес
/*------------------------------------------------------------------------------------------------------------------*/         
#define GET_CORE_CODE_PASS                      (6)  //(uint16_t)  Код доступа
#define GET_LOADER_PHY_ADDR                     (8)  //(uint16_t)  Физический адрес блока
/*------------------------------------------------------------------------------------------------------------------*/ 
#define GET_CORE_TIME                           (10)//(uint16_t)// Прочитать время (период) обновления диагностики
#define SET_CORE_TIME                           (11)//(uint16_t)// Установить время (период) обновления диагностики

#define GET_CORE_MASK_N00_TRANSL                (20)//(uint16_t * 16) //  Прочитать маску номер 0 ID_RS пакетов трансляции
#define SET_CORE_MASK_N00_TRANSL                (21)//(uint16_t * 16) // Установить маску номер 0 ID_RS пакетов трансляции
#define GET_CORE_MASK_N01_TRANSL                (22)//(uint16_t * 16) //  Прочитать маску номер 1 ID_RS пакетов трансляции
#define SET_CORE_MASK_N01_TRANSL                (23)//(uint16_t * 16) // Установить маску номер 1 ID_RS пакетов трансляции
#define GET_CORE_MASK_N02_TRANSL                (24)//(uint16_t * 16) //  Прочитать маску номер 2 ID_RS пакетов трансляции
#define SET_CORE_MASK_N02_TRANSL                (25)//(uint16_t * 16) // Установить маску номер 2 ID_RS пакетов трансляции
#define GET_CORE_MASK_N03_TRANSL                (26)//(uint16_t * 16) //  Прочитать маску номер 3 ID_RS пакетов трансляции
#define SET_CORE_MASK_N03_TRANSL                (27)//(uint16_t * 16) // Установить маску номер 3 ID_RS пакетов трансляции
#define GET_CORE_MASK_N04_TRANSL                (28)//(uint16_t * 16) //  Прочитать маску номер 4 ID_RS пакетов трансляции
#define SET_CORE_MASK_N04_TRANSL                (29)//(uint16_t * 16) // Установить маску номер 4 ID_RS пакетов трансляции
#define GET_CORE_MASK_N05_TRANSL                (30)//(uint16_t * 16) //  Прочитать маску номер 5 ID_RS пакетов трансляции
#define SET_CORE_MASK_N05_TRANSL                (31)//(uint16_t * 16) // Установить маску номер 5 ID_RS пакетов трансляции
#define GET_CORE_MASK_N06_TRANSL                (32)//(uint16_t * 16) //  Прочитать маску номер 6 ID_RS пакетов трансляции
#define SET_CORE_MASK_N06_TRANSL                (33)//(uint16_t * 16) // Установить маску номер 6 ID_RS пакетов трансляции
#define GET_CORE_MASK_N07_TRANSL                (34)//(uint16_t * 16) //  Прочитать маску номер 7 ID_RS пакетов трансляции
#define SET_CORE_MASK_N07_TRANSL                (35)//(uint16_t * 16) // Установить маску номер 7 ID_RS пакетов трансляции
#define GET_CORE_MASK_N08_TRANSL                (36)//(uint16_t * 16) //  Прочитать маску номер 8 ID_RS пакетов трансляции
#define SET_CORE_MASK_N08_TRANSL                (37)//(uint16_t * 16) // Установить маску номер 8 ID_RS пакетов трансляции
#define GET_CORE_MASK_N09_TRANSL                (38)//(uint16_t * 16) //  Прочитать маску номер 9 ID_RS пакетов трансляции
#define SET_CORE_MASK_N09_TRANSL                (39)//(uint16_t * 16) // Установить маску номер 9 ID_RS пакетов трансляции
#define GET_CORE_MASK_N10_TRANSL                (40)//(uint16_t * 16) //  Прочитать маску номер 0 ID_RS пакетов трансляции
#define SET_CORE_MASK_N10_TRANSL                (41)//(uint16_t * 16) // Установить маску номер 0 ID_RS пакетов трансляции
#define GET_CORE_MASK_N11_TRANSL                (42)//(uint16_t * 16) //  Прочитать маску номер 1 ID_RS пакетов трансляции
#define SET_CORE_MASK_N11_TRANSL                (43)//(uint16_t * 16) // Установить маску номер 1 ID_RS пакетов трансляции   
#define GET_CORE_MASK_N12_TRANSL                (44)//(uint16_t * 16) //  Прочитать маску номер 2 ID_RS пакетов трансляции
#define SET_CORE_MASK_N12_TRANSL                (45)//(uint16_t * 16) // Установить маску номер 2 ID_RS пакетов трансляции
#define GET_CORE_MASK_N13_TRANSL                (46)//(uint16_t * 16) //  Прочитать маску номер 3 ID_RS пакетов трансляции
#define SET_CORE_MASK_N13_TRANSL                (47)//(uint16_t * 16) // Установить маску номер 3 ID_RS пакетов трансляции

#define GET_CORE_NMASK_TRANSL                   (50)//(uint8_t) // Прочитать номер маски ID_RS пакетов трансляции в CORE
#define SET_CORE_NMASK_TRANSL                   (51)//(uint8_t) // Установить номер маски ID_RS пакетов трансляции в CORE
                                                
#define GET_CORE_MASK_INPUT_PORT_ID             (70)//(uint32_t)// Прочитать маску ID портов разрешенных для приема в данный порт
#define SET_CORE_MASK_INPUT_PORT_ID             (71)//(uint32_t)// Установить маску ID портов разрешенных для приема в данный порт

#define GET_CORE_NGROUP_RING_ID                 (80)//(uint8_t)// Прочитать номер группы кольца (0 - без группы)
#define SET_CORE_NGROUP_RING_ID                 (81)//(uint8_t)// Установить номер группы кольца (0 - без группы)
#define GET_CORE_PRIORITY_RING_ID               (82)//(uint8_t)// Прочитать приоритет в группе кольца (0 - без группы)
#define SET_CORE_PRIORITY_RING_ID               (83)//(uint8_t)// Установить приоритет в группе кольца (0 - без группы)

//==============================================================================
//      Настройки для RS
//==============================================================================
#define RS_Type                                 (2)             // Тип ресурса
#define RS_A_port                               (1)             // Номер порта
#define RS_B_port                               (2)             // Номер порта

//      SETUP_ID настроек для пакетов RS
#define GET_RS_MODE                             (2) //(uint8_t) // Прочитать режим работы
#define SET_RS_MODE                             (3) //(uint8_t) // Установить режим работы 
#define GET_RS_BAUDRATE                         (4) //(uint32_t)// Прочитать скорость RS
#define SET_RS_BAUDRATE                         (5) //(uint32_t)// Установить скорость RS
    
#define GET_RS_NMASK_TRANSL                     (50)//(uint8_t) // Прочитать номер текущей маски ID_RS пакетов трансляции
#define SET_RS_NMASK_TRANSL                     (51)//(uint8_t) // Установить номер текущей маски ID_RS пакетов трансляции
                                                
#define GET_RS_MASK_INPUT_PORT_ID               (70)//(uint32_t)// Прочитать маску ID портов разрешенных для приема в данный порт
#define SET_RS_MASK_INPUT_PORT_ID               (71)//(uint32_t)// Установить маску ID портов разрешенных для приема в данный порт

//==============================================================================
//      Настройки для CODEC
//==============================================================================
#define CODEC_Type                              (4)             // Тип ресурса
#define CODEC_A_port                            (1)             // Номер порта
#define CODEC_B_port                            (2)             // Номер порта
#define CODEC_C_port                            (3)             // Номер порта


#define GET_CODEC_CHANEL                        (10)//(uint8_t) // Прочитать номер канала закреленный за кодекам 
#define SET_CODEC_CHANEL                        (11)//(uint8_t) // Установить 

#define GET_CODEC_PRIORITY_CH_PCM               (12)//(uint8_t) // Прочитать приоритет голосового канала pcm 
#define SET_CODEC_PRIORITY_CH_PCM               (13)//(uint8_t) // Установить 
#define GET_CODEC_PRIORITY_CH_CMX               (14)//(uint8_t) // Прочитать приоритет голосового канала cmx 
#define SET_CODEC_PRIORITY_CH_CMX               (15)//(uint8_t) // Установить 

#define GET_CODEC_MASK_SOURSE_SOFT_PORT         (20)//(uint32_t) // Прочитать маску портов разрешенных для приема в данный порт из програмного роутера
#define SET_CODEC_MASK_SOURSE_SOFT_PORT         (21)//(uint32_t) 
#define GET_CODEC_MASK_CHANEL_SOFT_PORT         (22)//(uint32_t) // Прочитать маску каналов разрешенных для приема в данный порт из програмного роутера
#define SET_CODEC_MASK_CHANEL_SOFT_PORT         (23)//(uint32_t) // 

#define GET_CODEC_NMASK_TRANSL                  (54)//(uint8_t) // Прочитать номер текущей маски ID_RS пакетов трансляции
#define SET_CODEC_NMASK_TRANSL                  (55)//(uint8_t) // Установить номер текущей маски ID_RS пакетов трансляции
    
#define GET_CODEC_MASK_INPUT_PORT_ID            (70)//(uint32_t)// Прочитать маску ID портов разрешенных для приема в данный порт
#define SET_CODEC_MASK_INPUT_PORT_ID            (71)//(uint32_t)// Установить маску ID портов разрешенных для приема в данный порт


//==============================================================================
//      Настройки для ANALOG CODEC
//==============================================================================
#define ANALOG_Type                             (7)             // Тип ресурса
#define ANALOG_port                             (1)             // Номер порта

#define GET_ANALOG_MODE                         (8)
#define SET_ANALOG_MODE                         (9)

#define GET_ANALOG_CHANEL                       (10)//(uint8_t) // Прочитать номер канала закреленный за кодекам 
#define SET_ANALOG_CHANEL                       (11)//(uint8_t) // Установить 

#define GET_ANALOG_PRIORITY_DISCRET              (12)//(uint8_t) // Прочитать приоритет голосового канала pcm 
#define SET_ANALOG_PRIORITY_DISCRET              (13)//(uint8_t) // Установить 

#define GET_ANALOG_PRIORITY_ANALOG              (14)//(uint8_t) // Прочитать приоритет голосового канала cmx 
#define SET_ANALOG_PRIORITY_ANALOG              (15)//(uint8_t) // Установить 

#define GET_ANALOG_MASK_SOURSE_SOFT_PORT        (20)//(uint32_t) // Прочитать маску портов разрешенных для приема в данный порт из програмного роутера
#define SET_ANALOG_MASK_SOURSE_SOFT_PORT        (21)//(uint32_t) 

#define GET_ANALOG_MASK_CHANEL_SOFT_PORT        (22)//(uint32_t) // Прочитать маску каналов разрешенных для приема в данный порт из програмного роутера
#define SET_ANALOG_MASK_CHANEL_SOFT_PORT        (23)//(uint32_t) // 

//==============================================================================
//      Настройки для ETH
//==============================================================================
#define ETH_Type                                (5)             // Тип ресурса
#define ETH_N_port                              (1)             // Номер порта
//      SETUP_ID настроек для пакетов ETH       
#define GET_ETH_IP_ADDR                         (2) //(uint8_t * 4)//  Прочитать IP адрес 
#define GET_ETH_IP_MASK                         (4) //(uint8_t * 4)//  Прочитать маску IP адреса 
#define GET_ETH_IP_GATE                         (6) //(uint8_t * 4)//  Прочитать IP адрес шлюза 

#define GET_ETH_IP_PORT_LOADER                  (8) //(uint16_t)//  Прочитать IP порт загрузчика для обновления ПО
#define GET_ETH_MAC_ADDR                        (10) //(uint8_t * 6)  //  Прочитать MAC адрес 
    
#define GET_ETH_IP_PORT_VOICE                   (20) //(uint16_t)//  Прочитать IP порт приема передачи голосовых сообщений
#define SET_ETH_IP_PORT_VOICE                   (21) //(uint16_t)// Установить IP порт приема передачи голосовых сообщений

#define GET_ETH_IP_ADDR_TABLE_SRV_MAIN          (30)//(uint8_t * 4)//  Прочитать IP адрес главного сервера раздающего таблицу рассылки
#define SET_ETH_IP_ADDR_TABLE_SRV_MAIN          (31)//(uint8_t * 4)// Установить IP адрес главного сервера раздающего таблицу рассылки
#define GET_ETH_IP_PORT_TABLE_SRV_MAIN          (32)//(uint16_t)//  Прочитать IP порт главного сервера раздающего таблицу рассылки
#define SET_ETH_IP_PORT_TABLE_SRV_MAIN          (33)//(uint16_t)// Установить IP порт главного сервера раздающего таблицу рассылки
    
#define GET_ETH_IP_ADDR_TABLE_SRV_REZV          (34)//(uint8_t * 4)//  Прочитать IP адрес резервного сервера раздающего таблицу рассылки
#define SET_ETH_IP_ADDR_TABLE_SRV_REZV          (35)//(uint8_t * 4)// Установить IP адрес резервного сервера раздающего таблицу рассылки
#define GET_ETH_IP_PORT_TABLE_SRV_REZV          (36)//(uint16_t)//  Прочитать IP порт  резервного сервера раздающего таблицу рассылки
#define SET_ETH_IP_PORT_TABLE_SRV_REZV          (37)//(uint16_t)// Установить IP порт  резервного сервера раздающего таблицу рассылки
    
#define GET_PHY_ADDR_DISP_1                     (40)//(uint16_t)//  Прочитать phy адрес диспетчера по кнопке 1
#define SET_PHY_ADDR_DISP_1                     (41)//(uint16_t)// Установить phy адрес диспетчера по кнопке 1
#define GET_PHY_ADDR_DISP_2                     (42)//(uint16_t)//  Прочитать phy адрес диспетчера по кнопке 2
#define SET_PHY_ADDR_DISP_2                     (43)//(uint16_t)// Установить phy адрес диспетчера по кнопке 2

#define GET_ETH_NMASK_TRANSL_VOICE              (50)//(uint8_t) //  Прочитать номер текущей маски ID_RS пакетов трансляции для голосового порта рассылки
#define SET_ETH_NMASK_TRANSL_VOICE              (51)//(uint8_t) // Установить номер текущей маски ID_RS пакетов трансляции для голосового порта рассылки

#define GET_ETH_NMASK_TRANSL_CONFIG             (54)//(uint8_t) //  Прочитать номер текущей маски ID_RS пакетов трансляции конфигурирования и загрузки
#define SET_ETH_NMASK_TRANSL_CONFIG             (55)//(uint8_t) // Установить номер текущей маски ID_RS пакетов трансляции конфигурирования и загрузки
  
#define GET_ETH_MASK_INPUT_PORT_ID_VOICE        (70)//(uint32_t)//  Прочитать маску ID портов разрешенных для приема в данный порт трансляции для голосового порта рассылки
#define SET_ETH_MASK_INPUT_PORT_ID_VOICE        (71)//(uint32_t)// Установить маску ID портов разрешенных для приема в данный порт трансляции для голосового порта рассылки

#define GET_ETH_MASK_INPUT_PORT_ID_CONFIG       (74)//(uint32_t)//  Прочитать маску ID портов разрешенных для приема в данный порт трансляции конфигурирования и загрузки
#define SET_ETH_MASK_INPUT_PORT_ID_CONFIG       (75)//(uint32_t)// Установить маску ID портов разрешенных для приема в данный порт трансляции конфигурирования и загрузки

#define GET_UDP_MASK_SOURSE_SOFT_PORT           (76)//(uint32_t)//  Прочитать портов разрешенных для приема в UDP порт из програмного роутера
#define SET_UDP_MASK_SOURSE_SOFT_PORT           (77)//(uint32_t)// Установить портов разрешенных для приема в UDP порт из програмного роутера

#define GET_UDP_MASK_CHANEL_SOFT_PORT           (78)//(uint32_t)//  Прочитать каналов разрешенных для приема в UDP порт из програмного роутера
#define SET_UDP_MASK_CHANEL_SOFT_PORT           (79)//(uint32_t)// Установить каналов разрешенных для приема в UDP порт из програмного роутера



// @brief Unique ID register address location
#define ID_UNIQUE_ADDRESS		0x1FFF7A10
// @brief Flash size register address
#define ID_FLASH_ADDRESS		0x1FFF7A22
// @brief Device ID register address
#define ID_DBGMCU_IDCODE		0xE0042000


/**
 * @brief  Get STM32F4xx device signature
 * @note   Defined as macro to get maximal response time
 * @param  None
 * @retval Device signature, bits 11:0 are valid, 15:12 are always 0.
 *           - 0x0413: STM32F405xx/07xx and STM32F415xx/17xx)
 *           - 0x0419: STM32F42xxx and STM32F43xxx
 *           - 0x0423: STM32F401xB/C
 *           - 0x0433: STM32F401xD/E
 *           - 0x0431: STM32F411xC/E
 */
#define GetSignature()	((*(uint16_t *) (ID_DBGMCU_IDCODE)) & 0x0FFF)

/**
 * @brief  Get STM32F4xx device revision
 * @note   Defined as macro to get maximal response time
 * @param  None
 * @retval Device revision value
 *           - 0x1000: Revision A
 *           - 0x1001: Revision Z
 *           - 0x1003: Revision Y
 *           - 0x1007: Revision 1
 *           - 0x2001: Revision 3
 */
#define GetRevision()		(uint16_t)((DBGMCU->IDCODE)>>16)

/**
 * @brief  Get STM32F4xx device's flash size in kilo bytes
 * @note   Defined as macro to get maximal response time
 * @param  None
 * @retval Flash size in kilo bytes
 */
#define GetFlashSize()	        (*(uint16_t *) (ID_FLASH_ADDRESS))

/**
 * @brief  Get unique ID number in 8-bit format
 * @note   STM32F4xx has 96bits long unique ID, so 12 bytes are available for read in 8-bit format
 * @note   Defined as macro to get maximal response time
 * @param  x: Byte number: specify which part of 8 bits you want to read
 *               - Values between 0 and 11 are allowed
 * @retval Unique ID address
 */
//#define GetUnique8(x)	((x >= 0 && x < 12) ? (*(uint8_t *) (ID_UNIQUE_ADDRESS + (x))) : 0)
#define GetUnique8(x)	(( x < 12 ) ? (*(uint8_t *) (ID_UNIQUE_ADDRESS + (x))) : 0)	
/**
 * @brief  Get unique ID number in 16-bit format
 * @note   STM32F4xx has 96bits long unique ID, so 6 2-bytes values are available for read in 16-bit format
 * @note   Defined as macro to get maximal response time
 * @param  x: Byte number: specify which part of 16 bits you want to read
 *               - Values between 0 and 5 are allowed
 * @retval Unique ID address
 */
#define GetUnique16(x)	((x >= 0 && x < 6) ? (*(uint16_t *) (ID_UNIQUE_ADDRESS + 2 * (x))) : 0)

/**
 * @brief  Get unique ID number in 32-bit format
 * @note   STM32F4xx has 96bits long unique ID, so 3 4-bytes values are available for read in 32-bit format
 * @note   Defined as macro to get maximal response time
 * @param  x: Byte number: specify which part of 16 bits you want to read
 *               - Values between 0 and 2 are allowed
 * @retval Unique ID address
 */
#define GetUnique32(x)	((x >= 0 && x < 3) ? (*(uint32_t *) (ID_UNIQUE_ADDRESS + 4 * (x))) : 0)


/**
  * @brief Функция обработка запросов конфигурирования
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  *
  * @retval bool true  - пакет обработан
  *              false - пакет не обработан
  */
bool ProcessingConfigBox(core_struct_t* core_st);

/**
  * @brief Функция формирования тестового запроса конфигурирования
  * @param QueueHandle_t xQueue_router - указатель на очередб в роутер
  * @param router_box_t  *data_rx - указатель на буфер приема данных
  * @param uint16_t   TimeUpdate - временной интервал с последнего обновления 
  *
  * @retval none
  */
void TestSetupReq(QueueHandle_t xQueue_router, router_box_t *data_rx , uint16_t   TimeUpdate);

/**
  * @brief Функция формирования ответа на запрос конфигурирования
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  *
  * @retval none
  */
void GenRespConfigBox(core_struct_t* core_st);


/**
  * @brief Функция формирования шапки ответа на запрос конфигурирования и отправка его в роутер
  * @param uint8_t data_size - размер передаваемых данных
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  *
  * @retval none
  */
void GenRespSetupBox(core_struct_t* core_st, uint8_t data_lenght);

#endif /* __CORE_CNTRL_SETTINGS_H */
/******************* (C)  COPYRIGHT 2019 DataExpress  *****END OF FILE****/
