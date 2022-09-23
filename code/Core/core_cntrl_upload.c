/**
  ******************************************************************************
  * @file    core_cntrl_upload.c
  * @author  Trembach Dmitry
  * @version V1.2.0
  * @date    21-06-2019
  * @brief   файл с функциями ядра для обновления програмного обеспечения
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2019 DataExpress</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "router_streams.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Core.h"
#include "core_cntrl_upload.h"
#include "loader_settings.h"
#include "settings.h"
  
#define _CMD_RESET_             (0x88)
#define _CMD_RESET_RS_          (0x80)
#define _CMD_WRITE_EEPROM_      (0x81)
#define _CMD_DEFAULT_           (0x82)


/**
  * @brief Функция формирования шапки ответа на запрос обновления програмного обеспечения
  * @param uint8_t data_size - размер передаваемых данных
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  *
  * @retval none
  */
void GenRespUploadBox(core_struct_t* core_st, uint8_t data_lenght)
{
  // получаем указатель на буфер команды ответа обновления програмного обеспечения
  rs_upload_box_t  *RespUpload = &(core_st->req_upload_tx);    
  
  // Формирование шапки запроса конфигурирования
  RespUpload->rs_head.pre = 0xAA55;                                             // Преамбула  0x55 0xAA
  RespUpload->rs_head.lenght = data_lenght + sizeof(head_box_t) + sizeof(RespUpload->upload_box.factory_addr) - SIZE_CRC;       // Длина пакета 
  RespUpload->rs_head.id = ID_BOOTLOADER_RESP;                                  // ID ответ команды конфигурации
  RespUpload->rs_head.dest = 0xFFFF;                                            // Адрес получателя
  RespUpload->rs_head.src = DataLoaderSto.Settings.phy_adr;                     // Устанавливаем свой физический адрес источника
  // Физический адрес источника и Cчетчик неприрывности пакетов 0..255 заполняется функцией update_rs_tx_box
  RespUpload->rs_head.reserv = 0x00;     //
  RespUpload->rs_head.status_box = 0x00; //
  // Заполнение шапки настроек пакета
  RespUpload->upload_box.factory_addr = DataLoaderSto.Settings.phy_adr;         // Адрес получателя   
  
  // Обновление RS пакета перед отправкой
  update_rs_tx_box(&(core_st->data_tx),&(core_st->contic_tx_box));
  
  // Отправляетм пакет в роутер
  if (core_st->xQueue_core_router != NULL)
  {
    // отправить пакет в роутер
    xQueueSend ( core_st->xQueue_core_router, ( void * )&(core_st->data_tx) , ( TickType_t ) 0 );
  } 
}

/**
  * @brief Функция формирования шапки ответа на запрос обновления програмного обеспечения
  * @param uint8_t data_size - размер передаваемых данных
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  *
  * @retval none
  */
