/**
  ******************************************************************************
  * @file    call_gate.h
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CALL_GATE_H
#define __CALL_GATE_H

#define CODE_PREAM  (0xEDB7)

//#define UDP_CALL_DEBUG            (1)

/* Описание структуры пакетов вызова */
typedef __packed struct
{
  uint16_t    pre;                      /* Преамбула  0xB7 0xED              */
  uint8_t     id_cmd;                   /* Идентификатор команды             */  
  uint8_t     call_cmd;                 /* Данные команды                    */  
  uint8_t     id_ch;                    /* Идентификатор канала              */
  uint16_t    addr_src;                 /* Физический адрес источника        */
  uint16_t    addr_dst;                 /* Физический адрес получателя       */
  uint16_t    addr_gate_src;            /* Физический адрес шлюза источника  */
  uint16_t    addr_gate_dst;            /* Физический адрес шлюза назначения */
  union
  {
    uint8_t     status_box;
    __packed struct
    {
      uint8_t   flag_eth_to_rs    : 1,  /* Флаги прохождения шлюзов */
                flag_rs_to_eth    : 1,  
                flag_rec_to_eth   : 1, 
                flag_eth_to_rec   : 1,                     
                reserv_bit        : 4;  
    };
  };
  uint8_t reserv;
}eth_call_box_t;

/* Описание структуры пакетов вызова */
typedef __packed struct
{
  uint16_t    pre;                      /* Преамбула  0xB7 0xED              */
  uint8_t     id_cmd;                   /* Идентификатор команды             */  
  uint8_t     call_cmd;                 /* Данные команды                    */  
  uint8_t     id_ch;                    /* Идентификатор канала              */
  uint16_t    addr_src;                 /* Физический адрес источника        */
  uint16_t    addr_dst;                 /* Физический адрес получателя       */
  uint16_t    addr_gate_src;            /* Физический адрес шлюза источника  */
  uint16_t    addr_gate_dst;            /* Физический адрес шлюза назначения */
}eth_call_rec_box_t;

/**
  * @brief Получили пакет формата call rs - преобразовали в call eth .
  * @param eth_call_box_t* eth_box_call - указатель на пакет ETH
  * @param RSFrameCall_t* FrameCall - указатель на пакет RS
  * @retval None
  */
void call_rs_to_eth( eth_call_box_t* eth_box_call, RSFrameCall_t* FrameCall );

/**
  * @brief Получили пакет формата call eth - преобразовали в call rs.
  * @param RSFrameCall_t* FrameCall - указатель на пакет RS
  * @param eth_call_box_t* eth_box_call - указатель на пакет ETH
  * @retval None
  */
void call_eth_to_rs( RSFrameCall_t* FrameCall, eth_call_box_t* eth_box_call );


#if (UDP_CALL_DEBUG==1)
/**
  * @brief Функция для формирования диагностического сообщения по рассылке команд вызова.
  * @param const char *header_mess - шапка сообщения 
  * @param eth_call_box_t* eth_box_call - указатель на пакет ETH
  * @param uint16_t addr_phy - адрес источника или назначения 
  * @retval None
  */
void diag_mes_call_eth( const char *header_mess, eth_call_box_t* eth_box_call, uint16_t addr_phy );

/**
  * @brief Функция для формирования диагностического сообщения по рассылке команд вызова.
  * @param const char *header_mess - шапка сообщения 
  * @param RSFrameCall_t* FrameCall - указатель на пакет RS
  * @param uint16_t addr_phy - адрес источника или назначения 
  * @retval None
  */
void diag_mes_call_rs( const char *header_mess, RSFrameCall_t* FrameCall, uint16_t addr_phy );
#else
#define diag_mes_call_eth( a, b, c );
#define diag_mes_call_rs( a, b, c );
#endif

#endif /* __UDP_VOICE_H */
/************************ (C) COPYRIGHT DEX *****END OF FILE*******************/

