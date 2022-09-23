/**
  ******************************************************************************
  * @file    cmx_gate.с
  * @author  Trembach Dmitry
  * @version V1.5.0
  * @date    13-02-2019
  * @brief   Инициализация задачи сопряжения кодека и мультиплексора
  *
  ******************************************************************************
  * @attention
  * 
  * Задача контролирующая подключение голосового кодека и контроллера консоли ПДШ
  * к мультиплексору.
  * 
  * 1.Получение и парсинг голосовых пакетов из мультиплексора.
  * 2.Подготовка, групировка и отправка голосовых пакетов в мультиплексор.
  * 3.Отправка в кодек пакетов только заданного канала и привязка к источнику сообщения.
  * 4.Отправка в кодек пакетов только с заданного массива.
  * 5.Получение из кодека пакетов и формирование из них групп, формирование шапки 
  * для задданного канала(каналов).
  * 6.Получение и парсинг команд вызова диспетчера 
  * 7.Отправка команды вызова диспетчера контроллеру ПДШ
  * 8.Получение от контроллера ПДШ команды вызова диспетчера.
  * 9.Формированиешапки для команды вызова диспетчера и отправки ее в мультиплексор.
  * 
  *
  * <h2><center>&copy; COPYRIGHT 2019 DataExpress</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "cmx_gate.h"
#include "Core.h"
#include "router_streams.h"
#include "settings.h"
#include "printf_dbg.h"    
#include "codec_cmx.h"
#include "codec_control.h"
    
uint8_t data_noise_a[] = { 0x40,0x67,0x1A,0xF0,0x7F,0x3A, 0x40,0x27,0xA8,0xF1,0x4F,0x2B, 0x83,0x11,0x46,0xF0,0xAF,0x3B };    
uint8_t data_noise_b[] = { 0x40,0x67,0x90,0xF1,0xFF,0x59, 0x03,0x88,0x22,0xF1,0x8F,0x2A, 0x8B,0xCA,0x58,0xF1,0x6F,0x5A }; 
uint8_t data_noise_c[] = { 0x40,0x67,0x1A,0xF0,0x7F,0x3A, 0x40,0x27,0xA8,0xF1,0x4F,0x2B, 0x83,0x11,0x46,0xF0,0xAF,0x3B }; 
    
//Очереди голосовых пакетов
QueueHandle_t xCMX618_Queue_IN_CODEC;
QueueHandle_t xCMX618_Queue_OUT_CODEC;

/* Массив кодеков */
pcm_cmx_t       cmx_pcm[MAX_CODEC_ARRAY];
/* Определение основной структуры порта подключения к router streams*/
codec_gate_t    PortCMX;    
/* Структура для подготовки сообщения CMX_PCM передаваемого кодеку */
mes_codec_t cmx_lock_mes;

/**
  * @brief  Функция приема голосового пакета из stream роутера потоков захват потока канала и отправка в CMX 
  * @param  none
  * @retval bool - true пакет обработан как голосовой
  *                false пакет не обработан или не получен
  */
