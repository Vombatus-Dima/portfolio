/**
  ******************************************************************************
  * @file    udp_spam_debug_gate.h
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UDP_SPAM_DEBUG_GATE_H
#define __UDP_SPAM_DEBUG_GATE_H

//#define UDP_SPAM_POS_DEBUG (1)

#if (UDP_SPAM_VOICE_DEBUG==1)
/**
  * @brief Функция для формирования диагностического сообщения по рассылке голосового пакета.
  * @param const char *header_mess - шапка сообщения 
  * @param RSFrameVoice_t* eth_box_voice - указатель на пакет ETH
  * @param uint16_t addr_phy - адрес источника или назначения 
  * @retval None
  */
void diag_mes_voice( const char *header_mess, RSFrameVoice_t* eth_box_voice, uint16_t addr_phy );
#else
#define diag_mes_voice( a, b, c );
#endif

#if (UDP_SPAM_POS_DEBUG==1)
/**
  * @brief Функция для формирования диагностического сообщения по рассылке пакета позиционирования.
  * @param const char *header_mess - шапка сообщения 
  * @param RSFrameTagNewPosV3_t* eth_box_pos - указатель на пакет ETH
  * @param uint16_t addr_phy - адрес источника или назначения 
  * @retval None
  */
void diag_mes_pos( const char *header_mess, RSFrameTagNewPosV3_t* eth_box_pos, uint16_t addr_phy );
#else
#define diag_mes_pos( a, b, c );
#endif

#endif /* __UDP_SPAM_DEBUG_GATE_H */
/************************ (C) COPYRIGHT DEX *****END OF FILE*******************/

