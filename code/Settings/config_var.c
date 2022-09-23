 /**
  ******************************************************************************
  * @file    сonfig_var.c
  * @author  Trembach D.N.
  * @version V1.2.0
  * @date    16-10-2014
  * @brief   Файл содержащий описание конфигурируемых переменных
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "config_var.h"
#include <stdlib.h>
#include "core_cntrl_settings.h"
#include "EEPROM_Flash.h" 
#include "settings.h"
#include "loader_settings.h"

// Определяем указатель на данные в FLASH 
#define  DataFLH   (*(FlashSettingsTypeDef*)((__IO uint32_t*) (FLASH_Setup_START_ADDR)))


//обьявим массив для секций редактирования переменных
Section_typedef_t SectionArray[] =
{	
  //секция для переменных недоступных для конфигурирования
  {
    NO_SECT,         														
    "OUT SECTIONS",										
    "OUT SECTIONS"											
  },
  //секция для переменных загрузчика 		
  {
    LOADER_SECT,         													
    "Параметры Загрузчика",										
    "Просмотр параметров Загрузчика"											
  },
  //секция для переменных конфигурирования 		
  {
    PARAM_SECT,         													
    "Настройки модуля",										
    "Настройки параметров модуля"											
  },
  //секция для переменных Ethernet 		
  {
    ETH_SECT,         													
    "Настройки Ethernet",										
    "Настройки параметров Ethernet"											
  },
  //секция для переменных Ethernet 		
  {
    CODEC_SECT,         													
    "Настройки кодеков",										
    "Настройки параметров кодеков"											
  },
  //секция для переменных Ethernet несжатых голосовых потоков  
  {
    ANALOG_SECT,         													
    "Настройки аналогового кодека",										
    "Настройки параметров аналогового кодека"											
  },
  //секция для переменных Ethernet несжатых голосовых потоков    
  
  {
    ETH_STREAM_SECT,         													
    "Настройки ETH Stream",										
    "Настройки параметров несжатых голосовых потоков ETH"											
  },
  //секция стоп	- пустая секция
  {
    STOP_SECT,         														
    "",										
    ""											
  }		
};

//обьявим массив для редактирования переменных
VAR_typedef_t SetVarArray[] =
{ 	
//==============================================================================
//      Настройки для Ядра
//==============================================================================   
  {//  0xFE01,		//uint16_t                              log_adr;                                // логический адрес блока
    PARAM_SECT,   					//номер секции
    VarID(Core_Type,Core_N_port,GET_CORE_LOG_ADDR,0),     //идентификатор переменной   
    ETH_R|ETH_W, 	                                //тип доступа к переменной
    UINT16,						//тип переменной;
    CODEHEX,        				//тип вывода на экран;
    (void*)&DataSto.Settings.mac_adr,  	        //сохраненное значение переменной;
    (void*)&DataNew.Settings.mac_adr,  	        //новое значение переменных;
    (void*)&DataDef.Settings.mac_adr,  	        //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.mac_adr,          //значение переменной во flash; 
    "Логический адрес блока"                        //описание переменных
  }, 
  {//  30000,    //uint16_t   time_update_diagn;              // Время (период) обновления диагностики
    PARAM_SECT,   					//номер секции
    VarID(Core_Type,Core_N_port,GET_CORE_TIME,0),     //идентификатор переменной   
    ETH_R|ETH_W, 	                                //тип доступа к переменной
    UINT16,						//тип переменной;
    CODEDEC,        				//тип вывода на экран;
    (void*)&DataSto.Settings.time_update_diagn,  	        //сохраненное значение переменной;
    (void*)&DataNew.Settings.time_update_diagn,  	        //новое значение переменных;
    (void*)&DataDef.Settings.time_update_diagn,  	        //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.time_update_diagn,          //значение переменной во flash; 
    "Время (период) обновления диагностики"                        //описание переменных
  },   
  //======================== Маски трансляции пакетоа по ID RS ====================================== 
  {// маска номер 0 ID_RS пакетов трансляции
    NO_SECT,   					                //номер секции
    VarID(Core_Type,Core_N_port,GET_CORE_MASK_N00_TRANSL,0),    //идентификатор переменной   
    ETH_R|ETH_W, 	                                        //тип доступа к переменной
    MASK_TYPE,						        //тип переменной;
    CODEDEC,        				                //тип вывода на экран;
    (void*)&DataSto.Settings.tabl_mask_box_id[0],  	        //сохраненное значение переменной;
    (void*)&DataNew.Settings.tabl_mask_box_id[0],  	        //новое значение переменных;
    (void*)&DataDef.Settings.tabl_mask_box_id[0],  	        //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.tabl_mask_box_id[0],               //значение переменной во flash; 
    "Маска 00 ID_RS пакетов трансляции"                         //описание переменных
  },    
  
  {// маска номер 1 ID_RS пакетов трансляции
    NO_SECT,   					                //номер секции
    VarID(Core_Type,Core_N_port,GET_CORE_MASK_N01_TRANSL,0),    //идентификатор переменной   
    ETH_R|ETH_W, 	                                        //тип доступа к переменной
    MASK_TYPE,						        //тип переменной;
    CODEDEC,        				                //тип вывода на экран;
    (void*)&DataSto.Settings.tabl_mask_box_id[1],  	        //сохраненное значение переменной;
    (void*)&DataNew.Settings.tabl_mask_box_id[1],  	        //новое значение переменных;
    (void*)&DataDef.Settings.tabl_mask_box_id[1],  	        //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.tabl_mask_box_id[1],            //значение переменной во flash; 
    "Маска 01 ID_RS пакетов трансляции"                         //описание переменных
  },   
  
  {// маска номер 2 ID_RS пакетов трансляции
    NO_SECT,   					                //номер секции
    VarID(Core_Type,Core_N_port,GET_CORE_MASK_N02_TRANSL,0),    //идентификатор переменной   
    ETH_R|ETH_W, 	                                        //тип доступа к переменной
    MASK_TYPE,						        //тип переменной;
    CODEDEC,        				                //тип вывода на экран;
    (void*)&DataSto.Settings.tabl_mask_box_id[2],  	        //сохраненное значение переменной;
    (void*)&DataNew.Settings.tabl_mask_box_id[2],  	        //новое значение переменных;
    (void*)&DataDef.Settings.tabl_mask_box_id[2],  	        //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.tabl_mask_box_id[2],            //значение переменной во flash; 
    "Маска 02 ID_RS пакетов трансляции"                         //описание переменных
  },   
  
  {// маска номер 3 ID_RS пакетов трансляции
    NO_SECT,   					                //номер секции
    VarID(Core_Type,Core_N_port,GET_CORE_MASK_N03_TRANSL,0),    //идентификатор переменной   
    ETH_R|ETH_W, 	                                        //тип доступа к переменной
    MASK_TYPE,						        //тип переменной;
    CODEDEC,        				                //тип вывода на экран;
    (void*)&DataSto.Settings.tabl_mask_box_id[3],  	        //сохраненное значение переменной;
    (void*)&DataNew.Settings.tabl_mask_box_id[3],  	        //новое значение переменных;
    (void*)&DataDef.Settings.tabl_mask_box_id[3],  	        //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.tabl_mask_box_id[3],            //значение переменной во flash; 
    "Маска 03 ID_RS пакетов трансляции"                         //описание переменных
  },
  
  {// маска номер 4 ID_RS пакетов трансляции
    NO_SECT,   					                //номер секции
    VarID(Core_Type,Core_N_port,GET_CORE_MASK_N04_TRANSL,0),    //идентификатор переменной   
    ETH_R|ETH_W, 	                                        //тип доступа к переменной
    MASK_TYPE,						        //тип переменной;
    CODEDEC,        				                //тип вывода на экран;
    (void*)&DataSto.Settings.tabl_mask_box_id[4],  	        //сохраненное значение переменной;
    (void*)&DataNew.Settings.tabl_mask_box_id[4],  	        //новое значение переменных;
    (void*)&DataDef.Settings.tabl_mask_box_id[4],  	        //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.tabl_mask_box_id[4],            //значение переменной во flash; 
    "Маска 04 ID_RS пакетов трансляции"                         //описание переменных
  },
  
  {// маска номер 5 ID_RS пакетов трансляции
    NO_SECT,   					                //номер секции
    VarID(Core_Type,Core_N_port,GET_CORE_MASK_N05_TRANSL,0),    //идентификатор переменной   
    ETH_R|ETH_W, 	                                        //тип доступа к переменной
    MASK_TYPE,						        //тип переменной;
    CODEDEC,        				                //тип вывода на экран;
    (void*)&DataSto.Settings.tabl_mask_box_id[5],  	        //сохраненное значение переменной;
    (void*)&DataNew.Settings.tabl_mask_box_id[5],  	        //новое значение переменных;
    (void*)&DataDef.Settings.tabl_mask_box_id[5],  	        //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.tabl_mask_box_id[5],            //значение переменной во flash; 
    "Маска 05 ID_RS пакетов трансляции"                         //описание переменных
  },    
  
  {// маска номер 6 ID_RS пакетов трансляции
    NO_SECT,   					                //номер секции
    VarID(Core_Type,Core_N_port,GET_CORE_MASK_N06_TRANSL,0),    //идентификатор переменной   
    ETH_R|ETH_W, 	                                        //тип доступа к переменной
    MASK_TYPE,						        //тип переменной;
    CODEDEC,        				                //тип вывода на экран;
    (void*)&DataSto.Settings.tabl_mask_box_id[6],  	        //сохраненное значение переменной;
    (void*)&DataNew.Settings.tabl_mask_box_id[6],  	        //новое значение переменных;
    (void*)&DataDef.Settings.tabl_mask_box_id[6],  	        //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.tabl_mask_box_id[6],            //значение переменной во flash; 
    "Маска 06 ID_RS пакетов трансляции"                         //описание переменных
  },   
  
  {// маска номер 7 ID_RS пакетов трансляции
    NO_SECT,   					                //номер секции
    VarID(Core_Type,Core_N_port,GET_CORE_MASK_N07_TRANSL,0),    //идентификатор переменной   
    ETH_R|ETH_W, 	                                        //тип доступа к переменной
    MASK_TYPE,						        //тип переменной;
    CODEDEC,        				                //тип вывода на экран;
    (void*)&DataSto.Settings.tabl_mask_box_id[7],  	        //сохраненное значение переменной;
    (void*)&DataNew.Settings.tabl_mask_box_id[7],  	        //новое значение переменных;
    (void*)&DataDef.Settings.tabl_mask_box_id[7],  	        //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.tabl_mask_box_id[7],            //значение переменной во flash; 
    "Маска 07 ID_RS пакетов трансляции"                         //описание переменных
  },   
  
  {// маска номер 8 ID_RS пакетов трансляции
    NO_SECT,   					                //номер секции
    VarID(Core_Type,Core_N_port,GET_CORE_MASK_N08_TRANSL,0),    //идентификатор переменной   
    ETH_R|ETH_W, 	                                        //тип доступа к переменной
    MASK_TYPE,						        //тип переменной;
    CODEDEC,        				                //тип вывода на экран;
    (void*)&DataSto.Settings.tabl_mask_box_id[8],  	        //сохраненное значение переменной;
    (void*)&DataNew.Settings.tabl_mask_box_id[8],  	        //новое значение переменных;
    (void*)&DataDef.Settings.tabl_mask_box_id[8],  	        //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.tabl_mask_box_id[8],            //значение переменной во flash; 
    "Маска 08 ID_RS пакетов трансляции"                         //описание переменных
  },
  
  {// маска номер 9 ID_RS пакетов трансляции
    NO_SECT,   					                //номер секции
    VarID(Core_Type,Core_N_port,GET_CORE_MASK_N09_TRANSL,0),    //идентификатор переменной   
    ETH_R|ETH_W, 	                                        //тип доступа к переменной
    MASK_TYPE,						        //тип переменной;
    CODEDEC,        				                //тип вывода на экран;
    (void*)&DataSto.Settings.tabl_mask_box_id[9],  	        //сохраненное значение переменной;
    (void*)&DataNew.Settings.tabl_mask_box_id[9],  	        //новое значение переменных;
    (void*)&DataDef.Settings.tabl_mask_box_id[9],  	        //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.tabl_mask_box_id[9],            //значение переменной во flash; 
    "Маска 09 ID_RS пакетов трансляции"                         //описание переменных
  },
  {// маска номер 10 ID_RS пакетов трансляции
    NO_SECT,   					                //номер секции
    VarID(Core_Type,Core_N_port,GET_CORE_MASK_N10_TRANSL,0),    //идентификатор переменной   
    ETH_R|ETH_W, 	                                        //тип доступа к переменной
    MASK_TYPE,						        //тип переменной;
    CODEDEC,        				                //тип вывода на экран;
    (void*)&DataSto.Settings.tabl_mask_box_id[10],  	        //сохраненное значение переменной;
    (void*)&DataNew.Settings.tabl_mask_box_id[10],  	        //новое значение переменных;
    (void*)&DataDef.Settings.tabl_mask_box_id[10],  	        //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.tabl_mask_box_id[10],               //значение переменной во flash; 
    "Маска 00 ID_RS пакетов трансляции"                         //описание переменных
  },    
  
  {// маска номер 11 ID_RS пакетов трансляции
    NO_SECT,   					                //номер секции
    VarID(Core_Type,Core_N_port,GET_CORE_MASK_N11_TRANSL,0),    //идентификатор переменной   
    ETH_R|ETH_W, 	                                        //тип доступа к переменной
    MASK_TYPE,						        //тип переменной;
    CODEDEC,        				                //тип вывода на экран;
    (void*)&DataSto.Settings.tabl_mask_box_id[11],  	        //сохраненное значение переменной;
    (void*)&DataNew.Settings.tabl_mask_box_id[11],  	        //новое значение переменных;
    (void*)&DataDef.Settings.tabl_mask_box_id[11],  	        //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.tabl_mask_box_id[11],            //значение переменной во flash; 
    "Маска 01 ID_RS пакетов трансляции"                         //описание переменных
  },   
  
  {// маска номер 12 ID_RS пакетов трансляции
    NO_SECT,   					                //номер секции
    VarID(Core_Type,Core_N_port,GET_CORE_MASK_N12_TRANSL,0),    //идентификатор переменной   
    ETH_R|ETH_W, 	                                        //тип доступа к переменной
    MASK_TYPE,						        //тип переменной;
    CODEDEC,        				                //тип вывода на экран;
    (void*)&DataSto.Settings.tabl_mask_box_id[12],  	        //сохраненное значение переменной;
    (void*)&DataNew.Settings.tabl_mask_box_id[12],  	        //новое значение переменных;
    (void*)&DataDef.Settings.tabl_mask_box_id[12],  	        //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.tabl_mask_box_id[12],            //значение переменной во flash; 
    "Маска 02 ID_RS пакетов трансляции"                         //описание переменных
  },   
  
  {// маска номер 13 ID_RS пакетов трансляции
    NO_SECT,   					                //номер секции
    VarID(Core_Type,Core_N_port,GET_CORE_MASK_N03_TRANSL,0),    //идентификатор переменной   
    ETH_R|ETH_W, 	                                        //тип доступа к переменной
    MASK_TYPE,						        //тип переменной;
    CODEDEC,        				                //тип вывода на экран;
    (void*)&DataSto.Settings.tabl_mask_box_id[13],  	        //сохраненное значение переменной;
    (void*)&DataNew.Settings.tabl_mask_box_id[13],  	        //новое значение переменных;
    (void*)&DataDef.Settings.tabl_mask_box_id[13],  	        //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.tabl_mask_box_id[13],              //значение переменной во flash; 
    "Маска 03 ID_RS пакетов трансляции"                         //описание переменных
  },
  //========================================================================================================  
 
  {   //      MaskBoxIDCore,//uint8_t    nmask_transl_core;              // Hомер текущей маски ID_RS пакетов ретрансляции
    PARAM_SECT,																				//номер секции
    VarID(Core_Type,Core_N_port,GET_CORE_NMASK_TRANSL,0),           //идентификатор переменной
    ETH_R|ETH_W,                               		//тип доступа к переменной
    UINT8,							//тип переменной;
    CODEDEC,        					//тип вывода на экран;	
    (void*)&DataSto.Settings.nmask_transl_core,	    	//сохраненное значение переменной;
    (void*)&DataNew.Settings.nmask_transl_core,	    	//новое значение переменных;
    (void*)&DataDef.Settings.nmask_transl_core,	    	//значение переменной по умолчанию;
    (void*)&DataFLH.Settings.nmask_transl_core,          //значение переменной во flash; 			
    "Hомер маски ID_RS пакетов ретрансляции"		  	//описание переменных	
  },                
  
  {//      BitID(RSPortID) | BitID(PosDiagETHPortID) | BitID(ETHtubeRSPortID) | BitID(RFPortID) | BitID(CodecPortID),//uint32_t   mask_inpup_port_id_core;        // Маска ID портов разрешенных для приема в данный порт  
    PARAM_SECT,																				//номер секции
    VarID(Core_Type,Core_N_port,GET_CORE_MASK_INPUT_PORT_ID,0),           //идентификатор переменной
    ETH_R|ETH_W,                               		//тип доступа к переменной
    UINT32,							//тип переменной;
    CODEHEX,        					//тип вывода на экран;	
    (void*)&DataSto.Settings.mask_inpup_port_id_core,	    	//сохраненное значение переменной;
    (void*)&DataNew.Settings.mask_inpup_port_id_core,	    	//новое значение переменных;
    (void*)&DataDef.Settings.mask_inpup_port_id_core,	    	//значение переменной по умолчанию;
    (void*)&DataFLH.Settings.mask_inpup_port_id_core,          //значение переменной во flash; 			
    "Маска ID портов разрешенных для приема в данный порт "		  	//описание переменных	
  },

  { //0,//uint8_t  group_ring;    /* Номер группы кольца (0 - без группы)  */       
    PARAM_SECT,                                                   //номер секции
    VarID(Core_Type,Core_N_port,GET_CORE_NGROUP_RING_ID,0),         //идентификатор переменной
    ETH_R|ETH_W,                                                  //тип доступа к переменной
    UINT8,                                                        //тип переменной;
    CODEDEC,                                                      //тип вывода на экран;	
    (void*)&DataSto.Settings.group_ring,                          //сохраненное значение переменной;
    (void*)&DataNew.Settings.group_ring,                          //новое значение переменных;
    (void*)&DataDef.Settings.group_ring,                          //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.group_ring,                          //значение переменной во flash; 			
    "Номер группы кольца (0 - без группы)"                        //описание переменных	
  },                
  
  {  //0,//uint8_t  priority_ring; /* Приоритет в группе кольца             */  
    PARAM_SECT,                                                   //номер секции
    VarID(Core_Type,Core_N_port,GET_CORE_PRIORITY_RING_ID,0),     //идентификатор переменной
    ETH_R|ETH_W,                                                  //тип доступа к переменной
    UINT8,                                                        //тип переменной;
    CODEDEC,                                                      //тип вывода на экран;	
    (void*)&DataSto.Settings.priority_ring,                       //сохраненное значение переменной;
    (void*)&DataNew.Settings.priority_ring,                       //новое значение переменных;
    (void*)&DataDef.Settings.priority_ring,                       //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.priority_ring,                       //значение переменной во flash; 			
    "Приоритет в группе кольца"                                   //описание переменных	
  },  
    
  //==============================================================================
  //      Настройки для LOADER_SECT
  //============================================================================== 
  {//  uint8_t       ip_ad0;                 // Адрес IPv4 
    LOADER_SECT,   				         /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_ADDR,0),        /* идентификатор переменной             */
    ETH_R, 	                                 /* тип доступа к переменной             */
    IP_TYPE,					         /* тип переменной;                      */
    CODEDEC,        				         /* тип вывода на экран;	         */
    (void*)&DataLoaderSto.Settings.ip_ad0,               /* сохраненное значение переменной;     */
    (void*)&DataLoaderSto.Settings.ip_ad0,               /* новое значение переменных;           */
    (void*)&DataLoaderSto.Settings.ip_ad0,               /* значение переменной по умолчанию;    */
    (void*)&DataLoaderSto.Settings.ip_ad0,               /* значение переменной во flash;        */
    "IP-адрес"				                 /* описание переменных                  */
  },
  
  { //  uint8_t       ip_ad1;                 // Адрес IPv4 
    NO_SECT,   					         /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_ADDR,1),        /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                 /* тип доступа к переменной             */
    IP_TYPE,					         /* тип переменной;                      */
    CODEDEC,        				         /* тип вывода на экран;	         */
    (void*)&DataLoaderSto.Settings.ip_ad1,               /* сохраненное значение переменной;     */
    (void*)&DataLoaderSto.Settings.ip_ad1,               /* новое значение переменных;           */
    (void*)&DataLoaderSto.Settings.ip_ad1,               /* значение переменной по умолчанию;    */
    (void*)&DataLoaderSto.Settings.ip_ad1,               /* значение переменной во flash;        */
    ""						         /* описание переменных                  */
  },
  
  {//  uint8_t       ip_ad2;                 // Адрес IPv4 
    NO_SECT,   					         /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_ADDR,2),        /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                 /* тип доступа к переменной             */
    IP_TYPE,					         /* тип переменной;                      */
    CODEDEC,        				         /* тип вывода на экран;	         */
    (void*)&DataLoaderSto.Settings.ip_ad2,               /* сохраненное значение переменной;     */
    (void*)&DataLoaderSto.Settings.ip_ad2,               /* новое значение переменных;           */
    (void*)&DataLoaderSto.Settings.ip_ad2,               /* значение переменной по умолчанию;    */
    (void*)&DataLoaderSto.Settings.ip_ad2,               /* значение переменной во flash;        */
    ""						         /* описание переменных                  */
  },
  
  {//  uint8_t       ip_ad3;                 // Адрес IPv4
    NO_SECT,   					         /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_ADDR,3),        /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                 /* тип доступа к переменной             */
    IP_TYPE,					         /* тип переменной;                      */
    CODEDEC,        				         /* тип вывода на экран;	         */
    (void*)&DataLoaderSto.Settings.ip_ad3,               /* сохраненное значение переменной;     */
    (void*)&DataLoaderSto.Settings.ip_ad3,               /* новое значение переменных;           */
    (void*)&DataLoaderSto.Settings.ip_ad3,               /* значение переменной по умолчанию;    */
    (void*)&DataLoaderSto.Settings.ip_ad3,               /* значение переменной во flash;        */
    ""						         /* описание переменных                  */
  },  
 
  {//  uint8_t       ip_gt0;                 // Шлюз  
    LOADER_SECT,   				         /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_GATE,0),        /* идентификатор переменной             */
    ETH_R, 	                                         /* тип доступа к переменной             */
    IP_TYPE,					         /* тип переменной;                      */
    CODEDEC,        				         /* тип вывода на экран;	         */
    (void*)&DataLoaderSto.Settings.ip_gt0,               /* сохраненное значение переменной;     */
    (void*)&DataLoaderSto.Settings.ip_gt0,               /* новое значение переменных;           */
    (void*)&DataLoaderSto.Settings.ip_gt0,               /* значение переменной по умолчанию;    */
    (void*)&DataLoaderSto.Settings.ip_gt0,               /* значение переменной во flash;        */
    "IP-шлюз"				                 /* описание переменных                  */
  },
  
  { //  uint8_t       ip_gt1;                 // Шлюз 
    NO_SECT,   					         /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_GATE,1),        /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                 /* тип доступа к переменной             */
    IP_TYPE,					         /* тип переменной;                      */
    CODEDEC,        				         /* тип вывода на экран;	         */
    (void*)&DataLoaderSto.Settings.ip_gt1,               /* сохраненное значение переменной;     */
    (void*)&DataLoaderSto.Settings.ip_gt1,               /* новое значение переменных;           */
    (void*)&DataLoaderSto.Settings.ip_gt1,               /* значение переменной по умолчанию;    */
    (void*)&DataLoaderSto.Settings.ip_gt1,               /* значение переменной во flash;        */
    ""						         /* описание переменных                  */
  },
  
  {//  uint8_t       ip_gt2;                 // Шлюз 
    NO_SECT,   					         /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_GATE,2),        /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                 /* тип доступа к переменной             */
    IP_TYPE,					         /* тип переменной;                      */
    CODEDEC,        				         /* тип вывода на экран;	         */
    (void*)&DataLoaderSto.Settings.ip_gt2,               /* сохраненное значение переменной;     */
    (void*)&DataLoaderSto.Settings.ip_gt2,               /* новое значение переменных;           */
    (void*)&DataLoaderSto.Settings.ip_gt2,               /* значение переменной по умолчанию;    */
    (void*)&DataLoaderSto.Settings.ip_gt2,               /* значение переменной во flash;        */
    ""						         /* описание переменных                  */
  },
  
  {//  uint8_t       ip_gt3;                 // Шлюз
    NO_SECT,   					         /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_GATE,3),        /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                 /* тип доступа к переменной             */
    IP_TYPE,					         /* тип переменной;                      */
    CODEDEC,        				         /* тип вывода на экран;	         */
    (void*)&DataLoaderSto.Settings.ip_gt3,               /* сохраненное значение переменной;     */
    (void*)&DataLoaderSto.Settings.ip_gt3,               /* новое значение переменных;           */
    (void*)&DataLoaderSto.Settings.ip_gt3,               /* значение переменной по умолчанию;    */
    (void*)&DataLoaderSto.Settings.ip_gt3,               /* значение переменной во flash;        */
    ""						         /* описание переменных                  */
  },         
 
  {//  uint8_t       ip_mask0;               // Маска подсети 
    LOADER_SECT,   				         /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_MASK,0),        /* идентификатор переменной             */
    ETH_R, 	                                 /* тип доступа к переменной             */
    IP_TYPE,					         /* тип переменной;                      */
    CODEDEC,        				         /* тип вывода на экран;	         */
    (void*)&DataLoaderSto.Settings.ip_mask0,             /* сохраненное значение переменной;     */
    (void*)&DataLoaderSto.Settings.ip_mask0,             /* новое значение переменных;           */
    (void*)&DataLoaderSto.Settings.ip_mask0,             /* значение переменной по умолчанию;    */
    (void*)&DataLoaderSto.Settings.ip_mask0,             /* значение переменной во flash;        */
    "IP-Маска подсети"				         /* описание переменных                  */
  },  
  
  {//  uint8_t       ip_mask1;               // Маска подсети 
    NO_SECT,   					         /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_MASK,1),        /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                 /* тип доступа к переменной             */
    IP_TYPE,					         /* тип переменной;                      */
    CODEDEC,        				         /* тип вывода на экран;	         */
    (void*)&DataLoaderSto.Settings.ip_mask1,             /* сохраненное значение переменной;     */
    (void*)&DataLoaderSto.Settings.ip_mask1,             /* новое значение переменных;           */
    (void*)&DataLoaderSto.Settings.ip_mask1,             /* значение переменной по умолчанию;    */
    (void*)&DataLoaderSto.Settings.ip_mask1,             /* значение переменной во flash;        */
    ""						         /* описание переменных                  */
  },
  
  {//  uint8_t       ip_mask2;               // Маска подсети  
    NO_SECT,   					         /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_MASK,2),        /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                 /* тип доступа к переменной             */
    IP_TYPE,					         /* тип переменной;                      */
    CODEDEC,        				         /* тип вывода на экран;	         */
    (void*)&DataLoaderSto.Settings.ip_mask2,             /* сохраненное значение переменной;     */
    (void*)&DataLoaderSto.Settings.ip_mask2,             /* новое значение переменных;           */
    (void*)&DataLoaderSto.Settings.ip_mask2,             /* значение переменной по умолчанию;    */
    (void*)&DataLoaderSto.Settings.ip_mask2,             /* значение переменной во flash;        */
    ""						         /* описание переменных                  */
  },
  
  {//  uint8_t       ip_mask3;               // Маска подсети 
    NO_SECT,   					         /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_MASK,3),        /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                 /* тип доступа к переменной             */
    IP_TYPE,					         /* тип переменной;                      */
    CODEDEC,        				         /* тип вывода на экран;	         */
    (void*)&DataLoaderSto.Settings.ip_mask3,             /* сохраненное значение переменной;     */
    (void*)&DataLoaderSto.Settings.ip_mask3,             /* новое значение переменных;           */
    (void*)&DataLoaderSto.Settings.ip_mask3,             /* значение переменной по умолчанию;    */
    (void*)&DataLoaderSto.Settings.ip_mask3,             /* значение переменной во flash;        */
    ""						         /* описание переменных                  */
  },    
