/**
  ******************************************************************************
  * @file    codec_control.c
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
#include "codec_control.h"
#include "rs_frame_header.h"
#include "printf_dbg.h" 
#include "cmx_gate.h"
#include "codec_cmx.h" 
#include "hooks.h"
#include "hard_dma_gpio.h"
#include "board.h"
#include "codec_pcm.h"

QueueHandle_t      x_codec_message;  
uint8_t            codec_sync_index = 0;

/* Буфер для хранения пакета полученного из кодека              */
uint8_t   codec_to_data[CMX618_VOICE_LEN]; 
/* Буфер для хранения принятого пакета перед передачей в кодек  */
uint8_t   data_to_codec[CMX618_VOICE_LEN]; 
/*============================================================================*/
/* Структура для подготовки сообщения передаваемого таймером */
mes_codec_t timer_mes;
/* Структура для подготовки сообщения передаваемого прерыванием формируемым SYNC */
mes_codec_t sync_mes;

/**
  * @brief  Функция отработки програмного таймера 
  * @param  TimerHandle_t pxTimer - указатель на таймер вызвавщий функцию
  * @retval None
  */
void TimUpdateNoteCodecTask( TimerHandle_t pxTimer )
{
  /* Функция обработки програмного таймера.*/
  if ( ( PortCMX.set_port_cmx_router.HandleTask ) != NULL  )
  {/* Устанавливаем событие срабатывания програмного таймера для периодического уведомления */
    xTaskNotify( PortCMX.set_port_cmx_router.HandleTask,   /* Указатель на уведомлюемую задачу                         */
                TIMER_NOTE,                                /* Значения уведомления                                     */
                eSetBits );                                /* Текущее уведомление добавляются к уже прописанному       */
  }
}

/**
  * @brief  Функция установки програмного таймера
  * @param  codec_type_t* codec - указатель на структуру кодека
  * @param  TickType_t time - временной интервал в тиках RTOS
  * @retval None
  */
void soft_timer_set( pcm_cmx_t* CODECx, TickType_t time )
{
  if (CODECx->xTimer_CODECx != NULL)
  {
    /* Остановить таймер                                             */
    xTimerStop( CODECx->xTimer_CODECx, 0 );
    /* Установить таймаут запуск аппаратного конфигурирования кодека */
    xTimerChangePeriod( CODECx->xTimer_CODECx, time, 0 );
    /* Запустить таймер                                              */
    xTimerStart( CODECx->xTimer_CODECx, 0 );
  }
}

/**
  * @brief  Функция обратного вызова таймера кодека
  * @param  TimerHandle_t xTimer  - указатель на таймера кодека вызвавшего функции
  * @retval uint8_t статус инициализации
  * 
  */
void CodecTimerCB( TimerHandle_t xTimer )
{
  if (xTimer != NULL)
  {
    for (uint8_t codec_contic = 0 ; codec_contic <  MAX_CODEC; codec_contic++ )
    {
      if ( cmx_pcm[codec_contic].xTimer_CODECx == xTimer)
      {
        //Запустить таймер
        soft_timer_set( &(cmx_pcm[codec_contic]), 1000 );
        /* Отправляем сообщение */
        timer_mes.data_event = TIMEOUT_EV;
        timer_mes.index_codec = cmx_pcm[codec_contic].index_codec;        
        xQueueSend( x_codec_message , (void*)&(timer_mes), ( TickType_t ) 0 );
      }
    }
  }
}

/**
  * @brief  Прерывание таймера формирование SYNC.
  * @param  None
  * @retval None
  */
void CODEC_TIM_IRQHandler( void )
{  
  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;  
  
  /*  */  
  if( TIM_GetITStatus( CODEC_TIM, TIM_IT_Update) != RESET)
  {/* Прерывание для установки сигналов SYNC и формирование события */ 
    /* Переключение управления SYNC */
    codec_sync_index++;
    /* Проверка выхода за диапазон SYNC */    
    if (codec_sync_index >= MAX_CODEC ) codec_sync_index = 0;    
    
    if (cmx_pcm[codec_sync_index].SYNC_PORTx != NULL)
    {
      cmx_pcm[codec_sync_index].SYNC_PORTx->BSRRL = cmx_pcm[codec_sync_index].SYNC_Pin;   
    }
    /* Отправляем сообщение */
    if(x_codec_message != NULL)
    {
      /* Отправляем сообщение */
      sync_mes.data_event = SYNC_CODEC_EV;
      sync_mes.index_codec = cmx_pcm[codec_sync_index].index_codec;         
      xQueueSendFromISR( x_codec_message , (void*)&(sync_mes), &xHigherPriorityTaskWoken );
    }    
    
    /* Сброс флагов прерывания таймера */
    TIM_ClearITPendingBit( CODEC_TIM, TIM_IT_Update );  
  }
  /*  */
  if( TIM_GetITStatus( CODEC_TIM, TIM_IT_CC1) != RESET)
  {/* Прерывание для сброса сигналов SYNC */ 
    if (codec_sync_index < MAX_CODEC )
    {
      if (cmx_pcm[codec_sync_index].SYNC_PORTx != NULL)
      {
        cmx_pcm[codec_sync_index].SYNC_PORTx->BSRRH = cmx_pcm[codec_sync_index].SYNC_Pin;   
      }
    }
    /* Сброс флагов прерывания таймера */
    TIM_ClearITPendingBit( CODEC_TIM, TIM_IT_CC1 );  
  }  
  
  if( xHigherPriorityTaskWoken )
  {
    // Установим запрос на переключение контента по завершению прерывания  
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
  }  
}

/**
  * @brief  Функция инициализации интерфейса CMX кодека
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @retval none
  * 
  */
