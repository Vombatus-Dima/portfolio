/**
  ******************************************************************************
  * @file    udp_spam_debug_gate.c
  * @author  Trembach D.N.
  * @version V1.0.0
  * @date    01-12-2020
  * @brief   Файл функций диагностики рассылки пакетов
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
#include "udp_spam_debug_gate.h" 

#if (UDP_SPAM_VOICE_DEBUG==1)
/**
  * @brief Функция для формирования диагностического сообщения по рассылке голосового пакета.
  * @param const char *header_mess - шапка сообщения 
  * @param RSFrameVoice_t* eth_box_voice - указатель на пакет ETH
  * @param uint16_t addr_phy - адрес источника или назначения 
  * @retval None
  */
void diag_mes_voice( const char *header_mess, RSFrameVoice_t* eth_box_voice, uint16_t addr_phy )
{
  printf("|%.16s|0x%.4X| |%.3d|\r\n",
         header_mess,
         addr_phy,
         eth_box_voice->NwkFrameVoice.header.nwkDstPanId);        /* Идентификатор канала */
}
#endif

#if (UDP_SPAM_POS_DEBUG==1)
/**
  * @brief Функция для формирования диагностического сообщения по рассылке пакета позиционирования.
  * @param const char *header_mess - шапка сообщения 
  * @param RSFrameTagNewPosV3_t* eth_box_pos - указатель на пакет ETH
  * @param uint16_t addr_phy - адрес источника или назначения 
  * @retval None
  */
void diag_mes_pos( const char *header_mess, RSFrameTagNewPosV3_t* eth_box_pos, uint16_t addr_phy )
{
  printf("|%.16s|0x%.4X|0x%.4X|%.3d|\r\n",
         header_mess,
         addr_phy,
         eth_box_pos->frame_tag_pos.own_physical_addr,
         eth_box_pos->frame_tag_pos.size);        /* Идентификатор канала */
}
#endif

/************************ (C) COPYRIGHT DEX *****END OF FILE************************/