//        

  
  {//  uint8_t       mac_ad0;                // MAC адрес 
    LOADER_SECT,   				         /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_MAC_ADDR,0),       /* идентификатор переменной             */
    ETH_R, 	                                 /* тип доступа к переменной             */
    MAC_TYPE,					         /* тип переменной;                      */
    CODEHEX,        				         /* тип вывода на экран;	         */
    (void*)&DataLoaderSto.Settings.mac_ad0,              /* сохраненное значение переменной;     */
    (void*)&DataLoaderSto.Settings.mac_ad0,              /* новое значение переменных;           */
    (void*)&DataLoaderSto.Settings.mac_ad0,              /* значение переменной по умолчанию;    */
    (void*)&DataLoaderSto.Settings.mac_ad0,              /* значение переменной во flash;        */
    "MAC-адрес"				         /* описание переменных                  */
  },   
  
  {//  uint8_t       mac_ad1;                // MAC адрес 
    NO_SECT,   					         /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_MAC_ADDR,1),       /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                 /* тип доступа к переменной             */
    MAC_TYPE,					         /* тип переменной;                      */
    CODEHEX,        				         /* тип вывода на экран;	         */
    (void*)&DataLoaderSto.Settings.mac_ad1,              /* сохраненное значение переменной;     */
    (void*)&DataLoaderSto.Settings.mac_ad1,              /* новое значение переменных;           */
    (void*)&DataLoaderSto.Settings.mac_ad1,              /* значение переменной по умолчанию;    */
    (void*)&DataLoaderSto.Settings.mac_ad1,              /* значение переменной во flash;        */
    ""						         /* описание переменных                  */
  },
  
  {//  uint8_t       mac_ad2;                // MAC адрес  
    NO_SECT,   					         /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_MAC_ADDR,2),       /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                 /* тип доступа к переменной             */
    MAC_TYPE,					         /* тип переменной;                      */
    CODEHEX,        				         /* тип вывода на экран;	         */
    (void*)&DataLoaderSto.Settings.mac_ad2,              /* сохраненное значение переменной;     */
    (void*)&DataLoaderSto.Settings.mac_ad2,              /* новое значение переменных;           */
    (void*)&DataLoaderSto.Settings.mac_ad2,              /* значение переменной по умолчанию;    */
    (void*)&DataLoaderSto.Settings.mac_ad2,              /* значение переменной во flash;        */
    ""						         /* описание переменных                  */
  },
  
  {//  uint8_t       mac_ad3;                // MAC адрес 
    NO_SECT,   					         /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_MAC_ADDR,3),       /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                 /* тип доступа к переменной             */
    MAC_TYPE,					         /* тип переменной;                      */
    CODEHEX,        				         /* тип вывода на экран;	         */
    (void*)&DataLoaderSto.Settings.mac_ad3,              /* сохраненное значение переменной;     */
    (void*)&DataLoaderSto.Settings.mac_ad3,              /* новое значение переменных;           */
    (void*)&DataLoaderSto.Settings.mac_ad3,              /* значение переменной по умолчанию;    */
    (void*)&DataLoaderSto.Settings.mac_ad3,              /* значение переменной во flash;        */
    ""						         /* описание переменных                  */
  },   
  
  {//  uint8_t       mac_ad4;                // MAC адрес 
    NO_SECT,   					         /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_MAC_ADDR,4),       /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                 /* тип доступа к переменной             */
    MAC_TYPE,					         /* тип переменной;                      */
    CODEHEX,        				         /* тип вывода на экран;	         */
    (void*)&DataLoaderSto.Settings.mac_ad4,              /* сохраненное значение переменной;     */
    (void*)&DataLoaderSto.Settings.mac_ad4,              /* новое значение переменных;           */
    (void*)&DataLoaderSto.Settings.mac_ad4,              /* значение переменной по умолчанию;    */
    (void*)&DataLoaderSto.Settings.mac_ad4,              /* значение переменной во flash;        */
    ""						         /* описание переменных                  */
  },
  
  {//  uint8_t       mac_ad5;                // MAC адрес 
    NO_SECT,   					         /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_MAC_ADDR,5),       /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                 /* тип доступа к переменной             */
    MAC_TYPE,					         /* тип переменной;                      */
    CODEHEX,        				         /* тип вывода на экран;	         */
    (void*)&DataLoaderSto.Settings.mac_ad5,              /* сохраненное значение переменной;     */
    (void*)&DataLoaderSto.Settings.mac_ad5,              /* новое значение переменных;           */
    (void*)&DataLoaderSto.Settings.mac_ad5,              /* значение переменной по умолчанию;    */
    (void*)&DataLoaderSto.Settings.mac_ad5,              /* значение переменной во flash;        */
    ""						         /* описание переменных                  */
  },     
        
  {//  uint16_t      ip_port_loader;         // Порт загрузчика
    LOADER_SECT,   					 /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_PORT_LOADER,0), /* идентификатор переменной             */
    ETH_R, 	                                         /* тип доступа к переменной             */
    UINT16,						 /* тип переменной;                      */
    CODEDEC,        					 /* тип вывода на экран;	         */
    (void*)&DataLoaderSto.Settings.ip_port_loader,  	 /* сохраненное значение переменной;     */
    (void*)&DataLoaderSto.Settings.ip_port_loader,  	 /* новое значение переменных;           */
    (void*)&DataLoaderSto.Settings.ip_port_loader,  	 /* значение переменной по умолчанию;    */
    (void*)&DataLoaderSto.Settings.ip_port_loader,       /* значение переменной во flash;        */
    "IP порт загрузчика-конфигурирования"                /* описание переменных	                 */
  },  
    
  { //12345,	  uint16_t      code_pass;                // Код доступа
    LOADER_SECT,   					/* номер секции                         */			
    VarID(Core_Type,Core_N_port,GET_CORE_CODE_PASS,0),  /* идентификатор переменной             */
    ETH_R, 	                                        /* тип доступа к переменной             */
    UINT16,						/* тип переменной;                      */					   
    CODEDEC,        					/* тип вывода на экран;	                */
    (void*)&DataLoaderSto.Settings.code_pass,  	        /* сохраненное значение переменной;     */
    (void*)&DataLoaderSto.Settings.code_pass,  	        /* новое значение переменных;           */
    (void*)&DataLoaderSto.Settings.code_pass,  	        /* значение переменной по умолчанию;    */
    (void*)&DataLoaderSto.Settings.code_pass,           /* значение переменной во flash;        */	
    "Код доступа"					/* описание переменных	                */	
  },       
  
  {//	0x1234,    //uint16_t 	phy_adr; 	  	// физический адрес блока	
    LOADER_SECT, 				        /* номер секции                         */			
    VarID(Core_Type,Core_N_port,GET_LOADER_PHY_ADDR,0), /* идентификатор переменной             */
    ETH_R, 	                                        /* тип доступа к переменной             */
    UINT16,						/* тип переменной;                      */					    
    CODEHEX,        					/* тип вывода на экран;	                */
    (void*)&DataLoaderSto.Settings.phy_adr,  	        /* сохраненное значение переменной;     */
    (void*)&DataLoaderSto.Settings.phy_adr,             /* новое значение переменных;           */
    (void*)&DataLoaderSto.Settings.phy_adr,  	        /* значение переменной по умолчанию;    */
    (void*)&DataLoaderSto.Settings.phy_adr,             /* значение переменной во flash;        */	
    "Физический адрес блока"				/* описание переменных	                */	
  },      
  //==============================================================================
  //      Настройки для ETH
  //============================================================================== 
  {//7757,			//uint16_t 					ip_port_voice; 				// Порт приема передачи голосовых сообщений
    ETH_SECT,   					                                           /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_PORT_VOICE,0),                                            /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                                                           /* тип доступа к переменной             */
    UINT16,						                                           /* тип переменной;                      */
    CODEDEC,        					                                           /* тип вывода на экран;	           */
    (void*)&DataSto.Settings.ip_port_voice,  	                                                   /* сохраненное значение переменной;     */
    (void*)&DataNew.Settings.ip_port_voice,  	                                                   /* новое значение переменных;           */
    (void*)&DataDef.Settings.ip_port_voice,  	                                                   /* значение переменной по умолчанию;    */
    (void*)&DataFLH.Settings.ip_port_voice,                                                        /* значение переменной во flash;        */
    "IP порт приема передачи голосовых сообщений"                                                  /* описание переменных	           */
  },

  {//192,			//uint8_t 					ip_table_main_srv_ad0; 		// Адрес IPv4 сервера раздающего  таблицу рассылки
    ETH_SECT,   											//номер секции
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_ADDR_TABLE_SRV_MAIN,0),   //идентификатор переменной
    ETH_R|ETH_W, 	                                  		    	//тип доступа к переменной
    IP_TYPE,												    //тип переменной;
    CODEDEC,        											//тип вывода на экран;
    (void*)&DataSto.Settings.ip_table_main_srv_ad0,  //сохраненное значение переменной;
    (void*)&DataNew.Settings.ip_table_main_srv_ad0,  //новое значение переменных;
    (void*)&DataDef.Settings.ip_table_main_srv_ad0,  //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.ip_table_main_srv_ad0,          //значение переменной во flash; 
    "IP-адрес главного сервера раздающего  таблицу рассылки"								  												//описание переменных
  },
  
  { //168,			//uint8_t 					ip_table_main_srv_ad1; 		// Адрес IPv4 сервера раздающего  таблицу рассылки
    NO_SECT,   													//номер секции
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_ADDR_TABLE_SRV_MAIN,1),   //идентификатор переменной
    ETH_R|ETH_W, 	                                  				//тип доступа к переменной
    IP_TYPE,													//тип переменной;
    CODEDEC,        											//тип вывода на экран;
    (void*)&DataSto.Settings.ip_table_main_srv_ad1,  //сохраненное значение переменной;
    (void*)&DataNew.Settings.ip_table_main_srv_ad1,  //новое значение переменных;
    (void*)&DataDef.Settings.ip_table_main_srv_ad1,  //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.ip_table_main_srv_ad1,          //значение переменной во flash; 
    ""									      					// продолжение переменной типа IP_TYPE
  },
  
  {//1,			//uint8_t 					ip_table_main_srv_ad2; 	// Адрес IPv4 сервера раздающего  таблицу рассылки
    NO_SECT,   							                                   /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_ADDR_TABLE_SRV_MAIN,2),                                   /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                  	                                   /* тип доступа к переменной             */
    IP_TYPE,							                                   /* тип переменной;                      */
    CODEDEC,        						                                   /* тип вывода на экран;	           */
    (void*)&DataSto.Settings.ip_table_main_srv_ad2,                                                /* сохраненное значение переменной;     */
    (void*)&DataNew.Settings.ip_table_main_srv_ad2,                                                /* новое значение переменных;           */
    (void*)&DataDef.Settings.ip_table_main_srv_ad2,                                                /* значение переменной по умолчанию;    */
    (void*)&DataFLH.Settings.ip_table_main_srv_ad2,                                                /* значение переменной во flash;        */	
    ""								                                   /* описание переменных	           */
  },
  
  {//59,			//uint8_t 					ip_table_main_srv_ad3; 	// Адрес IPv4 сервера раздающего  таблицу рассылки
    NO_SECT,   							                                   /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_ADDR_TABLE_SRV_MAIN,3),                                   /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                  	                                   /* тип доступа к переменной             */
    IP_TYPE,							                                   /* тип переменной;                      */
    CODEDEC,        						                                   /* тип вывода на экран;	           */
    (void*)&DataSto.Settings.ip_table_main_srv_ad3,                                                /* сохраненное значение переменной;     */
    (void*)&DataNew.Settings.ip_table_main_srv_ad3,                                                /* новое значение переменных;           */
    (void*)&DataDef.Settings.ip_table_main_srv_ad3,                                                /* значение переменной по умолчанию;    */
    (void*)&DataFLH.Settings.ip_table_main_srv_ad3,                                                /* значение переменной во flash;        */	
    ""								                                   /* описание переменных	           */
  },
  
  {//7757,			//uint16_t 					ip_table_main_srv_port; 	// Порт сервера раздающего  таблицу рассылки
    ETH_SECT,   						                                   /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_PORT_TABLE_SRV_MAIN,0),                                   /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                  	                                   /* тип доступа к переменной             */
    UINT16,							                                   /* тип переменной;                      */
    CODEDEC,        						                                   /* тип вывода на экран;	           */
    (void*)&DataSto.Settings.ip_table_main_srv_port,                                               /* сохраненное значение переменной;     */
    (void*)&DataNew.Settings.ip_table_main_srv_port,                                               /* новое значение переменных;           */
    (void*)&DataDef.Settings.ip_table_main_srv_port,                                               /* значение переменной по умолчанию;    */
    (void*)&DataFLH.Settings.ip_table_main_srv_port,                                               /* значение переменной во flash;        */	
    "IP порт главного сервера раздающего  таблицу рассылки"	                                   /* описание переменных	           */
  },
  
  
  {//192,			//uint8_t 					ip_table_main_srv_ad0; 		// Адрес IPv4 сервера раздающего  таблицу рассылки
    ETH_SECT,   						                                   /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_ADDR_TABLE_SRV_REZV,0),                                   /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                  	                                   /* тип доступа к переменной             */
    IP_TYPE,							                                   /* тип переменной;                      */
    CODEDEC,        						                                   /* тип вывода на экран;	           */
    (void*)&DataSto.Settings.ip_table_rezv_srv_ad0,                                                /* сохраненное значение переменной;     */
    (void*)&DataNew.Settings.ip_table_rezv_srv_ad0,                                                /* новое значение переменных;           */
    (void*)&DataDef.Settings.ip_table_rezv_srv_ad0,                                                /* значение переменной по умолчанию;    */
    (void*)&DataFLH.Settings.ip_table_rezv_srv_ad0,                                                /* значение переменной во flash;        */	
    "IP-адрес резервного сервера раздающего  таблицу рассылки"	                                   /* описание переменных	           */						
  },
  
  { //168,			//uint8_t 					ip_table_main_srv_ad1; 		// Адрес IPv4 сервера раздающего  таблицу рассылки
    NO_SECT,   							                                   /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_ADDR_TABLE_SRV_REZV,1),                                   /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                  	                                   /* тип доступа к переменной             */
    IP_TYPE,							                                   /* тип переменной;                      */
    CODEDEC,        						                                   /* тип вывода на экран;	           */
    (void*)&DataSto.Settings.ip_table_rezv_srv_ad1,                                                /* сохраненное значение переменной;     */
    (void*)&DataNew.Settings.ip_table_rezv_srv_ad1,                                                /* новое значение переменных;           */
    (void*)&DataDef.Settings.ip_table_rezv_srv_ad1,                                                /* значение переменной по умолчанию;    */
    (void*)&DataFLH.Settings.ip_table_rezv_srv_ad1,                                                /* значение переменной во flash;        */	
    ""								                                   /* описание переменных	           */
  },
  
  {//1,			//uint8_t 					ip_table_main_srv_ad2; 	// Адрес IPv4 сервера раздающего  таблицу рассылки
    NO_SECT,   							                                   /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_ADDR_TABLE_SRV_REZV,2),                                   /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                  	                                   /* тип доступа к переменной             */
    IP_TYPE,							                                   /* тип переменной;                      */
    CODEDEC,        						                                   /* тип вывода на экран;	           */
    (void*)&DataSto.Settings.ip_table_rezv_srv_ad2,                                                /* сохраненное значение переменной;     */
    (void*)&DataNew.Settings.ip_table_rezv_srv_ad2,                                                /* новое значение переменных;           */
    (void*)&DataDef.Settings.ip_table_rezv_srv_ad2,                                                /* значение переменной по умолчанию;    */
    (void*)&DataFLH.Settings.ip_table_rezv_srv_ad2,                                                /* значение переменной во flash;        */	
    ""								                                   /* описание переменных	           */
  },
  
  {//59,			//uint8_t 					ip_table_main_srv_ad3; 	// Адрес IPv4 сервера раздающего  таблицу рассылки
    NO_SECT,   							                                   /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_ADDR_TABLE_SRV_REZV,3),                                   /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                                                           /* тип доступа к переменной             */
    IP_TYPE,						                                           /* тип переменной;                      */
    CODEDEC,        					                                           /* тип вывода на экран;	           */
    (void*)&DataSto.Settings.ip_table_rezv_srv_ad3,                                                /* сохраненное значение переменной;     */
    (void*)&DataNew.Settings.ip_table_rezv_srv_ad3,                                                /* новое значение переменных;           */
    (void*)&DataDef.Settings.ip_table_rezv_srv_ad3,                                                /* значение переменной по умолчанию;    */
    (void*)&DataFLH.Settings.ip_table_rezv_srv_ad3,                                                /* значение переменной во flash;        */	
    ""								                                   /* описание переменных	           */
  },
  
  {//7757,			//uint16_t 					ip_table_main_srv_port; 	// Порт сервера раздающего  таблицу рассылки
    ETH_SECT,   						                                   /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_ETH_IP_PORT_TABLE_SRV_REZV,0),                                   /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                  	                                   /* тип доступа к переменной             */
    UINT16,							                                   /* тип переменной;                      */
    CODEDEC,        						                                   /* тип вывода на экран;	           */
    (void*)&DataSto.Settings.ip_table_rezv_srv_port,                                               /* сохраненное значение переменной;     */
    (void*)&DataNew.Settings.ip_table_rezv_srv_port,                                               /* новое значение переменных;           */
    (void*)&DataDef.Settings.ip_table_rezv_srv_port,                                               /* значение переменной по умолчанию;    */
    (void*)&DataFLH.Settings.ip_table_rezv_srv_port,                                               /* значение переменной во flash;        */	
    "IP порт резервного сервера раздающего таблицу рассылки"	                                   /* описание переменных	           */
  },
  
  {//0xFFFF,    //uint16_t      phy_addr_disp_1;                                /* Физический адрес диспетчера 1      */ 
    ETH_SECT,                                                                                      /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_PHY_ADDR_DISP_1,0),                                              /* идентификатор переменной             */
    ETH_R|ETH_W, 	                                  		                           /* тип доступа к переменной             */
    UINT16,								                           /* тип переменной;                      */
    CODEDEC,          							                           /* тип вывода на экран;	           */
    (void*)&DataSto.Settings.phy_addr_disp_1 ,	                                                   /* сохраненное значение переменной;     */
    (void*)&DataNew.Settings.phy_addr_disp_1 ,	                                                   /* новое значение переменных;           */
    (void*)&DataDef.Settings.phy_addr_disp_1 ,	                                                   /* значение переменной по умолчанию;    */
    (void*)&DataFLH.Settings.phy_addr_disp_1 ,                                                     /* значение переменной во flash;        */	
    "PHY адрес диспетчера вызываемого по кнопке 1"                                                 /* описание переменных	           */
  },    
  
  {//  0xFFFF,    //uint16_t      phy_addr_disp_2;                              /* Физический адрес диспетчера 2      */ 
    ETH_SECT,   					                                          /* номер секции                         */
    VarID(ETH_Type,ETH_N_port,GET_PHY_ADDR_DISP_2,0),                                             /* идентификатор переменной             */
    ETH_R|ETH_W,                                                                                  /* тип доступа к переменной             */
    UINT16,	      	                                                                          /* тип переменной;                      */
    CODEDEC,                                                                                      /* тип вывода на экран;	           */
    (void*)&DataSto.Settings.phy_addr_disp_2,	                                                  /* сохраненное значение переменной;     */
    (void*)&DataNew.Settings.phy_addr_disp_2,	                                                  /* новое значение переменных;           */
    (void*)&DataDef.Settings.phy_addr_disp_2,	                                                  /* значение переменной по умолчанию;    */
    (void*)&DataFLH.Settings.phy_addr_disp_2,                                                     /* значение переменной во flash;        */	
    "PHY адрес диспетчера вызываемого по кнопке 2"                                                /* описание переменных	           */
  },    
  
  {//     MaskBoxIDVoiceETH,         //uint8_t    nmask_transl_voice;             // Hомер маски ID_RS пакетов трансляции для голосового порта рассылки
    ETH_SECT,   											    //номер секции
    VarID(ETH_Type,ETH_N_port,GET_ETH_NMASK_TRANSL_VOICE,0),   //идентификатор переменной
    ETH_R|ETH_W, 	                                  				//тип доступа к переменной
    UINT8,														//тип переменной;
    CODEDEC,          											//тип вывода на экран;
    (void*)&DataSto.Settings.nmask_transl_voice,	//сохраненное значение переменной;
    (void*)&DataNew.Settings.nmask_transl_voice,	//новое значение переменных;
    (void*)&DataDef.Settings.nmask_transl_voice,	//значение переменной по умолчанию;
    (void*)&DataFLH.Settings.nmask_transl_voice,          //значение переменной во flash; 
    "Hомер маски ID_RS пакетов трансляции для голосового порта рассылки"							//описание переменных
  },  

  {// MaskBoxIDETHtubeRS,              //uint8_t    nmask_transl_position;          // Hомер текущей маски ID_RS пакетов трансляции позиционирования и диагностики 
    ETH_SECT,   											    //номер секции
    VarID(ETH_Type,ETH_N_port,GET_ETH_NMASK_TRANSL_CONFIG,0),   //идентификатор переменной
    ETH_R|ETH_W, 	                                  				//тип доступа к переменной
    UINT8,														//тип переменной;
    CODEDEC,              											//тип вывода на экран;
    (void*)&DataSto.Settings.nmask_transl_config,	//сохраненное значение переменной;
    (void*)&DataNew.Settings.nmask_transl_config,	//новое значение переменных;
    (void*)&DataDef.Settings.nmask_transl_config,	//значение переменной по умолчанию;
    (void*)&DataFLH.Settings.nmask_transl_config,          //значение переменной во flash; 
    "Hомер маски ID_RS пакетов трансляции трансляции конфигурирования и загрузки"							//описание переменных
  },      

  {//BitID(CorePortID) | BitID(RSPortID) | BitID(RFPortID) | BitID(CodecPortID),//uint32_t   mask_inpup_port_id_voice;       // Маска ID портов разрешенных для приема в данный порт трансляции для голосового порта рассылки 
    ETH_SECT,   											    //номер секции
    VarID(ETH_Type,ETH_N_port,GET_ETH_MASK_INPUT_PORT_ID_VOICE,0),   //идентификатор переменной
    ETH_R|ETH_W, 	                                  				//тип доступа к переменной
    UINT32,														//тип переменной;
    CODEHEX,           											//тип вывода на экран;
    (void*)&DataSto.Settings.mask_inpup_port_id_voice,	//сохраненное значение переменной;
    (void*)&DataNew.Settings.mask_inpup_port_id_voice,	//новое значение переменных;
    (void*)&DataDef.Settings.mask_inpup_port_id_voice,	//значение переменной по умолчанию;
    (void*)&DataFLH.Settings.mask_inpup_port_id_voice,          //значение переменной во flash; 
    "Маска ID портов разрешенных для приема в данный порт трансляции для голосового порта рассылки" //описание переменных
	
  },     
  
  {//     BitID(CorePortID) | BitID(RSPortID),                                       //uint32_t   mask_inpup_port_id_config;      // Маска ID портов разрешенных для приема в данный порт трансляции конфигурирования и загрузки     
    ETH_SECT,   											    //номер секции
    VarID(ETH_Type,ETH_N_port,GET_ETH_MASK_INPUT_PORT_ID_CONFIG,0),   //идентификатор переменной
    ETH_R|ETH_W, 	                                  	      //тип доступа к переменной
    UINT32,														//тип переменной;
    CODEHEX,        											//тип вывода на экран;
    (void*)&DataSto.Settings.mask_inpup_port_id_config,	//сохраненное значение переменной;
    (void*)&DataNew.Settings.mask_inpup_port_id_config,	//новое значение переменных;
    (void*)&DataDef.Settings.mask_inpup_port_id_config,	//значение переменной по умолчанию;
    (void*)&DataFLH.Settings.mask_inpup_port_id_config, //значение переменной во flash; 
    "Маска ID портов разрешенных для приема в данный порт трансляции конфигурирования и загрузки"							//описание переменных
  },   
   
  /* Настройки порта отправки несжатого потока в ETH */                                      
  {  //uint32_t  udp_mask_source_soft_port;           /* Маска портов разрешенных для приема в UDP порт из програмного роутера */ 
    ETH_SECT,   						                //номер секции
    VarID(ETH_Type,ETH_N_port,GET_UDP_MASK_SOURSE_SOFT_PORT,0),                 //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT32,						                        //тип переменной;
    CODEHEX,              			      	                        //тип вывода на экран;
    (void*)&DataSto.Settings.udp_mask_source_soft_port,                         //сохраненное значение переменной;
    (void*)&DataNew.Settings.udp_mask_source_soft_port,                         //новое значение переменных;
    (void*)&DataDef.Settings.udp_mask_source_soft_port,                         //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.udp_mask_source_soft_port,                         //значение переменной во flash; 
    "Маска портов разрешенных для приема в данный порт из програмного роутера"  //описание переменных
  },      

  {  // uint32_t  udp_codec_mask_chanel_soft_port;    /* Маска каналов разрешенных для приема пакетов в UDP порт из програмного роутера */
    ETH_SECT,   						                //номер секции
    VarID(ETH_Type,ETH_N_port,GET_UDP_MASK_CHANEL_SOFT_PORT,0),                 //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT32,						                        //тип переменной;
    CODEHEX,           					                        //тип вывода на экран;
    (void*)&DataSto.Settings.udp_mask_chanel_soft_port,                         //сохраненное значение переменной;
    (void*)&DataNew.Settings.udp_mask_chanel_soft_port,                         //новое значение переменных;
    (void*)&DataDef.Settings.udp_mask_chanel_soft_port,                         //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.udp_mask_chanel_soft_port,                         //значение переменной во flash; 
    "Маска каналов разрешенных для приема пакетов в UDP порт из програмного роутера" //описание переменных
  },   
  
  /*==============================================================================*/
  /*      Настройки для RS A                                                      */
  /*==============================================================================*/ 
  { //  SET_RS422,      //uint8_t  Type_RS_a;     //  Значениe адреса устр-ва по UART  
    PARAM_SECT,					           /* номер секции                      */
    VarID(RS_Type,RS_A_port,GET_RS_MODE,0),                /* идентификатор переменной          */
    ETH_R|ETH_W,                               	           /* тип доступа к переменной          */
    UINT8,					           /* тип переменной;                   */
    CODEDEC,        				           /* тип вывода на экран;	        */
    (void*)&DataSto.Settings.Type_RS_a,	    	           /* сохраненное значение переменной;  */
    (void*)&DataNew.Settings.Type_RS_a,	    	           /* новое значение переменных;        */
    (void*)&DataDef.Settings.Type_RS_a,	    	           /* значение переменной по умолчанию; */
    (void*)&DataFLH.Settings.Type_RS_a,                    /* значение переменной во flash;     */		
    "Режим работы порта RS_A UART"                         /* описание переменных	        */
  },
  
  {   // 230400,   //uint32_t rs_bit_rate_a; 	  // Скорость интерфейса RS
    PARAM_SECT,					           /* номер секции                       */
    VarID(RS_Type,RS_A_port,GET_RS_BAUDRATE,0),            /* идентификатор переменной           */
    ETH_R|ETH_W,                               	           /* тип доступа к переменной           */
    UINT32,					           /* тип переменной;                    */
    CODEDEC,        				           /* тип вывода на экран;	         */
    (void*)&DataSto.Settings.rs_bit_rate_a,	           /* сохраненное значение переменной;   */
    (void*)&DataNew.Settings.rs_bit_rate_a,	           /* новое значение переменных;         */
    (void*)&DataDef.Settings.rs_bit_rate_a,	           /* значение переменной по умолчанию;  */
    (void*)&DataFLH.Settings.rs_bit_rate_a,                /* значение переменной во flash;      */	
    "Скорость интерфейса RS_A UART"		           /* описание переменных	         */
  },
  
  { //  MaskBoxIDRSAPort, //uint8_t  nmask_transl_rs_a; // Номер текущей маски ID_RS пакетов трансляции
    PARAM_SECT,					           /* номер секции                        */
    VarID(RS_Type,RS_A_port,GET_RS_NMASK_TRANSL,0),        /* идентификатор переменной            */
    ETH_R|ETH_W,                               	           /* тип доступа к переменной            */
    UINT8,					           /* тип переменной;                     */
    CODEDEC,        				           /* тип вывода на экран;	          */
    (void*)&DataSto.Settings.nmask_transl_rs_a,	           /* сохраненное значение переменной;    */
    (void*)&DataNew.Settings.nmask_transl_rs_a,	           /* новое значение переменных;          */
    (void*)&DataDef.Settings.nmask_transl_rs_a,	           /* значение переменной по умолчанию;   */
    (void*)&DataFLH.Settings.nmask_transl_rs_a,            /* значение переменной во flash;       */	
    "Номер маски ID_RS_A пакетов трансляции"	           /* описание переменных	          */
  },                
  
  {  //  BitID(VoiceInfoID), //uint32_t mask_inpup_port_id_rs_a;   // Mаска ID портов разрешенных для приема в данный порт
    PARAM_SECT,						   /* номер секции                         */
    VarID(RS_Type,RS_A_port,SET_RS_MASK_INPUT_PORT_ID,0),  /* идентификатор переменной             */
    ETH_R|ETH_W,                               		   /* тип доступа к переменной             */
    UINT32,						   /* тип переменной;                      */
    CODEHEX,        					   /* тип вывода на экран;	           */
    (void*)&DataSto.Settings.mask_inpup_port_id_rs_a,	   /* сохраненное значение переменной;     */
    (void*)&DataNew.Settings.mask_inpup_port_id_rs_a,	   /* новое значение переменных;           */
    (void*)&DataDef.Settings.mask_inpup_port_id_rs_a,	   /* значение переменной по умолчанию;    */
    (void*)&DataFLH.Settings.mask_inpup_port_id_rs_a,      /* значение переменной во flash;        */	
    "Mаска ID портов разрешенных для приема в порт RS_A"   /* описание переменных	           */
  },     

  /*==============================================================================*/
  /*      Настройки для RS B                                                      */
  /*==============================================================================*/  
  { //  SET_RS422,      //uint8_t  Type_RS_b;     //  Значениe адреса устр-ва по UART  
    PARAM_SECT,					           /* номер секции                      */
    VarID(RS_Type,RS_B_port,GET_RS_MODE,0),                /* идентификатор переменной          */
    ETH_R|ETH_W,                               	           /* тип доступа к переменной          */
    UINT8,					           /* тип переменной;                   */
    CODEDEC,        				           /* тип вывода на экран;	        */
    (void*)&DataSto.Settings.Type_RS_b,	    	           /* сохраненное значение переменной;  */
    (void*)&DataNew.Settings.Type_RS_b,	    	           /* новое значение переменных;        */
    (void*)&DataDef.Settings.Type_RS_b,	    	           /* значение переменной по умолчанию; */
    (void*)&DataFLH.Settings.Type_RS_b,                    /* значение переменной во flash;     */		
    "Режим работы порта RS_B UART"                         /* описание переменных	        */
  },
  
  {   // 230400,   //uint32_t rs_bit_rate_b; 	  // Скорость интерфейса RS
    PARAM_SECT,					           /* номер секции                       */
    VarID(RS_Type,RS_B_port,GET_RS_BAUDRATE,0),            /* идентификатор переменной           */
    ETH_R|ETH_W,                               	           /* тип доступа к переменной           */
    UINT32,					           /* тип переменной;                    */
    CODEDEC,        				           /* тип вывода на экран;	         */
    (void*)&DataSto.Settings.rs_bit_rate_b,	           /* сохраненное значение переменной;   */
    (void*)&DataNew.Settings.rs_bit_rate_b,	           /* новое значение переменных;         */
    (void*)&DataDef.Settings.rs_bit_rate_b,	           /* значение переменной по умолчанию;  */
    (void*)&DataFLH.Settings.rs_bit_rate_b,                /* значение переменной во flash;      */	
    "Скорость интерфейса RS_B UART"		           /* описание переменных	         */
  },
  
  { //  MaskBoxIDRSAPort, //uint8_t  nmask_transl_rs_b; // Номер текущей маски ID_RS пакетов трансляции
    PARAM_SECT,					           /* номер секции                        */
    VarID(RS_Type,RS_B_port,GET_CODEC_MASK_SOURSE_SOFT_PORT,0),        /* идентификатор переменной            */
    ETH_R|ETH_W,                               	           /* тип доступа к переменной            */
    UINT8,					           /* тип переменной;                     */
    CODEDEC,        				           /* тип вывода на экран;	          */
    (void*)&DataSto.Settings.nmask_transl_rs_b,	           /* сохраненное значение переменной;    */
    (void*)&DataNew.Settings.nmask_transl_rs_b,	           /* новое значение переменных;          */
    (void*)&DataDef.Settings.nmask_transl_rs_b,	           /* значение переменной по умолчанию;   */
    (void*)&DataFLH.Settings.nmask_transl_rs_b,            /* значение переменной во flash;       */	
    "Номер маски ID_RS_B пакетов трансляции"	           /* описание переменных	          */
  },                
  
  {  //  BitID(VoiceInfoID), //uint32_t mask_inpup_port_id_rs_b;   // Mаска ID портов разрешенных для приема в данный порт
    PARAM_SECT,						   /* номер секции                         */
    VarID(RS_Type,RS_B_port,SET_RS_MASK_INPUT_PORT_ID,0),  /* идентификатор переменной             */
    ETH_R|ETH_W,                               		   /* тип доступа к переменной             */
    UINT32,						   /* тип переменной;                      */
    CODEHEX,        					   /* тип вывода на экран;	           */
    (void*)&DataSto.Settings.mask_inpup_port_id_rs_b,	   /* сохраненное значение переменной;     */
    (void*)&DataNew.Settings.mask_inpup_port_id_rs_b,	   /* новое значение переменных;           */
    (void*)&DataDef.Settings.mask_inpup_port_id_rs_b,	   /* значение переменной по умолчанию;    */
    (void*)&DataFLH.Settings.mask_inpup_port_id_rs_b,      /* значение переменной во flash;        */	
    "Mаска ID портов разрешенных для приема в порт RS_B"   /* описание переменных	           */
  },    
  
  /*============================================================================*/
  /*     Настройки для CODEC                                                    */
  /*============================================================================*/
  /*========================= Настройки кодека канала А ========================*/
  {//    uint8_t  codec_a_chanel;                  /* Номер канала закреленный за кодеком */ 
    CODEC_SECT,   						                //номер секции
    VarID(CODEC_Type,CODEC_A_port,GET_CODEC_CHANEL,0),                          //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT8,					                                //тип переменной;
    CODEDEC,          				                                //тип вывода на экран;
    (void*)&DataSto.Settings.codec_a_chanel,                                    //сохраненное значение переменной;
    (void*)&DataNew.Settings.codec_a_chanel,                                    //новое значение переменных;
    (void*)&DataDef.Settings.codec_a_chanel,                                    //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.codec_a_chanel,                                    //значение переменной во flash; 
    "Номер канала закрепленный за кодеком A"                                     //описание переменных
  },   
  
  {//    uint8_t  codec_a_priority_ch_mic;         /* Приоритет голосового канала микрофона по умолчанию (1..30) */ 
    CODEC_SECT,   						                //номер секции
    VarID(CODEC_Type,CODEC_A_port,GET_CODEC_PRIORITY_CH_PCM,0),                 //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT8,					                                //тип переменной;
    CODEDEC,          				                                //тип вывода на экран;
    (void*)&DataSto.Settings.codec_a_priority_ch_pcm,                           //сохраненное значение переменной;
    (void*)&DataNew.Settings.codec_a_priority_ch_pcm,                           //новое значение переменных;
    (void*)&DataDef.Settings.codec_a_priority_ch_pcm,                           //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.codec_a_priority_ch_pcm,                           //значение переменной во flash; 
    "Приоритет голосового канала pcm кодека А по умолчанию (1..30)"       //описание переменных
  },  
  
  {//    uint8_t  codec_a_priority_ch_spkr;        /* Приоритет голосового канала спикера по умолчанию   (1..30) */ 
    CODEC_SECT,   						                //номер секции
    VarID(CODEC_Type,CODEC_A_port,GET_CODEC_PRIORITY_CH_CMX,0),                //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT8,					                                //тип переменной;
    CODEDEC,          				                                //тип вывода на экран;
    (void*)&DataSto.Settings.codec_a_priority_ch_cmx,                          //сохраненное значение переменной;
    (void*)&DataNew.Settings.codec_a_priority_ch_cmx,                          //новое значение переменных;
    (void*)&DataDef.Settings.codec_a_priority_ch_cmx,                          //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.codec_a_priority_ch_cmx,                          //значение переменной во flash; 
    "Приоритет голосового канала cmx кодека А по умолчанию (1..30)"            //описание переменных
  },  
  
  {//    uint32_t codec_a_mask_source_soft_port;   /* Маска портов разрешенных для приема в данный порт из програмного роутера */ 
    CODEC_SECT,   						                //номер секции
    VarID(CODEC_Type,CODEC_A_port,GET_CODEC_MASK_SOURSE_SOFT_PORT,0),           //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT32,						                        //тип переменной;
    CODEHEX,        					                        //тип вывода на экран;
    (void*)&DataSto.Settings.codec_a_mask_source_soft_port,	                //сохраненное значение переменной;
    (void*)&DataNew.Settings.codec_a_mask_source_soft_port,	                //новое значение переменных;
    (void*)&DataDef.Settings.codec_a_mask_source_soft_port,	                //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.codec_a_mask_source_soft_port,                     //значение переменной во flash; 
    "Маска портов разрешенных для приема в порт кодека А из програмного роутера"//описание переменных
  },    
  
  {//    uint32_t codec_a_mask_chanel_soft_port;   /* Маска каналов разрешенных для приема в данный порт из програмного роутера */
    CODEC_SECT,   						                //номер секции
    VarID(CODEC_Type,CODEC_A_port,GET_CODEC_MASK_CHANEL_SOFT_PORT,0),           //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT32,						                        //тип переменной;
    CODEHEX,        					                        //тип вывода на экран;
    (void*)&DataSto.Settings.codec_a_mask_chanel_soft_port,	                //сохраненное значение переменной;
    (void*)&DataNew.Settings.codec_a_mask_chanel_soft_port,	                //новое значение переменных;
    (void*)&DataDef.Settings.codec_a_mask_chanel_soft_port,	                //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.codec_a_mask_chanel_soft_port,                     //значение переменной во flash; 
    "Маска каналов разрешенных для приема в порт кодека А из програмного роутера"//описание переменных
  }, 
  /*========================= Настройки кодека канала B ========================*/
  {//    uint8_t  codec_b_chanel;                  /* Номер канала закреленный за кодеком */ 
    CODEC_SECT,   						                //номер секции
    VarID(CODEC_Type,CODEC_B_port,GET_CODEC_CHANEL,0),                          //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT8,					                                //тип переменной;
    CODEDEC,          				                                //тип вывода на экран;
    (void*)&DataSto.Settings.codec_b_chanel,                                    //сохраненное значение переменной;
    (void*)&DataNew.Settings.codec_b_chanel,                                    //новое значение переменных;
    (void*)&DataDef.Settings.codec_b_chanel,                                    //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.codec_b_chanel,                                    //значение переменной во flash; 
    "Номер канала закрепленный за кодеком B"                                     //описание переменных
  },   
  
  {//    uint8_t  codec_b_priority_ch_mic;         /* Приоритет голосового канала микрофона по умолчанию (1..30) */ 
    CODEC_SECT,   						                //номер секции
    VarID(CODEC_Type,CODEC_B_port,GET_CODEC_PRIORITY_CH_PCM,0),                 //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT8,					                                //тип переменной;
    CODEDEC,          				                                //тип вывода на экран;
    (void*)&DataSto.Settings.codec_b_priority_ch_pcm,                           //сохраненное значение переменной;
    (void*)&DataNew.Settings.codec_b_priority_ch_pcm,                           //новое значение переменных;
    (void*)&DataDef.Settings.codec_b_priority_ch_pcm,                           //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.codec_b_priority_ch_pcm,                           //значение переменной во flash; 
    "Приоритет голосового канала pcm кодека B по умолчанию (1..30)"       //описание переменных
  },  
  
  {//    uint8_t  codec_b_priority_ch_spkr;        /* Приоритет голосового канала спикера по умолчанию   (1..30) */ 
    CODEC_SECT,   						                //номер секции
    VarID(CODEC_Type,CODEC_B_port,GET_CODEC_PRIORITY_CH_CMX,0),                //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT8,					                                //тип переменной;
    CODEDEC,          				                                //тип вывода на экран;
    (void*)&DataSto.Settings.codec_b_priority_ch_cmx,                          //сохраненное значение переменной;
    (void*)&DataNew.Settings.codec_b_priority_ch_cmx,                          //новое значение переменных;
    (void*)&DataDef.Settings.codec_b_priority_ch_cmx,                          //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.codec_b_priority_ch_cmx,                          //значение переменной во flash; 
    "Приоритет голосового канала cmx кодека B по умолчанию (1..30)"         //описание переменных
  },  
  
  {//    uint32_t codec_b_mask_source_soft_port;   /* Маска портов разрешенных для приема в данный порт из програмного роутера */ 
    CODEC_SECT,   						                //номер секции
    VarID(CODEC_Type,CODEC_B_port,GET_CODEC_MASK_SOURSE_SOFT_PORT,0),           //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT32,						                        //тип переменной;
    CODEHEX,        					                        //тип вывода на экран;
    (void*)&DataSto.Settings.codec_b_mask_source_soft_port,	                //сохраненное значение переменной;
    (void*)&DataNew.Settings.codec_b_mask_source_soft_port,	                //новое значение переменных;
    (void*)&DataDef.Settings.codec_b_mask_source_soft_port,	                //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.codec_b_mask_source_soft_port,                     //значение переменной во flash; 
    "Маска портов разрешенных для приема в порт кодека B из програмного роутера"//описание переменных
  },    
  
  {//    uint32_t codec_b_mask_chanel_soft_port;   /* Маска каналов разрешенных для приема в данный порт из програмного роутера */
    CODEC_SECT,   						                //номер секции
    VarID(CODEC_Type,CODEC_B_port,GET_CODEC_MASK_CHANEL_SOFT_PORT,0),           //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT32,						                        //тип переменной;
    CODEHEX,        					                        //тип вывода на экран;
    (void*)&DataSto.Settings.codec_b_mask_chanel_soft_port,	                //сохраненное значение переменной;
    (void*)&DataNew.Settings.codec_b_mask_chanel_soft_port,	                //новое значение переменных;
    (void*)&DataDef.Settings.codec_b_mask_chanel_soft_port,	                //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.codec_b_mask_chanel_soft_port,                     //значение переменной во flash; 
    "Маска каналов разрешенных для приема в порт кодека B из програмного роутера"//описание переменных
  }, 
    
  /*========================= Настройки кодека канала C ========================*/
  {//    uint8_t  codec_c_chanel;                  /* Номер канала закреленный за кодеком */ 
    CODEC_SECT,   						                //номер секции
    VarID(CODEC_Type,CODEC_C_port,GET_CODEC_CHANEL,0),                          //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT8,					                                //тип переменной;
    CODEDEC,          				                                //тип вывода на экран;
    (void*)&DataSto.Settings.codec_c_chanel,                                    //сохраненное значение переменной;
    (void*)&DataNew.Settings.codec_c_chanel,                                    //новое значение переменных;
    (void*)&DataDef.Settings.codec_c_chanel,                                    //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.codec_c_chanel,                                    //значение переменной во flash; 
    "Номер канала закрепленный за кодеком C"                                     //описание переменных
  },   
  
  {//    uint8_t  codec_c_priority_ch_mic;         /* Приоритет голосового канала микрофона по умолчанию (1..30) */ 
    CODEC_SECT,   						                //номер секции
    VarID(CODEC_Type,CODEC_C_port,GET_CODEC_PRIORITY_CH_PCM,0),                 //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT8,					                                //тип переменной;
    CODEDEC,          				                                //тип вывода на экран;
    (void*)&DataSto.Settings.codec_c_priority_ch_pcm,                           //сохраненное значение переменной;
    (void*)&DataNew.Settings.codec_c_priority_ch_pcm,                           //новое значение переменных;
    (void*)&DataDef.Settings.codec_c_priority_ch_pcm,                           //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.codec_c_priority_ch_pcm,                           //значение переменной во flash; 
    "Приоритет голосового канала pcm кодека C по умолчанию (1..30)"       //описание переменных
  },  
  
  {//    uint8_t  codec_c_priority_ch_spkr;        /* Приоритет голосового канала спикера по умолчанию   (1..30) */ 
    CODEC_SECT,   						                //номер секции
    VarID(CODEC_Type,CODEC_C_port,GET_CODEC_PRIORITY_CH_CMX,0),                //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT8,					                                //тип переменной;
    CODEDEC,          				                                //тип вывода на экран;
    (void*)&DataSto.Settings.codec_c_priority_ch_cmx,                          //сохраненное значение переменной;
    (void*)&DataNew.Settings.codec_c_priority_ch_cmx,                          //новое значение переменных;
    (void*)&DataDef.Settings.codec_c_priority_ch_cmx,                          //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.codec_c_priority_ch_cmx,                          //значение переменной во flash; 
    "Приоритет голосового канала cmx кодека C по умолчанию (1..30)"         //описание переменных
  },  
  
  {//    uint32_t codec_c_mask_source_soft_port;   /* Маска портов разрешенных для приема в данный порт из програмного роутера */ 
    CODEC_SECT,   						                //номер секции
    VarID(CODEC_Type,CODEC_C_port,GET_CODEC_MASK_SOURSE_SOFT_PORT,0),           //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT32,						                        //тип переменной;
    CODEHEX,        					                        //тип вывода на экран;
    (void*)&DataSto.Settings.codec_c_mask_source_soft_port,	                //сохраненное значение переменной;
    (void*)&DataNew.Settings.codec_c_mask_source_soft_port,	                //новое значение переменных;
    (void*)&DataDef.Settings.codec_c_mask_source_soft_port,	                //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.codec_c_mask_source_soft_port,                     //значение переменной во flash; 
    "Маска портов разрешенных для приема в порт кодека C из програмного роутера"//описание переменных
  },    
  
  {//    uint32_t codec_c_mask_chanel_soft_port;   /* Маска каналов разрешенных для приема в данный порт из програмного роутера */
    CODEC_SECT,   						                //номер секции
    VarID(CODEC_Type,CODEC_C_port,GET_CODEC_MASK_CHANEL_SOFT_PORT,0),           //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT32,						                        //тип переменной;
    CODEHEX,        					                        //тип вывода на экран;
    (void*)&DataSto.Settings.codec_c_mask_chanel_soft_port,	                //сохраненное значение переменной;
    (void*)&DataNew.Settings.codec_c_mask_chanel_soft_port,	                //новое значение переменных;
    (void*)&DataDef.Settings.codec_c_mask_chanel_soft_port,	                //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.codec_c_mask_chanel_soft_port,                     //значение переменной во flash; 
    "Маска каналов разрешенных для приема в порт кодека C из програмного роутера"//описание переменных
  }, 
  /* Настройки общие для всех кодеков */                                      
  {  //    uint8_t  codec_nmask_transl_codec;        /* Hомер текущей маски ID_RS пакетов трансляции в порт кодека  */
    CODEC_SECT,   						                //номер секции
    VarID(CODEC_Type,CODEC_A_port,GET_CODEC_NMASK_TRANSL,0),                 //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT8,					      	                        //тип переменной;
    CODEDEC,              			      	                        //тип вывода на экран;
    (void*)&DataSto.Settings.codec_nmask_transl_codec,                          //сохраненное значение переменной;
    (void*)&DataNew.Settings.codec_nmask_transl_codec,                          //новое значение переменных;
    (void*)&DataDef.Settings.codec_nmask_transl_codec,                          //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.codec_nmask_transl_codec,                          //значение переменной во flash; 
    "Hомер текущей маски ID_RS пакетов трансляции в порт кодека"                //описание переменных
  },      

  {  //    uint32_t codec_mask_inpup_port_id_codec;  /* Маска ID портов разрешенных для приема в порт кодека        */   
    CODEC_SECT,   						                //номер секции
    VarID(CODEC_Type,CODEC_A_port,GET_CODEC_MASK_INPUT_PORT_ID,0),            //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT32,						                        //тип переменной;
    CODEHEX,           					                        //тип вывода на экран;
    (void*)&DataSto.Settings.codec_mask_inpup_port_id_codec,                    //сохраненное значение переменной;
    (void*)&DataNew.Settings.codec_mask_inpup_port_id_codec,                    //новое значение переменных;
    (void*)&DataDef.Settings.codec_mask_inpup_port_id_codec,                    //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.codec_mask_inpup_port_id_codec,                    //значение переменной во flash; 
    "Маска ID портов разрешенных для приема в порт кодека"                      //описание переменных
  }, 

  /* Настройки аналогового кодека     */   
  {//0,// uint8_t   analog_codec_mode;                  /* Режим работы аналогового кодека     */
    ANALOG_SECT,   						                //номер секции
    VarID(ANALOG_Type,ANALOG_port,GET_ANALOG_MODE,0),                          //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT8,					                                //тип переменной;
    CODEDEC,          				                                //тип вывода на экран;
    (void*)&DataSto.Settings.analog_codec_mode,                                    //сохраненное значение переменной;
    (void*)&DataNew.Settings.analog_codec_mode,                                    //новое значение переменных;
    (void*)&DataDef.Settings.analog_codec_mode,                                    //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.analog_codec_mode,                                    //значение переменной во flash; 
    "Режим работы аналогового кодека"                                     //описание переменных
  },    
  
  {// 5,//uint8_t   analog_codec_chanel;                /* Номер канала закреленный за кодеком */     
    ANALOG_SECT,   						                //номер секции
    VarID(ANALOG_Type,ANALOG_port,GET_ANALOG_CHANEL,0),                          //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT8,					                                //тип переменной;
    CODEDEC,          				                                //тип вывода на экран;
    (void*)&DataSto.Settings.analog_codec_chanel,                                    //сохраненное значение переменной;
    (void*)&DataNew.Settings.analog_codec_chanel,                                    //новое значение переменных;
    (void*)&DataDef.Settings.analog_codec_chanel,                                    //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.analog_codec_chanel,                                    //значение переменной во flash; 
    "Номер канала закреленный за кодеком"                                     //описание переменных
  },   
  
  {// 5,//uint8_t   analog_codec_priority_discrt;       /* Приоритет голосового дискретного канала по умолчанию   (1..5) */ 
    ANALOG_SECT,   						                //номер секции
    VarID(ANALOG_Type,ANALOG_port,GET_ANALOG_PRIORITY_DISCRET,0),                 //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT8,					                                //тип переменной;
    CODEDEC,          				                                //тип вывода на экран;
    (void*)&DataSto.Settings.analog_codec_priority_discrt,                           //сохраненное значение переменной;
    (void*)&DataNew.Settings.analog_codec_priority_discrt,                           //новое значение переменных;
    (void*)&DataDef.Settings.analog_codec_priority_discrt,                           //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.analog_codec_priority_discrt,                           //значение переменной во flash; 
    "Приоритет голосового дискретного канала по умолчанию   (1..5)"       //описание переменных
  },  
  
  {//5,//uint8_t   analog_codec_priority_analog;       /* Приоритет голосового аналогового канала по умолчанию   (1..5) */
    ANALOG_SECT,   						                //номер секции
    VarID(ANALOG_Type,ANALOG_port,GET_ANALOG_PRIORITY_ANALOG,0),                //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT8,					                                //тип переменной;
    CODEDEC,          				                                //тип вывода на экран;
    (void*)&DataSto.Settings.analog_codec_priority_analog,                          //сохраненное значение переменной;
    (void*)&DataNew.Settings.analog_codec_priority_analog,                          //новое значение переменных;
    (void*)&DataDef.Settings.analog_codec_priority_analog,                          //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.analog_codec_priority_analog,                          //значение переменной во flash; 
    "Приоритет голосового аналогового канала по умолчанию   (1..5)"         //описание переменных
  },  
  
  {// Bit_SoftPID( SPICS_ANALOG_SoftPID ) | Bit_SoftPID( UDPGate_SoftPID ) | Bit_SoftPID( LOOP_SoftPID ) | Bit_SoftPID( ANALOG_A_SoftPID ) | Bit_SoftPID( ANALOG_B_SoftPID ) | Bit_SoftPID( ANALOG_C_SoftPID );//uint32_t  analog_codec_mask_source_soft_port; /* Маска портов разрешенных для приема в данный порт из програмного роутера 
    ANALOG_SECT,   						                //номер секции
    VarID(ANALOG_Type,ANALOG_port,GET_ANALOG_MASK_SOURSE_SOFT_PORT,0),           //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT32,						                        //тип переменной;
    CODEHEX,        					                        //тип вывода на экран;
    (void*)&DataSto.Settings.analog_codec_mask_source_soft_port,	                //сохраненное значение переменной;
    (void*)&DataNew.Settings.analog_codec_mask_source_soft_port,	                //новое значение переменных;
    (void*)&DataDef.Settings.analog_codec_mask_source_soft_port,	                //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.analog_codec_mask_source_soft_port,                     //значение переменной во flash; 
    "Маска портов разрешенных для приема в данный порт из програмного роутера"//описание переменных
  },    
  
  {// BitChID(5);//uint32_t  analog_codec_mask_chanel_soft_port; /* Маска каналов разрешенных для приема в данный порт из програмного роутера */
    ANALOG_SECT,   						                //номер секции
    VarID(ANALOG_Type,ANALOG_port,GET_ANALOG_MASK_CHANEL_SOFT_PORT,0),           //идентификатор переменной
    ETH_R|ETH_W, 	                                                        //тип доступа к переменной
    UINT32,						                        //тип переменной;
    CODEHEX,        					                        //тип вывода на экран;
    (void*)&DataSto.Settings.analog_codec_mask_chanel_soft_port,	                //сохраненное значение переменной;
    (void*)&DataNew.Settings.analog_codec_mask_chanel_soft_port,	                //новое значение переменных;
    (void*)&DataDef.Settings.analog_codec_mask_chanel_soft_port,	                //значение переменной по умолчанию;
    (void*)&DataFLH.Settings.analog_codec_mask_chanel_soft_port,                     //значение переменной во flash; 
    "Маска каналов разрешенных для приема в данный порт из програмного роутера"//описание переменных
  },  
  //================ не удалять пустую переменную - добавлять новые перед ней ============
  // Пустая переменная для отслеживания завершения массива
  {		STOP_SECT,   																//номер секции
  0,	
  0,
  UINT8,																			
  CODEDEC,        														
  (void*)NULL,		
  (void*)NULL,		
  (void*)NULL,		
  "Free var"			
  }			
};

