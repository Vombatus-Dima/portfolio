/**
  ******************************************************************************
  * @file    call_gate.c
  * @author  Trembach D.N.
  * @version V1.0.0
  * @date    24-10-2020
  * @brief   Файл контроля команд вызова на шлюз 
  ******************************************************************************
  * @attention
  * 
  *
  *   
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "lwip/memp.h"
#include <stdio.h>
#include <string.h>
#include "stm32f4x7_eth.h"
#include "FreeRTOS.h"
#include "task.h"
#include "udp_voice.h"
#include "call_gate.h"
#include "Core.h"    
#include "settings.h"
#include "loader_settings.h"
#include "printf_dbg.h"    
#include "board.h" 
#include "call_gate.h" 

static uint8_t contic_call = 0;

/**
  * @brief Получили пакет формата call rs - преобразовали в call eth .
  * @param eth_call_box_t* eth_box_call - указатель на пакет ETH
  * @param RSFrameCall_t* FrameCall - указатель на пакет RS
  * @retval None
  */
void call_rs_to_eth( eth_call_box_t* eth_box_call, RSFrameCall_t* FrameCall )
{
  /* Заполнение пакета вызова eth*/
  eth_box_call->pre = CODE_PREAM;                                                               /* uint16_t  Преамбула  0xB7 0xED                        */
  eth_box_call->id_cmd = FrameCall->NwkFrameCall.header.id1;                                /* uint8_t   Идентификатор команды                       */
  eth_box_call->call_cmd = FrameCall->NwkFrameCall.call_cmd;                                /* uint8_t   Данные команды                              */ 
  eth_box_call->id_ch = FrameCall->NwkFrameCall.header.nwkDstPanId;                         /* uint8_t   Идентификатор канала                        */
  eth_box_call->addr_src = FrameCall->NwkFrameCall.header.macSrcAddr;                       /* uint16_t  Физический адрес источника                  */
  eth_box_call->addr_dst = FrameCall->NwkFrameCall.header.macDstAddr;                       /* uint16_t  Физический адрес получателя                 */
  eth_box_call->addr_gate_src = DataLoaderSto.Settings.phy_adr;                             /* uint16_t  Физический адрес шлюза источника            */
  eth_box_call->addr_gate_dst = 0xFFFF;                                                     /* uint16_t  Физический адрес шлюза назначения           */
  eth_box_call->status_box = FrameCall->rs_head.status_box;                                 /* uint8_t   статус передаваемого пакета                 */
  eth_box_call->reserv = FrameCall->rs_head.reserv;                                         /* uint8_t   резерв                                      */   
  
  /* установка адреса шлюза назначения в зависимости от команды  */  
  if ( eth_box_call->id_cmd == _RF_DATA_RMAV_CALL_DISP_)
  {
    /* чтение адреса диспетчера для команды 1 */
    if ( FrameCall->NwkFrameCall.call_cmd == 1)  eth_box_call->addr_gate_dst = DataSto.Settings.phy_addr_disp_1;
    /* чтение адреса диспетчера для команды 2 */
    if ( FrameCall->NwkFrameCall.call_cmd == 2)  eth_box_call->addr_gate_dst = DataSto.Settings.phy_addr_disp_2;   
  }
}

/**
  * @brief Получили пакет формата call eth - преобразовали в call rs.
  * @param RSFrameCall_t* FrameCall - указатель на пакет RS
  * @param eth_call_box_t* eth_box_call - указатель на пакет ETH
  * @retval None
  */
