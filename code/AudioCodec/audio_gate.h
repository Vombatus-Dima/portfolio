/**
  ******************************************************************************
  * @file    audio_gate.h
  * @author  Trembach D.N.
  * @version V2.3.0
  * @date    15-06-2020
  * @brief   Файл содержит функции шлюза аудиокодека 
  ******************************************************************************
  * @attention
  *
  * 
  ******************************************************************************
  */ 
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIO_GATE_H
#define __AUDIO_GATE_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "lwip/ip_addr.h"
#include "udp_socket.h"
#include "soft_router.h"
#include "codec_cmx.h"


/* Управляющие вводы - выводы */
/* Подключение выхода усилителя кодека к линии */
#define V_RX_SW_LINE_PIN                GPIO_Pin_6
#define V_RX_SW_LINE_GPIO_PORT          GPIOG
#define V_RX_SW_LINE_GPIO_CLK           RCC_AHB1Periph_GPIOG
#define V_RX_SW_LINE_HIGH()             ( V_RX_SW_LINE_GPIO_PORT->BSRRL = V_RX_SW_LINE_PIN )
#define V_RX_SW_LINE_LOW()              ( V_RX_SW_LINE_GPIO_PORT->BSRRH = V_RX_SW_LINE_PIN )
#define V_RX_SW_LINE_TGL()              ( V_RX_SW_LINE_GPIO_PORT->ODR ^=  V_RX_SW_LINE_PIN )

/* Отключение линии FXO*/
#define NUMBER_LINE_PIN                 GPIO_Pin_8
#define NUMBER_LINE_GPIO_PORT           GPIOC
#define NUMBER_LINE_GPIO_CLK            RCC_AHB1Periph_GPIOC
#define NUMBER_LINE_HIGH()              ( NUMBER_LINE_GPIO_PORT->BSRRL = POLARITY1_LINE_PIN )
#define NUMBER_LINE_LOW()               ( NUMBER_LINE_GPIO_PORT->BSRRH = POLARITY1_LINE_PIN )
#define NUMBER_LINE_TGL()               ( NUMBER_LINE_GPIO_PORT->ODR ^=  POLARITY1_LINE_PIN )

/* Вводы контроля полярности */
#define POLARITY1_LINE_PIN              GPIO_Pin_7
#define POLARITY1_LINE_GPIO_PORT        GPIOG
#define POLARITY1_LINE_GPIO_CLK         RCC_AHB1Periph_GPIOG

#define POLARITY2_LINE_PIN              GPIO_Pin_8
#define POLARITY2_LINE_GPIO_PORT        GPIOG
#define POLARITY2_LINE_GPIO_CLK         RCC_AHB1Periph_GPIOG


#define SIZE_SAMPLE_AUDIO_BUFF (960)    /* Число выборок в пакете */
#define N_SAMPLE_AUDIO_BUFF    (2)      /* Число пакетов в буфере */

typedef enum
{
  MODE_ANALOG_IN =   0x00, /* Режим дискретизации аналогового сигнала        */
  MODE_ANALOG_OUT =  0x01, /* Режим восстановления аналогового сигнала       */
  MODE_ANALOG_AUTO = 0x02  /* Режим автоматического переключения направления */
}analog_mode_e;