bool VoiceFrameToCodecCMX( void )
{
  uint8_t index_codec = 0 ; 
  bool processing_status = false;
  uint8_t temp_priority = 0;    
  
  /* Проверка параметров пакета на соответствие  */
  if ( PortCMX.set_port_cmx_router.QueueOutRouter == NULL ) return processing_status;
  /* Проверяем очередь на наличие пакета и принимаем пакет  */
  if ((xQueueReceive ( PortCMX.set_port_cmx_router.QueueOutRouter, PortCMX.damp_data_rx , ( TickType_t ) 0 ) ) == pdTRUE ) 
  {
    /* Принят пакет - начать обработку   */
    /* ID по Rs                          */
    if(PortCMX.voice_data_rx.rs_head.id != ID_VOICE) return processing_status;                                                  /* ID RS пакета   */
    /* Обработка голосового пакета  */
    switch(PortCMX.voice_data_rx.NwkFrameVoice.header.id1)
    {
    case _RF_DATA_RMAV_VOICE_: 
      temp_priority = VOICE_PRIOR_NORMAL;
      break;    
    case _RF_DATA_RMAV_VOICE_PRIOR_HIGH_: 
      temp_priority = VOICE_PRIOR_HIGH;
      break;         
    case _RF_DATA_RMAV_VOICE_PRIOR_ABOVE_NORMAL_: 
      temp_priority = VOICE_PRIOR_ABOVE_NORMAL;
      break;         
    case _RF_DATA_RMAV_VOICE_PRIOR_BELOW_NORMAL_: 
      temp_priority = VOICE_PRIOR_BELOW_NORMAL;
      break;         
    case _RF_DATA_RMAV_VOICE_PRIOR_LOW_: 
      temp_priority = VOICE_PRIOR_LOW;
      break;         
    default: 
      return processing_status;
      break;          
    }
    /* Проверка всех кодеков */  
    for( index_codec = 0 ; index_codec < MAX_CODEC; index_codec++ )
    {
      /* Проверка соответствия канала */
      if ((PortCMX.voice_data_rx.NwkFrameVoice.header.nwkDstPanId == cmx_pcm[index_codec].codec_ch_id ) || ( PortCMX.voice_data_rx.NwkFrameVoice.header.nwkDstPanId == 0xFF ))
      { 
        /* Отработка режима захвата */       
        /* Переключение автомата    */
        switch (cmx_pcm[index_codec].router_to_fr_fst)
        {
        case CAPT_SRC:    /* Захват канала */
          /* Проверяем источник полученного пакета */
          if ( ( (cmx_pcm[index_codec].cmx_lock_src_phy_addr) == (PortCMX.voice_data_rx.NwkFrameVoice.header.macSrcAddr) ) && ( cmx_pcm[index_codec].cmx_lock_dest_phy_addr) == (PortCMX.voice_data_rx.NwkFrameVoice.header.nwkDstAddr  ) )
          {
            /* Дозагрузка пакетов в очередь кодека                  */
            /* Предварительная установка индекса начала загрузки    */
            cmx_pcm[index_codec].cmx_cnt_index_start = 0;
            /* Цикл вычисления индекса с которого нужно скопировать данные  */
            for(cmx_pcm[index_codec].cmx_cnt_index_rx_data = 0; cmx_pcm[index_codec].cmx_cnt_index_rx_data < SIZE_BUFF_BOX; (cmx_pcm[index_codec].cmx_cnt_index_rx_data)++)
            {
              /* Сравнить индексы полученных голосовых пакетов с индексом следующего для передачи */
              if( ( PortCMX.voice_data_rx.NwkFrameVoice.box[(cmx_pcm[index_codec].cmx_cnt_index_rx_data)].index_box ) == ( cmx_pcm[index_codec].cmx_old_index_box ) )
              { /*  если найдены одинаковые, то ...   */
                /* с него начинаем писать             */
                cmx_pcm[index_codec].cmx_cnt_index_start = cmx_pcm[index_codec].cmx_cnt_index_rx_data;
                break;						
              }
            }
            /* Функция заполнения очереди отправки в CMX  */
            /* копирование новых данных                   */
            for( cmx_pcm[index_codec].cmx_cnt_index_rx_data = cmx_pcm[index_codec].cmx_cnt_index_start; cmx_pcm[index_codec].cmx_cnt_index_rx_data < SIZE_BUFF_BOX; (cmx_pcm[index_codec].cmx_cnt_index_rx_data)++)
            {
              /* Если очередь открыта отправляем данные в кодек */
              if ( cmx_pcm[index_codec].QueueInCodecCMX != NULL)
              {
                /* Отправка пакета в очередь кодека */
                xQueueSend ( cmx_pcm[index_codec].QueueInCodecCMX , ( PortCMX.voice_data_rx.NwkFrameVoice.box[(cmx_pcm[index_codec].cmx_cnt_index_rx_data)].data_voice ) , ( TickType_t ) 0 );
                /* пакет отправлен обновляем хвост  */
                cmx_pcm[index_codec].cmx_old_index_box = ( PortCMX.voice_data_rx.NwkFrameVoice.box[(cmx_pcm[index_codec].cmx_cnt_index_rx_data)].index_box ) + 1;  
                /* Сброс тайм аута */
                cmx_pcm[index_codec].cmx_time_out_rx_box = MAX_TIME_WAIT_ROUTER_BOX;  
              } 
            }
            
            /* Обновление приоритета захваченного канала */
            cmx_pcm[index_codec].cmx_lock_ch_priority = temp_priority;
            /* Отправить сообщение кодеку о получении пакета с заданным приоритетом */
            cmx_lock_mes.data_event = SPEAKER_EV;                                         /* Идентификатор сообщения   */
            cmx_lock_mes.index_codec = cmx_pcm[index_codec].index_codec;                  /* Идентификатор кодека      */ 
            cmx_lock_mes.data_codec = cmx_pcm[index_codec].cmx_lock_ch_priority;          /* Уровень приоритета        */
            xQueueSend( x_codec_message , (void*)&(cmx_lock_mes), ( TickType_t ) 0 );     /* Отправка сообщения        */  
            /* Пакет обработан  */
            processing_status = true;
          }
          else
          {
            /* Проверка приоритета текущего захвыченного потока  - если новый пакет имеет более высокий приоритет - новый захват */
            if (  temp_priority < (cmx_pcm[index_codec].cmx_lock_ch_priority) )
            {
              /* Производим захват нового источника голосовых сообщений - Фиксирование данных пакета захваченного источника */
              cmx_pcm[index_codec].cmx_lock_src_phy_addr = PortCMX.voice_data_rx.NwkFrameVoice.header.macSrcAddr;              /* Физический адрес источника    */
              cmx_pcm[index_codec].cmx_lock_dest_phy_addr = PortCMX.voice_data_rx.NwkFrameVoice.header.nwkDstAddr;             /* Физический адрес получателя   */     
              cmx_pcm[index_codec].cmx_lock_codec_phy_addr = cmx_pcm[index_codec].codec_phy_addr;                              /* Физический адрес шлюза/кодека */
              cmx_pcm[index_codec].cmx_lock_ch_id = PortCMX.voice_data_rx.NwkFrameVoice.header.nwkDstPanId;                    /* Идентификатор канала          */     
              cmx_pcm[index_codec].cmx_lock_ch_priority = temp_priority;                                                       /* Приоритет канала              */          
              
              /* Установка режима - источник захвачен */
              cmx_pcm[index_codec].router_to_fr_fst = CAPT_SRC;  
              
              /* Функция заполнения очереди отправки в CMX */
              /* копирование новых данных                  */
              for( cmx_pcm[index_codec].cmx_cnt_index_rx_data = 0; cmx_pcm[index_codec].cmx_cnt_index_rx_data < SIZE_BUFF_BOX; (cmx_pcm[index_codec].cmx_cnt_index_rx_data)++)
              {
                /* Если очередь открыта отправляем данные в кодек */
                if ( cmx_pcm[index_codec].QueueInCodecCMX != NULL)
                {
                  /* Отправка пакета в очередь кодека */
                  xQueueSend ( cmx_pcm[index_codec].QueueInCodecCMX , ( PortCMX.voice_data_rx.NwkFrameVoice.box[(cmx_pcm[index_codec].cmx_cnt_index_rx_data)].data_voice ) , ( TickType_t ) 0 );
                  /* пакет отправлен обновляем хвост  */
                  cmx_pcm[index_codec].cmx_old_index_box = ( PortCMX.voice_data_rx.NwkFrameVoice.box[(cmx_pcm[index_codec].cmx_cnt_index_rx_data)].index_box ) + 1; 
                  /* Сброс тайм аута */
                  cmx_pcm[index_codec].cmx_time_out_rx_box = MAX_TIME_WAIT_ROUTER_BOX;  
                } 
              }
              /* Отправить сообщение кодеку о получении пакета с заданным приоритетом */
              cmx_lock_mes.data_event = SPEAKER_EV;                                         /* Идентификатор сообщения   */
              cmx_lock_mes.index_codec = cmx_pcm[index_codec].index_codec;                  /* Идентификатор кодека      */ 
              cmx_lock_mes.data_codec = cmx_pcm[index_codec].cmx_lock_ch_priority;          /* Уровень приоритета        */
              xQueueSend( x_codec_message , (void*)&(cmx_lock_mes), ( TickType_t ) 0 );     /* Отправка сообщения        */  
              /* Пакет обработан  */
              processing_status = true;
            }
          }  
          break;
        case WAIT_CAPT:   /* Ожидание пакетов */
        default:
          /* Производим захват источника голосовых сообщений - Фиксирование данных пакета захваченного источника */
          cmx_pcm[index_codec].cmx_lock_src_phy_addr = PortCMX.voice_data_rx.NwkFrameVoice.header.macSrcAddr;              /* Физический адрес источника    */
          cmx_pcm[index_codec].cmx_lock_dest_phy_addr = PortCMX.voice_data_rx.NwkFrameVoice.header.nwkDstAddr;             /* Физический адрес получателя   */     
          cmx_pcm[index_codec].cmx_lock_codec_phy_addr = cmx_pcm[index_codec].codec_phy_addr;                              /* Физический адрес шлюза/кодека */
          cmx_pcm[index_codec].cmx_lock_ch_id = PortCMX.voice_data_rx.NwkFrameVoice.header.nwkDstPanId;                    /* Идентификатор канала          */     
          cmx_pcm[index_codec].cmx_lock_ch_priority = temp_priority;                                                       /* Приоритет канала              */          
   
          /* Установка режима - источник захвачен */
          cmx_pcm[index_codec].router_to_fr_fst = CAPT_SRC;  
          
          /* Функция заполнения очереди отправки в CMX */
          /* копирование новых данных                  */
          for( cmx_pcm[index_codec].cmx_cnt_index_rx_data = 0; cmx_pcm[index_codec].cmx_cnt_index_rx_data < SIZE_BUFF_BOX; (cmx_pcm[index_codec].cmx_cnt_index_rx_data)++)
          {
            /* Если очередь открыта отправляем данные в кодек */
            if ( cmx_pcm[index_codec].QueueInCodecCMX != NULL)
            {
              /* Отправка пакета в очередь кодека */
              xQueueSend ( cmx_pcm[index_codec].QueueInCodecCMX , ( PortCMX.voice_data_rx.NwkFrameVoice.box[(cmx_pcm[index_codec].cmx_cnt_index_rx_data)].data_voice ) , ( TickType_t ) 0 );
              /* пакет отправлен обновляем хвост  */
              cmx_pcm[index_codec].cmx_old_index_box = ( PortCMX.voice_data_rx.NwkFrameVoice.box[(cmx_pcm[index_codec].cmx_cnt_index_rx_data)].index_box ) + 1; 
              /* Сброс тайм аута */
              cmx_pcm[index_codec].cmx_time_out_rx_box = MAX_TIME_WAIT_ROUTER_BOX; 
            } 
          }
          /* Отправить сообщение кодеку о получении пакета с заданным приоритетом */
          cmx_lock_mes.data_event = SPEAKER_EV;                                         /* Идентификатор сообщения   */
          cmx_lock_mes.index_codec = cmx_pcm[index_codec].index_codec;                  /* Идентификатор кодека      */ 
          cmx_lock_mes.data_codec = cmx_pcm[index_codec].cmx_lock_ch_priority;          /* Уровень приоритета        */
          xQueueSend( x_codec_message , (void*)&(cmx_lock_mes), ( TickType_t ) 0 );     /* Отправка сообщения        */ 
          /* Пакет обработан  */
          processing_status = true;
        }   
      }
    }
  }
  return processing_status;    
}