void cmx_spi_init( pcm_cmx_t* CODECx)
{
  GPIO_InitTypeDef            GPIO_InitStructure;   
  NVIC_InitTypeDef            NVIC_InitStructure;  
  EXTI_InitTypeDef            EXTI_InitStructure;

  if ( CODECx->fsm_codec == FSM_INIT_POINT)
  {
    /* Инициализация выводов */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;              
    
    /* настройка вывода CS_PIN */
    RCC_AHB1PeriphClockCmd( CODECx->CS_CLK_GPIO , ENABLE );
    GPIO_InitStructure.GPIO_Pin = CODECx->CS_Pin;
    GPIO_Init(CODECx->CS_PORTx, &GPIO_InitStructure);
    
    /* настройка вывода RST */
    RCC_AHB1PeriphClockCmd( CODECx->RESET_CLK_GPIO , ENABLE );
    GPIO_InitStructure.GPIO_Pin = CODECx->RESET_Pin;
    GPIO_Init(CODECx->RESET_PORTx, &GPIO_InitStructure);
    
    /* настройка вывода SYNC */
    RCC_AHB1PeriphClockCmd( CODECx->SYNC_CLK_GPIO , ENABLE );
    GPIO_InitStructure.GPIO_Pin = CODECx->SYNC_Pin;
    GPIO_Init(CODECx->SYNC_PORTx, &GPIO_InitStructure);  
 
    /* LED MIC */ 
    if ( ( CODECx->LED_MIC_PORTx ) != NULL )
    {
      /* Включить тактирование GPIO LED PCM */
      RCC_AHB1PeriphClockCmd( CODECx->LED_MIC_CLK_GPIO , ENABLE);
      
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;             /* Используется порт как выход                  */
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;          /* Скорость                                     */
      GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;            /* Push pool                                    */
      GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;         /*                                              */  
      GPIO_InitStructure.GPIO_Pin = CODECx->LED_MIC_Pin;        /* Биты сигналов                                */
      GPIO_Init(CODECx->LED_MIC_PORTx, &GPIO_InitStructure);    /* Установка настроек битов порта               */
      
      /* Погасить светодиод */
      CODECx->LED_MIC_PORTx->BSRRH = CODECx->LED_MIC_Pin;
    }
    
    /* LED SPK */ 
    if ( ( CODECx->LED_SPK_PORTx ) != NULL )
    {
      /* Включить тактирование GPIO LED PCM */
      RCC_AHB1PeriphClockCmd( CODECx->LED_SPK_CLK_GPIO , ENABLE);
      
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;             /* Используется порт как выход                  */
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;          /* Скорость                                     */
      GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;            /* Push pool                                    */
      GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;         /*                                              */  
      GPIO_InitStructure.GPIO_Pin = CODECx->LED_SPK_Pin;        /* Биты сигналов                                */
      GPIO_Init(CODECx->LED_SPK_PORTx, &GPIO_InitStructure);    /* Установка настроек битов порта               */
      
      /* Погасить светодиод */
      CODECx->LED_SPK_PORTx->BSRRH = CODECx->LED_SPK_Pin;
    }    
    
    /* предустановка состояний выводов */
    /*  Установить CS в 1              */ 
    GPIO_SetBits( CODECx->CS_PORTx , CODECx->CS_Pin );
    /*  Установить RST в 1             */  
    GPIO_SetBits( CODECx->RESET_PORTx , CODECx->RESET_Pin );  
    /*  Установить SYNC в 0            */  
    GPIO_ResetBits( CODECx->SYNC_PORTx , CODECx->SYNC_Pin );    
    
    /************ Настройка EXTI (внешнего прерывания) для IRQ кодека************/
    /* настройка ввода IRQ */
    RCC_AHB1PeriphClockCmd( CODECx->IRQ_CLK_GPIO , ENABLE );
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                
    GPIO_InitStructure.GPIO_Pin =  CODECx->IRQ_Pin;
    GPIO_Init(CODECx->IRQ_PORTx, &GPIO_InitStructure);    
    
    /* Enable SYSCFG clock  */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    /* Выбор вывода GPIO используется в качестве линии IRQ   */
    SYSCFG_EXTILineConfig( CODECx->IRQ_EXTI_PortSourceGPIOx,  CODECx->IRQ_EXTI_PinSourcex);    
    
    /* Настройка внешнего IRQ    */
    /* Линия куда подключено IRQ */
    EXTI_InitStructure.EXTI_Line = CODECx->IRQ_EXTI_Line;
    /* Выбран режим прерывание  */
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    /* срабатываем по заднему (спадающему) фронту импульса */
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    /* включить                                            */ 
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    /* Инициализирует IRQi с заданными параметрами в EXTI_InitStruct. */
    EXTI_Init(&EXTI_InitStructure);  
    
    /* * Включить и установить IRQ прерывание *               */
    /* канал IRQ                                              */
    NVIC_InitStructure.NVIC_IRQChannel = CODECx->IRQ_NVICIRQCh;
    /* приоритет канала (0 (самый приоритетный) - 15) //при Priority <= 4 не работает RTOS  */
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8; 
    /* приоритет подгруппы                                                          */
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
    /* включить прерывание                                                          */ 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    /* Инициализация NVIC в соответствии с заданными параметрами в NVIC_InitStruct. */ 
    NVIC_Init(&NVIC_InitStructure);
    /****************************************************************************/
    /* открытие програмного таймера для кодека  */
    CODECx->xTimer_CODECx = xTimerCreate( CODECx->pcTimerName,            /* Назначим имя таймеру, только для отладки.    */
                                         1,                               /* Период таймера в тиках.                      */                     
                                         pdFALSE,                         /* Автоперезагрузка отключена.                  */
                                         NULL,                            /* Нет связи с индивидуальным индентификатором. */
                                         CodecTimerCB );                  /* Функция обратного вызова вызываемая таймером.*/
    /****************************************************************************/  
    /* Открываем очередь для передачи голосовых пакетов              */
    CODECx->QueueOutCodecCMX = xQueueCreate( 6 , SIZE_DATA_CMX618_VOICE);              
    /* Открываем очередь для получения голосовых пакетов             */
    CODECx->QueueInCodecCMX  = xQueueCreate( 6 , SIZE_DATA_CMX618_VOICE);  
    /* Установить режим инициализации                                */
    CODECx->fsm_codec = FSM_WAIT_INIT_CMX;
  }
}    

/**
  * @brief  Функция чтения статуса кодека
  * @param  codec_type_t* codec - указатель на структуру кодека
  * @retval uint16_t - статус кодека
  */
uint16_t get_codec_status( pcm_cmx_t* CODECx )
{
  uint16_t status;
  /* Читаем статус - сбрасываем флаги прерываний */
  status = spi_wr_rd_reg_word(CODECx, STATUS, SPI_DUMMY_16);
  /* Задежка 1 мкс */
  ns100_delay(10);
  /* Возвращаем статус кодека */
  return status;
}

/**
  * @brief  Функция конфигурирования кодека
  * @param  codec_type_t* CODECx - указатель на структуру кодека
  * @param  data_event_list_t ev_data - сообщение для кодека
  * @retval bool статус инициализации true - инициализация закончена
  *                                   false - инициализация не закончена 
  */
