/**
  ******************************************************************************
  * @file    settings.c
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

/* Includes ------------------------------------------------------------------*/
#include "settings.h"
#include "stm32f4xx_crc.h"
#include "EEPROM_Flash.h"
#include "main.h"
#include "Core.h"
#include "soft_router.h"

// структура содержит значения по умолчанию для всех переменных которые будем хранить в EEPROM FLASH
// если добавляем переменную - то обязательно вносим ее в таблицу значений по умолчанию	!!!
const	FlashSettingsTypeDef DataDef =
{
       0, 	/* контрольная сумма структуры параметров и счетчика числа стираний FLASH*/  
       ID_SETTINGS, /* идентификатор структуры */
//==============================================================================
//      Настройки для Ядра
//==============================================================================     
      0xFF01,   //uint16_t   log_adr;                        // Логический адрес блока

      30000,    //uint16_t   time_update_diagn;              // Время (период) обновления диагностики
      {
        //#define MaskBoxIDCore          (0x00)
	//#define NEW_POSITION            0x05 //
	//#define DIAGNOSTICS_REQ         0x14 //20 Запросы команд диагностики
	//#define DIAGNOSTICS_RESP        0x15 //21 Ответы команд диагностики
	//#define CMD_TAG_REQ             0xF5 //245 Запросы команд для управления тегами
	//#define CMD_TAG_RESP            0xF6 //246 Ответы команд для управления тегами
	//#define REG_MASK_REQ            0xF7 //247 Запросы команд с регистрами масок портов
	//#define REG_MASK_RESP           0xF8 //248 Ответы команд с регистрами масок портов
	//#define SETUP_REQ               0xFB //251 Запросы команд настроек
	//#define SETUP_RESP              0xFC //252 Ответы команд настроек
	//#define BOOTLOADER_REQ          0xFD //253 Запросы команд бутлоадера
        //#define ID_MODBUS_MASTER        0xF9 //249 Пакет MODBUS от мастера к подчиненному
        //#define ID_SETUP_RF_REQ         0xE2 //251 Запросы команд настроек через RF
        //#define ID_BOOTLOADER_RF_REQ    0xE4 // Запросы команд бутлоадера

    { 
      CIDX5                                                          ,
      CIDX4 | CIDX5                                                  ,
      0x0000                                                         ,
      0x0000                                                         ,
      0x0000                                                         ,
      0x0000                                                         ,
      0x0000                                                         ,
      0x0000                                                         ,  
      0x0000                                                         ,
      0x0000                                                         ,
      0x0000                                                         ,
      0x0000                                                         ,  
      0x0000                                                         ,
      0x0000                                                         ,
      CIDX2 | CIDX4                                                  ,
      CIDX5 | CIDX6 | CIDX7 | CIDX8 | CIDX9 | CIDXB |  CIDXC |  CIDXD 
      },            


      //#define MaskBoxIDCodec          (0x01)  
      //#define VOICE                   0x01 //1 Голосовые данные
      //#define CALL_DISP               0x03 //3 Вызов диспетчера
      //#define REQ_DISP                0x04 //4 Подтверждение вызова диспетчера
        
    { CIDX1 | CIDX3 | CIDX4  ,
      0x0000                 ,
      0x0000                 ,
      0x0000                 ,
      0x0000                 ,
      0x0000                 ,
      0x0000                 ,
      0x0000                 ,  
      0x0000                 ,
      0x0000                 ,
      0x0000                 ,
      0x0000                 ,  
      0x0000                 ,
      0x0000                 ,
      0x0000                 ,
      0x0000                 
    },   
      
    //#define MaskBoxIDVoiceETH       (0x02)
    //#define VOICE                   0x01 //1 Голосовые данные
    //#define CALL_DISP               0x03 //3 Вызов диспетчера
    //#define REQ_DISP                0x04 //4 Подтверждение вызова диспетчера
    //#define NEW_POSITION            0x05 //
    //#define ID_INDIVID_RSN_CH       0x06 //6 Команда запроса на формировани индивидуального канала
        
    { CIDX1 | CIDX3 | CIDX4 | CIDX5 | CIDX6 ,
      0x0000                                ,
      0x0000                                ,
      0x0000                                ,
      0x0000                                ,
      0x0000                                ,
      0x0000                                ,
      0x0000                                ,  
      0x0000                                ,
      0x0000                                ,
      0x0000                                ,
      0x0000                                ,  
      0x0000                                ,
      0x0000                                ,
      0x0000                                ,
      0x0000                                
    },   
 
    //#define MaskBoxDublicate                (0x03)  
    //#define ID_VOICE                   0x01 //1 Голосовые данные
    //#define ID_RING_DISP               0x03 //3 Вызов диспетчера
    //#define ID_REQ_DISP                0x04 //4 Подтверждение вызова диспетчера
    //#define ID_INDIVID_RSN_CH          0x06 //6 Команда запроса на формировани индивидуального канала
    //#define ID_CMD_TAG_REQ             0xF5 //245 Запросы команд для управления тегами
    //#define ID_CMD_TAG_RESP            0xF6 //246 Ответы команд для управления тегами
    
    { CIDX1 | CIDX3 | CIDX4 | CIDX6,
      0x0000                       ,
      0x0000                       ,
      0x0000                       ,
      0x0000                       ,
      0x0000                       ,
      0x0000                       ,
      0x0000                       ,  
      0x0000                       ,
      0x0000                       ,
      0x0000                       ,
      0x0000                       ,  
      0x0000                       ,
      0x0000                       ,
      0x0000                       ,
      CIDX5 | CIDX6                
    },  
      
	//#define MaskBoxIDETH                    (0x04)     
    { 0x0000            ,
      0x0000            ,
      0x0000            ,
      0x0000            ,
      0x0000            ,
      0x0000            ,
      0x0000            ,
      0x0000            ,  
      0x0000            ,
      0x0000            ,
      0x0000            ,
      0x0000            ,  
      0x0000            ,
      0x0000            ,
      0x0000            ,
      0x0000            
    },  

	//#define MaskBoxIDtubeETH                (0x05)     
      
    { 0x0000            ,
      0x0000            ,
      0x0000            ,
      0x0000            ,
      0x0000            ,
      0x0000            ,
      0x0000            ,
      0x0000            ,  
      0x0000            ,
      0x0000            ,
      0x0000            ,
      0x0000            ,  
      0x0000            ,
      0x0000            ,
      0x0000            ,
      0x0000            
    },    
             
        //#define MaskBoxIDRSAPort                (0x06)   
	//#define ID_VOICE                   0x01 //1 Голосовые данные
	//#define ID_REQ_DISP                0x04 //4 Подтверждение вызова диспетчера
        //#define ID_INDIVID_RSN_CH          0x06 //6 Команда запроса на формировани индивидуального канала
	//#define ID_DIAGNOSTICS_REQ         0x14 //20 Запросы команд диагностики
        //#define ID_DIAG_HIGHWAY_RS_REQ     0x16 //22 Запросы команд диагностики магистрали RS
        //#define ID_DIAG_HIGHWAY_RS_RESP    0x17 //23 Ответы команд диагностики  магистрали RS
	//#define ID_CMD_TAG_REQ             0xF5 //245 Запросы команд для управления тегами
	//#define ID_REG_MASK_REQ            0xF7 //247 Запросы команд с регистрами масок портов
        //#define ID_MODBUS_MASTER           0xF9 //249 Пакет MODBUS от мастера к подчиненному
        //#define ID_MODBUS_SLAVE            0xFA //250 Пакет MODBUS от подчиненному к мастеру
	//#define ID_SETUP_REQ               0xFB //251 Запросы команд настроек
	//#define ID_BOOTLOADER_REQ          0xFD //253 Запросы команд бутлоадера
        
    { CIDX1 | CIDX4 | CIDX6                        ,
      CIDX4 | CIDX6 | CIDX7                        ,
      0x0000                                       ,
      0x0000                                       ,
      0x0000                                       ,
      0x0000                                       ,
      0x0000                                       ,
      0x0000                                       ,  
      0x0000                                       ,
      0x0000                                       ,
      0x0000                                       ,
      0x0000                                       ,  
      0x0000                                       ,
      0x0000                                       ,
      0x0000                                       ,
      CIDX5 | CIDX7 | CIDX9 | CIDXA | CIDXB | CIDXD
    },  
        //#define MaskBoxIDRSBPort                (0x07)   
	//#define ID_VOICE                   0x01 //1 Голосовые данные
	//#define ID_REQ_DISP                0x04 //4 Подтверждение вызова диспетчера
        //#define ID_INDIVID_RSN_CH          0x06 //6 Команда запроса на формировани индивидуального канала
	//#define ID_DIAGNOSTICS_REQ         0x14 //20 Запросы команд диагностики
        //#define ID_DIAG_HIGHWAY_RS_REQ     0x16 //22 Запросы команд диагностики магистрали RS
        //#define ID_DIAG_HIGHWAY_RS_RESP    0x17 //23 Ответы команд диагностики  магистрали RS
	//#define ID_CMD_TAG_REQ             0xF5 //245 Запросы команд для управления тегами
	//#define ID_REG_MASK_REQ            0xF7 //247 Запросы команд с регистрами масок портов
        //#define ID_MODBUS_MASTER           0xF9 //249 Пакет MODBUS от мастера к подчиненному
        //#define ID_MODBUS_SLAVE            0xFA //250 Пакет MODBUS от подчиненному к мастеру
	//#define ID_SETUP_REQ               0xFB //251 Запросы команд настроек
	//#define ID_BOOTLOADER_REQ          0xFD //253 Запросы команд бутлоадера
        
    { CIDX1 | CIDX4 | CIDX6                        ,
      CIDX4 | CIDX6 | CIDX7                        ,
      0x0000                                       ,
      0x0000                                       ,
      0x0000                                       ,
      0x0000                                       ,
      0x0000                                       ,
      0x0000                                       ,  
      0x0000                                       ,
      0x0000                                       ,
      0x0000                                       ,
      0x0000                                       ,  
      0x0000                                       ,
      0x0000                                       ,
      0x0000                                       ,
      CIDX5 | CIDX7 | CIDX9 | CIDXA | CIDXB | CIDXD
    },  

    //#define MaskBoxIDVoiceInfo              (0x08)    
    //#define ID_VOICE                   0x01 //1 Голосовые данные
    //#define ID_RING_DISP               0x03 //3 Вызов диспетчера
    //#define ID_REQ_DISP                0x04 //4 Подтверждение вызова диспетчера
    //#define ID_INDIVID_RSN_CH          0x06 //6 Команда запроса на формировани индивидуального канала
    //#define ID_CMD_TAG_REQ             0xF5 //245 Запросы команд для управления тегами
    //#define ID_CMD_TAG_RESP            0xF6 //246 Ответы команд для управления тегами
    
    { CIDX1    ,
      0x0000   ,
      0x0000   ,
      0x0000   ,
      0x0000   ,
      0x0000   ,
      0x0000   ,
      0x0000   ,  
      0x0000   ,
      0x0000   ,
      0x0000   ,
      0x0000   ,  
      0x0000   ,
      0x0000   ,
      0x0000   ,
      0x0000   
    },        
    
    //#define MaskBoxRSPOWER                  (0x09)  
    { 0x0000       ,
      CIDX4        ,
      0x0000       ,
      0x0000       ,
      0x0000       ,
      0x0000       ,
      0x0000       ,
      0x0000       ,  
      0x0000       ,
      0x0000       ,
      0x0000       ,
      0x0000       ,  
      0x0000       ,
      0x0000       ,
      0x0000       ,
      CIDXB | CIDXD
    }, 
    
    //#define       (0x0A)
    
    { 0x0000       ,
      0x0000       ,
      0x0000       ,
      0x0000       ,
      0x0000       ,
      0x0000       ,
      0x0000       ,
      0x0000       ,  
      0x0000       ,
      0x0000       ,
      0x0000       ,
      0x0000       ,  
      0x0000       ,
      0x0000       ,
      0x0000       ,
      0x0000
    }, 
    
    //#define           (0x0B)
    { 0x0000      ,
      0x0000      ,
      0x0000      ,
      0x0000      ,
      0x0000      ,
      0x0000      ,
      0x0000      ,
      0x0000      ,  
      0x0000      ,
      0x0000      ,
      0x0000      ,
      0x0000      ,  
      0x0000      ,
      0x0000      ,
      0x0000      ,
      0x0000
    },       
    
    //#define   (0x0C)
    
    { 0x0000      ,
      0x0000      ,
      0x0000      ,
      0x0000      ,
      0x0000      ,
      0x0000      ,
      0x0000      ,
      0x0000      ,  
      0x0000      ,
      0x0000      ,
      0x0000      ,
      0x0000      ,  
      0x0000      ,
      0x0000      ,
      0x0000      ,
      0x0000
    },       
    //#define   (0x0D)      
    { 0x0000      ,
      0x0000      ,
      0x0000      ,
      0x0000      ,
      0x0000      ,
      0x0000      ,
      0x0000      ,
      0x0000      ,  
      0x0000      ,
      0x0000      ,
      0x0000      ,
      0x0000      ,  
      0x0000      ,
      0x0000      ,
      0x0000      ,
      0x0000
    }},    
        
     MaskBoxIDCore,//uint8_t    nmask_transl_core;              // Hомер текущей маски ID_RS пакетов ретрансляции
     BitID(CodecPortID) | BitID(VoiceETHPortID) | BitID(RSAPortID) |  BitID(RSAPortID) | BitID(ETHtubeRSPortID) ,//uint32_t   mask_inpup_port_id_core;        // Маска ID портов разрешенных для приема в данный порт   
          
     0,//uint8_t  group_ring;    /* Номер группы кольца (0 - без группы)  */      
     0,//uint8_t  priority_ring; /* Приоритет в группе кольца             */    
     1,//uint8_t    number_position_vrm;                  // Число лучших врм при позиционировании тегов 
//==============================================================================
//==============================================================================
//      Настройки для ETH
//============================================================================== 

     7757,       //uint16_t 	ip_port_voice; 			// Порт приема передачи голосовых сообщений
     
     192,	//uint8_t 	ip_table_main_srv_ad0; 		// Адрес IPv4 сервера раздающего  таблицу рассылки
     168,	//uint8_t 	ip_table_main_srv_ad1; 		// Адрес IPv4 сервера раздающего  таблицу рассылки
     1,        //uint8_t 	ip_table_main_srv_ad2; 		// Адрес IPv4 сервера раздающего  таблицу рассылки
     57,        //uint8_t 	ip_table_main_srv_ad3; 		// Адрес IPv4 сервера раздающего  таблицу рассылки
     7757,	//uint16_t 	ip_table_main_srv_port; 	// Порт сервера раздающего  таблицу рассылки
     
     192,	//uint8_t 	ip_table_rezv_srv_ad0; 		// Адрес IPv4 сервера раздающего  таблицу рассылки
     168,	//uint8_t 	ip_table_rezv_srv_ad1; 		// Адрес IPv4 сервера раздающего  таблицу рассылки
     1,         //uint8_t 	ip_table_rezv_srv_ad2; 		// Адрес IPv4 сервера раздающего  таблицу рассылки
     59,        //uint8_t 	ip_table_rezv_srv_ad3; 		// Адрес IPv4 сервера раздающего  таблицу рассылки
     7757,	//uint16_t 	ip_table_rezv_srv_port; 	// Порт сервера раздающего  таблицу рассылки
 
     0xFFFF,    //uint16_t      phy_addr_disp_1;                /* Физический адрес диспетчера 1      */ 
     0xFFFF,    //uint16_t      phy_addr_disp_2;                /* Физический адрес диспетчера 2      */ 
     
     MaskBoxIDVoiceETH,         //uint8_t    nmask_transl_voice;             // Hомер текущей маски ID_RS пакетов трансляции для голосового порта рассылки
     MaskBoxIDtubeETH,          //uint8_t    nmask_transl_config;            // Hомер текущей маски ID_RS пакетов трансляции конфигурирования и загрузки
     
     BitID(CodecPortID) | BitID(RSAPortID) |  BitID(RSBPortID) | BitID(VoiceInfoID), //uint32_t   mask_inpup_port_id_voice;       // Маска ID портов разрешенных для приема в данный порт трансляции для голосового порта рассылки
     BitID(CorePortID)  ,  //uint32_t   mask_inpup_port_id_config;      // Маска ID портов разрешенных для приема в данный порт трансляции конфигурирования и загрузки     
     
     ////==============================================================================
     ////      Настройки для RS A
     ////==============================================================================   
     2,                  //uint8_t  Type_RS_а; 		       // Значениe адреса устр-ва по UART
     230400,             //uint32_t rs_bit_rate_a; 	       // Скорость интерфейса RS
     MaskBoxIDRSAPort,   //uint8_t  nmask_transl_rs_a;         // Номер текущей маски ID_RS пакетов трансляции
     BitID(CodecPortID) | BitID(CorePortID) | BitID(VoiceInfoID) | BitID(ETHtubeRSPortID), //uint32_t mask_inpup_port_id_rs_a;   // Mаска ID портов разрешенных для приема в данный порт
     ////==============================================================================     
     // 
     ////==============================================================================
     ////      Настройки для RS B
     ////==============================================================================   
     2,                  //uint8_t  Type_RS_b; 		       // Значениe адреса устр-ва по UART
     230400,             //uint32_t rs_bit_rate_b; 	       // Скорость интерфейса RS
     MaskBoxIDRSBPort,   //uint8_t  nmask_transl_rs_b;         // Номер текущей маски ID_RS пакетов трансляции
     BitID(CodecPortID) | BitID(CorePortID) | BitID(VoiceInfoID) | BitID(ETHtubeRSPortID), //uint32_t mask_inpup_port_id_rs_b;   // Mаска ID портов разрешенных для приема в данный порт
     ////========================================================================== 
     /*============================================================================*/
     /*     Настройки для CODEC                                                    */
     /*============================================================================*/
     ///* Настройки кодека канала А        */
     1,//uint8_t  codec_a_chanel;                  /* Номер канала закреленный за кодеком */     
     5,//uint8_t  codec_a_priority_ch_pcm;         /* Приоритет голосового канала pcm по умолчанию (1..30) */ 
     5,//uint8_t  codec_a_priority_ch_cmx;         /* Приоритет голосового канала cmx по умолчанию   (1..30) */ 
     Bit_SoftPID( AUDIO_CODEC_SoftPID ) | Bit_SoftPID( UDPGate_SoftPID ) | Bit_SoftPID( LOOP_SoftPID ) | Bit_SoftPID( CODEC_B_SoftPID ) | Bit_SoftPID( CODEC_C_SoftPID ),//uint32_t codec_a_mask_source_soft_port;   /* Маска портов разрешенных для приема в данный порт из програмного роутера */ 
     BitChID(1),//uint32_t codec_a_mask_chanel_soft_port;   /* Маска каналов разрешенных для приема в данный порт из програмного роутера */  
     ///* Настройки кодека канала А        */                                      
     2,//uint8_t  codec_b_chanel;                  /* Номер канала закреленный за кодеком */     
     5,//uint8_t  codec_b_priority_ch_pcm;         /* Приоритет голосового канала pcm по умолчанию (1..30) */ 
     5,//uint8_t  codec_b_priority_ch_cmx;         /* Приоритет голосового канала cmx по умолчанию   (1..30) */ 
     Bit_SoftPID( AUDIO_CODEC_SoftPID ) | Bit_SoftPID( UDPGate_SoftPID ) | Bit_SoftPID( LOOP_SoftPID ) | Bit_SoftPID( CODEC_A_SoftPID ) | Bit_SoftPID( CODEC_C_SoftPID ),//uint32_t codec_b_mask_source_soft_port;   /* Маска портов разрешенных для приема в данный порт из програмного роутера */ 
     BitChID(2),//uint32_t codec_b_mask_chanel_soft_port;   /* Маска каналов разрешенных для приема в данный порт из програмного роутера */  
     ///* Настройки кодека канала А        */                                      
     3,//uint8_t  codec_c_chanel;                  /* Номер канала закреленный за кодеком */     
     5,//uint8_t  codec_c_priority_ch_pcm;         /* Приоритет голосового канала pcm по умолчанию (1..30) */ 
     5,//uint8_t  codec_c_priority_ch_cmx;         /* Приоритет голосового канала cmx по умолчанию   (1..30) */ 
     Bit_SoftPID( AUDIO_CODEC_SoftPID ) | Bit_SoftPID( UDPGate_SoftPID ) | Bit_SoftPID( LOOP_SoftPID ) | Bit_SoftPID( CODEC_A_SoftPID ) | Bit_SoftPID( CODEC_B_SoftPID ),//uint32_t codec_c_mask_source_soft_port;   /* Маска портов разрешенных для приема в данный порт из програмного роутера */ 
     BitChID(3),//uint32_t codec_c_mask_chanel_soft_port;   /* Маска каналов разрешенных для приема в данный порт из програмного роутера */  
     ///* Настройки общие для всех кодеков */                                      
     MaskBoxIDCodec,//uint8_t   codec_nmask_transl_codec;        /* Hомер текущей маски ID_RS пакетов трансляции в порт кодека  */
     BitID(CorePortID) | BitID(VoiceETHPortID) | BitID(RSAPortID) |  BitID(RSBPortID) | BitID(VoiceInfoID), //uint32_t  codec_mask_inpup_port_id_codec;  /* Маска ID портов разрешенных для приема в порт кодека        */   
     /* Настройки аналогового кодека     */ 
     1,// uint8_t   analog_codec_mode;                  /* Режим работы аналогового кодека     */
     5,//uint8_t   analog_codec_chanel;                /* Номер канала закреленный за кодеком */     
     5,//uint8_t   analog_codec_priority_discrt;       /* Приоритет голосового дискретного канала по умолчанию   (1..5) */ 
     5,//uint8_t   analog_codec_priority_analog;       /* Приоритет голосового аналогового канала по умолчанию   (1..5) */ 
     Bit_SoftPID( UDPGate_SoftPID ) | Bit_SoftPID( LOOP_SoftPID ) | Bit_SoftPID( CODEC_A_SoftPID ) | Bit_SoftPID( CODEC_B_SoftPID ) | Bit_SoftPID( CODEC_C_SoftPID ),//uint32_t  analog_codec_mask_source_soft_port; /* Маска портов разрешенных для приема в данный порт из програмного роутера */ 
     BitChID(5), //uint32_t  analog_codec_mask_chanel_soft_port; /* Маска каналов разрешенных для приема в данный порт из програмного роутера */  
     /* Настройки порта ETH несжатых потов    */
     Bit_SoftPID( LOOP_SoftPID ) | Bit_SoftPID( AUDIO_CODEC_SoftPID ) | Bit_SoftPID( LOOP_SoftPID ) | Bit_SoftPID( CODEC_A_SoftPID ) | Bit_SoftPID( CODEC_B_SoftPID ) | Bit_SoftPID( CODEC_C_SoftPID ),//uint32_t  udp_mask_source_soft_port; /* Маска портов разрешенных для приема в UDP порт из програмного роутера */ 
     BitChID(1) | BitChID(2) | BitChID(3) |BitChID(4) | BitChID(5) | BitChID(6) ////uint32_t  udp_mask_chanel_soft_port; /* Маска каналов разрешенных для приема пакетов в UDP порт из програмного роутера */        
};	