/**
  * @brief  Функция обновления статуса захвата потока каналов
  * @param  uint16_t time_update - период обновления 
  * @retval none
  */
void UpdateVoiceFrameToCodecCMX( uint16_t time_update )
  {
  uint8_t index_codec = 0 ;   
  
    /* Проверка всех кодеков */  
    for( index_codec = 0 ; index_codec < MAX_CODEC; index_codec++ )
    {
      /* Нет данных - проверка тайм аута */
    if ( (cmx_pcm[index_codec].cmx_time_out_rx_box) > time_update)
      {
        /* Уменьшение тайм аута на период контроля */
      (cmx_pcm[index_codec].cmx_time_out_rx_box) = (cmx_pcm[index_codec].cmx_time_out_rx_box) - time_update;
      }
      else
      {
        if ( cmx_pcm[index_codec].router_to_fr_fst == CAPT_SRC )
        {
          /* Если тай аут более 1 сек - установка режима ожидания */
          cmx_pcm[index_codec].router_to_fr_fst = WAIT_CAPT;
          /* Обнуляем таблицу идентификаторов захваченного канала */
          cmx_pcm[index_codec].cmx_lock_src_phy_addr = 0;    /* Физический адрес источника    */
          cmx_pcm[index_codec].cmx_lock_dest_phy_addr = 0;   /* Физический адрес получателя   */     
          cmx_pcm[index_codec].cmx_lock_codec_phy_addr = 0;  /* Физический адрес шлюза/кодека */
          cmx_pcm[index_codec].cmx_lock_ch_id = 0;           /* Идентификатор канала          */     
          cmx_pcm[index_codec].cmx_lock_ch_priority = 0;     /* Приоритет канала              */  
        }
        /* Сброс тайм аута                                      */ 
        cmx_pcm[index_codec].cmx_time_out_rx_box = MAX_TIME_WAIT_ROUTER_BOX;        
      }  
    }
  }  