bool Config_CODEC( pcm_cmx_t* CODECx, data_event_list_t ev_data )
{
  switch (CODECx->step_cnfg)
  {  
  case CONFIG_NONE:
    
    /* Анализ полученного сообщения */
    if ( ev_data != TIMEOUT_EV )  return false; /* Инициализация не закончена */ 
    /*  Установить RST в 0 */  
    GPIO_ResetBits( CODECx->RESET_PORTx , CODECx->RESET_Pin ); 
    /*Установить таймаут  конфигурирования кодека             */
    soft_timer_set( CODECx, 2);
    /* переход на следующий этап конфигурирования */  
    CODECx->step_cnfg = HARD_RESET;    
    /* Инициализация не закончена */
    return false;
    
  case HARD_RESET:
    
    /* Анализ полученного сообщения */
    if ( ev_data != TIMEOUT_EV )   return false; /* Инициализация не закончена */    
    /*  Установить RST в 1 */  
    GPIO_SetBits( CODECx->RESET_PORTx , CODECx->RESET_Pin ); 
    /*Установить таймаут  конфигурирования кодека             */
    soft_timer_set( CODECx, 20);
    /* переход на следующий этап конфигурирования */       
    CODECx->step_cnfg = SOFT_REG_RESET;    
    /* Инициализация не закончена */
    return false;
    
  case SOFT_REG_RESET:
    
    /* Анализ полученного сообщения */
    if ( ( ev_data != TIMEOUT_EV ) &&  ( ev_data != IRQ_CODEC_EV ) )   return false; /* Инициализация не закончена */ 
    /* Read status codeca - clear IRQ */
    get_codec_status( CODECx );
    /* Perform software reset */
    spi_wr_rd_byte(CODECx, RESETN);
    /*Установить таймаут  конфигурирования кодека             */
    soft_timer_set( CODECx, 10); 
    /* переход на следующий этап конфигурирования */         
    CODECx->step_cnfg = SET_IRQ_CLOCK;    
    /* Инициализация не закончена */
    return false;    
    
  case SET_IRQ_CLOCK:
    
    /* Анализ полученного сообщения */
    if ( ( ev_data != TIMEOUT_EV ) && ( ev_data != IRQ_CODEC_EV ) )   return false; /* Инициализация не закончена */ 
    /* Read status codeca - clear IRQ */
    get_codec_status( CODECx ); 
    /*===== Enable interrupts and establish internal processor speed =========*/ 
    /*  Basic CMX618 adjustments */
    spi_wr_rd_reg_word(CODECx, IRQENAB , RDY | SVC | VDA  ); // | DFDA| EFDA
    /* Задежка 1 мкс */
    ns100_delay(10);
    /* Set clock */
    spi_wr_rd_reg_word(CODECx, CLOCK, 0x0005);
    /* Задежка 1 мкс */
    ns100_delay(10);
    /* Set DTMF Level */
    spi_wr_rd_reg_byte(CODECx, DTMFATTEN, 0x00);
    /*Установить таймаут  конфигурирования кодека             */
    soft_timer_set( CODECx, 10);   
    /* переход на следующий этап конфигурирования */         
    CODECx->step_cnfg = SET_POWER_SAVE;    
    /* Инициализация не закончена */
    return false; 

  case SET_POWER_SAVE:
    
    /* Анализ полученного сообщения */
    if ( ( ev_data != TIMEOUT_EV ) && ( ev_data != IRQ_CODEC_EV ) )    return false; /* Инициализация не закончена */ 
    /* Read status codeca - clear IRQ */
    get_codec_status( CODECx );
    /*=====  Turn on BIAS and internal A/D and D/A converter (CMX618/CMX638 only) ==*/
    /* Config power                                                                 */
    /* POWER_ADCON|POWER_DACON|POWER_CODEC|POWER_BIAS                               */
    spi_wr_rd_reg_byte(CODECx, POWERSAVE, 0x00);    
    /* NOTE: A check of SVCACK ($2E) b0 can be made at this point for debug purposes.          */
    /* SVCACK b0 only has meaning after a service has been performed and RDY has been asserted.*/
    /* If SVCACK b0 is not equal to 1 after RDY is asserted, an error has been made.           */
    /* In this case, a General Reset should be issued and device configuration restarted.      */
    /*Установить таймаут  конфигурирования кодека             */
    soft_timer_set( CODECx, 100);
    /* переход на следующий этап конфигурирования */         
    CODECx->step_cnfg = SET_VOCODER;    
    /* Инициализация не закончена */
    return false; 
    
  case SET_VOCODER:    
    /* Анализ полученного сообщения */
    if ( ev_data != TIMEOUT_EV )   return false; /* Инициализация не закончена */ 
    /*==================================  Configure Vocoder  =======================           */
    /* Config codec                                                                            */
    /* _2400BPS|_60MSEC */
    spi_wr_rd_reg_byte(CODECx, VCFG, 0x07);	     
    /* Задежка 1 мкс */
    ns100_delay(10);
    /*Установить таймаут  конфигурирования кодека             */
    soft_timer_set( CODECx, 10);
    /* переход на следующий этап конфигурирования */         
    CODECx->step_cnfg = SET_SYNC;    
    /* Инициализация не закончена */
    return false; 
    
  case SET_SYNC:    
    
    /* Анализ полученного сообщения */
    if ( ( ev_data != TIMEOUT_EV ) && ( ev_data != IRQ_CODEC_EV ) )    return false; /* Инициализация не закончена */ 
    /* Read status codeca - clear IRQ */
    get_codec_status( CODECx );
    /*  Config sync  */
    spi_wr_rd_reg_byte(CODECx, SYNCCTRL, 0x0F);   // SYNC_PERIOD_60MS|SYNC_OUTPUT
    /* Задежка 1 мкс */
    ns100_delay(10);
    /*  Config EXCODECCONT  */
    spi_wr_rd_reg_byte(CODECx, EXCODECCONT, 0x43);
    /* Задежка 1 мкс */
    ns100_delay(10);
    /*Установить таймаут  конфигурирования кодека             */
    soft_timer_set( CODECx, 10);   
    /* переход на следующий этап конфигурирования */         
    CODECx->step_cnfg = SET_EXCODECCONT_A;    
    /* Инициализация не закончена */
    return false;    
    
  case SET_EXCODECCONT_A:
    
    /* Анализ полученного сообщения */
    if ( ( ev_data != TIMEOUT_EV ) && ( ev_data != IRQ_CODEC_EV ) )    return false; /* Инициализация не закончена */ 
    /* Read status codeca - clear IRQ */
    get_codec_status( CODECx );
    /* Ext codec set  */   
    spi_wr_rd_reg_byte(CODECx, EXCODECCONT, 0x86);
    /* Установить таймаут  конфигурирования кодека */
    soft_timer_set( CODECx, 10); 
    /* переход на следующий этап конфигурирования */         
    CODECx->step_cnfg = SET_EXCODECCONT_B;    
    /* Инициализация не закончена */
    return false;     
    
  case SET_EXCODECCONT_B:
    
    /* Анализ полученного сообщения */
    if ( ( ev_data != TIMEOUT_EV ) && ( ev_data != IRQ_CODEC_EV ) )    return false; /* Инициализация не закончена */ 
    /* Read status codeca - clear IRQ */
    get_codec_status( CODECx );
    /* Ext codec set  */   
    spi_wr_rd_reg_byte(CODECx, EXCODECCONT, 0x82);
    /*Установить таймаут  конфигурирования кодека             */
    soft_timer_set( CODECx, 10);
    /* переход на следующий этап конфигурирования */         
    CODECx->step_cnfg = SET_EXCODECCONT_C;    
    /* Инициализация не закончена */
    return false;     
    
  case SET_EXCODECCONT_C:
    
    /* Анализ полученного сообщения */
    if ( ( ev_data != TIMEOUT_EV ) && ( ev_data != IRQ_CODEC_EV ) )    return false; /* Инициализация не закончена */ 
    /* Read status codeca - clear IRQ */
    get_codec_status( CODECx );
    /* Ext codec set  */   
    spi_wr_rd_reg_byte(CODECx, EXCODECCONT, 0x83);
    /*Установить таймаут  конфигурирования кодека             */
    soft_timer_set( CODECx, 10);  
    /* переход на следующий этап конфигурирования */         
    CODECx->step_cnfg = SET_EXCODECCONT_D;    
    /* Инициализация не закончена */
    return false;     
    
  case SET_EXCODECCONT_D:
    
    /* Анализ полученного сообщения */
    if ( ( ev_data != TIMEOUT_EV ) && ( ev_data != IRQ_CODEC_EV ) )    return false; /* Инициализация не закончена */ 
    /* Read status codeca - clear IRQ */
    get_codec_status( CODECx );
    /* Ext codec set  */   
    spi_wr_rd_reg_byte(CODECx, EXCODECCONT, 0x81);
    /*Установить таймаут  конфигурирования кодека             */
    soft_timer_set( CODECx, 10); 
    /* переход на следующий этап конфигурирования */         
    CODECx->step_cnfg = SET_EXCODECCONT_E;    
    /* Инициализация не закончена */
    return false;      
 
  case SET_EXCODECCONT_E:
    
    /* Анализ полученного сообщения */
    if ( ( ev_data != TIMEOUT_EV ) && ( ev_data != IRQ_CODEC_EV ) )   return false; /* Инициализация не закончена */ 
    /* Read status codeca - clear IRQ */
    get_codec_status( CODECx );
    /* Ext codec set  */    
    spi_wr_rd_reg_byte(CODECx, EXCODECCONT, 0x87);
    /*Установить таймаут  конфигурирования кодека             */
    soft_timer_set( CODECx, 10);   
    /* переход на следующий этап конфигурирования */         
    CODECx->step_cnfg = SET_IDD;    
    /* Инициализация не закончена */
    return false;  
    
  case SET_IDD:
    
    /* Анализ полученного сообщения */
    if ( ( ev_data != TIMEOUT_EV ) &&  ( ev_data != IRQ_CODEC_EV ) )    return false; /* Инициализация не закончена */ 
    /* Read status codeca - clear IRQ */
    get_codec_status( CODECx );
    /*  Config IDD  */
    spi_wr_rd_reg_byte(CODECx, IDD, 0x80);
    /*Установить таймаут  конфигурирования кодека             */
    soft_timer_set( CODECx, 10);
    /* переход на следующий этап конфигурирования */         
    CODECx->step_cnfg = SET_HIGH_WATER;    
    /* Инициализация не закончена */
    return false;  
 
  case SET_HIGH_WATER:
    
    /* Анализ полученного сообщения */
    if ( ( ev_data != TIMEOUT_EV ) &&  ( ev_data != IRQ_CODEC_EV ) )     return false; /* Инициализация не закончена */ 
    /* Read status codeca - clear IRQ */
    get_codec_status( CODECx );
    /* High and low watermarks for VDW bit High watermark=0x9E=158 samples as per datasheet example */
    spi_wr_rd_reg_word(CODECx, VDWHLWM, 0x809E);				
    /*Установить таймаут  конфигурирования кодека             */
    soft_timer_set( CODECx, 10);
    /* переход на следующий этап конфигурирования */         
    CODECx->step_cnfg = SET_LOW_WATER;    
    /* Инициализация не закончена */
    return false;       
 
  case SET_LOW_WATER:
    
    /* Анализ полученного сообщения */
    if ( ( ev_data != TIMEOUT_EV ) &&  ( ev_data != IRQ_CODEC_EV ) )    return false; /* Инициализация не закончена */ 
    /* Read status codeca - clear IRQ */
    get_codec_status( CODECx );
    /* Low watermark=0x48=72 samples as per datasheet example */
    spi_wr_rd_reg_word(CODECx, VDWHLWM, 0x01e0);				
    /*Установить таймаут  конфигурирования кодека             */
    soft_timer_set( CODECx, 10);
    /* переход на следующий этап конфигурирования */         
    CODECx->step_cnfg = SET_OUT_GAIN;    
    /* Инициализация не закончена */
    return false;     
    
  case SET_OUT_GAIN:
    
    /* Анализ полученного сообщения */
    if ( ( ev_data != TIMEOUT_EV ) &&  ( ev_data != IRQ_CODEC_EV ) )    return false; /* Инициализация не закончена */ 
    /* Read status codeca - clear IRQ */
    get_codec_status( CODECx );
    /* Output gain */
    spi_wr_rd_reg_byte(CODECx, AOG, CMX618_OUTPUT_GAIN);
    /*Установить таймаут  конфигурирования кодека             */
    soft_timer_set( CODECx, 10); 
    /* переход на следующий этап конфигурирования */         
    CODECx->step_cnfg = CONFIG_NONE;    
    /* Инициализация закончена */
    return true;    
        
  default: 
    /*  Установить RST в 0 */  
    GPIO_ResetBits( CODECx->RESET_PORTx , CODECx->RESET_Pin ); 
    /*Установить таймаут  конфигурирования кодека             */
    soft_timer_set( CODECx, 10);
    /* переход на следующий этап конфигурирования */  
    CODECx->step_cnfg = CONFIG_NONE;
    /* Инициализация не закончена */
    return false;        
  }
}