// структура для редактирования переменных которые будем хранить в EEPROM FLASH
// при запуске программы заполняется хранимыми в EEPROM значениями 	
FlashSettingsTypeDef DataNew;	

// структура переменных которые прочитали из EEPROM FLASH при запуске программы
FlashSettingsTypeDef DataSto;	
	
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Функция записи структуры параметров 
  * @param  FlashSettingsTypeDef *DataSettings указатель на записываемые данные
  * @retval None
  */
void Write_Settings(FlashSettingsTypeDef *DataSettings)
{
  //увеличиваем счетчик записи во флеш
  DataSettings->Cnt_WRFlash++; 
  //устанавливаем идентификатор структуры
  DataSettings->ID_Settings = ID_SETTINGS;
  //обновляем контрольную сумму
  Set_CRC_Settings(DataSettings);
  //Функция записи массива в флеш память
  Write_Setup_Flash((uint32_t *)DataSettings,sizeof(FlashSettingsTypeDef),FLASH_Setup_START_ADDR);	
}

/**
  * @brief  Функция чтения структуры параметров 
  * @param  FlashSettingsTypeDef *DataSettings указатель на массив для прочитанных данныx
  * @retval None
  */
void Read_Settings(FlashSettingsTypeDef *DataSettings)
{
  //чтения параметров из FLASH
  Read_Setup_Flash((uint32_t *)DataSettings,sizeof(FlashSettingsTypeDef),FLASH_Setup_START_ADDR);
}

