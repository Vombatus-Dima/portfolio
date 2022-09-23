/**
  ******************************************************************************
  * @file    codec_cmx_c.c
  * @author  Trembach Dmitry
  * @version V1.0.0
  * @date    19-04-2020
  * @brief   Инициализация задачи контроля SPI кодека
  *
  ******************************************************************************
  * @attention
  * 
  * 
  *
  * <h2><center>&copy; COPYRIGHT 2020 DataExpress</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/  
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "codec_cmx.h"  
#include "codec_control.h"
#include "board.h"
#include "soft_router.h"

/**
  * @brief  Функция прописывания указателей на индивидуальные ресурсы pcm интерфейса кодека  
  * @param  None
  * @retval None
  */
void init_pcm_codec_c(void)
{
  /*========================================================================================================================================*/
  /*========================================================================================================================================*/ 
  /*----------------------------------------------------------------------------------------------------------------------------------------*/ 
  cmx_pcm[CODEC_C_PCM_CMX].SPI_PORT                 = SPI5;                   /* порт SPI PCM                                                   */ 
  cmx_pcm[CODEC_C_PCM_CMX].SPI_GPIO_AF              = GPIO_AF_SPI5;            
  cmx_pcm[CODEC_C_PCM_CMX].SPI_RCC_APB_Periph       = RCC_APB2Periph_SPI5;     
  cmx_pcm[CODEC_C_PCM_CMX].RCC_SPI_ClockCmd         = RCC_APB2PeriphClockCmd;  
  /*----------------------------------------------------------------------------------------------------------------------------------------*/ 
  cmx_pcm[CODEC_C_PCM_CMX].MOSI_Pin                 = GPIO_Pin_9;             /* вывод MOSI кодека                                              */ 
  cmx_pcm[CODEC_C_PCM_CMX].MOSI_PORT                = GPIOF;                   
  cmx_pcm[CODEC_C_PCM_CMX].MOSI_CLK_GPIO            = RCC_AHB1Periph_GPIOF;    
  cmx_pcm[CODEC_C_PCM_CMX].MOSI_PinSource           = GPIO_PinSource9;         
  /*----------------------------------------------------------------------------------------------------------------------------------------*/ 
  cmx_pcm[CODEC_C_PCM_CMX].MISO_Pin                 = GPIO_Pin_8;             /* вывод MISO кодека                                               */ 
  cmx_pcm[CODEC_C_PCM_CMX].MISO_PORT                = GPIOF;                   
  cmx_pcm[CODEC_C_PCM_CMX].MISO_CLK_GPIO            = RCC_AHB1Periph_GPIOF;    
  cmx_pcm[CODEC_C_PCM_CMX].MISO_PinSource           = GPIO_PinSource8;         
  /*----------------------------------------------------------------------------------------------------------------------------------------*/
  cmx_pcm[CODEC_C_PCM_CMX].SCK_Pin                  = GPIO_Pin_7;             /* вывод SCK кодека                                                */ 
  cmx_pcm[CODEC_C_PCM_CMX].SCK_PORT                 = GPIOF;                   
  cmx_pcm[CODEC_C_PCM_CMX].SCK_CLK_GPIO             = RCC_AHB1Periph_GPIOF;    
  cmx_pcm[CODEC_C_PCM_CMX].SCK_PinSource            = GPIO_PinSource7;         
  /*----------------------------------------------------------------------------------------------------------------------------------------*/
  cmx_pcm[CODEC_C_PCM_CMX].SEL_Pin                  = GPIO_Pin_6;             /* вывод SEL кодека                                                */ 
  cmx_pcm[CODEC_C_PCM_CMX].SEL_PORT                 = GPIOF;                   
  cmx_pcm[CODEC_C_PCM_CMX].SEL_CLK_GPIO             = RCC_AHB1Periph_GPIOF;    
  cmx_pcm[CODEC_C_PCM_CMX].SEL_PinSource            = GPIO_PinSource6;         
  /*-------------------------------------------------------------------------------------------------------------------------------------------*/ 
  /*--------------------------------------------------------------------------------------------------------------------------------------------*/  
  cmx_pcm[CODEC_C_PCM_CMX].DMA_PORT                 = DMA2;                   /*  порт DMA                                                       */  
  cmx_pcm[CODEC_C_PCM_CMX].RCC_AHBPeriph_DMA        = RCC_AHB1Periph_DMA2;    /*  тактирование DMA                                               */  
  cmx_pcm[CODEC_C_PCM_CMX].DMA_Channel_RX           = DMA_Channel_2;          /*  Канал DMA для SPI_RX                                           */ 
  cmx_pcm[CODEC_C_PCM_CMX].DMA_Stream_RX            = DMA2_Stream3;           /*  Поток DMA для SPI_RX                                           */  
  cmx_pcm[CODEC_C_PCM_CMX].DMA_Stream_RX_IRQn       = DMA2_Stream3_IRQn;      /*  Номер прерывания потока DMA для SPI_RX                         */ 
  cmx_pcm[CODEC_C_PCM_CMX].DMA_Channel_TX           = DMA_Channel_2;          /*  Канал DMA для SPI_TX                                           */ 
  cmx_pcm[CODEC_C_PCM_CMX].DMA_Stream_TX            = DMA2_Stream4;           /*  Поток DMA для SPI_TX                                           */  
  /*========================================================================================================================================*/ 
  /*========================================================================================================================================*/
  cmx_pcm[CODEC_C_PCM_CMX].index_codec              = CODEC_C_PCM_CMX;             
  /*----------------------------------------------------------------------------------------------------------------------------------------*/ 
  cmx_pcm[CODEC_C_PCM_CMX].CS_Pin                   = GPIO_Pin_0;             /* вывод CS кодека                                                */ 
  cmx_pcm[CODEC_C_PCM_CMX].CS_PORTx                 = GPIOE;                  /* указатель на базовый адреc используемого порта вывода CS кодека*/ 
  cmx_pcm[CODEC_C_PCM_CMX].CS_CLK_GPIO              = RCC_AHB1Periph_GPIOE;   /* включение тактирования вывода                                  */ 
  /*----------------------------------------------------------------------------------------------------------------------------------------*/ 
  cmx_pcm[CODEC_C_PCM_CMX].RESET_Pin                = GPIO_Pin_5;             /* вывод RESET кодека                                             */ 
  cmx_pcm[CODEC_C_PCM_CMX].RESET_PORTx              = GPIOB;                  /* указатель на базовый адреc используемого порта вывода RESET кодека*/
  cmx_pcm[CODEC_C_PCM_CMX].RESET_CLK_GPIO           = RCC_AHB1Periph_GPIOB;   /* включение тактирования вывода                                  */ 
  /*----------------------------------------------------------------------------------------------------------------------------------------*/ 
  cmx_pcm[CODEC_C_PCM_CMX].SYNC_Pin                 = GPIO_Pin_6;             /* вывод SYNC кодека                                              */ 
  cmx_pcm[CODEC_C_PCM_CMX].SYNC_PORTx               = GPIOB;                  /* указатель на базовый адреc используемого порта вывода SYNC кодека*/
  cmx_pcm[CODEC_C_PCM_CMX].SYNC_CLK_GPIO            = RCC_AHB1Periph_GPIOB;  /* включение тактирования вывода                                  */ 
  /*--------------------------------------------------------------------------------------------------------------------------------------------*/     
  cmx_pcm[CODEC_C_PCM_CMX].LED_MIC_Pin        = GPIO_Pin_5;                      /* вывод LED CMX                                                  */
  cmx_pcm[CODEC_C_PCM_CMX].LED_MIC_PORTx      = GPIOF;                
  cmx_pcm[CODEC_C_PCM_CMX].LED_MIC_CLK_GPIO   = RCC_AHB1Periph_GPIOF;  
  /*--------------------------------------------------------------------------------------------------------------------------------------------*/   
  cmx_pcm[CODEC_C_PCM_CMX].LED_SPK_Pin        = GPIO_Pin_4;                      /* вывод LED SPK                                                  */
  cmx_pcm[CODEC_C_PCM_CMX].LED_SPK_PORTx      = GPIOF;                
  cmx_pcm[CODEC_C_PCM_CMX].LED_SPK_CLK_GPIO   = RCC_AHB1Periph_GPIOF;  
  /*--------------------------------------------------------------------------------------------------------------------------------------------*/ 
  cmx_pcm[CODEC_C_PCM_CMX].IRQ_Pin                  = GPIO_Pin_1;             /* вывод IRQ кодека                                               */ 
  cmx_pcm[CODEC_C_PCM_CMX].IRQ_PORTx                = GPIOE;                  /* указатель на базовый адреc используемого порта вывода IRQ кодека*/ 
  cmx_pcm[CODEC_C_PCM_CMX].IRQ_CLK_GPIO             = RCC_AHB1Periph_GPIOE;   /* включение тактирования вывода                                   */ 
  cmx_pcm[CODEC_C_PCM_CMX].IRQ_EXTI_Line            = EXTI_Line1;             /* Specifies the EXTI lines to be enabled or disabled.             */ 
  cmx_pcm[CODEC_C_PCM_CMX].IRQ_NVICIRQCh            = EXTI1_IRQn;             /* Specifies the IRQ channel to be enabled or disabled.            */ 
  cmx_pcm[CODEC_C_PCM_CMX].IRQ_EXTI_PortSourceGPIOx = EXTI_PortSourceGPIOE;      
  cmx_pcm[CODEC_C_PCM_CMX].IRQ_EXTI_PinSourcex      = GPIO_PinSource1;         
  /*----------------------------------------------------------------------------------------------------------------------------------------*/ 
  cmx_pcm[CODEC_C_PCM_CMX].xTimer_CODECx            = NULL;                   /*  указатель на программный таймер                           */ 
  cmx_pcm[CODEC_C_PCM_CMX].QueueOutCodecCMX         = NULL;                   /*  Определяем очередь для передачи голосовых пакетов         */
  cmx_pcm[CODEC_C_PCM_CMX].QueueInCodecCMX          = NULL;                   /*  Определяем очередь для получения голосовых пакетов        */
  /*----------------------------------------------------------------------------------------------------------------------------------------*/ 
  cmx_pcm[CODEC_C_PCM_CMX].step_cnfg                = CONFIG_NONE;            /*  Шаг конфигурирования кодека                               */ 
  /*----------------------------------------------------------------------------------------------------------------------------------------*/   
  cmx_pcm[CODEC_C_PCM_CMX].fsm_codec                = FSM_INIT_POINT;         /*  Состояние кодека                                          */ 
  /*----------------------------------------------------------------------------------------------------------------------------------------*/  
  
  /*======================================= Переменные настройки общие ==============================================================================*/   
  cmx_pcm[CODEC_C_PCM_CMX].codec_phy_addr = DataLoaderSto.Settings.phy_adr;         /* Физический адрес шлюза/кодека по умолчанию                    */ 
  cmx_pcm[CODEC_C_PCM_CMX].codec_ch_id = DataSto.Settings.codec_c_chanel;           /* Идентификатор канала захвата                                  */      
  /*=================================================================================================================================================*/ 

  /*======================================= Переменные настройки интерфейса pcm =====================================================================*/  
  cmx_pcm[CODEC_C_PCM_CMX].pcm_ch_priority       = DataSto.Settings.codec_c_priority_ch_pcm;       /* Приоритет канала интерфейса PCM по умолчанию   */ 
                                                
  cmx_pcm[CODEC_C_PCM_CMX].pcm_source_port       = CODEC_C_SoftPID;                                /* Собственный индекс порта                       */
  cmx_pcm[CODEC_C_PCM_CMX].pcm_mask_index_port   = DataSto.Settings.codec_c_mask_source_soft_port; /* Маска доступных для приема портов              */
  cmx_pcm[CODEC_C_PCM_CMX].pcm_mask_chanel       = DataSto.Settings.codec_c_mask_chanel_soft_port; /* Маска доступных каналов                        */   
  cmx_pcm[CODEC_C_PCM_CMX].pcm_type_box          = PCM_TYPE_BOX;                                   /* Формат данных пакета                           */ 
  /*=================================================================================================================================================*/    
                                                
  cmx_pcm[CODEC_C_PCM_CMX].note_codec_offset     = CODEC_C_PCM_CMX_NOTE;                       /* Смещение для уведомлениий                          */
  /*======================================= Переменные настройки интерфейса cmx =====================================================================*/  
  cmx_pcm[CODEC_C_PCM_CMX].cmx_ch_priority       = DataSto.Settings.codec_c_priority_ch_cmx;   /* Приоритет канала интерфейса СМХ по умолчанию       */   
  /*=================================================================================================================================================*/   
}
/**
  * @brief  Функция обработка прерывания IRQHandler 
  * @param  None
  * @retval None
  */