/**
  * @brief  Функция контроля кодека
  * @param  codec_type_t* codec - указатель на структуру кодека
  * @param  mes_codec_t* mess - указатель на структуру сообщения
  * @retval None
  */
void control_codec( pcm_cmx_t* CODECx, mes_codec_t* mess )
{
  /* Анализ и отработка автомата кодека*/
  switch (CODECx->fsm_codec)
  {
/*:::::::::::::::::::::::;;;;:::::::::::::::::::::::::::::::::::::::::::::::::*/      
  case FSM_NONE:
  case FSM_INIT_POINT:    
    /* Отработка автомата только после инициализации */
    break;     
/*:::::::::::::::::::::::;;;;:::::::::::::::::::::::::::::::::::::::::::::::::*/     
  case FSM_WAIT_INIT_CMX:/* ожидание запуска инициализации */     
    /* Установить режим инициализации                                */
    CODECx->fsm_codec = FSM_INIT_CMX;      
    /* Установить таймаут запуск аппаратного конфигурирования кодека */
    soft_timer_set( CODECx, 1);
    break;  
/*:::::::::::::::::::::::;;;;:::::::::::::::::::::::::::::::::::::::::::::::::*/      
  case FSM_INIT_CMX:  
    if ( Config_CODEC( CODECx, mess->data_event) )
    {
      /* Установить режим установки speaker                      */
      CODECx->fsm_codec = FSM_SET_SPEAKER_A;      
    } 
    break;
/*:::::::::::::::::::::::;;;;:::::::::::::::::::::::::::::::::::::::::::::::::*/      
  case FSM_SET_SPEAKER_A:  
    switch (mess->data_event)
    {
    case TIMEOUT_EV:  
      /* Установить режим установки speaker                      */
      CODECx->fsm_codec = FSM_SET_SPEAKER_B;      
      /* Установить таймаут запуска поэтапного включения speaker */
      soft_timer_set( CODECx, 10 );//100
      break;   
    case IRQ_CODEC_EV:  
      /* Прочитать статус - сбросить прерывания */
      get_codec_status( CODECx );
      break; 
    default:
      break; 
    }   
    break;
/*:::::::::::::::::::::::;;;;:::::::::::::::::::::::::::::::::::::::::::::::::*/       
  case FSM_SET_SPEAKER_B:  
    switch (mess->data_event)
    {
    case TIMEOUT_EV:  
      /* Установить режим установки speaker                      */
      CODECx->fsm_codec = FSM_SET_SPEAKER_C;      
      /* Установить таймаут запуска поэтапного включения speaker */
      soft_timer_set( CODECx, 10 );//100
      break;   
    case IRQ_CODEC_EV:  
      /* Прочитать статус - сбросить прерывания */
      get_codec_status( CODECx );
      break;     
    default:
      break; 
    }   
    break;
/*:::::::::::::::::::::::;;;;:::::::::::::::::::::::::::::::::::::::::::::::::*/      
  case FSM_SET_SPEAKER_C:  
    switch (mess->data_event)
    {
    case TIMEOUT_EV:  
      /* Прочитать статус - сбросить прерывания */
      get_codec_status( CODECx ); 
      /* Вкл. режим спикера */
      Set_Speaker( CODECx );
      
      /* Сброс приоритета полученного со спикера */
      //CODECx->codec_speaker_priority = VOICE_PRIOR_IDLE;    
      /* Установить режим установки speaker - ожидание           */
      CODECx->fsm_codec = FSM_SPEAKER_MUTE;      
      /* Отключаем таймаут - будем работать по SYNC              */
      soft_timer_set( CODECx, portMAX_DELAY);
      break;   
    case IRQ_CODEC_EV:  
      /* Прочитать статус - сбросить прерывания */
      get_codec_status( CODECx );
      break;     
    default:
      break; 
    }   
    break;
/*:::::::::::::::::::::::;;;;:::::::::::::::::::::::::::::::::::::::::::::::::*/      
  case FSM_SPEAKER_MUTE:  
    switch (mess->data_event)
    {
    case MIC_EV:  
      /* Обновить полученный приоритет канала микрофона          */
      CODECx->codec_mic_priority = mess->data_codec;       
      /* переключение кодека на режим микрофона                  */
      CODECx->fsm_codec = FSM_SET_MIC_A;
      /* Установить таймаут поэтапного включения mic             */
      soft_timer_set( CODECx, 5);
      break;   
    case IRQ_CODEC_EV:  
      /* Прочитать статус - сбросить прерывания */
      get_codec_status( CODECx );
      break; 
    default:

      break; 
    }   
    /* если появились данные в очереди - переводим систему в режим SPEAKER */
    if (CODECx->QueueInCodecCMX != NULL)
    {/* если очередь открыта */
      /* если в очереди есть данные для передачи кодеку */										
      if( uxQueueMessagesWaiting(CODECx->QueueInCodecCMX) > 0 )  
      {
        /* переводим в режим прослушки */
        CODECx->fsm_codec = FSM_SPEAKER; 
      } 															 
    }
    break;    
/*:::::::::::::::::::::::;;;;:::::::::::::::::::::::::::::::::::::::::::::::::*/      
  case FSM_SPEAKER:      
    switch (mess->data_event)
    {
    case IRQ_CODEC_EV:  
      /* Прочитать статус - сбросить прерывания */
      get_codec_status( CODECx );
      break; 
    case SYNC_CODEC_EV:  
      /* если очередь открыта */
      if (CODECx->QueueInCodecCMX != NULL)
      {       
        /* если есть данные, то ...                 */
        if( xQueueReceive( CODECx->QueueInCodecCMX , data_to_codec , ( TickType_t ) 0) )
        { /* переносим содержимое из очереди в кодек */ 
          /* Запись данных голоса в кодек CMX                        */
          Send_Voice( CODECx );
          /* Управление потоком данных спикера */
          switch (CODECx->codec_status_speaker)
          {
          case SLEEP_STREAM:
            CODECx->codec_status_speaker = START_STREAM;
            break;  
          case START_STREAM:
            CODECx->codec_status_speaker = ACTIVE_STREAM;
            break;  
          }  
        } 
        else
        { /* Данных нет */
          
          /* Глушим усилитель */
          
          /* Сброс приоритета полученного со спикера */
          //CODECx->codec_speaker_priority = VOICE_PRIOR_IDLE;     
          /* Управление потоком данных спикера */
          switch (CODECx->codec_status_speaker)
          {
          case ACTIVE_STREAM:
          case START_STREAM: 
            /* Переводим поток в спящий режим */
            CODECx->codec_status_speaker = SLEEP_STREAM; 
            break;  
          } 
          /* Установить режим установки speaker - ожидание           */
          CODECx->fsm_codec = FSM_SPEAKER_MUTE;      
          /* Отключаем таймаут - будем работать по SYNC              */
          soft_timer_set( CODECx, portMAX_DELAY);
        }
      }
      break;
    case MIC_EV:
      /* Обновить полученный приоритет микрофона */
      CODECx->codec_mic_priority = mess->data_codec; 
      /* сравниваем с текущим приоритетом канала микрофона */
      if ( CODECx->codec_mic_priority < CODECx->codec_speaker_priority  )
      {/* Приоритет канала микрофона выше приоритета канала спикера - переключаем режим кодека */
        /* Сброс приоритета полученного со спикера */
        //CODECx->codec_speaker_priority = VOICE_PRIOR_IDLE;   
        /* Установить режим установки микрофона                      */
        CODECx->fsm_codec = FSM_SET_MIC_A;   
        /* Установить таймаут запуска поэтапного включения микрофона */
        soft_timer_set( CODECx, 10); //::120       
      }
      break;      
    default:
      break;  
    }   
    break;
/*:::::::::::::::::::::::;;;;:::::::::::::::::::::::::::::::::::::::::::::::::*/      
  case FSM_SET_MIC_A:  
    switch (mess->data_event)
    {
    case TIMEOUT_EV:  
      /* Установить режим установки mic                          */
      CODECx->fsm_codec = FSM_SET_MIC_B;      
      /* Установить таймаут поэтапного включения mic             */
      soft_timer_set( CODECx, 5);
      break;   
    case IRQ_CODEC_EV:  
      /* Прочитать статус - сбросить прерывания */
      get_codec_status( CODECx );
      break;     
    default:
      break; 
    }   
    break;
/*:::::::::::::::::::::::;;;;:::::::::::::::::::::::::::::::::::::::::::::::::*/    
  case FSM_SET_MIC_B:  
    switch (mess->data_event)
    {
    case TIMEOUT_EV:  
      /* Установить режим установки mic                          */
      CODECx->fsm_codec = FSM_SET_MIC_C;      
      /* Вкл. режим микрофона */
      Set_Mic( CODECx );
      /* Установить таймаут поэтапного включения mic             */
      soft_timer_set( CODECx, 5);
      break;    
    case IRQ_CODEC_EV:  
      /* Прочитать статус - сбросить прерывания */
      get_codec_status( CODECx );
      break;    
    default:
      break; 
    }   
    break;
/*:::::::::::::::::::::::;;;;:::::::::::::::::::::::::::::::::::::::::::::::::*/  
  case FSM_SET_MIC_C:  
    switch (mess->data_event)
    {
    case TIMEOUT_EV:  
      /* Установить режим mic                                    */
      CODECx->fsm_codec = FSM_MIC;      
      /* Установить таймаут включения mic                        */
      soft_timer_set( CODECx, 80);//::240
      /* Сброс счетчика транслируеміх сообщений в режиме MIC */
      CODECx->codec_cnt_mic_box = 0;
      break;    
    case IRQ_CODEC_EV:  
      /* Прочитать статус - сбросить прерывания */
      get_codec_status( CODECx );
      break;    
    default:
      break; 
    }   
    break;
/*:::::::::::::::::::::::;;;;:::::::::::::::::::::::::::::::::::::::::::::::::*/      
  case FSM_MIC:  
    switch (mess->data_event)
    {
    case MIC_EV:
      /* Обновить полученный приоритет микрофона */
      CODECx->codec_mic_priority = mess->data_codec; 
      /* Установить таймаут отключения mic                        */
      soft_timer_set( CODECx, 240);
//      /* Контроль длительности сообщений */
//      if (CODECx->codec_cnt_mic_box < 50000)  
//      {/* Подсчет транслируемых сообщений в режиме MIC */
//        (CODECx->codec_cnt_mic_box)++; 
//      }
//      else
//      { /* Длительность сообщения более 30 сек */
//        /* Обнулить полученный приоритет микрофона */
//        CODECx->codec_mic_priority = 0; 
//        /* Перевести в режим неактивного микрофона           */
//        CODECx->fsm_codec = FSM_MIC_MUTE;         
//      }  
      break;        
    case SPEAKER_EV:      
      /* Обновить полученный приоритет спикера */
      CODECx->codec_speaker_priority = mess->data_codec; 
      /* сравниваем с текущим приоритетом канала микрофона */
      if ( CODECx->codec_speaker_priority < CODECx->codec_mic_priority  )
      {/* Приоритет канала спикера выше приоритета канала микрофона - переключаем режим кодека */
        /* Сброс приоритета микрофона */
        //CODECx->codec_mic_priority = VOICE_PRIOR_IDLE; 
        /* Установить режим установки speaker                      */
        CODECx->fsm_codec = FSM_SET_SPEAKER_A;   
        /* Установить таймаут запуска поэтапного включения speaker */
        soft_timer_set( CODECx, 10 ); //120       
      }
      break;  
    case IRQ_CODEC_EV: 
      /* Прочитать статус - сбросить прерывания */
      if (( (get_codec_status( CODECx )) & VDA) != 0 )
      {          
        /* Чтения данных голоса из кодека CMX                   */
        Read_Voice( CODECx );
      }
      break;        
    case SYNC_CODEC_EV:       

      break;        
    case TIMEOUT_EV:  
      /* Сброс приоритета микрофона */
      //CODECx->codec_mic_priority = VOICE_PRIOR_IDLE; 
      /* Установить режим установки speaker                      */
      CODECx->fsm_codec = FSM_SET_SPEAKER_A;      
      /* Установить таймаут запуска поэтапного включения speaker */
      soft_timer_set( CODECx, 10 );//120
      break; 
    default:
      break;  
    }   
    break;
/*:::::::::::::::::::::::;;;;:::::::::::::::::::::::::::::::::::::::::::::::::*/      
  case FSM_MIC_MUTE:         
    switch (mess->data_event)
    {
    case SPEAKER_EV:  
      /* Обновить полученный приоритет спикера */
      CODECx->codec_speaker_priority = mess->data_codec; 
      /* Установить режим установки speaker                      */
      CODECx->fsm_codec = FSM_SET_SPEAKER_A;      
      /* Установить таймаут запуска поэтапного включения speaker */
      soft_timer_set( CODECx, 10 );//120
      break;   
    case IRQ_CODEC_EV:  
      /* Прочитать статус - сбросить прерывания */
      get_codec_status( CODECx );
      break;  
    case MIC_EV: 
      /* Контроль потока микрофона */
      soft_timer_set( CODECx, 240); 
      break;       
    default:
      break;  
    }   
    break; 
/*:::::::::::::::::::::::;;;;:::::::::::::::::::::::::::::::::::::::::::::::::*/      
    default:     
      /* Остановить таймер */
      xTimerStop( CODECx->xTimer_CODECx, 0 );
      /* переключение кодека на режим инициализации */     
      CODECx->fsm_codec = FSM_WAIT_INIT_CMX;
      break;
  }
  
  /* Отработка индикации режимов кодека*/
  switch (CODECx->fsm_codec)
  {
  case FSM_SPEAKER: 
    /* LED MIC */ 
    if ( ( CODECx->LED_MIC_PORTx ) != NULL )
    {
      /* Погасить светодиод */
      CODECx->LED_MIC_PORTx->BSRRH = CODECx->LED_MIC_Pin;
    }
    /* LED SPK */ 
    if ( ( CODECx->LED_SPK_PORTx ) != NULL )
    {
      /* Погасить светодиод */
      CODECx->LED_SPK_PORTx->BSRRL = CODECx->LED_SPK_Pin;
    }
    break;
  case FSM_MIC: 
    /* LED MIC */ 
    if ( ( CODECx->LED_MIC_PORTx ) != NULL )
    {
      /* Погасить светодиод */
      CODECx->LED_MIC_PORTx->BSRRL = CODECx->LED_MIC_Pin;
    }
    
    /* LED SPK */ 
    if ( ( CODECx->LED_SPK_PORTx ) != NULL )
    {
      /* Погасить светодиод */
      CODECx->LED_SPK_PORTx->BSRRH = CODECx->LED_SPK_Pin;
    }
    break;
    
  case FSM_NONE:
  case FSM_INIT_POINT:    
  case FSM_WAIT_INIT_CMX:    
  case FSM_INIT_CMX:  
  case FSM_SET_SPEAKER_A:  
  case FSM_SET_SPEAKER_B:  
  case FSM_SET_SPEAKER_C:  
  case FSM_SPEAKER_MUTE:  
  case FSM_SET_MIC_A:  
  case FSM_SET_MIC_B:  
  case FSM_SET_MIC_C:  
  case FSM_MIC_MUTE:       
  default:     
    
    /* LED MIC */ 
    if ( ( CODECx->LED_MIC_PORTx ) != NULL )
    {
      /* Погасить светодиод */
      CODECx->LED_MIC_PORTx->BSRRH = CODECx->LED_MIC_Pin;
    }
    
    /* LED SPK */ 
    if ( ( CODECx->LED_SPK_PORTx ) != NULL )
    {
      /* Погасить светодиод */
      CODECx->LED_SPK_PORTx->BSRRH = CODECx->LED_SPK_Pin;
    }
    break;
  }
}