/**
  * @brief  Функция инициализации констант голосового фрейма
  * @param  pcm_cmx_t* cmx_pcm - указатель на структуру контроля кодека
  * @retval None
  */
void InitHeaderVoiceFrame( pcm_cmx_t* pcm_cmx )
{      
  /* Формирование шапки пакета по RS                                  */
  pcm_cmx->cmx_voice_data_tx.rs_head.pre = 0xAA55;                                  /* Преамбула  0x55 0xAA                                            */
  pcm_cmx->cmx_voice_data_tx.rs_head.lenght = 0x0076;                               /* Длина пакета (включая контрольную сумму, не включая преамбулу)  */
  pcm_cmx->cmx_voice_data_tx.rs_head.id = ID_VOICE;                                 /* Идентификатор пакета                                            */
  pcm_cmx->cmx_voice_data_tx.rs_head.dest = 0xFFFF;                                 /* Физический адрес получателя                                     */
  pcm_cmx->cmx_voice_data_tx.rs_head.src = DataLoaderSto.Settings.phy_adr;          /* Физический адрес источника                                      */
  pcm_cmx->cmx_voice_data_tx.NwkFrameVoice.size = 0x6d;                             /* длинна голосового пакета                                        */
  /* Формирование шапки пакета по RF                                  */
  /* Заголовок MAC в посылке                                          */    
  pcm_cmx->cmx_voice_data_tx.NwkFrameVoice.header.macFcf = 0x8841;                                /* frame control                                     */
  pcm_cmx->cmx_voice_data_tx.NwkFrameVoice.header.macDstPanId = 0xABCD;                           /* PANID кому предназначались данные                 */
  pcm_cmx->cmx_voice_data_tx.NwkFrameVoice.header.macDstAddr = 0xFFFF;                            /* Адрес кому предназначались данные                 */
  pcm_cmx->cmx_voice_data_tx.NwkFrameVoice.header.macSrcAddr = DataLoaderSto.Settings.phy_adr;    /* Адрес от кого предавались данные                  */
  /* Заголовок NWK в посылке (входит в PAYLOAD посылки по IEEE  802.15.4) */
  //pcm_cmx->cmx_voice_data_tx.NwkFrameVoice.header.id1 = _RF_DATA_RMAV_VOICE_PRIOR_LOW_;                     /* ID1 пакета                                        */
  //pcm_cmx->cmx_voice_data_tx.NwkFrameVoice.header.id2 = (uint8_t)~(unsigned)_RF_DATA_RMAV_VOICE_PRIOR_LOW_; /* ID2 пакета                                        */
  pcm_cmx->cmx_voice_data_tx.NwkFrameVoice.header.nwkFcf_u.val = 0x10;                            /* Поле настройки фрейма                             */
  pcm_cmx->cmx_voice_data_tx.NwkFrameVoice.header.nwkDstRouterAddr = 0xFF;                        /* Адрес роутера которому предназначаются данные     */
  pcm_cmx->cmx_voice_data_tx.NwkFrameVoice.header.nwkSrcPanId = 0x01;                             /* Сетевой PANID источника                           */
  pcm_cmx->cmx_voice_data_tx.NwkFrameVoice.header.nwkSrcHop = 0x00;                               /* Хоп источника данных                              */
  pcm_cmx->cmx_voice_data_tx.NwkFrameVoice.header.nwkOwnHop = 0x00;                               /* Собственный хоп                                   */
  pcm_cmx->cmx_voice_data_tx.NwkFrameVoice.header.nwkSrcAddr = DataLoaderSto.Settings.phy_adr;    /* Адрес источника                                   */
  pcm_cmx->cmx_voice_data_tx.NwkFrameVoice.header.nwkDstAddr = 0xFFFF;                            /* Адрес назначения                                  */
  pcm_cmx->cmx_voice_data_tx.NwkFrameVoice.header.nwkSrcEndpoint = 0x1;                           /* Endpoint источника                                */
  pcm_cmx->cmx_voice_data_tx.NwkFrameVoice.header.nwkDstEndpoint = 0x1;                           /* Endpoint  назначения                              */
  //pcm_cmx->cmx_voice_data_tx.NwkFrameVoice.header.nwk_src_factory_addr = DataLoaderSto.Settings.phy_adr;/* Заводской адрес источника текущей посылки   */
  pcm_cmx->cmx_voice_data_tx.NwkFrameVoice.header.nwk_own_factory_addr = DataLoaderSto.Settings.phy_adr;/* Заводской адрес инициатора посылки          */ 
  pcm_cmx->cmx_voice_data_tx.NwkFrameVoice.header.nwk_count_routing = 0x00;                       /* Счётчик маршрутизаций                             */
  pcm_cmx->cmx_voice_data_tx.NwkFrameVoice.header.reserv1 = 0x00;                                 /* резерв 1                                          */
  pcm_cmx->cmx_voice_data_tx.NwkFrameVoice.header.reserv2 = 0x0000;                               /* резерв 2                                          */
}