/* структура контроля голосового соединения */
typedef struct
{
  /*======================================= Переменные области общих данных ================================================*/   
  stream_status_t               codec_status_speaker;             /* статус режима вывода потока спикера                    */ 
  uint8_t                       codec_mic_priority;               /* Текущий приоритет канала микрофона                     */  
  uint8_t                       codec_speaker_priority;           /* Текущий приоритет канала спикера                       */ 
  uint16_t                      codec_cnt_mic_box;                /* Счетчик пакетов в голосовом сообщении в режиме mic     */ 
  /*======================================= Переменные настройки общие  ====================================================*/   
  uint16_t                      codec_phy_addr;                   /* Физический адрес шлюза/кодека по умолчанию             */ 
  uint8_t                       codec_ch_id;                      /* Идентификатор канала захвата                           */
  analog_mode_e                 analog_codec_mode;                /* Режим работы кодека                                    */
  /*========================================================================================================================*/  
  uint16_t                      audio_lock_src_phy_addr;          /* Физический адрес источника                             */
  uint16_t                      audio_lock_dest_phy_addr;         /* Физический адрес получателя                            */ 
  uint16_t                      audio_lock_codec_phy_addr;        /* Физический адрес шлюза/кодека                          */   
  uint8_t                       audio_lock_ch_id;                 /* Идентификатор канала                                   */  
  uint8_t                       audio_lock_ch_priority;           /* Приоритет канала                                       */
  /*========================================================================================================================*/ 
  uint8_t                       audio_ch_priority;                /* Приоритет канала интерфейса аудио по умолчанию         */ 
  /*========================================================================================================================*/  
  /*======================================= Переменные области данных pcm ==================================================*/   
  /*============================= Переменные идентификации захваченнного канала pcm ========================================*/  
  uint16_t                      pcm_lock_src_phy_addr;            /* Физический адрес источника                             */
  uint16_t                      pcm_lock_dest_phy_addr;           /* Физический адрес получателя                            */ 
  uint16_t                      pcm_lock_codec_phy_addr;          /* Физический адрес шлюза/кодека                          */   
  uint8_t                       pcm_lock_ch_id;                   /* Идентификатор канала                                   */  
  uint8_t                       pcm_lock_ch_priority;             /* Приоритет канала                                       */    
  /*======================================= Переменные настройки интерфейса pcm ============================================*/  
  uint8_t                       pcm_ch_priority;                  /* Приоритет канала интерфейса PCM по умолчанию           */   
  /*=======================  Собственные переменные идентификации порта роутера несжатых потоков ===========================*/
  uint8_t                       pcm_index_rd_box;                 /* Индекс для приема из буфер роутера                     */   
  soft_router_box_t*            pcm_pfifo_box;                    /* Переменная хранения указателя                          */  
  uint8_t                       pcm_source_port;                  /* Собственный индекс порта                               */
  uint32_t                      pcm_mask_index_port;              /* Маска доступных для приема портов                      */
  uint32_t                      pcm_mask_chanel;                  /* Маска доступных каналов                                */   
  uint8_t                       pcm_type_box;                     /* Формат данных пакета                                   */
  uint8_t                       pcm_cnt;                          /* Cчетчик неприрывности пакетов 0..255                   */
                                                                  
  uint32_t                      pcm_cnt_tx_box;                   /* Счетчик переданных пакетов                             */
  uint32_t                      pcm_cnt_tx_err;                   /* Счетчик принятых пакетов c ошибкой                     */   
  uint32_t                      pcm_cnt_rx_box;                   /* Счетчик принятых пакетов                               */ 
  uint32_t                      pcm_cnt_rx_err;                   /* Счетчик принятых пакетов c ошибкой                     */
  /*======================================= Буфер для хранения несжатых пакетов порта ======================================*/  
  pcm_box_t                     pcm_buf_box[MAX_BUF_RX_PCM_BOX];  /* буфер приема пакетов                                   */ 
  uint8_t                       pcm_rd_buf_index;                 /* индекс чтения из буфера приема пакетов                 */   
  uint8_t                       pcm_wr_buf_index;                 /* индекс записи в буфер приема пакетов                   */   
                                                                  
  uint8_t                       pcm_empty_interval_counter;       /* счетчик пустых интервалов */
  stream_status_t               pcm_status_wr;                    /* статус режима записи PCM в кодек                       */   
  /*======================================= Буфер памяти с которым работает контроллер ДМА =================================*/
  int16_t                       data_tx_i2s[N_SAMPLE_AUDIO_BUFF][SIZE_SAMPLE_AUDIO_BUFF];  /* Буфер для передачи в I2S      */
  int16_t                       data_rx_i2s[N_SAMPLE_AUDIO_BUFF][SIZE_SAMPLE_AUDIO_BUFF];  /* Буфер для приема из I2S       */
  /*========================================== Аппаратные ресурсы контроллера ДМА  =========================================*/
                                                      
  TimerHandle_t                 xSoftTimer;                       /* Програмный таймер периодического уведомления задачи    */
  TickType_t                    PeriodSoftTimer;                  /* Период уведомления задачи                              */
  uint32_t                      NotifiedValue;                    /* Содержимое сообщения полученного задачей               */
  
}audio_codec_gate_t;

/* Очередь сообщения для аудио кодека */
extern QueueHandle_t      x_audio_message; 
/* Указатель для уведомления задачи контроля данных аудио кодека */
extern TaskHandle_t       handle_codec_audio_task;
/* Создаем структуру контроля аудио кодека */
extern audio_codec_gate_t GateAudioCodec;

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */



/**
  * @brief  Функция создания задачи и организации шлюза аудио кодека
  * @param  None 
  * @retval None
  */
void Init_Gate_Audio_Codec(void);
  
#endif /* __AUDIO_GATE_H */
/************************ (C) COPYRIGHT DEX *****END OF FILE****/