/**
  * @brief  Функция формирования маски прослушки для каналов кодеков
  * @param  pcm_cmx_t*  cmx_codec_pcm - указатель на массив кодеков 
  * @retval none
  */
void SetMaskEncodeListing( pcm_cmx_t*  cmx_codec_pcm )
{
  uint8_t index_codec;
  
  if ( ( PortCMX.cnt_index_set_mask ) > 0 ) 
  {
    /* инкремент счетчика периода обновления маски прослушки */
    ( PortCMX.cnt_index_set_mask )--;
  }
  else
  {
    /* Установка периода обновления маски прослушки                             */
    PortCMX.cnt_index_set_mask = PERIOD_UPDATE_CODEC_LISTING_MASK;    
    
    /* Отправка актуальной маски каналов прослушки                              */
    if ((BaseQCMD[VoiceETHPortID].QueueCMD != NULL)&&(BaseQCMD[VoiceETHPortID].Status_QCMD == QCMD_ENABLE))
    {    
      /* Обнуление маски прослушки */
      PortCMX.data_core_tx_cmd.data_dword[0] = 0;
      /* Установка каналов всех кодеков */  
      for( index_codec = 0 ; index_codec < MAX_CODEC; index_codec++ )
      {
        PortCMX.data_core_tx_cmd.data_dword[0] = PortCMX.data_core_tx_cmd.data_dword[0] | BitChID( cmx_codec_pcm[index_codec].codec_ch_id );
      }  
      /* Сброс маски обновления                                                   */
      PortCMX.data_core_tx_cmd.data_dword[1] = 0;
      /* Установка источник команды                                               */
      PortCMX.data_core_tx_cmd.PortID = PortCMX.OwnPortID;                                     
      
      /* Маска не сжатых каналов */
      PortCMX.data_core_tx_cmd.CMD_ID = CMD_MASK_CODE_CH_CODEC_UPDATE;            
      /* Отправка команды */
      xQueueSend ( BaseQCMD[VoiceETHPortID].QueueCMD, ( void * )&(PortCMX.data_core_tx_cmd) , ( TickType_t ) 0 );
      
      /* Маска сжатых каналов */
      PortCMX.data_core_tx_cmd.CMD_ID = CMD_MASK_ENCODE_CH_UPDATE;            
      /* Отправка команды */
      xQueueSend ( BaseQCMD[VoiceETHPortID].QueueCMD, ( void * )&(PortCMX.data_core_tx_cmd) , ( TickType_t ) 0 );      
    }  
  }  
}