/**
  * @brief  Функция приема голосового пакета от CMX, формирования фрейма и отправки его в роутер 
  * @param  None
  * @retval None
  */
void VoiceFrameToRouterCMX( void )
{
  uint8_t index_codec = 0 ;   
  uint8_t cnt_index_cmx_tx_data = 0;
  uint8_t temp_priority = 0 ;     
  
  /* Проверка всех кодеков */  
  for( index_codec = 0 ; index_codec < MAX_CODEC; index_codec++ )
  {
    /* Проверяем входящую очередь от кодека         */
    if ( cmx_pcm[index_codec].QueueOutCodecCMX != NULL)
    {
      /* Проверка наличия данных в очереди                               */
      if ( uxQueueMessagesWaiting( cmx_pcm[index_codec].QueueOutCodecCMX ) > 0 )
      {
        /* данные в очереди есть                                              */
        /* Сдвиг содержимого пакета                                           */
        for( cnt_index_cmx_tx_data = 0 ; cnt_index_cmx_tx_data  < SIZE_DATA_CMX618_VOICE; (cnt_index_cmx_tx_data)++ )
        {
          /* если в буфере один пакет - остальное заполняем шумом */
          if ( (cmx_pcm[index_codec].fr_to_router_fst) == WAIT_BOX)
          {
            cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.box[0].data_voice[cnt_index_cmx_tx_data] = data_noise_a[cnt_index_cmx_tx_data];
            cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.box[1].data_voice[cnt_index_cmx_tx_data] = data_noise_b[cnt_index_cmx_tx_data];      
            cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.box[2].data_voice[cnt_index_cmx_tx_data] = data_noise_c[cnt_index_cmx_tx_data];             
          }
          else
          {
            cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.box[0].data_voice[cnt_index_cmx_tx_data] = cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.box[1].data_voice[cnt_index_cmx_tx_data];
            cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.box[1].data_voice[cnt_index_cmx_tx_data] = cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.box[2].data_voice[cnt_index_cmx_tx_data];      
            cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.box[2].data_voice[cnt_index_cmx_tx_data] = cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.box[3].data_voice[cnt_index_cmx_tx_data];          
          }  
        }
        /* Сдвиг индексов пакетов                                             */
        cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.box[0].index_box = cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.box[1].index_box;
        cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.box[1].index_box = cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.box[2].index_box;      
        cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.box[2].index_box = cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.box[3].index_box;    
        /* Загрузка данных                                                    */
        /* Проверяем наличие входящих пакетов                                 */
        xQueueReceive(  cmx_pcm[index_codec].QueueOutCodecCMX  , &(cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.box[3].data_voice[0]) , ( TickType_t ) 0);
        /* Запись индекса                                                     */  
        cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.box[3].index_box = cmx_pcm[index_codec].cmx_cnt_index_box;     
        
        /* Инкремент индекса бокса в фрейме с проверкой на переполнение       */
        if (( cmx_pcm[index_codec].cmx_cnt_index_box ) > 254)  ( cmx_pcm[index_codec].cmx_cnt_index_box ) = 0;    /* Обнуление индекса */  
        else    ( cmx_pcm[index_codec].cmx_cnt_index_box ) ++;     /* Инкремент индекса */
        
        /*   Сброс тайм аута                                                  */
        cmx_pcm[index_codec].cmx_time_out_tx_box = MAX_TIME_WAIT_CMX_BOX;        
        
        /*   Переключение автомата формирования голосовых пакетов для роутера */
        switch ( cmx_pcm[index_codec].fr_to_router_fst )
        {
          //        case ONE_RECEIVE_BOX:       /* 1 пакет в буфере                       */
          //          cmx_pcm[index_codec].fr_to_router_fst = TWO_RECEIVE_BOX;
          //          break;		
          //        case TWO_RECEIVE_BOX:       /* 2 пакета в буфере                      */
          //          cmx_pcm[index_codec].fr_to_router_fst = THREE_RECEIVE_BOX;
          //          break;		
          //        case THREE_RECEIVE_BOX:     /* 3 пакета в буфере                      */
          //          cmx_pcm[index_codec].fr_to_router_fst = RECEIVE_BOX_SEND_FRAME;
          //          break;		
        case RECEIVE_BOX_SEND_FRAME:/* Прием пакета - передача фрейма         */
          cmx_pcm[index_codec].fr_to_router_fst = RECEIVE_BOX_PASS_FRAME;
          break;		
        case RECEIVE_BOX_PASS_FRAME:/* Прием пакета - пропуск передачи фрейма */
          cmx_pcm[index_codec].fr_to_router_fst = RECEIVE_BOX_SEND_FRAME;
          break;						
        case WAIT_BOX:              /* Буфер пуст - ожидание пакетов          */
        default:
          /* 1 пакет в буфере          */ 
          /* остальное заполнено шумом */    
          cmx_pcm[index_codec].fr_to_router_fst = RECEIVE_BOX_SEND_FRAME;
          break;
        }
        
        /* Формирование фрейма для передачи */    
        if (( cmx_pcm[index_codec].fr_to_router_fst ) == RECEIVE_BOX_SEND_FRAME)
        {
          /* Инкремент индекса бокса в фрейме с проверкой на переполнение     */
          if (( cmx_pcm[index_codec].cmx_cnt_tx_voice_frame ) > 254)  ( cmx_pcm[index_codec].cmx_cnt_tx_voice_frame ) = 0; /* Обнуление индекса */
          else   ( cmx_pcm[index_codec].cmx_cnt_tx_voice_frame )++;   /* Инкремент индекса */  
          
          /* Инициализация констант голосового фрейма */
          InitHeaderVoiceFrame(&(cmx_pcm[index_codec]));     
          
          if ( ( cmx_pcm[index_codec].pcm_lock_ch_priority  > VOICE_PRIOR_LOW ) || ( cmx_pcm[index_codec].pcm_lock_ch_priority  < VOICE_PRIOR_HIGH ) )
          {
            temp_priority = cmx_pcm[index_codec].pcm_ch_priority;
          }
          else
          {
            temp_priority = cmx_pcm[index_codec].pcm_lock_ch_priority;
          }  
          
          /* Установка приоритета пакета */
          switch( temp_priority )
          {
        
          case VOICE_PRIOR_LOW:
            cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.header.id1 = _RF_DATA_RMAV_VOICE_PRIOR_LOW_;                     /* ID1 пакета                                        */
            cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.header.id2 = (uint8_t)~(unsigned)_RF_DATA_RMAV_VOICE_PRIOR_LOW_; /* ID2 пакета                                        */            
            break;            
            
          case VOICE_PRIOR_BELOW_NORMAL:
            cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.header.id1 = _RF_DATA_RMAV_VOICE_PRIOR_BELOW_NORMAL_;                     /* ID1 пакета                                        */
            cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.header.id2 = (uint8_t)~(unsigned)_RF_DATA_RMAV_VOICE_PRIOR_BELOW_NORMAL_; /* ID2 пакета                                        */            
            break;        
            
          default:            
          case VOICE_PRIOR_NORMAL:
            cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.header.id1 = _RF_DATA_RMAV_VOICE_;                     /* ID1 пакета                                        */
            cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.header.id2 = (uint8_t)~(unsigned)_RF_DATA_RMAV_VOICE_; /* ID2 пакета                                        */            
            break;            
            
          case VOICE_PRIOR_ABOVE_NORMAL:
            cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.header.id1 = _RF_DATA_RMAV_VOICE_PRIOR_ABOVE_NORMAL_;                     /* ID1 пакета                                        */
            cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.header.id2 = (uint8_t)~(unsigned)_RF_DATA_RMAV_VOICE_PRIOR_ABOVE_NORMAL_; /* ID2 пакета                                        */            
            break;            
            
          case VOICE_PRIOR_HIGH:
            cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.header.id1 = _RF_DATA_RMAV_VOICE_PRIOR_HIGH_;                     /* ID1 пакета                                        */
            cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.header.id2 = (uint8_t)~(unsigned)_RF_DATA_RMAV_VOICE_PRIOR_HIGH_; /* ID2 пакета                                        */            
            break;            
          }
          
//          cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.header.id1 = _RF_DATA_RMAV_VOICE_;                     /* ID1 пакета                                        */
//          cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.header.id2 = (uint8_t)~(unsigned)_RF_DATA_RMAV_VOICE_; /* ID2 пакета                                        */           
          
         
          cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.header.nwk_src_factory_addr = cmx_pcm[index_codec].pcm_lock_src_phy_addr;/* Заводской адрес источника текущей посылки   */
          cmx_pcm[index_codec].cmx_voice_data_tx.rs_head.cnt = cmx_pcm[index_codec].cmx_cnt_tx_voice_frame;                 /* Cчетчик неприрывности пакетов 0..255                  */
          cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.header.macSeq = cmx_pcm[index_codec].cmx_cnt_tx_voice_frame; /* Cчетчик неприрывности пакетов 0..255                  */
          cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.header.nwkSeq = cmx_pcm[index_codec].cmx_cnt_tx_voice_frame; /* Cчетчик неприрывности пакетов 0..255                  */
          cmx_pcm[index_codec].cmx_voice_data_tx.NwkFrameVoice.header.nwkDstPanId = cmx_pcm[index_codec].pcm_lock_ch_id;    /* Сетевой PANID назначения (он же зона для ретрасляции) */       
          /* Отправка пакета в роутер - проверка наличия очереди */
          if ( PortCMX.set_port_cmx_router.QueueInRouter != NULL )
          {
            /* Отправляем пакет */
            xQueueSend( PortCMX.set_port_cmx_router.QueueInRouter, cmx_pcm[index_codec].cmx_damp_data_tx, ( TickType_t ) 0 );
            
            if ( ( PortCMX.set_port_cmx_router.HandleTaskRouter ) != NULL )
            {
              xTaskNotify( PortCMX.set_port_cmx_router.HandleTaskRouter,                    /* Указатель на уведомлюемую задачу                         */
                          ( PortCMX.set_port_cmx_router.PortBitID ) << OFFSET_NOTE_PORD_ID, /* Значения уведомления                                     */
                          eSetBits );                                                           /* Текущее уведомление добавляются к уже прописанному       */
            } 
          } 
        }
      }
    }      
  }  
}

