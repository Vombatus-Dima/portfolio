/**
  ******************************************************************************
  * @file    codec_control.c
  * @author  Trembach Dmitry
  * @version V1.0.0
  * @date    19-04-2017
  * @brief   Инициализация задачи контроля SPI кодека
  *
  ******************************************************************************
  * @attention
  * 
  * 
  *
  * <h2><center>&copy; COPYRIGHT 2017 DataExpress</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/  
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "codec_cmx.h"  

volatile const char TIM_CODEC_0[] = "CODEC_0";

/**
  * @brief  Функция прописывания указателей на индивидуальные ресурсы кодека  
  * @param  None
  * @retval None
  */
void init_point_codec_0(void)
{
  CODEC_ARRAY[0].index_codec = 0;
  /*-------------------------------------------------------------------------------------------------------------------------------*/           
  CODEC_ARRAY[0].CS_Pin = GPIO_Pin_1;                                  /* вывод CS кодека                                                 */
  CODEC_ARRAY[0].CS_PORTx = GPIOD;                                     /* указатель на базовый адреc используемого порта вывода CS кодека */
  CODEC_ARRAY[0].CS_CLK_GPIO = RCC_AHB1Periph_GPIOD;                   /* включение тактирования вывода                                   */ 
  /*-------------------------------------------------------------------------------------------------------------------------------*/           
  CODEC_ARRAY[0].RESET_Pin = GPIO_Pin_2;                               /* вывод RESET кодека                                              */ 
  CODEC_ARRAY[0].RESET_PORTx = GPIOD;                                  /* указатель на базовый адреc используемого порта вывода RESET кодека*/ 
  CODEC_ARRAY[0].RESET_CLK_GPIO = RCC_AHB1Periph_GPIOD;                /* включение тактирования вывода                                   */   
  /*-------------------------------------------------------------------------------------------------------------------------------*/   
  CODEC_ARRAY[0].SYNC_Pin =       GPIO_Pin_11;         //                            GPIO_Pin_2;                                /* вывод RESET кодека                                              */ 
  CODEC_ARRAY[0].SYNC_PORTx =     GPIOE;               //                      GPIOD;                                   /* указатель на базовый адреc используемого порта вывода RESET кодека*/ 
  CODEC_ARRAY[0].SYNC_CLK_GPIO =  RCC_AHB1Periph_GPIOE;//                                      RCC_AHB1Periph_GPIOD;                 /* включение тактирования вывода                                   */   
  /*-------------------------------------------------------------------------------------------------------------------------------*/     
  CODEC_ARRAY[0].IRQ_Pin = GPIO_Pin_0;                                 /* вывод IRQ кодека                                                */   
  CODEC_ARRAY[0].IRQ_PORTx = GPIOD;                                    /* указатель на базовый адреc используемого порта вывода IRQ кодека*/
  CODEC_ARRAY[0].IRQ_CLK_GPIO = RCC_AHB1Periph_GPIOD;                  /* включение тактирования вывода                                   */   
  CODEC_ARRAY[0].IRQ_EXTI_Line = EXTI_Line0;                           /* Specifies the EXTI lines to be enabled or disabled.             */ 
  CODEC_ARRAY[0].IRQ_NVICIRQCh = EXTI0_IRQn;                           /* Specifies the IRQ channel to be enabled or disabled.            */ 
  CODEC_ARRAY[0].IRQ_EXTI_PortSourceGPIOx = EXTI_PortSourceGPIOD;        
  CODEC_ARRAY[0].IRQ_EXTI_PinSourcex = GPIO_PinSource0;              
  /*-------------------------------------------------------------------------------------------------------------------------------*/   
  CODEC_ARRAY[0].xTimer_CODECx = NULL;                                  /* указатель на программный таймер                                */
  CODEC_ARRAY[0].QueueOutCodec = NULL;                                  /* Определяем очередь для передачи голосовых пакетов              */
  CODEC_ARRAY[0].QueueInCodec = NULL;                                   /* Определяем очередь для получения голосовых пакетов             */
  /*-------------------------------------------------------------------------------------------------------------------------------*/ 
  CODEC_ARRAY[0].step_cnfg = CONFIG_NONE;                               /* Шаг конфигурирования кодека                                    */
  /*-------------------------------------------------------------------------------------------------------------------------------*/  
  CODEC_ARRAY[0].err_dma_cnt = 0;                                       /* счетчик ошибок                                                 */
}



/**
  * @brief  Функция обработка прерывания IRQHandler 
  * @param  None
  * @retval None
  */
void EXTI0_IRQHandler(void)
{
  mes_codec_t irg_mes;/* Структура для подготовки сообщения передаваемого таймером */
  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  
  if(EXTI_GetITStatus(EXTI_Line0) != RESET)
  {
    
    if(x_codec_message != NULL)
    {
      /* Отправляем сообщение */
      irg_mes.data_event = IRQ_CODEC_EV;
      irg_mes.index_codec = 0;        
      xQueueSendFromISR( x_codec_message , (void*)&(irg_mes), &xHigherPriorityTaskWoken );
    }    
    // Clear the EXTI line pending bit
    EXTI_ClearITPendingBit(EXTI_Line0);
  }
  
  if( xHigherPriorityTaskWoken )
  {
    // Установим запрос на переключение контента по завершению прерывания  
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
  }
}
/******************* (C) COPYRIGHT 2017 DataExpress *****END OF FILE****/