/**
  * @brief  Задача данными кодеков CMX
  * @param  pvParameters not used
  * @retval None
  */
void codec_data_task( void* pvParameters )
{
  uint8_t cnt_index_codec;

  /*=========== Функции инициализации аппаратных ресурсов pcm интерфейса кодека ========*/   
  for (cnt_index_codec = 0; cnt_index_codec < MAX_CODEC; cnt_index_codec++)
  { 
    pcm_spi_init(&(cmx_pcm[cnt_index_codec])); 
  }
  /* Запуск генератора патерна.*/
  start_stream_dma_data();    
  /*=====================================================================================*/  
  
  /*=========== Функции инициализации аппаратных ресурсов cmx интерфейса кодека==========*/   
  /* Ждем 200 млсек для нормализации питания */
  vTaskDelay(200);  
  /* Инициализация SPI */
  spi_interface_init(2000000);
  /* Функция инициализации аппаратных ресурсов CMX интерфейса кодека                   */ 
  for (cnt_index_codec = 0; cnt_index_codec < MAX_CODEC; cnt_index_codec++)
  {
    cmx_spi_init(&(cmx_pcm[cnt_index_codec]));
  }
   /* Открытие очереди сообщения */
  x_codec_message = xQueueCreate( 20, sizeof(mes_codec_t) );   
  /* Открытие задачи контроля кодеков                   */
  xTaskCreate( codec_control_task, "CMXCTR", configMINIMAL_STACK_SIZE*2, NULL, CNTRL_CODEC_TASK_PRIO, NULL ); 
  /*=====================================================================================*/    
  /* Функция инициализации порта сопряжения кодека и мультиплексора */
  InitPortCodecRouterStreams();   
  /* Функция подключения порта сопряжения кодека к мультиплексору */
  StartPortCodecRouterStreams();  
  /*=====================================================================================*/     

  /* Функция регистрации на рассылку. */
  reg_notify_port( CODEC_A_SoftPID , PortCMX.set_port_cmx_router.HandleTask );  
  
  PortCMX.PeriodSoftTimer = 10;
  
  /* Открытие таймера периодического уведомления задачи */
  PortCMX.xSoftTimer = xTimerCreate( "TmNoCodec",             /* Текстовое имя, не используется в RTOS kernel.                    */
                                    PortCMX.PeriodSoftTimer,  /* Период таймера в тиках.                                          */
                                    pdTRUE,                   /* Время будет автоматически перезагружать себя, когда оно истечет. */
                                    NULL,                     /* В функцию параметры не передаем                                  */
                                    TimUpdateNoteCodecTask ); /* Указатель на функцию , которую вызывает таймер.                  */
  
  
  if ( PortCMX.xSoftTimer != NULL ) 
  {
    /* Запуск програмных таймеров */
    xTimerStart( PortCMX.xSoftTimer , 0 );
  }
  else
  {
    /* Ошибка запуска таймера */
    while(1);
  }  
  
  /*  Функция формирования маски прослушки для каналов кодеков */
  SetMaskEncodeListing( cmx_pcm );  
  
  /* запускаем периодический запуск задачи 	                  */
  for( ;; )
  {
    /* Обнуляем сообщение */
    PortCMX.NotifiedValue = 0;
    /*================================== Проверка наличия сообщений ========================================*/
    xTaskNotifyWait(0x00000000,               /* Не очищайте биты уведомлений при входе               */
                    0xFFFFFFFF,               /* Сбросить значение уведомления до 0 при выходе        */
                    &(PortCMX.NotifiedValue), /* Значение уведомленное передается в  ulNotifiedValue  */
                    portMAX_DELAY );          /* Ждем пока не посучим сообщения                       */
    
    /*====================== контроль ресурсов PCM кодека  ================================*/     
    /* Проверка всех кодеков */  
    for(uint8_t index_codec = 0 ; index_codec < MAX_CODEC; index_codec++ )
    {  
      /* Получено уведомление. Проверяем, какие биты были установлены. */
      if( ( ( PortCMX.NotifiedValue ) & (PCM_BUFF_A_CMPL<<(cmx_pcm[index_codec].note_codec_offset)) ) != 0 )
      {/* Буфер A PCM DMA отработан. */
        /* Функция функция обработки уведомления из прерывания DMA PCM */
        ProcessingNotifyDMA( &(cmx_pcm[index_codec]) , 0);
      }
      
      if( ( ( PortCMX.NotifiedValue ) & (PCM_BUFF_B_CMPL<<(cmx_pcm[index_codec].note_codec_offset)) ) != 0 )
      {/* Буфер B PCM DMA отработан. */
        /* Функция функция обработки уведомления из прерывания DMA PCM */
        ProcessingNotifyDMA( &(cmx_pcm[index_codec]) , 1);
      }
    } 
    /*=====================================================================================*/  
    /*=====================================================================================*/     
    if( ( ( PortCMX.NotifiedValue ) & ROUTER_NOTE ) != 0 )
    {  
      /* Функция приема голосового пакета от CMX, формирования фрейма и отправки его в роутер */
      VoiceFrameToCodecCMX();      
    }
    /*=====================================================================================*/      
    /*=====================================================================================*/      
    if( ( ( PortCMX.NotifiedValue ) & RECEIVING_NOTE ) != 0 )
    {   
      /* Функция приема голосового пакета из stream роутера потоков захват потока канала и отправка в CMX */
      VoiceFrameToRouterCMX();
    }    
    else
    {
      /* Повторный запуск функции проверки (если уведомления не было) */
      VoiceFrameToRouterCMX();    
    }  

    if( ( ( PortCMX.NotifiedValue ) & TIMER_NOTE ) != 0 )
    { /* Периодическое уведомление по таймеру */
      
      /*Функция обновления статуса захвата потока каналов */
      UpdateVoiceFrameToCodecCMX((uint16_t)(PortCMX.PeriodSoftTimer));
      /* Функция обновления статуса захвата канала отправки данных с CMX в потоковый роутер */
      UpdateStatusVoiceFrameToRouterCMX((uint16_t)(PortCMX.PeriodSoftTimer));
    }  
    if( ( ( PortCMX.NotifiedValue ) & SOFT_ROUTER_NOTE ) != 0 )
    { /* Периодическое уведомление по таймеру */
      /* Функция контроля интерфейсов PCM кодека */
      ProcessingPCM();   
    }  
    else
    {
      /* Повторный запуск функции проверки (если уведомления не было) */
      /* Функция контроля интерфейсов PCM кодека */
      ProcessingPCM();   
    }
  }    
}