/**
  * @brief  Функция обновления статуса захвата канала отправки данных с CMX в потоковый роутер 
  * @param  uint16_t time_update - период обновления 
  * @retval None
  */
void UpdateStatusVoiceFrameToRouterCMX( uint16_t time_update )
{
  uint8_t index_codec = 0 ;   
  /* Проверка всех кодеков */  
  for( index_codec = 0 ; index_codec < MAX_CODEC; index_codec++ )
  {
    /* Проверяем входящую очередь от кодека         */
    if ( cmx_pcm[index_codec].QueueOutCodecCMX != NULL)
    {
      /* Нет данных - проверка тайм аута */
      if ( ( cmx_pcm[index_codec].cmx_time_out_tx_box ) > time_update)
      {
        /* Уменьшение тайм аута на период контроля */
        ( cmx_pcm[index_codec].cmx_time_out_tx_box ) = ( cmx_pcm[index_codec].cmx_time_out_tx_box ) - time_update;
      }
      else
      {
        /* Если тай аут более 120 млсек - установка режима ожидания */
        cmx_pcm[index_codec].fr_to_router_fst = WAIT_BOX;
        /* Сброс тайм аута */
        cmx_pcm[index_codec].cmx_time_out_tx_box = MAX_TIME_WAIT_CMX_BOX;  
      }  
    }  
  }  
}