/**
  * @brief  Функция проверки контрольной суммы структуры параметров 
  * @param  FlashSettingsTypeDef *DataSettings указатель на структуру параметров
  * @retval uint8_t 0 - контрольная сумма совпала
  *                 1 - контрольная сумма не совпала
  */
uint8_t Test_CRC_Settings(FlashSettingsTypeDef *DataSettings)
{
  uint32_t CRC_tempa_Read; 	
  
  // Cброс вычислителя CRC 	
  CRC_ResetDR();  
  
  //вычисление CRC структуры параметров
  CRC_tempa_Read  = CRC_CalcBlockCRC((uint32_t *)(&(DataSettings->Cnt_WRFlash)), ((sizeof(DataSettings->Settings) + sizeof(DataSettings->Cnt_WRFlash))>>2) + 1);
  
  //проверка CRC структуры параметров 	
  if ( (CRC_tempa_Read == DataSettings->CRC_Array)&&(CRC_tempa_Read != 0xFFFF)&&( DataSettings->ID_Settings == ID_SETTINGS ) )	 
  {
    //контрольная сумма совпала
    return 0;
  }
  else 
  {
    //контрольная сумма не совпала
    return 1;
  } 		 
}

/**
  * @brief  Функция обновления контрольной суммы структуры параметров 
  * @param  FlashSettingsTypeDef *DataSettings указатель на структуру параметров
  * @retval None
  */