void call_eth_to_rs( RSFrameCall_t* FrameCall, eth_call_box_t* eth_box_call )
{
  /* Заполнение пакета вызова rs*/
  switch( eth_box_call->id_cmd )
  {
  case _RF_DATA_RMAV_RESP_CALL_DISP_:        
    FrameCall->rs_head.id = ID_REQ_DISP;                                                   /* Идентификатор пакета                                  */
    FrameCall->NwkFrameCall.header.id1 = _RF_DATA_RMAV_RESP_CALL_DISP_;                     /* ID1 пакета                                            */
    FrameCall->NwkFrameCall.header.id2 = (uint8_t)~(unsigned)_RF_DATA_RMAV_RESP_CALL_DISP_; /* ID2 пакета                                            */
    break;     
  case _RF_DATA_RMAV_CALL_DISP_:
    FrameCall->rs_head.id =ID_RING_DISP;                                                    /* Идентификатор пакета                                  */
    FrameCall->NwkFrameCall.header.id1 = _RF_DATA_RMAV_CALL_DISP_;                          /* ID1 пакета                                            */
    FrameCall->NwkFrameCall.header.id2 = (uint8_t)~(unsigned)_RF_DATA_RMAV_CALL_DISP_;      /* ID2 пакета                                            */  
  default:   
    break;              
  } 
  contic_call++;
  
  FrameCall->rs_head.cnt = contic_call;                                         /* Cчетчик неприрывности пакетов 0..255                            */
  FrameCall->NwkFrameCall.header.macSeq = contic_call;                          /* sequence number                                                 */
  FrameCall->NwkFrameCall.header.nwkSeq = contic_call;                          /* идентификатор последовательности кадра                          */ 
  
  /* Подготовка отправки подтверждения                                          */
  /* Формирование шапки пакета по RS                                            */
  FrameCall->rs_head.pre = 0xAA55;                                              /* Преамбула  0x55 0xAA                                            */
  FrameCall->rs_head.lenght = 0x2B;                                             /* Длина пакета (включая контрольную сумму, не включая преамбулу)  */
  FrameCall->rs_head.dest = 0xFFFF;                                             /* Физический адрес получателя                                     */
  FrameCall->rs_head.src = DataLoaderSto.Settings.phy_adr;                      /* Физический адрес источника                                      */
  FrameCall->rs_head.status_box = eth_box_call->status_box;
  FrameCall->rs_head.reserv = 0x00;  
  FrameCall->NwkFrameCall.size = 0x22;                                          /* длинна голосового пакета                                        */
  /* Формирование шапки пакета по RF  */
  /* * заголовок MAC в посылке *      */
  FrameCall->NwkFrameCall.header.macFcf = 0x8841;                               /* frame control                                                   */
  FrameCall->NwkFrameCall.header.macDstPanId = 0xABCD;                          /* PANID кому предназначались данные                               */
  FrameCall->NwkFrameCall.header.macDstAddr = eth_box_call->addr_dst;           /* Адрес кому предназначались данные                               */
  FrameCall->NwkFrameCall.header.macSrcAddr = DataLoaderSto.Settings.phy_adr;   /* Адрес от кого предавались данные                                */
  /* заголовок NWK в посылке (входит в PAYLOAD посылки по IEEE  802.15.4) */
  FrameCall->NwkFrameCall.header.nwkFcf_u.val = 0x12;                           /* * Поле настройки фрейма *                                       */
  FrameCall->NwkFrameCall.header.nwkSrcPanId = 0x01;                            /* Сетевой PANID источника (он же зона для ретрасляции)            */
  FrameCall->NwkFrameCall.header.nwkDstPanId = eth_box_call->id_ch;             /* Сетевой PANID назначения (он же зона для ретрасляции)           */
  FrameCall->NwkFrameCall.header.nwkDstRouterAddr = 0xFF;                       /* Адрес роутера которому предназначаются данные                   */
  FrameCall->NwkFrameCall.header.nwkSrcHop = 0x00;                              /* Хоп источника данных                                            */
  FrameCall->NwkFrameCall.header.nwkOwnHop = 0x00;                              /* Собственный хоп                                                 */
  FrameCall->NwkFrameCall.header.nwkSrcAddr = eth_box_call->addr_src;           /* Адрес источника                                                 */
  FrameCall->NwkFrameCall.header.nwkDstAddr = eth_box_call->addr_dst;           /* Адрес назначения                                                */
                                                                                
  FrameCall->NwkFrameCall.header.nwkSrcEndpoint = 0x1;                          /* Endpoint источника                                              */
  FrameCall->NwkFrameCall.header.nwkDstEndpoint = 0x1;                          /* Endpoint  назначения                                            */
  FrameCall->NwkFrameCall.header.nwk_count_routing = 0x00;                      /* Счётчик маршрутизаций                                           */
  FrameCall->NwkFrameCall.header.nwk_src_factory_addr = eth_box_call->addr_src; /* Заводской адрес истоника текущей посылки                        */
  FrameCall->NwkFrameCall.header.nwk_own_factory_addr = eth_box_call->addr_src; /* Заводской адрес инициатора посылки                              */
  FrameCall->NwkFrameCall.header.reserv1 = 0x00;                                /* резерв 1                                                        */
  FrameCall->NwkFrameCall.header.reserv2 = 0x0000;                              /* резерв 2                                                        */
  
  FrameCall->NwkFrameCall.call_cmd = eth_box_call->call_cmd;                    /*                                                                 */
}
  