void GenRespRFUploadBox(core_struct_t* core_st, uint8_t data_lenght)
{
  
  // Переменная дляы указателя на буфер запроса обновления програмного обеспечения
  RSFrameUPLOAD_t  *ReqUpload = &(core_st->req_nwk_upload_rx); 
  // Переменная дляы указателя на буфер для формирования ответа на запрос обновления програмного обеспечения
  RSFrameUPLOAD_t  *RespUpload = &(core_st->req_nwk_upload_tx);

  /* Заполнение шапки RS пакета */
  RespUpload->rs_head.pre = 0xAA55;                                                              /* Преамбула  0x55 0xAA                                                 */
  /* Длина пакета (включая контрольную сумму, не включая преамбулу)       */
  RespUpload->rs_head.lenght = data_lenght + sizeof(head_box_t) + sizeof(head_setup_t) + (1 + sizeof(NwkFrameHeader_t)) + SIZE_ID_BOX - SIZE_CRC;     
  RespUpload->rs_head.id = ID_BOOTLOADER_RF_RESP;                                                     /* Идентификатор пакета                                                 */
  RespUpload->rs_head.dest = 0xFFFF;                                                             /* Физический адрес получателя                                          */
  RespUpload->rs_head.src = DataLoaderSto.Settings.phy_adr;                                      /* Физический адрес источника                                           */
  RespUpload->rs_head.cnt = 00;                                                                  /* Cчетчик неприрывности пакетов 0..255                                 */
  RespUpload->rs_head.reserv = 0x00;     //                                                      /* Резерв                                                               */
  RespUpload->rs_head.status_box = 0x00; //
  /* длинна пакета модбас */
  RespUpload->NwkFrameUpload.size = RespUpload->rs_head.lenght - sizeof(head_setup_t) - SIZE_PRE - SIZE_CRC;  
  /* Формирование шапки пакета по RF  */
  /* * заголовок MAC в посылке *      */
  RespUpload->NwkFrameUpload.header.macFcf = 0x8841;                                             /* frame control                                                        */
  RespUpload->NwkFrameUpload.header.macSeq = core_st->contic_tx_box;                             /* sequence number                                                      */
  RespUpload->NwkFrameUpload.header.macDstPanId = 0xABCD;                                        /* PANID кому предназначались данные                                    */
  RespUpload->NwkFrameUpload.header.macDstAddr = 0xFFFF;                                         /* Адрес кому предназначались данные                                    */
  RespUpload->NwkFrameUpload.header.macSrcAddr = DataSto.Settings.mac_adr;                       /* Адрес от кого предавались данные                                     */
    
  /* * заголовок NWK в посылке (входит в PAYLOAD посылки по IEEE  802.15.4) * */        
  RespUpload->NwkFrameUpload.header.id1 = _NWK_DATA_BOOT_RESP_;                                  /* ID1 пакета                                                           */
  RespUpload->NwkFrameUpload.header.id2 = (uint8_t)~(unsigned)_NWK_DATA_BOOT_RESP_;              /* ID2 пакета                                                           */
  RespUpload->NwkFrameUpload.header.nwkFcf_u.val = 0x12;                                         /* * Поле настройки фрейма *                                            */
    
  RespUpload->NwkFrameUpload.header.nwkSeq = core_st->contic_tx_box;                             /* идентификатор последовательности кадра                               */
  RespUpload->NwkFrameUpload.header.nwkSrcPanId = 0x01;                                          /* Сетевой PANID источника (он же зона для ретрасляции)                 */
  RespUpload->NwkFrameUpload.header.nwkDstPanId = 0xFF;                                          /* Сетевой PANID назначения (он же зона для ретрасляции)                */
  RespUpload->NwkFrameUpload.header.nwkDstRouterAddr = 0xFF;                                     /* Адрес роутера которому предназначаются данные                        */
  RespUpload->NwkFrameUpload.header.nwkSrcHop = 0xFF;                                            /* Хоп источника данных                                                 */
  RespUpload->NwkFrameUpload.header.nwkOwnHop = 0xFF;                                            /* Собственный хоп                                                      */
  RespUpload->NwkFrameUpload.header.nwkSrcAddr = DataSto.Settings.mac_adr;                       /* Адрес источника                                                      */
  RespUpload->NwkFrameUpload.header.nwkDstAddr = 0xFFFF;                                         /* Адрес назначения                                                     */
          
  RespUpload->NwkFrameUpload.header.nwkSrcEndpoint = 0x1;                                        /* Endpoint источника                                                   */
  RespUpload->NwkFrameUpload.header.nwkDstEndpoint = 0x1;                                        /* Endpoint  назначения                                                 */
    
  RespUpload->NwkFrameUpload.header.nwk_count_routing = 0x00;                                    /* Счётчик маршрутизаций                                                */
  RespUpload->NwkFrameUpload.header.nwk_src_factory_addr = DataLoaderSto.Settings.phy_adr;       /* Заводской адрес истоника текущей посылки                             */
  RespUpload->NwkFrameUpload.header.nwk_own_factory_addr = DataLoaderSto.Settings.phy_adr;       /* Заводской адрес инициатора посылки                                   */
  RespUpload->NwkFrameUpload.header.reserv1 = 0x00;                                              /* резерв 1                                                             */
  RespUpload->NwkFrameUpload.header.reserv2 = 0x0000;                                            /* резерв 2                                                             */
   
  /* установка  физического адреса получателя пакета модбас */ 
  RespUpload->NwkFrameUpload.dest_addr = 0xFFFF;  
  /* установка физического адреса источника пакета модбас   */ 
  RespUpload->NwkFrameUpload.src_addr = DataLoaderSto.Settings.phy_adr; 
  /* установка идентификатора соединения */
  RespUpload->NwkFrameUpload.index_connect = ReqUpload->NwkFrameUpload.index_connect;
  // Адрес получателя
  RespUpload->NwkFrameUpload.upload_box.factory_addr = DataLoaderSto.Settings.phy_adr;            
  
  // Обновление RS пакета перед отправкой
  update_rs_tx_box((router_box_t*)RespUpload,&(core_st->contic_tx_box));
  
  // Отправляетм пакет в роутер
  if (core_st->xQueue_core_router != NULL)
  {
    // отправить пакет в роутер
    xQueueSend ( core_st->xQueue_core_router, ( void * )&(core_st->data_tx) , ( TickType_t ) 0 );
  } 
}