/**
  * @brief  Функция инициализации порта сопряжения кодека и мультиплексора 
  * @param  None
  * @retval None
  */
void InitPortCodecRouterStreams( void )
{
  /* Зададим размер очереди маршрутизатор - cmx */
  PortCMX.size_queue_router_cmx  = 6;
  /* Зададим размер очереди cmx - маршрутизатор */  
  PortCMX.size_queue_cmx_router  = 6;
    
  /* Открытие очереди для пакетов router to cmx */
  if ((PortCMX.set_port_cmx_router.QueueOutRouter == NULL)&&(PortCMX.size_queue_router_cmx > 0))
  {
    //открытие очереди для size_queue_rs_router пакетов
    PortCMX.set_port_cmx_router.QueueOutRouter = xQueueCreate( PortCMX.size_queue_router_cmx , sizeof(router_box_t)); 
  }	
  /* Открытие очереди для пакетов cmx to router */  
  if ((PortCMX.set_port_cmx_router.QueueInRouter == NULL)&&(PortCMX.size_queue_cmx_router > 0))
  {
    //открытие очереди для size_queue_rs_router пакетов
    PortCMX.set_port_cmx_router.QueueInRouter = xQueueCreate( PortCMX.size_queue_cmx_router , sizeof(router_box_t)); 
  }
  // Открытие очереди для приема комманд
  if ( PortCMX.xQueueCoreCMD == NULL )
  {
    //открытие очереди для приема MAX_SIZE_QUEUE_CNTRL_CMD комманд
    PortCMX.xQueueCoreCMD = xQueueCreate( MAX_SIZE_QUEUE_CNTRL_CMD , sizeof(cntrl_cmd_t));
  }
  // Прописать идентификатор порта
  PortCMX.OwnPortID = CodecPortID;  
  // Проводим инициализацию подключения к мультиплексору
  PortCMX.set_port_cmx_router.FlagLockAddr = LOCK_ADDR;                                     /* блокирует трансляцию в порт со своим адресом   */
  PortCMX.set_port_cmx_router.MaskPortID = DataSto.Settings.codec_mask_inpup_port_id_codec; /* установка маску ID портов                      */
  PortCMX.set_port_cmx_router.NumMaskBoxID = DataSto.Settings.codec_nmask_transl_codec;     /* используем маску ID пакетов                    */
  PortCMX.set_port_cmx_router.PortBitID = BitID(CodecPortID);                               /* идентификатор порта порт                       */
  PortCMX.set_port_cmx_router.HandleTaskRouter = NULL;                                      /* Указатель на задачу роутера - используется для отправки сообщений в задачу роутера */ 
}