void EXTI1_IRQHandler(void)
{
  mes_codec_t irg_mes;/* Структура для подготовки сообщения передаваемого таймером */
  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  
  if(EXTI_GetITStatus(EXTI_Line1) != RESET)
  {
    if(x_codec_message != NULL)
    {
      /* Отправляем сообщение */
      irg_mes.data_event = IRQ_CODEC_EV;
      irg_mes.index_codec = cmx_pcm[CODEC_C_PCM_CMX].index_codec;        
      xQueueSendFromISR( x_codec_message , (void*)&(irg_mes), &xHigherPriorityTaskWoken );
    }    
    // Clear the EXTI line pending bit
    EXTI_ClearITPendingBit(EXTI_Line1);
  }
  
  if( xHigherPriorityTaskWoken )
  {
    // Установим запрос на переключение контента по завершению прерывания  
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
  }
}

/* Прерывание SPI RX DMA PCM */
void DMA2_Stream3_IRQHandler()
{
  /* Проверка, на завершение прерывание потока DMA приёма(DMA Stream Transfer ) */
  if (DMA_GetITStatus(DMA2_Stream3, DMA_IT_TCIF3)) 
  {
    /* Очистка бита прерывания потока DMA */
    DMA_ClearITPendingBit(DMA2_Stream3, DMA_IT_TCIF3 );
    /* Если группа событий активна - отправляем флаг заполнения буфера */
    if ( PortCMX.set_port_cmx_router.HandleTask != NULL )
    {
      xTaskNotifyFromISR( PortCMX.set_port_cmx_router.HandleTask, /* Указатель на уведомлюемую задачу                         */
                         PCM_BUFF_B_CMPL<<(CODEC_C_PCM_CMX_NOTE),      /* Значения уведомления                                     */
                         eSetBits,             /* Текущее уведомление добавляются к уже прописанному       */
                         NULL );               /* Внеочередного переключения планировщика не запрашиваем   */
    }     
  }
  
  /* Проверка, на завершение прерывание потока DMA приёма(DMA Stream Transfer ) */
  if (DMA_GetITStatus(DMA2_Stream3, DMA_IT_HTIF3)) 
  {
    /* Очистка бита прерывания потока DMA */
    DMA_ClearITPendingBit(DMA2_Stream3,  DMA_IT_HTIF3 );
    /* Если группа событий активна  - отправляем флаг заполнения буфера */
    if ( PortCMX.set_port_cmx_router.HandleTask != NULL )
    {
      xTaskNotifyFromISR( PortCMX.set_port_cmx_router.HandleTask, /* Указатель на уведомлюемую задачу                         */
                         PCM_BUFF_A_CMPL<<(CODEC_C_PCM_CMX_NOTE),      /* Значения уведомления                                     */
                         eSetBits,             /* Текущее уведомление добавляются к уже прописанному       */
                         NULL );               /* Внеочередного переключения планировщика не запрашиваем   */
    }   
  }
}
/******************* (C) COPYRIGHT 2020 DataExpress *****END OF FILE****/