void Set_CRC_Settings(FlashSettingsTypeDef *DataSettings)
{
  // Cброс вычислителя CRC 	
  CRC_ResetDR();
  // Вычисление и сохранение CRC структуры параметров
  DataSettings->CRC_Array = CRC_CalcBlockCRC((uint32_t *)(&(DataSettings->Cnt_WRFlash)), ((sizeof(DataSettings->Settings) + sizeof(DataSettings->Cnt_WRFlash))>>2) + 1);
}

/**
  * @brief  Функция обновления структуры параметров из флеш памяти 
  * @param  None
  * @retval uint8_t 0 - контрольная сумма совпала
  *                 1 - контрольная сумма не совпала
  *                 2 - контрольная сумма не совпала установлены параметры по умолчанию
  */
uint8_t Load_Settings(void)
{
  //чтения параметров из FLASH
  Read_Settings(&DataSto);
  //проверяем контольную сумму
  if (Test_CRC_Settings(&DataSto))
  {
    //несовпадение контрольной суммы - записываем в структуру параметры по умолчанию
    Copy_Settings((FlashSettingsTypeDef*)&DataDef,&DataSto);
    //запись параметров в FLASH
    Write_Settings(&DataSto);	
    //чтения параметров из FLASH
    Read_Settings(&DataSto);
    //проверяем контольную сумму
    if (Test_CRC_Settings(&DataSto)) return 1;
    //заносим данные в массив редактирования
    Copy_Settings(&DataSto,&DataNew);	
    return 2;
  }
  //заносим данные в массив редактирования
  Copy_Settings(&DataSto,&DataNew);			
  return 0;	
}

/**
  * @brief  Функция проверка коррекности структуры параметров в оперативной памяти
  * @param  None
  * @retval None
  */
void Control_Settings(void)
{
  //проверяем контольную сумму
  if (Test_CRC_Settings(&DataSto))  
  {
    //при ошибке загружаем структуру повторно 
    Load_Settings();
  }	
}

/**
  * @brief  Функция копирования структур параметров
  * @param  FlashSettingsTypeDef *DataSource - структура источник
  * @param  FlashSettingsTypeDef *DataDestination - структура назначение
  * @retval None
  */
void Copy_Settings(FlashSettingsTypeDef *DataSource,FlashSettingsTypeDef *DataDestination)
{
  DataDestination->Settings = DataSource->Settings;
}

/**
  * @brief  Функция записи отредактированных параметров в Flash память 
  * @param  None
  * @retval None
  */
void Write_Edit_Settings(void)
{
  Write_Settings(&DataNew);	
}
/************************ (C) COPYRIGHT DEX *****END OF FILE****/