#if (UDP_CALL_DEBUG==1)
/**
  * @brief Функция для формирования диагностического сообщения по рассылке команд вызова.
  * @param const char *header_mess - шапка сообщения 
  * @param eth_call_box_t* eth_box_call - указатель на пакет ETH
  * @param uint16_t addr_phy - адрес источника или назначения 
  * @retval None
  */
void diag_mes_call_eth( const char *header_mess, eth_call_box_t* eth_box_call, uint16_t addr_phy )
{
   printf("|%.16s|0x%.4X| |%.3d|%.3d|%.3d|%.3d| |0x%.4X|0x%.4X|0x%.4X|0x%.4X|\r\n",
            header_mess,
            addr_phy,
            eth_box_call->id_cmd,        /* Идентификатор команды             */
            eth_box_call->call_cmd,      /* Данные команды                    */ 
            eth_box_call->id_ch,         /* Идентификатор канала              */
            eth_box_call->status_box,    /* Cтатус передаваемого пакета       */  
            eth_box_call->addr_src,      /* Физический адрес источника        */
            eth_box_call->addr_dst,      /* Физический адрес получателя       */
            eth_box_call->addr_gate_src, /* Физический адрес шлюза источника  */
            eth_box_call->addr_gate_dst);/* Физический адрес шлюза назначения */             
}

/**
  * @brief Функция для формирования диагностического сообщения по рассылке команд вызова.
  * @param const char *header_mess - шапка сообщения 
  * @param RSFrameCall_t* FrameCall - указатель на пакет RS
  * @param uint16_t addr_phy - адрес источника или назначения 
  * @retval None
  */
void diag_mes_call_rs( const char *header_mess, RSFrameCall_t* FrameCall, uint16_t addr_phy )
{
   printf("|%.16s|0x%.4X| |%.3d|%.3d|%.3d|%.3d| |0x%.4X|0x%.4X|0x%.4X|0x%.4X|\r\n",
            header_mess,
            addr_phy,
            FrameCall->NwkFrameCall.header.id1,        /* Идентификатор команды             */
            FrameCall->NwkFrameCall.call_cmd,          /* Данные команды                    */ 
            FrameCall->NwkFrameCall.header.nwkDstPanId,/* Идентификатор канала              */
            FrameCall->rs_head.status_box,             /* Cтатус передаваемого пакета       */  
            FrameCall->NwkFrameCall.header.nwkSrcAddr, /* Физический адрес источника        */
            FrameCall->NwkFrameCall.header.macDstAddr, /* Физический адрес получателя       */
            0xFFFF,                                    /* Физический адрес шлюза источника  */
            0xFFFF);                                   /* Физический адрес шлюза назначения */        
}
#endif
/************************ (C) COPYRIGHT DEX *****END OF FILE************************/