/**
  * @brief  функция подсчета переменных в массиве SetVarArray.
  * @param  None
  * @retval uint16_t - число переменных.
  */
uint16_t SizeSetVarArray(void)
{
  return sizeof(SetVarArray) / sizeof(VAR_typedef_t); 			
}

/**
  * @brief  Функция для восстанавливления значение по умолчанию в области редактирования для всех переменных текущей секции.
  * @param  section_t num_sect - номер секции для которой необходимо установить значение по умолчанию
  * @retval none
  */
void SecDefaultToNew(section_t num_sect)
{
  uint16_t var_cnt_num;
  uint16_t var_cnt_max;	
  
  /* получаем размер массива */
  var_cnt_max = SizeSetVarArray();
  /* цикл по всем переменным массива SetVarArray */
  for(var_cnt_num = 0; var_cnt_num < var_cnt_max; var_cnt_num++)
  {
    /* если текущая переменная - из данной секции обновляем редактируемое значение */
    if (SetVarArray[var_cnt_num].number_section == num_sect)
    {
      /* копирование значения по умолчанию в зону редактирования */ 
      DefaultToNew(var_cnt_num);  
    }	
  }
}

/**
  * @brief  функция копирования значения по умолчанию в зону редактирования  
  * @param  uint16_t порядковый номер переменной
  * @retval none 
  *
  */