/**
  * @brief  Функция подключения порта сопряжения кодека к мультиплексору 
  * @param  None
  * @retval None
  */
void StartPortCodecRouterStreams( void )
{
  // Регистрирование очереди команд в таблице
  if (PortCMX.xQueueCoreCMD != NULL)
  {
    // Ожидание инициализации указателя в таблице команд
    while(BaseQCMD[PortCMX.OwnPortID].Status_QCMD != QCMD_INIT)
    {
      // Ожидание 1 млсек
      vTaskDelay(1);
    }
    // Обнуление укаазтеля
    BaseQCMD[PortCMX.OwnPortID].QueueCMD = PortCMX.xQueueCoreCMD;
    // Установка статуса
    BaseQCMD[PortCMX.OwnPortID].Status_QCMD = QCMD_ENABLE;          
  }    
  
  /*---------- Подключение данного ресурса к мультиплексору ------------------*/ 
  // Запрашиваем ID порта маршрутизации
  while(request_port_pnt(&(PortCMX.index_router_port)) != true) vTaskDelay(1);
  // Инициализация порта
  settings_port_pnt(PortCMX.index_router_port,&(PortCMX.set_port_cmx_router));
  // Включаем порт
  enable_port_pnt(PortCMX.index_router_port);
  /*--------------------------------------------------------------------------*/
}
/******************* (C) COPYRIGHT 2020 DataExpress *****END OF FILE****/