/**
  * @brief  Задача управления кодеками CMX
  * @param  pvParameters not used
  * @retval None
  */
void codec_control_task( void* pvParameters )
{
  uint8_t       index_codec_old;
  mes_codec_t   mess_codec; 

  /* Инициализация очереди на 20 сообщений */
  x_codec_message = xQueueCreate( 20 , sizeof(mes_codec_t));
  
  /* запускаем периодический запуск задачи 	                  */
  for( ;; )
  {
    /* Для сообщений используем очередь                                 */
    /* Проверка наличия сообщения - ожидание получения сообщения 1 сек. */
    if ( xQueueReceive(x_codec_message , &mess_codec , ( TickType_t )5) == pdTRUE )        
    {
      /* Чтение сообщения    */
      if (mess_codec.index_codec < MAX_CODEC) 
      {
        /* Функция контроля кодека */
        control_codec( &cmx_pcm[mess_codec.index_codec], &mess_codec );
      }
    }
    else
    { /* Нет сообщений слишком долго */
      /* Вычисление следующего индекса контроля кодека */
      index_codec_old++;
      /* Проверка индекса на выход за границу диапазона */ 
      if (index_codec_old >= MAX_CODEC) 
      {
        /* Выход за границу - обнуляем индекс */
        index_codec_old = 0;
      }
      
      /* Формируем пустое сообщение*/
      mess_codec.index_codec = index_codec_old;
      mess_codec.data_event = NONE_EV;
      mess_codec.data_codec = 0;
      /* Отправляем пустое сообщение кодеку */
      xQueueSend( x_codec_message , (void*)&(mess_codec), ( TickType_t )0 );
    }  
  }    
}

/**
  * @brief  Функция инициализации порта и запуска задачи сопряжения кодека и мультиплексора 
  * @param  None
  * @retval None
  */
void InitCodecTaskControl( void )
{ 
  /* Функция прописывания указателей на индивидуальные ресурсы интерфейсов кодека */
  init_pcm_codec_a();  
  init_pcm_codec_b();  
  init_pcm_codec_c();    
  /* Открытие задачи сопряжения кодека и мультиплексора */
  xTaskCreate( codec_data_task, "CMXDAT", configMINIMAL_STACK_SIZE*2, NULL, CNTRL_CODEC_TASK_PRIO, &(PortCMX.set_port_cmx_router.HandleTask) );
}
/******************* (C) COPYRIGHT 2020 DataExpress *****END OF FILE****/