void DefaultToNew(uint16_t NumberVar)
{
  //анализ типа переменой
  switch (SetVarArray[NumberVar].type_var)
  {
  case IP_TYPE:
    //преобразуем строковое значение переменной в числовое
    *(uint8_t*)SetVarArray[NumberVar].new_var = *(uint8_t*)SetVarArray[NumberVar].default_var;
    *(uint8_t*)SetVarArray[NumberVar + 1].new_var = *(uint8_t*)SetVarArray[NumberVar + 1].default_var;			
    *(uint8_t*)SetVarArray[NumberVar + 2].new_var = *(uint8_t*)SetVarArray[NumberVar + 2].default_var;			
    *(uint8_t*)SetVarArray[NumberVar + 3].new_var = *(uint8_t*)SetVarArray[NumberVar + 3].default_var;			
    break;			
  case MAC_TYPE:
    //преобразуем строковое значение переменной в числовое
    *(uint8_t*)SetVarArray[NumberVar].new_var = *(uint8_t*)SetVarArray[NumberVar].default_var;
    *(uint8_t*)SetVarArray[NumberVar + 1].new_var = *(uint8_t*)SetVarArray[NumberVar + 1].default_var;			
    *(uint8_t*)SetVarArray[NumberVar + 2].new_var = *(uint8_t*)SetVarArray[NumberVar + 2].default_var;			
    *(uint8_t*)SetVarArray[NumberVar + 3].new_var = *(uint8_t*)SetVarArray[NumberVar + 3].default_var;	
    *(uint8_t*)SetVarArray[NumberVar + 4].new_var = *(uint8_t*)SetVarArray[NumberVar + 4].default_var;			
    *(uint8_t*)SetVarArray[NumberVar + 5].new_var = *(uint8_t*)SetVarArray[NumberVar + 5].default_var;			
    break;			
  case INT8:
    //преобразуем строковое значение переменной в числовое
    *(int8_t*)SetVarArray[NumberVar].new_var = *(int8_t*)SetVarArray[NumberVar].default_var;
    break;
  case UINT16:
    //преобразуем строковое значение переменной в числовое
    *(uint16_t*)SetVarArray[NumberVar].new_var = *(uint16_t*)SetVarArray[NumberVar].default_var;
    break;
  case INT16:
    //преобразуем строковое значение переменной в числовое
    *(int16_t*)SetVarArray[NumberVar].new_var =*(int16_t*)SetVarArray[NumberVar].default_var ;
    break;
  case UINT32:
    //преобразуем строковое значение переменной в числовое
    *(uint32_t*)SetVarArray[NumberVar].new_var = *(uint32_t*)SetVarArray[NumberVar].default_var;
    break;
  case INT32:
    //преобразуем строковое значение переменной в числовое
    *(int32_t*)SetVarArray[NumberVar].new_var = *(int32_t*)SetVarArray[NumberVar].default_var;
    break;
  case FLOAT:
  case BOOL:
  case CHARS:
  case STRING:
  default:
    break;
  }
}		

/************************ (C) COPYRIGHT DEX *****END OF FILE****/