/**
  * @brief Функция обработка обновления програмного обеспечения
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  * @param  upload_box_t* ReqUpload - указатель на буфер запроса бновления програмного обеспечения
  * @param  upload_box_t* RespUpload - указатель на буфер для формирования ответа на запрос бновления програмного обеспечения
  * @param  uint8_t* data_lenght_resp - указатель на переменую длинны ответа
  *
  * @retval none
  */
void ProcessingUpload(core_struct_t* core_st, upload_box_t* ReqUpload, upload_box_t* RespUpload , uint8_t* data_lenght_resp)
{
  // Обработка полученных команд 
  // Анализируем ID пакета и вызываем соответствующего обработчика
  switch (ReqUpload->data_upload[0]) 
  {
    // Обработка комманды сброс   
  case _CMD_RESET_:
  case _CMD_RESET_RS_:    
    //програмный сброс
    // Устанавливаем значени для перезапуска
    *CodeAppStart = KODE_RESTART;
    // перезапуск контроллера
    NVIC_SystemReset();
    break;  
    
    // Обработка комманды установки параметров по умолчанию   
  case _CMD_DEFAULT_:
    //записываем в структуру параметры по умолчанию
    Copy_Settings((FlashSettingsTypeDef*)&DataDef,&DataNew);
    break;  
    
    // Обработка комманды записи настроек в EEPROM   
  case _CMD_WRITE_EEPROM_:
    // Функция записи отредактированных параметров в Flash память 
    Write_Settings(&DataNew);
    break;      
    
    //отработка пакета ответ ошибка   
  default:
    // отработка пакета ответ ошибка 
    /* установить идентификатор команды пакет ответа - ошибка соединения */					
    RespUpload->data_upload[0] = 0xFF;
    /* сообщить об ошибке адреса */	
    RespUpload->data_upload[1] = 0x20;
    /* установка длинны сообщения */	
    *data_lenght_resp = 2;	
    break;
  }
  /* ответа нет */
  return;
}

/**
  * @brief Функция обработка запросов обновления програмного обеспечения
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  *
  * @retval bool true  - пакет обработан
  *              false - пакет не обработан
  */
bool ProcessingUploadBox(core_struct_t* core_st)
{
  // Переменна длинны ответа
  uint8_t data_lenght_resp = 0;
  // Переменная дляы указателя на буфер запроса обновления програмного обеспечения
  upload_box_t  *ReqUpload;
  // Переменная дляы указателя на буфер для формирования ответа на запрос обновления програмного обеспечения
  upload_box_t  *RespUpload;
  
  // Проверка ID RS Запрос команд конфигурации 
  switch(core_st->data_rx.id)
  {
  case ID_BOOTLOADER_REQ:
    // получаем указатель на буфер запроса конфигурации
    ReqUpload = &(core_st->req_upload_rx.upload_box); 
    // получаем указатель на буфер для формирования ответа на запрос конфигурации
    RespUpload = &(core_st->req_upload_tx.upload_box); 
    // Проверка PHY_Addr
    if ((ReqUpload->factory_addr) != DataLoaderSto.Settings.phy_adr) return false;    
    /* Функция обработка обновления програмного обеспечения */ 
    ProcessingUpload( core_st, ReqUpload, RespUpload, &data_lenght_resp );
    // формирования шапки ответа на запрос конфигурирования и отправка его в роутер
    GenRespUploadBox( core_st, data_lenght_resp); 
    /* пакет обработан */
    return true;    
    
  case ID_BOOTLOADER_RF_REQ:
    // получаем указатель на буфер запроса конфигурации
    ReqUpload = &(core_st->req_nwk_upload_rx.NwkFrameUpload.upload_box); 
    // получаем указатель на буфер для формирования ответа на запрос конфигурации
    RespUpload = &(core_st->req_nwk_upload_tx.NwkFrameUpload.upload_box); 
    // Проверка PHY_Addr
    if ((ReqUpload->factory_addr) != DataLoaderSto.Settings.phy_adr) return false;    
    /* Функция обработка обновления програмного обеспечения */ 
    ProcessingUpload( core_st, ReqUpload, RespUpload, &data_lenght_resp );
    // формирования шапки ответа на запрос конфигурирования и отправка его в роутер
    GenRespRFUploadBox( core_st, data_lenght_resp);      
    /* пакет обработан */
    return true;   
  }
  /* пакет не обработан */
  return false;     
}
/******************* (C) COPYRIGHT 2019 DataExpress *****END OF FILE****/
