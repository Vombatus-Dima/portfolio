/**
  ******************************************************************************
  * @file    core_cntrl_rsn.с
  * @author  Trembach Dmitry
  * @version V1.1.0
  * @date    12-03-2019
  * @brief   файл с функциями ядра для контроля РСН в своей области
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
#include "core_cntrl_rsn.h"
#include "rf_frame_header.h"
#include "settings.h"    
#include "board.h"  
#include "printf_dbg.h"

/* Переменные для хранения маски каналов прослушки */
volatile uint16_t cnt_update_timelife = 0;   

/* счетчик периода позиционирования    */
volatile uint16_t control_position_time = 0;

/* размер данных под теги */
#define MAX_SIZE_DATA  (sizeof(NwkFrame_TX_t) - sizeof(RSFrameTagNewPosV3_t))

volatile uint16_t       cnt_up_reg;   // счетчик индекса регистрации
volatile uint16_t       cnt_uptime;   // счетчик индекса обновления времени

/**
  * @brief Функция получения индекса порта источника по заданному ID  
  * @param uint8_t src_port_ID - порт ID источника
  * @param uint8_t* pindex_port_ID - указатель на индекс порта источника ID источника
  * @retval bool - true - есть индекс для порта
  *		   false - нет индекса для порта 
  */
bool get_index_for_port_scr_tag( uint8_t src_port_ID, uint8_t* pindex_port_ID )  
{
  /* поиск индекса */
  for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
  { 
    if ( st_core.tag_ch_stat[cnt_tab].SrcPortID == src_port_ID )
    {
      /* индекс найден */
      *pindex_port_ID = cnt_tab;       
      return true;
    }
  } 
  /* индекс не найден */
  return false;
}

/*============================= Работаем с каналами ==========================*/
/**
  * @brief Функция обнуления таблицы каналов голосовых тегов
  * @param cntrol_chanel_t*  tab_chanel - указатель на таблицу активных каналов
  * @retval none
  */
void ClearTabChanel( cntrol_chanel_t*  tab_chanel )
{
  for( uint8_t  cnt_reg_ch = 0 ; cnt_reg_ch < MAX_SIZE_TAB_CH_VOICE_TAG; cnt_reg_ch++)
  {
    /* Обнуление переменной подсчета числа активных голосовых тегов */
    tab_chanel[cnt_reg_ch].TempNumberVoiceTag = 0;    
  }
}

/**
  * @brief Функция добавления в таблицы каналов голосовых тегов
  * @param uint8_t chanel - номер канала
  * @param cntrol_chanel_t*  tab_chanel - указатель на таблицу активных каналов
  * @retval none
  */
void AddChanel( uint8_t chanel,  cntrol_chanel_t*  tab_chanel )
{
  if ( chanel < MAX_SIZE_TAB_CH_VOICE_TAG)
  {
    /* Обнуление переменной подсчета числа активных голосовых тегов */
    (tab_chanel[chanel].TempNumberVoiceTag)++;    
  }
}

/**
  * @brief Функция копирования таблицы каналов голосовых тегов
  * @param cntrol_chanel_t*  tab_chanel - указатель на таблицу активных каналов
  * @retval uint32_t - маска зарегистрированных каналов голосовых тегов
  */
uint32_t UpdateTabChanel( cntrol_chanel_t* tab_chanel )
{
  uint32_t masc_voice_tag_reg = 0;
  /* Цикл по всей таблице */ 
  for( uint8_t  cnt_reg_ch = 1 ; cnt_reg_ch < MAX_SIZE_TAB_CH_VOICE_TAG; cnt_reg_ch++)
  {
    /* Если новое число зарегистрированных тегов на канале меньше чем старое */
    if ( tab_chanel[cnt_reg_ch].NumberVoiceTag > tab_chanel[cnt_reg_ch].TempNumberVoiceTag)
    {
      /* Отсчитываем время регистрации */
      if ( ( tab_chanel[cnt_reg_ch].ChanelTimeLife ) > 0)
      {
        (tab_chanel[cnt_reg_ch].ChanelTimeLife)--; 
      }
      else
      {
        /* Обновления числа регистрации */
        tab_chanel[cnt_reg_ch].NumberVoiceTag = tab_chanel[cnt_reg_ch].TempNumberVoiceTag;
        tab_chanel[cnt_reg_ch].ChanelTimeLife = MAX_TTL_TAB_REG_CH_RSN;
      }  
    }
    else
    {
      /* Обновления числа регистрации */
      tab_chanel[cnt_reg_ch].NumberVoiceTag = tab_chanel[cnt_reg_ch].TempNumberVoiceTag;
      tab_chanel[cnt_reg_ch].ChanelTimeLife = MAX_TTL_TAB_REG_CH_RSN;
    }
    
    /* Если есть зарегистрированные теги - устанавливаем маску */
    if ( tab_chanel[cnt_reg_ch].NumberVoiceTag > 0 ) 
    {
      masc_voice_tag_reg = masc_voice_tag_reg | BitChID(cnt_reg_ch); 
    }
  }
  return masc_voice_tag_reg;
}

/**
  * @brief Функция проверяет таблицу на наличие голосовых тегов на канале 
  * @param uint8_t Chanel - номер канала
  * @param uint8_t src_port_ID - порт ID источника
  * @retval bool - true - есть зарегистрированные теги
  *		 false  - нет зарегистрированныз тегов 
  */
bool GetRegVoiceTagChanel( uint8_t Chanel, uint8_t src_port_ID )
{
  /* Поиск таблицы каналов для заданного порта */
  for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
  { 
    if ( st_core.tag_ch_stat[cnt_tab].SrcPortID == src_port_ID )
    {
      /* Таблица найдена - проверка наличия тегов*/
      if ( st_core.tag_ch_stat[cnt_tab].TabChanel[Chanel].ChanelTimeLife > 0) return true;
      else return false;
    }
  }
  return false;
}

/**
  * @brief Функция проверяет таблицу на наличие голосовых тегов на канале 
  * @param uint8_t Chanel - номер канала
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - колличество тегов  
  */
uint8_t GetNumVoiceTagChanel( uint8_t Chanel, uint8_t src_port_ID )
{
  /* Поиск таблицы каналов для заданного порта */
  for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
  { 
    if ( st_core.tag_ch_stat[cnt_tab].SrcPortID == src_port_ID )
    {
      return st_core.tag_ch_stat[cnt_tab].TabChanel[Chanel].NumberVoiceTag;
    }
  }
  return 0;
}

/**
  * @brief Функция возвращает число зарегистрированных каналов  
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - колличество каналов 
  */
uint8_t GetRegChanel( uint8_t src_port_ID )
{
  /* индекс порта источника ID  */
  uint8_t pindex_id_port;
  /* число каналов*/
  uint8_t num_chanel = 0;
  
  /* получения индекса порта источника по заданному ID */
  if ( get_index_for_port_scr_tag( src_port_ID, &pindex_id_port ) == false ) 
  {
    /* некорректный порт ID источника */  
    return 0; 
  }    
  
  /* Цикл по всей таблице каналов*/ 
  for( uint8_t  cnt_reg_ch = 1 ; cnt_reg_ch < MAX_SIZE_TAB_CH_VOICE_TAG; cnt_reg_ch++)
  {
    /* Если новое число зарегистрированных тегов на канале больше нуля */
    if (  st_core.tag_ch_stat[pindex_id_port].TabChanel[cnt_reg_ch].NumberVoiceTag > 0)
    {
      /* Подсчет каналов */
      num_chanel++;
    }
  }  
  return num_chanel;
}

/**
  * @brief Функция возвращает число зарегистрированных/видимых тегов светильников  
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - колличество тегов 
  */
uint8_t GetRegTagRMA( uint8_t src_port_ID )
{
  /* Поиск таблицы для заданного порта */
  for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
  { 
    if ( st_core.tag_ch_stat[cnt_tab].SrcPortID == src_port_ID )
    {
      return st_core.tag_ch_stat[cnt_tab].TagStatistic.cnt_rma_mems_v3;
    }
  }
  return 0;
}

/**
  * @brief Функция возвращает число зарегистрированных/видимых тегов радиостанций  
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - колличество тегов 
  */
uint8_t GetRegTagRSN( uint8_t src_port_ID )
{
  /* Поиск таблицы для заданного порта */
  for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
  { 
    if ( st_core.tag_ch_stat[cnt_tab].SrcPortID == src_port_ID )
    {
      return st_core.tag_ch_stat[cnt_tab].TagStatistic.cnt_rsn_v3 + st_core.tag_ch_stat[cnt_tab].TagStatistic.cnt_rsn_se2435_v3 + \
        st_core.tag_ch_stat[cnt_tab].TagStatistic.cnt_rss_se2435_v3 + st_core.tag_ch_stat[cnt_tab].TagStatistic.cnt_rsnp_v3;
    }
  }
  return 0;
}

/**
  * @brief Функция возвращает число зарегистрированных/видимых тегов газоанализаторов  
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - колличество тегов 
  */
uint8_t GetRegTagGas( uint8_t src_port_ID )
{
  /* Поиск таблицы для заданного порта */
  for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
  { 
    if ( st_core.tag_ch_stat[cnt_tab].SrcPortID == src_port_ID )
    {
      return st_core.tag_ch_stat[cnt_tab].TagStatistic.cnt_rma_methane_sputnik_v3;
    }
  }
  return 0;
}

/**
  * @brief Функция возвращает число зарегистрированных/видимых тегов   
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - колличество тегов 
  */
uint8_t GetRegTagAll( uint8_t src_port_ID )
{
  /* Поиск таблицы для заданного порта */
  for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
  { 
    if ( st_core.tag_ch_stat[cnt_tab].SrcPortID == src_port_ID )
    {
      return st_core.tag_ch_stat[cnt_tab].TagStatistic.cnt_rma_methane_sputnik_v3 + \
        st_core.tag_ch_stat[cnt_tab].TagStatistic.cnt_rsn_v3 + st_core.tag_ch_stat[cnt_tab].TagStatistic.cnt_rsn_se2435_v3 + \
        st_core.tag_ch_stat[cnt_tab].TagStatistic.cnt_rss_se2435_v3 + st_core.tag_ch_stat[cnt_tab].TagStatistic.cnt_rsnp_v3 + \
        st_core.tag_ch_stat[cnt_tab].TagStatistic.cnt_rma_mems_v3;
    }
  }
  return 0;
}

/**
  * @brief Функция проверяет таблицу на наличие голосовых тегов 
  * @param uint8_t src_port_ID - порт ID источника
  * @retval bool - true - есть зарегистрированные теги
  *		 false  - нет зарегистрированныз тегов 
  */
bool GetRegVoiceTag( uint8_t src_port_ID )
{
  for( uint8_t cnt_reg_ch = 1 ; cnt_reg_ch < MAX_SIZE_TAB_CH_VOICE_TAG; cnt_reg_ch++)
  {
    if ( ( GetRegVoiceTagChanel(cnt_reg_ch, src_port_ID) )> 0) return true;
  }
  return false;  
}

/**
  * @brief Функция возвращает число голосовых тегов зарегистрированных на заданном канале  
  * @param uint8_t Chanel  - канал регистрации РСН
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t  число голосовых тегов зарегистрированных на заданном канале
  */
uint8_t GetNumRegVoiceTagChanel( uint8_t Chanel , uint8_t src_port_ID)
{
  /* Поиск таблицы каналов для заданного порта */
  for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
  { 
    if ( st_core.tag_ch_stat[cnt_tab].SrcPortID == src_port_ID )
    {
      /* Таблица найдена - возвращаем число голосовых тегов зарегистрированных на заданном канале */
      return st_core.tag_ch_stat[cnt_tab].TabChanel[Chanel].NumberVoiceTag;
    }
  }  
  /* Нет зарегистрированных голосовых тегов */
  return 0;
}
/*======================  Функции управления тегами  =========================*/
/*  1.Обнуление записей базы тегов.                                           */
/*  2.Занесение тега в базу                                                   */
/*  3.Удаление тега из базы                                                   */
/*  4.Формирование отчетов по тегам                                           */
/*  5.Функции парсинга для каждого формата тегов                              */
/*                                                                            */
/*                                                                            */

/* Указатель на облась памяти хранения тегов */
info_tag_t *tag_base = (info_tag_t *)0x1000D000; 
/* Доступно 12272 байт -> 264 тега */
#define MAX_NUM_TAG ((uint16_t)( ( ( 0x1000FFF0 - 0x1000D000 ) / (sizeof(info_tag_t)) ) - 2 )) 

/**
  * @brief Функция определяет для задданого индекса максимальный тайминг времени жизни по ресурсам
  * @param uint16_t IndexTag  - указатель на индекс тега в таблице
  * @retval uint8_t - тайминг времени жизни
  */
uint8_t GetMaxTOFL( uint16_t IndexTag )
{
  /* Предустановка переменной максимума */
  uint8_t MaxTOFL = 0;
  /* Поиск максимума */
  for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
  { 
    if ( ( tag_base[IndexTag].TOFL[cnt_tab] ) > MaxTOFL )
    {
      /* Обновление максимума */
      MaxTOFL = tag_base[IndexTag].TOFL[cnt_tab];
    }
  } 
  return MaxTOFL;
}

/**
  * @brief Функция анализа режима обновления времени и регистрации тега  
  * @param uint16_t src_addr - адрес источник пакета позиционирования
  * @param uint16_t vrm_addr - адрес врм где позиционируется тег
  * @param uint16_t index_tag - индекс тега в таблице
  * @param uint8_t stat_recd_tag - статус возвращенного индекса
  *                   0 - нет индекса нет места.
  *                   1 - есть уже в таблице.
  *                   2 - нет идекса есть место в таблице 
  * @param uint8_t src_port_id - порт ID источника пакета позиционирования
  * @retval bool true - требуется обновления данных тега в таблице 
  *              false - не требуется обновления  
  */
bool UpdateTagTimeReg( uint16_t src_addr, uint16_t vrm_addr, uint16_t index_tag, uint8_t stat_recd_tag, uint8_t src_port_id )
{
  /* индекс порта источника ID  */
  uint8_t p_index_id;    
  
  if ( ( stat_recd_tag == 2 ) || ( src_addr == vrm_addr ) )
  { /* Если источник этого пакета лучший ВРМ или это первая запись        */
    if ( vrm_addr == DataLoaderSto.Settings.phy_adr ) 
    {/* Если лучший ВРМ текущий - обновление регистрации позиционирования */
      if ( get_index_for_port_scr_tag( PositionID, &p_index_id )) 
      {
        tag_base[index_tag].TOFL[p_index_id] = MAX_TIME_OF_LIFE_ACTIVE_TAG; 
      }       
    }
    /* Обновление регистрации видимости по ресурсу                        */
    if ( get_index_for_port_scr_tag( src_port_id, &p_index_id )) 
    {
      tag_base[index_tag].TOFL[p_index_id] = MAX_TIME_OF_LIFE_TAG; 
    } 
    /* Установка запроса обновления таблицы                               */
    return true;
  }
  else
  { /* Если нет - обновление только времени  ресурса                      */
    /* Обновление регистрации видимости по ресурсу                        */
    if ( get_index_for_port_scr_tag( src_port_id, &p_index_id )) 
    {
      /* Ищем по всем портам максимальное время и заносим в данный ресурс   */    
      tag_base[index_tag].TOFL[p_index_id] = GetMaxTOFL( index_tag ); 
    }
    /* Обновления в таблицу не заносим */
    return false;  
  }
}

/**
  * @brief Функция обнуления записи в таблице тегов
  * @param info_tag_t *tag_record - указатель на запись в таблице тегов
  * @retval none
  */
void ClearTagTable(info_tag_t *tag_record)
{
  for ( uint8_t cntik_tag_byte = 0; cntik_tag_byte < sizeof(info_tag_t); cntik_tag_byte++ )
  {
    ((uint8_t*)tag_record)[cntik_tag_byte] = 0;
  }
}

/**
  * @brief Функция обнуления всей таблицы тегов
  * @param none
  * @retval none
  */
void ClearTableTag( void )
{
  for ( uint16_t cntik_tag_record = 0; cntik_tag_record < MAX_NUM_TAG; cntik_tag_record++ )
  {
    ClearTagTable(&(tag_base[cntik_tag_record]));
  }
}

/**
  * @brief Функция обнуления всей статистики тегов
  * @param tag_stat_t* status_tag - указатель на массив статистики тегов
  * @retval none
  */
void ClearStaticTag( tag_stat_t* status_tag )
{
  status_tag->cnt_no = 0;                     // RF_TYPE_ABON_NO                      = 0x00,
  status_tag->cnt_rma = 0;                    // RF_TYPE_ABON_RMA                     = 0x01,
  status_tag->cnt_rmat = 0;                   // RF_TYPE_ABON_RMAT                    = 0x10,
  status_tag->cnt_rsn = 0;                    // RF_TYPE_ABON_RSN                     = 0x20,
  status_tag->cnt_rsn_se2435l = 0;            // RF_TYPE_ABON_RSN_SE2435L             = 0x21, 
  status_tag->cnt_rsnp = 0;                   // RF_TYPE_ABON_RSNP                    = 0x30, 
  status_tag->cnt_rma_methane_sputnik_v3 = 0; // RF_TYPE_ABON_RMA_METHANE_SPUTNIK_pv3 = 0x50, 
  status_tag->cnt_rma_mems_v3 = 0;            // RF_TYPE_ABON_RMA_MEMS_pv3            = 0x60, 
  status_tag->cnt_rsn_v3 = 0;                 // RF_TYPE_ABON_RSN_pv3                 = 0x70, 
  status_tag->cnt_rsn_se2435_v3 = 0;          // RF_TYPE_ABON_RSN_SE2435L_pv3         = 0x71,   
  status_tag->cnt_rss_se2435_v3 = 0;          // RF_TYPE_ABON_RSS_SE2435L_pv3         = 0x72, 
  status_tag->cnt_rsnp_v3 = 0;                // RF_TYPE_ABON_RSNP_pv3                = 0x73  
}

/**
  * @brief Функция обновления статистики тегов
  * @param uint8_t tag_type - тип регистрируемого тега
  * @param tag_stat_t* status_tag - указатель на массив статистики тегов
  * @retval none
  */
void UpdateStaticTag( uint8_t tag_type,  tag_stat_t* status_tag )
{
  /*  Формирование статистических данных */
  switch( tag_type )
  {
  case RF_TYPE_ABON_NO                     :  (status_tag->cnt_no)++;                      break;
  case RF_TYPE_ABON_RMA                    :  (status_tag->cnt_rma)++;                     break;
  case RF_TYPE_ABON_RMAT                   :  (status_tag->cnt_rmat)++;                    break;
  case RF_TYPE_ABON_RSN                    :  (status_tag->cnt_rsn)++;                     break;
  case RF_TYPE_ABON_RSN_SE2435L            :  (status_tag->cnt_rsn_se2435l)++;             break;
  case RF_TYPE_ABON_RSNP                   :  (status_tag->cnt_rsnp)++;                    break;
  case RF_TYPE_ABON_RMA_METHANE_SPUTNIK_pv3:  (status_tag->cnt_rma_methane_sputnik_v3)++;  break;
  case RF_TYPE_ABON_RMA_MEMS_pv3           :  (status_tag->cnt_rma_mems_v3)++;             break;   
  case RF_TYPE_ABON_RSN_pv3                :  (status_tag->cnt_rsn_v3)++;                  break;
  case RF_TYPE_ABON_RSN_SE2435L_pv3        :  (status_tag->cnt_rsn_se2435_v3)++;           break;
  case RF_TYPE_ABON_RSS_SE2435L_pv3        :  (status_tag->cnt_rss_se2435_v3)++;           break;
  case RF_TYPE_ABON_RSNP_pv3               :  (status_tag->cnt_rsnp_v3)++;                 break;   
  
  default:
    break;
  }
}

/**
  * @brief Функция обновляет время жизни зарегистрированного тега, если время жизни исчерпано
  *        удаляем тег из таблицы
  *
  * @param uint16_t update_time - период обновления
  * @retval uint16_t - число зарегистрированных тегов в таблице
  */
uint16_t UpdateTagTable( uint16_t update_time )
{
  uint16_t cntik_tag_record = 0;
  uint16_t index_tag_offset_table = 0;  
  uint8_t flag_end_of_life;
  
  /* Обнуление вспомогательных таблиц статистики голосовых тегов */
  for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
  {
    ClearTabChanel( st_core.tag_ch_stat[cnt_tab].TabChanel );  
    ClearStaticTag( &(st_core.tag_ch_stat[cnt_tab].TagStatistic) );  
  }
  
  /* Инициализация индекса обновления */
  cntik_tag_record = 0;

  /* Выполняем обновление времени жизни рассылок */
  while( cntik_tag_record < MAX_NUM_TAG )
  {
    /* Если все активные теги обновлены  - выход */
    if ( tag_base[cntik_tag_record].ADDR_Tag == 0x0000 ) break;    
      
    /* Установка флага завершения активности тега  */
    flag_end_of_life = 1;
    /* Цикл по всем источникам */
    for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )    
    {
      /* Обновление времени жизни записи по активным источникам*/
      if ( tag_base[cntik_tag_record].TOFL[cnt_tab] > 0 )
      {
        /* Уменьшение времени жизни */
        (tag_base[cntik_tag_record].TOFL[cnt_tab])--;    
        /* Обновление статистики по тегу */
        UpdateStaticTag(tag_base[cntik_tag_record].TypeTag, &(st_core.tag_ch_stat[cnt_tab].TagStatistic) );
        /* Формирование статистических данных по каналам*/
        /* Статистические данные по регистрации тегов по каналам */
        switch( tag_base[cntik_tag_record].TypeTag )
        {
        case RF_TYPE_ABON_RSN_pv3:
        case RF_TYPE_ABON_RSN_SE2435L_pv3: 
        case RF_TYPE_ABON_RSS_SE2435L_pv3:  
        case RF_TYPE_ABON_RSNP_pv3:           
          /* Если индекс порта совпадает в статистику видимости по порту */
          AddChanel(((data_tag_rsn_t*)(tag_base[cntik_tag_record].DATA))->ZoneTag,st_core.tag_ch_stat[cnt_tab].TabChanel);
          break;
        default:
          break;
        }       
        
        /* Сброс флага завершения активности тега  */
        flag_end_of_life = 0;        
        
      }      
    }
    /* Время жизни тега вышло */
    if ( flag_end_of_life > 0 )
    {
      /* Поиск последней записи */
      for ( index_tag_offset_table = cntik_tag_record; index_tag_offset_table < ( MAX_NUM_TAG - 1 ); index_tag_offset_table++ )
      {
        /* Если следующая запись пустая - выход */
        if ( tag_base[index_tag_offset_table+1].ADDR_Tag == 0x0000 ) break; 
      }
      /* Смещение данных идентификации устройства */             
      for ( uint8_t cntik_tag_byte = 0; cntik_tag_byte < sizeof(info_tag_t); cntik_tag_byte++ )
      {
        /* Копирование данных тега */
        ((uint8_t*)&(tag_base[cntik_tag_record]))[cntik_tag_byte] = ((uint8_t*)&(tag_base[index_tag_offset_table]))[cntik_tag_byte];
        /* Обнуление дубликата записи в таблице */
        ((uint8_t*)&(tag_base[index_tag_offset_table]))[cntik_tag_byte] = 0;
      }
    }
    /* Инкрементирование индексов тегов */
    cntik_tag_record++;
  } 
    
  /* Обновление числа голосовых тегов на канале и формирование маски прослушки  */
  for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
  { 
    st_core.tag_ch_stat[cnt_tab].MaskCodeChReg = UpdateTabChanel( st_core.tag_ch_stat[cnt_tab].TabChanel ); 
  }
  
  /* Число тегов в таблице */
  return cntik_tag_record;
}

/**
  * @brief Функция обновляет маску каналов прослушки
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  * @param uint16_t   TimeUpdateReg - временной интервал с последнего обновления регистрации
  * @retval none
  */
void UpdateChanelMask( core_struct_t* core_st, uint16_t   TimeUpdateReg )
{
  /* Обнуление переменной для маски каналов */
  core_st->TempMaskCodeChReg = 0;
  /* Формирование общей маски каналов */
  for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
  {
    core_st->TempMaskCodeChReg = core_st->TempMaskCodeChReg | st_core.tag_ch_stat[cnt_tab].MaskCodeChReg;     
  }
    
  /* Проверяем маски каналов RS */
  if ( ( core_st->MaskCodeChReg ) != ( core_st->TempMaskCodeChReg ) )
  {
    /* Если есть разница  */
    /* Отправка актуальной маски каналов прослушки                                                                */
    if ((BaseQCMD[VoiceETHPortID].QueueCMD != NULL)&&(BaseQCMD[VoiceETHPortID].Status_QCMD == QCMD_ENABLE))
    {
      /* Подготовка команды команды */
      core_st->data_core_tx_cmd.CMD_ID = CMD_MASK_CODE_CH_RS_UPDATE;            /* Тип команды                     */
      core_st->data_core_tx_cmd.PortID = CorePortID;                            /* Источник команды                */
      core_st->data_core_tx_cmd.data_dword[0] = ( core_st->TempMaskCodeChReg );                                     /* Отправка диагностики на сервер  */
      core_st->data_core_tx_cmd.data_dword[1] = ( core_st->TempMaskCodeChReg ) ^ ( core_st->MaskCodeChReg );        /* Отправка диагностики на сервер  */   

      /* Отправка команды */
      xQueueSend ( BaseQCMD[VoiceETHPortID].QueueCMD, ( void * )&(core_st->data_core_tx_cmd) , ( TickType_t ) 0 );
    }
    /* Выполняем обновление времени жизни регистраций тегов рсн */
    cnt_update_timelife = MAX_UPDATE_TTL_TAB_REG_CH_TAG;   
  }
    /* Сохраняем маску */
    core_st->MaskCodeChReg = core_st->TempMaskCodeChReg;    

  /* Проверяем счетчик периода обновления */
  if (cnt_update_timelife > TimeUpdateReg)
  {
    /* Отсчитываем период обновления */
    cnt_update_timelife = cnt_update_timelife - TimeUpdateReg;
  }
  else
  {
    /* Выполняем обновление времени жизни регистраций тегов рсн */
    cnt_update_timelife = MAX_UPDATE_TTL_TAB_REG_CH_TAG;

    /* Отправка актуальной маски каналов прослушки                                                                */
    if ((BaseQCMD[VoiceETHPortID].QueueCMD != NULL)&&(BaseQCMD[VoiceETHPortID].Status_QCMD == QCMD_ENABLE))
    {
      /* Подготовка команды команды */
      core_st->data_core_tx_cmd.CMD_ID = CMD_MASK_CODE_CH_RS_UPDATE;           /* Тип команды                     */
      core_st->data_core_tx_cmd.PortID = CorePortID;                           /* Источник команды                */
      core_st->data_core_tx_cmd.data_dword[0] = core_st->MaskCodeChReg;      /* Отправка диагностики на сервер  */
      core_st->data_core_tx_cmd.data_dword[1] = 0;                             /* Отправка диагностики на сервер  */   

      /* Отправка команды */
      xQueueSend ( BaseQCMD[VoiceETHPortID].QueueCMD, ( void * )&(core_st->data_core_tx_cmd) , ( TickType_t ) 0 );
    }
  }
}

/**
  * @brief Функция проверяет таблицу на наличие зарегистрированного тега, если в таблице уже
  *        такой адрес тега есть - заносим индекс, если нет, но есть место
  *        в таблице заносим индекс, если места нет - ответ нет места индекс нулевой
  *
  * @param uint16_t  AddrTag  - адрес позиционируемого тега
  * @param uint16_t  *IndexTag  - указатель на индекс тега в таблице
  * @retval uint8_t - статус возвращенного индекса
  *                   0 - нет индекса нет места.
  *                   1 - есть уже в таблице.
  *                   2 - нет идекса есть место в таблице 
  */
uint8_t GetIndexTagTable(uint16_t Addr_Tag, uint16_t *IndexTag )
{
  /* Цикл по таблице тегов */
  for ( uint16_t cntik_tag_record = 0; cntik_tag_record < MAX_NUM_TAG; cntik_tag_record++ )
  {
    if ( tag_base[cntik_tag_record].ADDR_Tag == 0x0000 )
    {
      /* Установка индека */
      *IndexTag = cntik_tag_record;
      /* Нет идекса есть место в таблице */
      return 2;
    }
    
    if ( tag_base[cntik_tag_record].ADDR_Tag == Addr_Tag )
    {
      /* Установка индека */
      *IndexTag = cntik_tag_record;
      /* Нет идекса есть место в таблице */
      return 1;
    }
  }
  /* Установка индека */
  *IndexTag = 0;
  /* 0 - нет индекса нет места */
  return 0;
}

/**
  * @brief Функция проверки регистрации тега эфире на  
  * @param uint16_t ADDR_Tag - адрес тега
  * @param uint8_t src_port_ID - порт ID источника
  * @retval bool - true зарегистрирован
  *                false не зарегистрирован
  */
bool CheckRegTag( uint16_t ADDR_Tag, uint8_t src_port_ID )    
{
  /* индекс порта источника ID  */
  uint8_t pindex_id_port;
  /* получения индекса порта источника по заданному ID */
  if ( get_index_for_port_scr_tag( src_port_ID, &pindex_id_port ) == false ) 
  {
    /* некорректный порт ID источника */  
    return false; 
  }  
  /* Цикл по таблице тегов */
  for ( uint16_t cntik_tag_record = 0; cntik_tag_record < MAX_NUM_TAG; cntik_tag_record++ )
  {
    if ( tag_base[cntik_tag_record].ADDR_Tag == 0x0000 )
    {
      /* Нет зарегистрированного тега таблице */
      return false;
    }
    
    if ( ( tag_base[cntik_tag_record].ADDR_Tag == ADDR_Tag ) && ( tag_base[cntik_tag_record].TOFL[pindex_id_port] > 0 ) )
    {
      /* Тег зарегистрирован в таблице */
      return true;
    }
  }
  /* Нет нет зарегистрированного тега таблице */
  return false;
}    
  
/*____________________________________________________________________________*/

/*____________________________________________________________________________*/
/*______________________________RMA stm_______________________________________*/

/**
  * @brief Функция формировани отчета по тегу RMA - stm загрузчик программа в таблице    
  * @param pos_tag_rma_stm_loader_t *tag_box - указатель на данные позиционирования
  * @param uint16_t index_tag_report - индекс тега 
  * @retval uint8_t - размер обработанных данных
  */
uint8_t ReportTagStmLoaderTable( pos_tag_rma_stm_loader_t *tag_box, uint16_t index_tag_report )
{
  tag_box->head_tag.TypeTag = tag_base[index_tag_report].TypeTag;            /*	   Тип позиционируемого тега          */
  tag_box->head_tag.ADDR_Tag = tag_base[index_tag_report].ADDR_Tag;          /*    Адрес позиционируемого тега        */    
  tag_box->head_tag.ED_RMA_VRM = tag_base[index_tag_report].ED_RMA_VRM;      /*	   Уровень сигнала от тега к ВРМ      */  
  tag_box->head_tag.DATA_ID = tag_base[index_tag_report].DATA_ID;            /*	   ID данных пакета позиционирования  */     
  
  /* Заполнение поля данных */                                             
  for ( uint8_t contik_data_report = 0 ; contik_data_report < sizeof(data_tag_rma_stm_loader_t); contik_data_report++ ) 
  {
    ((uint8_t*)&(tag_box->DATA_TAG))[contik_data_report] = tag_base[index_tag_report].DATA[contik_data_report];
  }
  
  /* Функция для формирования диагностического сообщения по рассылке пакета позиционирования.*/
  diag_tx_mes_pos( "<<_REPORT_POS_", tag_box->head_tag.TypeTag, tag_box->head_tag.ADDR_Tag, tag_box->vrm[0].PHY_ADDR_VRM );
  
  switch ( DataSto.Settings.number_position_vrm )                                                    
  {                                                                        
  case 0:                                                                  
    return sizeof(pos_tag_rma_stm_loader_t) - 3*(sizeof(ed_best_vrm_t));          
  case 1:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    return sizeof(pos_tag_rma_stm_loader_t) - 2*(sizeof(ed_best_vrm_t));          
  case 2:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    tag_box->vrm[1].ED_VRM_RMA =   tag_base[index_tag_report].vrm[1].ED_VRM_RMA;    
    tag_box->vrm[1].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[1].PHY_ADDR_VRM;  
    return sizeof(pos_tag_rma_stm_loader_t) - 1*(sizeof(ed_best_vrm_t));          
  case 3:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    tag_box->vrm[1].ED_VRM_RMA =   tag_base[index_tag_report].vrm[1].ED_VRM_RMA;    
    tag_box->vrm[1].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[1].PHY_ADDR_VRM;  
    tag_box->vrm[2].ED_VRM_RMA =   tag_base[index_tag_report].vrm[2].ED_VRM_RMA;    
    tag_box->vrm[2].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[2].PHY_ADDR_VRM; 
    return sizeof(pos_tag_rma_stm_loader_t);
  default:
    /* Oбработанных данных нет */
    return 0;
  }
}

/**
  * @brief Функция регистрации тега RMA - stm загрузчик программа в таблице 
  * @param pos_tag_rma_stm_loader_t *tag_box - указатель на данные позиционирования
  * @param uint8_t number_vrm - число врм для позиционирования
  * @param uint16_t src_phy_addr - физический адрес источника
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - размер обработанных данных
  */
uint8_t RegTagStmLoaderTable( pos_tag_rma_stm_loader_t *tag_box, uint8_t number_vrm, uint16_t src_phy_addr, uint8_t src_port_ID )
{
  /* Объявляем переменную для индекса */
  uint16_t index_teg = 0;
  /* Переменная статуса записи в таблицу */
  uint8_t stat_record = 0;  
  
  /* Запрос индекса таблицы тегов     */
  stat_record = GetIndexTagTable( tag_box->head_tag.ADDR_Tag, &index_teg );
    
  if ( stat_record > 0 )
  {/* Получен индекс для записи в таблицу */
    /* Функция анализа режима обновления времени и регистрации тега */
    if ( UpdateTagTimeReg( src_phy_addr, tag_box->vrm[0].PHY_ADDR_VRM, index_teg, stat_record, src_port_ID ) )
    {
    /* Заполнение таблицы тегов */
    tag_base[index_teg].VERSION_ID = 3;		                             /*	   Версия пакета                      */
    tag_base[index_teg].src_phy_addr = src_phy_addr;                         /*	   Физический адрес источника         */
                                                                             
    tag_base[index_teg].TypeTag = tag_box->head_tag.TypeTag;	             /*	   Тип позиционируемого тега          */
    tag_base[index_teg].ADDR_Tag = tag_box->head_tag.ADDR_Tag;	             /*    Адрес позиционируемого тега        */
    tag_base[index_teg].ED_RMA_VRM = tag_box->head_tag.ED_RMA_VRM;           /*	   Уровень сигнала от тега к ВРМ      */
    tag_base[index_teg].DATA_ID = tag_box->head_tag.DATA_ID;                 /*	   ID данных пакета позиционирования  */        
    
    /* Заполнение поля данных */                                             
    for ( uint8_t contik_data_tag = 0 ; contik_data_tag < MAX_SIZE_DATA_POZ; contik_data_tag++ ) 
    {
      if (contik_data_tag < sizeof(data_tag_rma_stm_loader_t) )
      {
        tag_base[index_teg].DATA[contik_data_tag] = ((uint8_t*)&(tag_box->DATA_TAG))[contik_data_tag];
      }
      else
      {
        tag_base[index_teg].DATA[contik_data_tag] = 0; 
      }  
    }    
    
    tag_base[index_teg].vrm[0].MAC_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */     
    tag_base[index_teg].vrm[1].MAC_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */
    tag_base[index_teg].vrm[2].MAC_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */        
                                       
    /* Функция для формирования диагностического сообщения по парсингу пакета позиционирования.*/
    diag_reg_rs_pos( "<<_REG_RS__POS_", src_phy_addr, tag_box->head_tag.TypeTag, tag_box->head_tag.ADDR_Tag, tag_box->vrm[0].PHY_ADDR_VRM );
    
    switch ( number_vrm )                                                    
    {                                                                        
    case 0:                                                                  
      tag_base[index_teg].Number_BPM = 0;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */      
      tag_base[index_teg].vrm[1].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      return sizeof(pos_tag_rma_stm_loader_t) - 3*(sizeof(ed_best_vrm_t));   
    case 1:                                                                  
      tag_base[index_teg].Number_BPM = 1;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */      
      tag_base[index_teg].vrm[1].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */ 
      return sizeof(pos_tag_rma_stm_loader_t) - 2*(sizeof(ed_best_vrm_t));   
    case 2:                                                                  
      tag_base[index_teg].Number_BPM = 2;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */        
      tag_base[index_teg].vrm[1].ED_VRM_RMA = tag_box->vrm[1].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = tag_box->vrm[1].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */       
      return sizeof(pos_tag_rma_stm_loader_t) - 1*(sizeof(ed_best_vrm_t));   
    case 3:                                                                  
      tag_base[index_teg].Number_BPM = 3;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */        
      tag_base[index_teg].vrm[1].ED_VRM_RMA = tag_box->vrm[1].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = tag_box->vrm[1].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */     
      tag_base[index_teg].vrm[2].ED_VRM_RMA = tag_box->vrm[2].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = tag_box->vrm[2].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */  
      return sizeof(pos_tag_rma_stm_loader_t);
    default:
      /* Oбработанных данных нет */
      return 255;
    }
    }
  }
   
  /* Если Тег в таблице уже есть и источник позиционирования не лучший ВРМ - данные в таблице не обновляем */
  switch ( number_vrm )                                                    
  {                                                                        
  case 0:  return sizeof(pos_tag_rma_stm_loader_t) - 3*(sizeof(ed_best_vrm_t));   
  case 1:  return sizeof(pos_tag_rma_stm_loader_t) - 2*(sizeof(ed_best_vrm_t));   
  case 2:  return sizeof(pos_tag_rma_stm_loader_t) - 1*(sizeof(ed_best_vrm_t));   
  case 3:  return sizeof(pos_tag_rma_stm_loader_t);
  default: return 255;  /* Oбработанных данных нет */
  }
}

/**
  * @brief Функция формировани отчета по тегу RMA - stm основная программа в таблице   
  * @param pos_tag_rma_stm_t *tag_box - указатель на данные позиционирования
  * @param uint16_t index_tag_report - индекс тега 
  * @retval uint8_t - размер обработанных данных
  */
uint8_t ReportTagStmTable( pos_tag_rma_stm_t *tag_box, uint16_t index_tag_report )
{
  tag_box->head_tag.TypeTag = tag_base[index_tag_report].TypeTag;            /*	   Тип позиционируемого тега          */
  tag_box->head_tag.ADDR_Tag = tag_base[index_tag_report].ADDR_Tag;          /*    Адрес позиционируемого тега        */    
  tag_box->head_tag.ED_RMA_VRM = tag_base[index_tag_report].ED_RMA_VRM;      /*	   Уровень сигнала от тега к ВРМ      */  
  tag_box->head_tag.DATA_ID = tag_base[index_tag_report].DATA_ID;            /*	   ID данных пакета позиционирования  */     
  
  /* Заполнение поля данных */                                             
  for ( uint8_t contik_data_report = 0 ; contik_data_report < sizeof(data_tag_rma_stm_t); contik_data_report++ ) 
  {
    ((uint8_t*)&(tag_box->DATA_TAG))[contik_data_report] = tag_base[index_tag_report].DATA[contik_data_report];
  }
  
  /* Функция для формирования диагностического сообщения по рассылке пакета позиционирования.*/
  diag_tx_mes_pos( "<<_REPORT_POS_", tag_box->head_tag.TypeTag, tag_box->head_tag.ADDR_Tag, tag_box->vrm[0].PHY_ADDR_VRM );
  
  switch ( DataSto.Settings.number_position_vrm )                                                    
  {                                                                        
  case 0:                                                                  
    return sizeof(pos_tag_rma_stm_t) - 3*(sizeof(ed_best_vrm_t));          
  case 1:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    return sizeof(pos_tag_rma_stm_t) - 2*(sizeof(ed_best_vrm_t));          
  case 2:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    tag_box->vrm[1].ED_VRM_RMA =   tag_base[index_tag_report].vrm[1].ED_VRM_RMA;    
    tag_box->vrm[1].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[1].PHY_ADDR_VRM;  
    return sizeof(pos_tag_rma_stm_t) - 1*(sizeof(ed_best_vrm_t));          
  case 3:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    tag_box->vrm[1].ED_VRM_RMA =   tag_base[index_tag_report].vrm[1].ED_VRM_RMA;    
    tag_box->vrm[1].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[1].PHY_ADDR_VRM;  
    tag_box->vrm[2].ED_VRM_RMA =   tag_base[index_tag_report].vrm[2].ED_VRM_RMA;    
    tag_box->vrm[2].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[2].PHY_ADDR_VRM; 
    return sizeof(pos_tag_rma_stm_t);
  default:
    /* Oбработанных данных нет */
    return 0;
  }
}

/**
  * @brief Функция регистрации тега RMA - спутник основная программа в таблице 
  * @param pos_tag_rma_stm_t *tag_box - указатель на данные позиционирования
  * @param uint8_t number_vrm - число врм для позиционирования
  * @param uint16_t src_phy_addr - физический адрес источника
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - размер обработанных данных
  */
uint8_t RegTagStmTable( pos_tag_rma_stm_t *tag_box, uint8_t number_vrm, uint16_t src_phy_addr, uint8_t src_port_ID )
{
  /* Объявляем переменную для индекса */
  uint16_t index_teg = 0;  
  /* Переменная статуса записи в таблицу */
  uint8_t stat_record = 0;  
  
  /* Запрос индекса таблицы тегов     */
  stat_record = GetIndexTagTable( tag_box->head_tag.ADDR_Tag, &index_teg );
       
  if ( stat_record > 0 )
  {/* Получен индекс для записи в таблицу */
    /* Функция анализа режима обновления времени и регистрации тега */
    if ( UpdateTagTimeReg( src_phy_addr, tag_box->vrm[0].PHY_ADDR_VRM, index_teg, stat_record, src_port_ID ) )
    {
    /* Заполнение таблицы тегов */
    tag_base[index_teg].VERSION_ID = 3;		                             /*	   Версия пакета                      */
    tag_base[index_teg].src_phy_addr = src_phy_addr;                         /*	   Физический адрес источника         */
                                                                             
    tag_base[index_teg].TypeTag = tag_box->head_tag.TypeTag;	             /*	   Тип позиционируемого тега          */
    tag_base[index_teg].ADDR_Tag = tag_box->head_tag.ADDR_Tag;	             /*    Адрес позиционируемого тега        */
    tag_base[index_teg].ED_RMA_VRM = tag_box->head_tag.ED_RMA_VRM;           /*	   Уровень сигнала от тега к ВРМ      */
    tag_base[index_teg].DATA_ID = tag_box->head_tag.DATA_ID;                 /*	   ID данных пакета позиционирования  */         
      
    /* Заполнение поля данных */                                             
    for ( uint8_t contik_data_tag = 0 ; contik_data_tag < MAX_SIZE_DATA_POZ; contik_data_tag++ ) 
    {
      if (contik_data_tag < sizeof(data_tag_rma_stm_t) )
      {
        tag_base[index_teg].DATA[contik_data_tag] = ((uint8_t*)&(tag_box->DATA_TAG))[contik_data_tag];
      }
      else
      {
        tag_base[index_teg].DATA[contik_data_tag] = 0; 
      }  
    }
    
    tag_base[index_teg].vrm[0].MAC_ADDR_VRM = 0;                             /*      Поле данных лучших ВРМ             */     
    tag_base[index_teg].vrm[1].MAC_ADDR_VRM = 0;                             /*      Поле данных лучших ВРМ             */
    tag_base[index_teg].vrm[2].MAC_ADDR_VRM = 0;                             /*      Поле данных лучших ВРМ             */        
                                                                 
    /* Функция для формирования диагностического сообщения по парсингу пакета позиционирования.*/
    diag_reg_rs_pos( "<<_REG_RS__POS_", src_phy_addr, tag_box->head_tag.TypeTag, tag_box->head_tag.ADDR_Tag, tag_box->vrm[0].PHY_ADDR_VRM );
                                                                   
    switch ( number_vrm )                                                    
    {                                                                        
    case 0:                                                                  
      tag_base[index_teg].Number_BPM = 0;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */      
      tag_base[index_teg].vrm[1].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      return sizeof(pos_tag_rma_stm_t) - 3*(sizeof(ed_best_vrm_t));          
    case 1:                                                                  
      tag_base[index_teg].Number_BPM = 1;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */      
      tag_base[index_teg].vrm[1].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */ 
      return sizeof(pos_tag_rma_stm_t) - 2*(sizeof(ed_best_vrm_t));          
    case 2:                                                                  
      tag_base[index_teg].Number_BPM = 2;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */        
      tag_base[index_teg].vrm[1].ED_VRM_RMA = tag_box->vrm[1].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = tag_box->vrm[1].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */       
      return sizeof(pos_tag_rma_stm_t) - 1*(sizeof(ed_best_vrm_t));          
    case 3:                                                                  
      tag_base[index_teg].Number_BPM = 3;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */        
      tag_base[index_teg].vrm[1].ED_VRM_RMA = tag_box->vrm[1].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = tag_box->vrm[1].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */     
      tag_base[index_teg].vrm[2].ED_VRM_RMA = tag_box->vrm[2].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = tag_box->vrm[2].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */  
      return sizeof(pos_tag_rma_stm_t);
    default:
      /* Oбработанных данных нет */
      return 255;
    }
    }
  }
   
  /* Если Тег в таблице уже есть и источник позиционирования не лучший ВРМ - данные в таблице не обновляем */
  switch ( number_vrm )                                                    
  {                                                                        
  case 0:  return sizeof(pos_tag_rma_stm_t) - 3*(sizeof(ed_best_vrm_t));   
  case 1:  return sizeof(pos_tag_rma_stm_t) - 2*(sizeof(ed_best_vrm_t));   
  case 2:  return sizeof(pos_tag_rma_stm_t) - 1*(sizeof(ed_best_vrm_t));   
  case 3:  return sizeof(pos_tag_rma_stm_t);
  default: return 255;  /* Oбработанных данных нет */
  }
}

/*______________________________RMA спутник___________________________________*/
/**
  * @brief Функция формировани отчета по тегу RMA - спутник загрузчик программа в таблице  
  * @param pos_tag_sputnic_loader_t *tag_box - указатель на данные позиционирования
  * @param uint16_t index_tag_report - индекс тега 
  * @retval uint8_t - размер обработанных данных
  */
uint8_t ReportTagSputnicLoaderTable( pos_tag_sputnic_loader_t *tag_box, uint16_t index_tag_report )
{
  tag_box->head_tag.TypeTag = tag_base[index_tag_report].TypeTag;            /*	   Тип позиционируемого тега          */
  tag_box->head_tag.ADDR_Tag = tag_base[index_tag_report].ADDR_Tag;          /*    Адрес позиционируемого тега        */    
  tag_box->head_tag.ED_RMA_VRM = tag_base[index_tag_report].ED_RMA_VRM;      /*	   Уровень сигнала от тега к ВРМ      */  
  tag_box->head_tag.DATA_ID = tag_base[index_tag_report].DATA_ID;            /*	   ID данных пакета позиционирования  */     
  
  /* Заполнение поля данных */                                             
  for ( uint8_t contik_data_report = 0 ; contik_data_report < sizeof(data_tag_sputnic_loader_t); contik_data_report++ ) 
  {
    ((uint8_t*)&(tag_box->DATA_TAG))[contik_data_report] = tag_base[index_tag_report].DATA[contik_data_report];
  }
  
  /* Функция для формирования диагностического сообщения по рассылке пакета позиционирования.*/
  diag_tx_mes_pos( "<<_REPORT_POS_", tag_box->head_tag.TypeTag, tag_box->head_tag.ADDR_Tag, tag_box->vrm[0].PHY_ADDR_VRM );

  switch ( DataSto.Settings.number_position_vrm )                                                    
  {                                                                        
  case 0:                                                                  
    return sizeof(pos_tag_sputnic_loader_t) - 3*(sizeof(ed_best_vrm_t));          
  case 1:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    return sizeof(pos_tag_sputnic_loader_t) - 2*(sizeof(ed_best_vrm_t));          
  case 2:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    tag_box->vrm[1].ED_VRM_RMA =   tag_base[index_tag_report].vrm[1].ED_VRM_RMA;    
    tag_box->vrm[1].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[1].PHY_ADDR_VRM;  
    return sizeof(pos_tag_sputnic_loader_t) - 1*(sizeof(ed_best_vrm_t));          
  case 3:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    tag_box->vrm[1].ED_VRM_RMA =   tag_base[index_tag_report].vrm[1].ED_VRM_RMA;    
    tag_box->vrm[1].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[1].PHY_ADDR_VRM;  
    tag_box->vrm[2].ED_VRM_RMA =   tag_base[index_tag_report].vrm[2].ED_VRM_RMA;    
    tag_box->vrm[2].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[2].PHY_ADDR_VRM; 
    return sizeof(pos_tag_sputnic_loader_t);
  default:
    /* Oбработанных данных нет */
    return 0;
  }
}

/**
  * @brief Функция регистрации тега RMA - спутник загрузчик программа в таблице 
  * @param pos_tag_rsn_t *tag_box - указатель на данные позиционирования
  * @param uint8_t number_vrm - число врм для позиционирования
  * @param uint16_t src_phy_addr - физический адрес источника
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - размер обработанных данных
  */
uint8_t RegTagSputnicLoaderTable( pos_tag_sputnic_loader_t *tag_box, uint8_t number_vrm, uint16_t src_phy_addr, uint8_t src_port_ID )
{
  /* Объявляем переменную для индекса */
  uint16_t index_teg = 0;
  /* Переменная статуса записи в таблицу */
  uint8_t stat_record = 0;  
  
  /* Запрос индекса таблицы тегов     */
  stat_record = GetIndexTagTable( tag_box->head_tag.ADDR_Tag, &index_teg );
      
  if ( stat_record > 0 )
  {/* Получен индекс для записи в таблицу */
    /* Функция анализа режима обновления времени и регистрации тега */
    if ( UpdateTagTimeReg( src_phy_addr, tag_box->vrm[0].PHY_ADDR_VRM, index_teg, stat_record, src_port_ID ) )
    {
    /* Заполнение таблицы тегов */
    tag_base[index_teg].VERSION_ID = 3;		                             /*	   Версия пакета                      */
    tag_base[index_teg].src_phy_addr = src_phy_addr;                         /*	   Физический адрес источника         */
                                                                             
    tag_base[index_teg].TypeTag = tag_box->head_tag.TypeTag;	             /*	   Тип позиционируемого тега          */
    tag_base[index_teg].ADDR_Tag = tag_box->head_tag.ADDR_Tag;	             /*    Адрес позиционируемого тега        */
    tag_base[index_teg].ED_RMA_VRM = tag_box->head_tag.ED_RMA_VRM;           /*	   Уровень сигнала от тега к ВРМ      */
    tag_base[index_teg].DATA_ID = tag_box->head_tag.DATA_ID;                 /*	   ID данных пакета позиционирования  */                                   
      
    /* Заполнение поля данных */                                             
    for ( uint8_t contik_data_tag = 0 ; contik_data_tag < MAX_SIZE_DATA_POZ; contik_data_tag++ ) 
    {
      if (contik_data_tag < sizeof(data_tag_sputnic_loader_t) )
      {
        tag_base[index_teg].DATA[contik_data_tag] = ((uint8_t*)&(tag_box->DATA_TAG))[contik_data_tag];
      }
      else
      {
        tag_base[index_teg].DATA[contik_data_tag] = 0; 
      }  
    }
    
    tag_base[index_teg].vrm[0].MAC_ADDR_VRM = 0;                             /*      Поле данных лучших ВРМ             */     
    tag_base[index_teg].vrm[1].MAC_ADDR_VRM = 0;                             /*      Поле данных лучших ВРМ             */
    tag_base[index_teg].vrm[2].MAC_ADDR_VRM = 0;                             /*      Поле данных лучших ВРМ             */        
                                                                 
    /* Функция для формирования диагностического сообщения по парсингу пакета позиционирования.*/
    diag_reg_rs_pos( "<<_REG_RS__POS_", src_phy_addr, tag_box->head_tag.TypeTag, tag_box->head_tag.ADDR_Tag, tag_box->vrm[0].PHY_ADDR_VRM );
                                                                 
    switch ( number_vrm )                                                    
    {                                                                        
    case 0:                                                                  
      tag_base[index_teg].Number_BPM = 0;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */      
      tag_base[index_teg].vrm[1].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      return sizeof(pos_tag_sputnic_loader_t) - 3*(sizeof(ed_best_vrm_t));   
    case 1:                                                                  
      tag_base[index_teg].Number_BPM = 1;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */      
      tag_base[index_teg].vrm[1].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */ 
      return sizeof(pos_tag_sputnic_loader_t) - 2*(sizeof(ed_best_vrm_t));   
    case 2:                                                                  
      tag_base[index_teg].Number_BPM = 2;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */        
      tag_base[index_teg].vrm[1].ED_VRM_RMA = tag_box->vrm[1].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = tag_box->vrm[1].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */       
      return sizeof(pos_tag_sputnic_loader_t) - 1*(sizeof(ed_best_vrm_t));   
    case 3:                                                                  
      tag_base[index_teg].Number_BPM = 3;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */        
      tag_base[index_teg].vrm[1].ED_VRM_RMA = tag_box->vrm[1].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = tag_box->vrm[1].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */     
      tag_base[index_teg].vrm[2].ED_VRM_RMA = tag_box->vrm[2].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = tag_box->vrm[2].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */  
      return sizeof(pos_tag_sputnic_loader_t);
    default:
      /* Oбработанных данных нет */
      return 255;
    }
    }
  }
   
  /* Если Тег в таблице уже есть и источник позиционирования не лучший ВРМ - данные в таблице не обновляем */
  switch ( number_vrm )                                                    
  {                                                                        
  case 0:  return sizeof(pos_tag_sputnic_loader_t) - 3*(sizeof(ed_best_vrm_t));   
  case 1:  return sizeof(pos_tag_sputnic_loader_t) - 2*(sizeof(ed_best_vrm_t));   
  case 2:  return sizeof(pos_tag_sputnic_loader_t) - 1*(sizeof(ed_best_vrm_t));   
  case 3:  return sizeof(pos_tag_sputnic_loader_t);
  default: return 255;  /* Oбработанных данных нет */
  }
}

/*______________________________RMA спутник___________________________________*/
/**
  * @brief Функция формировани отчета по тегу RMA - спутник загрузчик программа в таблице  
  * @param pos_tag_sputnic_loader_no_protocol_t *tag_box - указатель на данные позиционирования
  * @param uint16_t index_tag_report - индекс тега 
  * @retval uint8_t - размер обработанных данных
  */
uint8_t ReportTagSputnicLoaderNoProtocolTable( pos_tag_sputnic_loader_no_protocol_t *tag_box, uint16_t index_tag_report )
{
  tag_box->head_tag.TypeTag = tag_base[index_tag_report].TypeTag;            /*	   Тип позиционируемого тега          */
  tag_box->head_tag.ADDR_Tag = tag_base[index_tag_report].ADDR_Tag;          /*    Адрес позиционируемого тега        */    
  tag_box->head_tag.ED_RMA_VRM = tag_base[index_tag_report].ED_RMA_VRM;      /*	   Уровень сигнала от тега к ВРМ      */  
  tag_box->head_tag.DATA_ID = tag_base[index_tag_report].DATA_ID;            /*	   ID данных пакета позиционирования  */     
  
  /* Заполнение поля данных */                                             
  for ( uint8_t contik_data_report = 0 ; contik_data_report < sizeof(data_tag_sputnic_loader_no_protocol_t); contik_data_report++ ) 
  {
    ((uint8_t*)&(tag_box->DATA_TAG))[contik_data_report] = tag_base[index_tag_report].DATA[contik_data_report];
  }
  
  /* Функция для формирования диагностического сообщения по рассылке пакета позиционирования.*/
  diag_tx_mes_pos( "<<_REPORT_POS_", tag_box->head_tag.TypeTag, tag_box->head_tag.ADDR_Tag, tag_box->vrm[0].PHY_ADDR_VRM );

  switch ( DataSto.Settings.number_position_vrm )                                                    
  {                                                                        
  case 0:
    return sizeof(pos_tag_sputnic_loader_no_protocol_t) - 3*(sizeof(ed_best_vrm_t));          
  case 1:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    return sizeof(pos_tag_sputnic_loader_no_protocol_t) - 2*(sizeof(ed_best_vrm_t));          
  case 2:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    tag_box->vrm[1].ED_VRM_RMA =   tag_base[index_tag_report].vrm[1].ED_VRM_RMA;    
    tag_box->vrm[1].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[1].PHY_ADDR_VRM;  
    return sizeof(pos_tag_sputnic_loader_no_protocol_t) - 1*(sizeof(ed_best_vrm_t));          
  case 3:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    tag_box->vrm[1].ED_VRM_RMA =   tag_base[index_tag_report].vrm[1].ED_VRM_RMA;    
    tag_box->vrm[1].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[1].PHY_ADDR_VRM;  
    tag_box->vrm[2].ED_VRM_RMA =   tag_base[index_tag_report].vrm[2].ED_VRM_RMA;    
    tag_box->vrm[2].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[2].PHY_ADDR_VRM; 
    return sizeof(pos_tag_sputnic_loader_no_protocol_t);
  default:
    /* Oбработанных данных нет */
    return 0;
  }
}

/**
  * @brief Функция регистрации тега RMA - спутник загрузчик программа в таблице 
  * @param pos_tag_sputnic_loader_no_protocol_t *tag_box - указатель на данные позиционирования
  * @param uint8_t number_vrm - число врм для позиционирования
  * @param uint16_t src_phy_addr - физический адрес источника
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - размер обработанных данных
  */
uint8_t RegTagSputnicLoaderNoProtocolTable( pos_tag_sputnic_loader_no_protocol_t *tag_box, uint8_t number_vrm, uint16_t src_phy_addr, uint8_t src_port_ID )
{
  /* Объявляем переменную для индекса */
  uint16_t index_teg = 0;
  /* Переменная статуса записи в таблицу */
  uint8_t stat_record = 0;  
  
  /* Запрос индекса таблицы тегов     */
  stat_record = GetIndexTagTable( tag_box->head_tag.ADDR_Tag, &index_teg );
      
  if ( stat_record > 0 )
  {/* Получен индекс для записи в таблицу */
    /* Функция анализа режима обновления времени и регистрации тега */
    if ( UpdateTagTimeReg( src_phy_addr, tag_box->vrm[0].PHY_ADDR_VRM, index_teg, stat_record, src_port_ID ) )
    {
    /* Заполнение таблицы тегов */
    tag_base[index_teg].VERSION_ID = 3;		                             /*	   Версия пакета                      */
    tag_base[index_teg].src_phy_addr = src_phy_addr;                         /*	   Физический адрес источника         */
                                                                             
    tag_base[index_teg].TypeTag = tag_box->head_tag.TypeTag;	             /*	   Тип позиционируемого тега          */
    tag_base[index_teg].ADDR_Tag = tag_box->head_tag.ADDR_Tag;	             /*    Адрес позиционируемого тега        */
    tag_base[index_teg].ED_RMA_VRM = tag_box->head_tag.ED_RMA_VRM;           /*	   Уровень сигнала от тега к ВРМ      */
    tag_base[index_teg].DATA_ID = tag_box->head_tag.DATA_ID;                 /*	   ID данных пакета позиционирования  */                                   
      
    /* Заполнение поля данных */                                             
    for ( uint8_t contik_data_tag = 0 ; contik_data_tag < MAX_SIZE_DATA_POZ; contik_data_tag++ ) 
    {
        if (contik_data_tag < sizeof(data_tag_sputnic_loader_no_protocol_t) )
      {
        tag_base[index_teg].DATA[contik_data_tag] = ((uint8_t*)&(tag_box->DATA_TAG))[contik_data_tag];
      }
      else
      {
        tag_base[index_teg].DATA[contik_data_tag] = 0; 
      }  
    }
    
    tag_base[index_teg].vrm[0].MAC_ADDR_VRM = 0;                             /*      Поле данных лучших ВРМ             */     
    tag_base[index_teg].vrm[1].MAC_ADDR_VRM = 0;                             /*      Поле данных лучших ВРМ             */
    tag_base[index_teg].vrm[2].MAC_ADDR_VRM = 0;                             /*      Поле данных лучших ВРМ             */        
                                                                 
    /* Функция для формирования диагностического сообщения по парсингу пакета позиционирования.*/
    diag_reg_rs_pos( "<<_REG_RS__POS_", src_phy_addr, tag_box->head_tag.TypeTag, tag_box->head_tag.ADDR_Tag, tag_box->vrm[0].PHY_ADDR_VRM );
                                                                 
    switch ( number_vrm )                                                    
    {                                                                        
    case 0:                                                                  
      tag_base[index_teg].Number_BPM = 0;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */      
      tag_base[index_teg].vrm[1].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      return sizeof(pos_tag_sputnic_loader_no_protocol_t) - 3*(sizeof(ed_best_vrm_t));   
    case 1:                                                                  
      tag_base[index_teg].Number_BPM = 1;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */      
      tag_base[index_teg].vrm[1].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */ 
      return sizeof(pos_tag_sputnic_loader_no_protocol_t) - 2*(sizeof(ed_best_vrm_t));   
    case 2:                                                                  
      tag_base[index_teg].Number_BPM = 2;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */        
      tag_base[index_teg].vrm[1].ED_VRM_RMA = tag_box->vrm[1].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = tag_box->vrm[1].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */       
      return sizeof(pos_tag_sputnic_loader_no_protocol_t) - 1*(sizeof(ed_best_vrm_t));   
    case 3:                                                                  
      tag_base[index_teg].Number_BPM = 3;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */        
      tag_base[index_teg].vrm[1].ED_VRM_RMA = tag_box->vrm[1].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = tag_box->vrm[1].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */     
      tag_base[index_teg].vrm[2].ED_VRM_RMA = tag_box->vrm[2].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = tag_box->vrm[2].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */  
      return sizeof(pos_tag_sputnic_loader_no_protocol_t);
    default:
      /* Oбработанных данных нет */
      return 255;
    }
    }
  }
   
  /* Если Тег в таблице уже есть и источник позиционирования не лучший ВРМ - данные в таблице не обновляем */
  switch ( number_vrm )                                                    
  {                                                                        
  case 0:  return sizeof(pos_tag_sputnic_loader_no_protocol_t) - 3*(sizeof(ed_best_vrm_t));   
  case 1:  return sizeof(pos_tag_sputnic_loader_no_protocol_t) - 2*(sizeof(ed_best_vrm_t));   
  case 2:  return sizeof(pos_tag_sputnic_loader_no_protocol_t) - 1*(sizeof(ed_best_vrm_t));   
  case 3:  return sizeof(pos_tag_sputnic_loader_no_protocol_t);
  default: return 255;  /* Oбработанных данных нет */
  }
}

/**
  * @brief Функция формировани отчета по тегу RMA - спутник основная программа в таблице 
  * @param pos_tag_sputnic_t *tag_box - указатель на данные позиционирования
  * @param uint16_t index_tag_report - индекс тега 
  * @retval uint8_t - размер обработанных данных
  */
uint8_t ReportTagSputnicTable( pos_tag_sputnic_t *tag_box, uint16_t index_tag_report )
{
  tag_box->head_tag.TypeTag = tag_base[index_tag_report].TypeTag;            /*	   Тип позиционируемого тега          */
  tag_box->head_tag.ADDR_Tag = tag_base[index_tag_report].ADDR_Tag;          /*    Адрес позиционируемого тега        */    
  tag_box->head_tag.ED_RMA_VRM = tag_base[index_tag_report].ED_RMA_VRM;      /*	   Уровень сигнала от тега к ВРМ      */  
  tag_box->head_tag.DATA_ID = tag_base[index_tag_report].DATA_ID;            /*	   ID данных пакета позиционирования  */     
  
  /* Заполнение поля данных */                                             
  for ( uint8_t contik_data_report = 0 ; contik_data_report < sizeof(data_tag_sputnic_t); contik_data_report++ ) 
  {
    ((uint8_t*)&(tag_box->DATA_TAG))[contik_data_report] = tag_base[index_tag_report].DATA[contik_data_report];
  }
  
  /* Функция для формирования диагностического сообщения по рассылке пакета позиционирования.*/
  diag_tx_mes_pos( "<<_REPORT_POS_", tag_box->head_tag.TypeTag, tag_box->head_tag.ADDR_Tag, tag_box->vrm[0].PHY_ADDR_VRM );
  
  switch ( DataSto.Settings.number_position_vrm )                                                    
  {                                                                        
  case 0:                                                                  
    return sizeof(pos_tag_sputnic_t) - 3*(sizeof(ed_best_vrm_t));          
  case 1:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    return sizeof(pos_tag_sputnic_t) - 2*(sizeof(ed_best_vrm_t));          
  case 2:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    tag_box->vrm[1].ED_VRM_RMA =   tag_base[index_tag_report].vrm[1].ED_VRM_RMA;    
    tag_box->vrm[1].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[1].PHY_ADDR_VRM;  
    return sizeof(pos_tag_sputnic_t) - 1*(sizeof(ed_best_vrm_t));          
  case 3:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    tag_box->vrm[1].ED_VRM_RMA =   tag_base[index_tag_report].vrm[1].ED_VRM_RMA;    
    tag_box->vrm[1].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[1].PHY_ADDR_VRM;  
    tag_box->vrm[2].ED_VRM_RMA =   tag_base[index_tag_report].vrm[2].ED_VRM_RMA;    
    tag_box->vrm[2].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[2].PHY_ADDR_VRM; 
    return sizeof(pos_tag_sputnic_t);
  default:
    /* Oбработанных данных нет */
    return 0;
  }
}

/**
  * @brief Функция регистрации тега RMA - спутник основная программа в таблице 
  * @param pos_tag_sputnic_t *tag_box - указатель на данные позиционирования
  * @param uint8_t number_vrm - число врм для позиционирования
  * @param uint16_t src_phy_addr - физический адрес источника
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - размер обработанных данных
  */
uint8_t RegTagSputnicTable( pos_tag_sputnic_t *tag_box, uint8_t number_vrm, uint16_t src_phy_addr, uint8_t src_port_ID )
{
  /* Объявляем переменную для индекса */
  uint16_t index_teg = 0;
  /* Переменная статуса записи в таблицу */
  uint8_t stat_record = 0;  
  
  /* Запрос индекса таблицы тегов     */
  stat_record = GetIndexTagTable( tag_box->head_tag.ADDR_Tag, &index_teg );
    
  if ( stat_record > 0 )
  {/* Получен индекс для записи в таблицу */
    /* Функция анализа режима обновления времени и регистрации тега */
    if ( UpdateTagTimeReg( src_phy_addr, tag_box->vrm[0].PHY_ADDR_VRM, index_teg, stat_record, src_port_ID ) )
    {
    /* Заполнение таблицы тегов */
    tag_base[index_teg].VERSION_ID = 3;		                             /*	   Версия пакета                      */
    tag_base[index_teg].src_phy_addr = src_phy_addr;                         /*	   Физический адрес источника         */
                                                                             
    tag_base[index_teg].TypeTag = tag_box->head_tag.TypeTag;	             /*	   Тип позиционируемого тега          */
    tag_base[index_teg].ADDR_Tag = tag_box->head_tag.ADDR_Tag;	             /*      Адрес позиционируемого тега      */
    tag_base[index_teg].ED_RMA_VRM = tag_box->head_tag.ED_RMA_VRM;           /*	   Уровень сигнала от тега к ВРМ      */
    tag_base[index_teg].DATA_ID = tag_box->head_tag.DATA_ID;                 /*	   ID данных пакета позиционирования  */                                  
      
    /* Заполнение поля данных */                                             
    for ( uint8_t contik_data_tag = 0 ; contik_data_tag < MAX_SIZE_DATA_POZ; contik_data_tag++ ) 
    {
      if (contik_data_tag < sizeof(data_tag_sputnic_t) )
      {
        tag_base[index_teg].DATA[contik_data_tag] = ((uint8_t*)&(tag_box->DATA_TAG))[contik_data_tag];
      }
      else
      {
        tag_base[index_teg].DATA[contik_data_tag] = 0; 
      }  
    }
     
    tag_base[index_teg].vrm[0].MAC_ADDR_VRM = 0;                             /*      Поле данных лучших ВРМ             */     
    tag_base[index_teg].vrm[1].MAC_ADDR_VRM = 0;                             /*      Поле данных лучших ВРМ             */
    tag_base[index_teg].vrm[2].MAC_ADDR_VRM = 0;                             /*      Поле данных лучших ВРМ             */        
                                                                
    /* Функция для формирования диагностического сообщения по парсингу пакета позиционирования.*/
    diag_reg_rs_pos( "<<_REG_RS__POS_", src_phy_addr, tag_box->head_tag.TypeTag, tag_box->head_tag.ADDR_Tag, tag_box->vrm[0].PHY_ADDR_VRM );
                                                                
    switch ( number_vrm )                                                    
    {                                                                        
    case 0:                                                                  
      tag_base[index_teg].Number_BPM = 0;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */      
      tag_base[index_teg].vrm[1].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      return sizeof(pos_tag_sputnic_t) - 3*(sizeof(ed_best_vrm_t));          
    case 1:                                                                  
      tag_base[index_teg].Number_BPM = 1;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */      
      tag_base[index_teg].vrm[1].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */ 
      return sizeof(pos_tag_sputnic_t) - 2*(sizeof(ed_best_vrm_t));          
    case 2:                                                                  
      tag_base[index_teg].Number_BPM = 2;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */        
      tag_base[index_teg].vrm[1].ED_VRM_RMA = tag_box->vrm[1].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = tag_box->vrm[1].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */       
      return sizeof(pos_tag_sputnic_t) - 1*(sizeof(ed_best_vrm_t));          
    case 3:                                                                  
      tag_base[index_teg].Number_BPM = 3;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */        
      tag_base[index_teg].vrm[1].ED_VRM_RMA = tag_box->vrm[1].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = tag_box->vrm[1].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */     
      tag_base[index_teg].vrm[2].ED_VRM_RMA = tag_box->vrm[2].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = tag_box->vrm[2].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */  
      return sizeof(pos_tag_sputnic_t);
    default:
      /* Oбработанных данных нет */
      return 255;
    }
    }
  }
   
  /* Если Тег в таблице уже есть и источник позиционирования не лучший ВРМ - данные в таблице не обновляем */
  switch ( number_vrm )                                                    
  {                                                                        
  case 0:  return sizeof(pos_tag_sputnic_t) - 3*(sizeof(ed_best_vrm_t));   
  case 1:  return sizeof(pos_tag_sputnic_t) - 2*(sizeof(ed_best_vrm_t));   
  case 2:  return sizeof(pos_tag_sputnic_t) - 1*(sizeof(ed_best_vrm_t));   
  case 3:  return sizeof(pos_tag_sputnic_t);
  default: return 255;  /* Oбработанных данных нет */
  }
}

/**
  * @brief Функция формировани отчета по тегу RMA - спутник основная программа в таблице 
  * @param pos_tag_sputnic_no_protocol_t *tag_box - указатель на данные позиционирования
  * @param uint16_t index_tag_report - индекс тега 
  * @retval uint8_t - размер обработанных данных
  */
uint8_t ReportTagSputnicNoProtocolTable( pos_tag_sputnic_no_protocol_t *tag_box, uint16_t index_tag_report )
{
  tag_box->head_tag.TypeTag = tag_base[index_tag_report].TypeTag;            /*	   Тип позиционируемого тега          */
  tag_box->head_tag.ADDR_Tag = tag_base[index_tag_report].ADDR_Tag;          /*    Адрес позиционируемого тега        */    
  tag_box->head_tag.ED_RMA_VRM = tag_base[index_tag_report].ED_RMA_VRM;      /*	   Уровень сигнала от тега к ВРМ      */  
  tag_box->head_tag.DATA_ID = tag_base[index_tag_report].DATA_ID;            /*	   ID данных пакета позиционирования  */     
  
  /* Заполнение поля данных */                                             
  for ( uint8_t contik_data_report = 0 ; contik_data_report < sizeof(data_tag_sputnic_no_protocol_t); contik_data_report++ ) 
  {
    ((uint8_t*)&(tag_box->DATA_TAG))[contik_data_report] = tag_base[index_tag_report].DATA[contik_data_report];
  }
  
  /* Функция для формирования диагностического сообщения по рассылке пакета позиционирования.*/
  diag_tx_mes_pos( "<<_REPORT_POS_", tag_box->head_tag.TypeTag, tag_box->head_tag.ADDR_Tag, tag_box->vrm[0].PHY_ADDR_VRM );
  
  switch ( DataSto.Settings.number_position_vrm )                                                    
  {                                                                        
  case 0:
    return sizeof(pos_tag_sputnic_no_protocol_t) - 3*(sizeof(ed_best_vrm_t));          
  case 1:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    return sizeof(pos_tag_sputnic_no_protocol_t) - 2*(sizeof(ed_best_vrm_t));          
  case 2:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    tag_box->vrm[1].ED_VRM_RMA =   tag_base[index_tag_report].vrm[1].ED_VRM_RMA;    
    tag_box->vrm[1].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[1].PHY_ADDR_VRM;  
    return sizeof(pos_tag_sputnic_no_protocol_t) - 1*(sizeof(ed_best_vrm_t));          
  case 3:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    tag_box->vrm[1].ED_VRM_RMA =   tag_base[index_tag_report].vrm[1].ED_VRM_RMA;    
    tag_box->vrm[1].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[1].PHY_ADDR_VRM;  
    tag_box->vrm[2].ED_VRM_RMA =   tag_base[index_tag_report].vrm[2].ED_VRM_RMA;    
    tag_box->vrm[2].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[2].PHY_ADDR_VRM; 
    return sizeof(pos_tag_sputnic_no_protocol_t);
  default:
    /* Oбработанных данных нет */
    return 0;
  }
}

/**
  * @brief Функция регистрации тега RMA - спутник основная программа в таблице 
  * @param pos_tag_sputnic_no_protocol_t *tag_box - указатель на данные позиционирования
  * @param uint8_t number_vrm - число врм для позиционирования
  * @param uint16_t src_phy_addr - физический адрес источника
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - размер обработанных данных
  */
uint8_t RegTagSputnicNoProtocolTable( pos_tag_sputnic_no_protocol_t *tag_box, uint8_t number_vrm, uint16_t src_phy_addr, uint8_t src_port_ID )
{
  /* Объявляем переменную для индекса */
  uint16_t index_teg = 0;
  /* Переменная статуса записи в таблицу */
  uint8_t stat_record = 0;  
  
  /* Запрос индекса таблицы тегов     */
  stat_record = GetIndexTagTable( tag_box->head_tag.ADDR_Tag, &index_teg );
    
  if ( stat_record > 0 )
  {/* Получен индекс для записи в таблицу */
    /* Функция анализа режима обновления времени и регистрации тега */
    if ( UpdateTagTimeReg( src_phy_addr, tag_box->vrm[0].PHY_ADDR_VRM, index_teg, stat_record, src_port_ID ) )
    {
    /* Заполнение таблицы тегов */
    tag_base[index_teg].VERSION_ID = 3;		                             /*	   Версия пакета                      */
    tag_base[index_teg].src_phy_addr = src_phy_addr;                         /*	   Физический адрес источника         */
                                                                             
    tag_base[index_teg].TypeTag = tag_box->head_tag.TypeTag;	             /*	   Тип позиционируемого тега          */
    tag_base[index_teg].ADDR_Tag = tag_box->head_tag.ADDR_Tag;	             /*      Адрес позиционируемого тега      */
    tag_base[index_teg].ED_RMA_VRM = tag_box->head_tag.ED_RMA_VRM;           /*	   Уровень сигнала от тега к ВРМ      */
    tag_base[index_teg].DATA_ID = tag_box->head_tag.DATA_ID;                 /*	   ID данных пакета позиционирования  */                                  
      
    /* Заполнение поля данных */                                             
    for ( uint8_t contik_data_tag = 0 ; contik_data_tag < MAX_SIZE_DATA_POZ; contik_data_tag++ ) 
    {
        if (contik_data_tag < sizeof(data_tag_sputnic_no_protocol_t) )
      {
        tag_base[index_teg].DATA[contik_data_tag] = ((uint8_t*)&(tag_box->DATA_TAG))[contik_data_tag];
      }
      else
      {
        tag_base[index_teg].DATA[contik_data_tag] = 0; 
      }  
    }
     
    tag_base[index_teg].vrm[0].MAC_ADDR_VRM = 0;                             /*      Поле данных лучших ВРМ             */     
    tag_base[index_teg].vrm[1].MAC_ADDR_VRM = 0;                             /*      Поле данных лучших ВРМ             */
    tag_base[index_teg].vrm[2].MAC_ADDR_VRM = 0;                             /*      Поле данных лучших ВРМ             */        
                                                                
    /* Функция для формирования диагностического сообщения по парсингу пакета позиционирования.*/
    diag_reg_rs_pos( "<<_REG_RS__POS_", src_phy_addr, tag_box->head_tag.TypeTag, tag_box->head_tag.ADDR_Tag, tag_box->vrm[0].PHY_ADDR_VRM );
                                                                
    switch ( number_vrm )                                                    
    {                                                                        
    case 0:                                                                  
      tag_base[index_teg].Number_BPM = 0;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */      
      tag_base[index_teg].vrm[1].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      return sizeof(pos_tag_sputnic_no_protocol_t) - 3*(sizeof(ed_best_vrm_t));          
    case 1:                                                                  
      tag_base[index_teg].Number_BPM = 1;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */      
      tag_base[index_teg].vrm[1].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */ 
      return sizeof(pos_tag_sputnic_no_protocol_t) - 2*(sizeof(ed_best_vrm_t));          
    case 2:                                                                  
      tag_base[index_teg].Number_BPM = 2;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */        
      tag_base[index_teg].vrm[1].ED_VRM_RMA = tag_box->vrm[1].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = tag_box->vrm[1].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */       
      return sizeof(pos_tag_sputnic_no_protocol_t) - 1*(sizeof(ed_best_vrm_t));          
    case 3:                                                                  
      tag_base[index_teg].Number_BPM = 3;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */        
      tag_base[index_teg].vrm[1].ED_VRM_RMA = tag_box->vrm[1].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = tag_box->vrm[1].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */     
      tag_base[index_teg].vrm[2].ED_VRM_RMA = tag_box->vrm[2].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = tag_box->vrm[2].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */  
      return sizeof(pos_tag_sputnic_no_protocol_t);
    default:
      /* Oбработанных данных нет */
      return 255;
    }
    }
  }
   
  /* Если Тег в таблице уже есть и источник позиционирования не лучший ВРМ - данные в таблице не обновляем */
  switch ( number_vrm )                                                    
  {                                                                        
  case 0:  return sizeof(pos_tag_sputnic_no_protocol_t) - 3*(sizeof(ed_best_vrm_t));   
  case 1:  return sizeof(pos_tag_sputnic_no_protocol_t) - 2*(sizeof(ed_best_vrm_t));   
  case 2:  return sizeof(pos_tag_sputnic_no_protocol_t) - 1*(sizeof(ed_best_vrm_t));   
  case 3:  return sizeof(pos_tag_sputnic_no_protocol_t);
  default: return 255;  /* Oбработанных данных нет */
  }
}

/*________________________________RSN_________________________________________*/

/**
  * @brief Функция формировани отчета по тегу РСН загрузчик  в таблице  
  * @param pos_tag_rsn_loader_t *tag_box - указатель на данные позиционирования
  * @param uint16_t index_tag_report - индекс тега 
  * @retval uint8_t - размер обработанных данных
  */
uint8_t ReportTagRSNLoaderTable( pos_tag_rsn_loader_t *tag_box, uint16_t index_tag_report )
{
  tag_box->head_tag.TypeTag = tag_base[index_tag_report].TypeTag;            /*	   Тип позиционируемого тега          */
  tag_box->head_tag.ADDR_Tag = tag_base[index_tag_report].ADDR_Tag;          /*    Адрес позиционируемого тега        */    
  tag_box->head_tag.ED_RMA_VRM = tag_base[index_tag_report].ED_RMA_VRM;      /*	   Уровень сигнала от тега к ВРМ      */  
  tag_box->head_tag.DATA_ID = tag_base[index_tag_report].DATA_ID;            /*	   ID данных пакета позиционирования  */     
  
  /* Заполнение поля данных */                                             
  for ( uint8_t contik_data_report = 0 ; contik_data_report < sizeof(data_tag_rsn_loader_t); contik_data_report++ ) 
  {
    ((uint8_t*)&(tag_box->DATA_TAG))[contik_data_report] = tag_base[index_tag_report].DATA[contik_data_report];
  }
  
  /* Функция для формирования диагностического сообщения по рассылке пакета позиционирования.*/
  diag_tx_mes_pos( "<<_REPORT_POS_", tag_box->head_tag.TypeTag, tag_box->head_tag.ADDR_Tag, tag_box->vrm[0].PHY_ADDR_VRM );
  
  switch ( DataSto.Settings.number_position_vrm )                                                    
  {                                                                        
  case 0:                                                                  
    return sizeof(pos_tag_rsn_loader_t) - 3*(sizeof(ed_best_vrm_t));          
  case 1:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    return sizeof(pos_tag_rsn_loader_t) - 2*(sizeof(ed_best_vrm_t));          
  case 2:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    tag_box->vrm[1].ED_VRM_RMA =   tag_base[index_tag_report].vrm[1].ED_VRM_RMA;    
    tag_box->vrm[1].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[1].PHY_ADDR_VRM;  
    return sizeof(pos_tag_rsn_loader_t) - 1*(sizeof(ed_best_vrm_t));          
  case 3:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    tag_box->vrm[1].ED_VRM_RMA =   tag_base[index_tag_report].vrm[1].ED_VRM_RMA;    
    tag_box->vrm[1].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[1].PHY_ADDR_VRM;  
    tag_box->vrm[2].ED_VRM_RMA =   tag_base[index_tag_report].vrm[2].ED_VRM_RMA;    
    tag_box->vrm[2].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[2].PHY_ADDR_VRM; 
    return sizeof(pos_tag_rsn_loader_t);
  default:
    /* Oбработанных данных нет */
    return 0;
  }
}

/**
  * @brief Функция регистрации тега РСН загрузчик  в таблице 
  * @param pos_tag_rsn_loader_t *tag_box - указатель на данные позиционирования
  * @param uint8_t number_vrm - число врм для позиционирования
  * @param uint16_t src_phy_addr - физический адрес источника
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - размер обработанных данных
  */
uint8_t RegTagRSNLoaderTable( pos_tag_rsn_loader_t *tag_box, uint8_t number_vrm, uint16_t src_phy_addr, uint8_t src_port_ID )
{
  /* Объявляем переменную для индекса */
  uint16_t index_teg = 0;
  /* Переменная статуса записи в таблицу */
  uint8_t stat_record = 0;  
  
  /* Запрос индекса таблицы тегов     */
  stat_record = GetIndexTagTable( tag_box->head_tag.ADDR_Tag, &index_teg );
    
  if ( stat_record > 0 )
  {/* Получен индекс для записи в таблицу */
    /* Функция анализа режима обновления времени и регистрации тега */
    if ( UpdateTagTimeReg( src_phy_addr, tag_box->vrm[0].PHY_ADDR_VRM, index_teg, stat_record, src_port_ID ) )
    {
    /* Заполнение таблицы тегов */
    tag_base[index_teg].VERSION_ID = 3;		                             /*	   Версия пакета                      */
    tag_base[index_teg].src_phy_addr = src_phy_addr;                         /*	   Физический адрес источника         */
                                                                             
    tag_base[index_teg].TypeTag = tag_box->head_tag.TypeTag;	             /*	   Тип позиционируемого тега          */
    tag_base[index_teg].ADDR_Tag = tag_box->head_tag.ADDR_Tag;	             /*    Адрес позиционируемого тега      */
    tag_base[index_teg].ED_RMA_VRM = tag_box->head_tag.ED_RMA_VRM;           /*	   Уровень сигнала от тега к ВРМ      */
    tag_base[index_teg].DATA_ID = tag_box->head_tag.DATA_ID;                 /*	   ID данных пакета позиционирования  */                                   
      
    /* Заполнение поля данных */                                             
    for ( uint8_t contik_data_tag = 0 ; contik_data_tag < MAX_SIZE_DATA_POZ; contik_data_tag++ ) 
    {
      if (contik_data_tag < sizeof(data_tag_rsn_loader_t) )
      {
        tag_base[index_teg].DATA[contik_data_tag] = ((uint8_t*)&(tag_box->DATA_TAG))[contik_data_tag];
      }
      else
      {
        tag_base[index_teg].DATA[contik_data_tag] = 0; 
      }  
    }
                                                                 
    tag_base[index_teg].vrm[0].MAC_ADDR_VRM = 0;                             /*      Поле данных лучших ВРМ             */     
    tag_base[index_teg].vrm[1].MAC_ADDR_VRM = 0;                             /*      Поле данных лучших ВРМ             */
    tag_base[index_teg].vrm[2].MAC_ADDR_VRM = 0;                             /*      Поле данных лучших ВРМ             */        
                
    /* Функция для формирования диагностического сообщения по парсингу пакета позиционирования.*/
    diag_reg_rs_pos( "<<_REG_RS__POS_", src_phy_addr, tag_box->head_tag.TypeTag, tag_box->head_tag.ADDR_Tag, tag_box->vrm[0].PHY_ADDR_VRM );
                                                                 
    switch ( number_vrm )                                                    
    {                                                                        
    case 0:                                                                  
      tag_base[index_teg].Number_BPM = 0;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */      
      tag_base[index_teg].vrm[1].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      return sizeof(pos_tag_rsn_loader_t) - 3*(sizeof(ed_best_vrm_t));       
    case 1:                                                                  
      tag_base[index_teg].Number_BPM = 1;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */      
      tag_base[index_teg].vrm[1].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */ 
      return sizeof(pos_tag_rsn_loader_t) - 2*(sizeof(ed_best_vrm_t));       
    case 2:                                                                  
      tag_base[index_teg].Number_BPM = 2;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */        
      tag_base[index_teg].vrm[1].ED_VRM_RMA = tag_box->vrm[1].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = tag_box->vrm[1].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */       
      return sizeof(pos_tag_rsn_loader_t) - 1*(sizeof(ed_best_vrm_t));       
    case 3:                                                                  
      tag_base[index_teg].Number_BPM = 3;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */        
      tag_base[index_teg].vrm[1].ED_VRM_RMA = tag_box->vrm[1].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = tag_box->vrm[1].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */     
      tag_base[index_teg].vrm[2].ED_VRM_RMA = tag_box->vrm[2].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = tag_box->vrm[2].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */  
      return sizeof(pos_tag_rsn_loader_t);
    default:
      /* Oбработанных данных нет */
      return 255;
    }
    }
  }
  
  /* Если Тег в таблице уже есть и источник позиционирования не лучший ВРМ - данные в таблице не обновляем */
  switch ( number_vrm )                                                    
  {                                                                        
  case 0:  return sizeof(pos_tag_rsn_loader_t) - 3*(sizeof(ed_best_vrm_t));   
  case 1:  return sizeof(pos_tag_rsn_loader_t) - 2*(sizeof(ed_best_vrm_t));   
  case 2:  return sizeof(pos_tag_rsn_loader_t) - 1*(sizeof(ed_best_vrm_t));   
  case 3:  return sizeof(pos_tag_rsn_loader_t);
  default: return 255;  /* Oбработанных данных нет */
  }
}

/**
  * @brief Функция формировани отчета по тегу РСН основная программа в таблице   
  * @param pos_tag_rsn_t *tag_box - указатель на данные позиционирования
  * @param uint16_t index_tag_report - индекс тега 
  * @retval uint8_t - размер обработанных данных
  */
uint8_t ReportTagRSNTable( pos_tag_rsn_t *tag_box, uint16_t index_tag_report )
{
  tag_box->head_tag.TypeTag = tag_base[index_tag_report].TypeTag;            /*	   Тип позиционируемого тега          */
  tag_box->head_tag.ADDR_Tag = tag_base[index_tag_report].ADDR_Tag;          /*    Адрес позиционируемого тега        */    
  tag_box->head_tag.ED_RMA_VRM = tag_base[index_tag_report].ED_RMA_VRM;      /*	   Уровень сигнала от тега к ВРМ      */  
  tag_box->head_tag.DATA_ID = tag_base[index_tag_report].DATA_ID;            /*	   ID данных пакета позиционирования  */     
  
  /* Заполнение поля данных */                                             
  for ( uint8_t contik_data_report = 0 ; contik_data_report < sizeof(data_tag_rsn_t); contik_data_report++ ) 
  {
    ((uint8_t*)&(tag_box->DATA_TAG))[contik_data_report] = tag_base[index_tag_report].DATA[contik_data_report];
  }
  
  /* Функция для формирования диагностического сообщения по рассылке пакета позиционирования.*/
  diag_tx_mes_pos( "<<_REPORT_POS_", tag_box->head_tag.TypeTag, tag_box->head_tag.ADDR_Tag, tag_box->vrm[0].PHY_ADDR_VRM );
  
  switch ( DataSto.Settings.number_position_vrm )                                                    
  {                                                                        
  case 0:                                                                  
    return sizeof(pos_tag_rsn_t) - 3*(sizeof(ed_best_vrm_t));          
  case 1:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    return sizeof(pos_tag_rsn_t) - 2*(sizeof(ed_best_vrm_t));          
  case 2:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    tag_box->vrm[1].ED_VRM_RMA =   tag_base[index_tag_report].vrm[1].ED_VRM_RMA;    
    tag_box->vrm[1].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[1].PHY_ADDR_VRM;  
    return sizeof(pos_tag_rsn_t) - 1*(sizeof(ed_best_vrm_t));          
  case 3:                                                                  
    tag_box->vrm[0].ED_VRM_RMA =   tag_base[index_tag_report].vrm[0].ED_VRM_RMA;    
    tag_box->vrm[0].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[0].PHY_ADDR_VRM;     
    tag_box->vrm[1].ED_VRM_RMA =   tag_base[index_tag_report].vrm[1].ED_VRM_RMA;    
    tag_box->vrm[1].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[1].PHY_ADDR_VRM;  
    tag_box->vrm[2].ED_VRM_RMA =   tag_base[index_tag_report].vrm[2].ED_VRM_RMA;    
    tag_box->vrm[2].PHY_ADDR_VRM = tag_base[index_tag_report].vrm[2].PHY_ADDR_VRM; 
    return sizeof(pos_tag_rsn_t);
  default:
    /* Oбработанных данных нет */
    return 0;
  }
}

/**
  * @brief Функция регистрации тега РСН основная программа в таблице 
  * @param pos_tag_rsn_t *tag_box - указатель на данные позиционирования
  * @param uint8_t number_vrm - число врм для позиционирования
  * @param uint16_t src_phy_addr - физический адрес источника
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - размер обработанных данных
  */
uint8_t RegTagRSNTable( pos_tag_rsn_t *tag_box, uint8_t number_vrm, uint16_t src_phy_addr, uint8_t src_port_ID )
{
  /* Объявляем переменную для индекса */
  uint16_t index_teg = 0;
  /* Переменная статуса записи в таблицу */
  uint8_t stat_record = 0;  
  
  /* Запрос индекса таблицы тегов     */
  stat_record = GetIndexTagTable( tag_box->head_tag.ADDR_Tag, &index_teg );
    
  if ( stat_record > 0 )
  {/* Получен индекс для записи в таблицу */
    /* Функция анализа режима обновления времени и регистрации тега */
    if ( UpdateTagTimeReg( src_phy_addr, tag_box->vrm[0].PHY_ADDR_VRM, index_teg, stat_record, src_port_ID ) )
    {
    /* Заполнение таблицы тегов */
    tag_base[index_teg].VERSION_ID = 3;		                             /*	   Версия пакета                      */
    tag_base[index_teg].src_phy_addr = src_phy_addr;                         /*	   Физический адрес источника         */
                                                                             
    tag_base[index_teg].TypeTag = tag_box->head_tag.TypeTag;	             /*	   Тип позиционируемого тега          */
    tag_base[index_teg].ADDR_Tag = tag_box->head_tag.ADDR_Tag;	             /*    Адрес позиционируемого тега        */
    tag_base[index_teg].ED_RMA_VRM = tag_box->head_tag.ED_RMA_VRM;           /*	   Уровень сигнала от тега к ВРМ      */
    tag_base[index_teg].DATA_ID = tag_box->head_tag.DATA_ID;                 /*	   ID данных пакета позиционирования  */                                   
      
    /* Заполнение поля данных */                                             
    for ( uint8_t contik_data_tag = 0 ; contik_data_tag < MAX_SIZE_DATA_POZ; contik_data_tag++ ) 
    {
      if (contik_data_tag < sizeof(data_tag_rsn_t) )
      {
        tag_base[index_teg].DATA[contik_data_tag] = ((uint8_t*)&(tag_box->DATA_TAG))[contik_data_tag];
      }
      else
      {
        tag_base[index_teg].DATA[contik_data_tag] = 0; 
      }  
    } 
    
    tag_base[index_teg].vrm[0].MAC_ADDR_VRM = 0;                             /*      Поле данных лучших ВРМ             */     
    tag_base[index_teg].vrm[1].MAC_ADDR_VRM = 0;                             /*      Поле данных лучших ВРМ             */
    tag_base[index_teg].vrm[2].MAC_ADDR_VRM = 0;                             /*      Поле данных лучших ВРМ             */        
    
    /* Функция для формирования диагностического сообщения по парсингу пакета позиционирования.*/
    diag_reg_rs_pos( "<<_REG_RS__POS_", src_phy_addr, tag_box->head_tag.TypeTag, tag_box->head_tag.ADDR_Tag, tag_box->vrm[0].PHY_ADDR_VRM );
    
    switch ( number_vrm )                                                    
    {                                                                        
    case 0:                                                                  
      tag_base[index_teg].Number_BPM = 0;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */      
      tag_base[index_teg].vrm[1].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      return sizeof(pos_tag_rsn_t) - 3*(sizeof(ed_best_vrm_t));
    case 1:
      tag_base[index_teg].Number_BPM = 1;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */      
      tag_base[index_teg].vrm[1].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */ 
      return sizeof(pos_tag_rsn_t) - 2*(sizeof(ed_best_vrm_t));          
    case 2:                                                                  
      tag_base[index_teg].Number_BPM = 2;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */        
      tag_base[index_teg].vrm[1].ED_VRM_RMA = tag_box->vrm[1].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = tag_box->vrm[1].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */   
      tag_base[index_teg].vrm[2].ED_VRM_RMA = 0;                             /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = 0;                           /*      Поле данных лучших ВРМ             */       
      return sizeof(pos_tag_rsn_t) - 1*(sizeof(ed_best_vrm_t));          
    case 3:                                                                  
      tag_base[index_teg].Number_BPM = 3;		                     /*	     Число лучших ВРМ                   */
      tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->vrm[0].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->vrm[0].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */        
      tag_base[index_teg].vrm[1].ED_VRM_RMA = tag_box->vrm[1].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[1].PHY_ADDR_VRM = tag_box->vrm[1].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */     
      tag_base[index_teg].vrm[2].ED_VRM_RMA = tag_box->vrm[2].ED_VRM_RMA;    /*      Поле данных лучших ВРМ             */
      tag_base[index_teg].vrm[2].PHY_ADDR_VRM = tag_box->vrm[2].PHY_ADDR_VRM;/*      Поле данных лучших ВРМ             */  
      return sizeof(pos_tag_rsn_t);
    default:
      /* Oбработанных данных нет */
      return 255;
    }
    }
  }
  
  /* Если Тег в таблице уже есть и источник позиционирования не лучший ВРМ - данные в таблице не обновляем */
  switch ( number_vrm )                                                    
  {                                                                        
  case 0:  return sizeof(pos_tag_rsn_t) - 3*(sizeof(ed_best_vrm_t));   
  case 1:  return sizeof(pos_tag_rsn_t) - 2*(sizeof(ed_best_vrm_t));   
  case 2:  return sizeof(pos_tag_rsn_t) - 1*(sizeof(ed_best_vrm_t));   
  case 3:  return sizeof(pos_tag_rsn_t);
  default: return 255;  /* Oбработанных данных нет */
  }
}

/**
  * @brief Функция формирования отчета регистрации тега в таблице 
  * @param uint8_t* tаg_box - указатель начала тега
  * @param uint16_t cntik_tag_report - индекс тега 
  * @param uint8_t max_data_size - максимально допустимый размер данных 
  * @retval uint8_t - размер обработанных данных
  */
uint8_t ProcessingTagReport( uint8_t* tag_box, uint16_t index_tag_report, uint8_t max_data_size )
{
  switch ( tag_base[index_tag_report].TypeTag )
  {
  case RF_TYPE_ABON_RMA_METHANE_SPUTNIK_pv3:
    /* RMA - спутник */
    switch ( tag_base[index_tag_report].DATA_ID )
    {
    case 0x01:
      /* Загрузчик */
      if ( max_data_size <= sizeof(pos_tag_sputnic_loader_t ) ) return 255;
      return ReportTagSputnicLoaderTable( (pos_tag_sputnic_loader_t*)tag_box, index_tag_report );      
      break;
    case 0x09:
      /* Загрузчик */
      if ( max_data_size <= sizeof(pos_tag_sputnic_loader_no_protocol_t ) ) return 255;
      return ReportTagSputnicLoaderNoProtocolTable( (pos_tag_sputnic_loader_no_protocol_t*)tag_box, index_tag_report );      
      break;
    case 0x0A:
      /* Основная программа */  
      if ( max_data_size <= sizeof(pos_tag_sputnic_t ) ) return 255;
      return ReportTagSputnicTable( (pos_tag_sputnic_t*)tag_box, index_tag_report );
      break;
    case 0x0B:
      /* Основная программа */  
      if ( max_data_size <= sizeof(pos_tag_sputnic_no_protocol_t ) ) return 255;
      return ReportTagSputnicNoProtocolTable( (pos_tag_sputnic_no_protocol_t*)tag_box, index_tag_report );
      break;          
    default:
      /* Oбработанных данных нет */
      return 0;
    }
    break;
  case RF_TYPE_ABON_RMA_MEMS_pv3:
    /* RMA - STM */
    switch ( tag_base[index_tag_report].DATA_ID )
    {
    case 0x01:
      /* Загрузчик */ 
      if ( max_data_size <= sizeof(pos_tag_rma_stm_loader_t ) ) return 255;
      return ReportTagStmLoaderTable( (pos_tag_rma_stm_loader_t*)tag_box, index_tag_report );  
      break;
    case 0x0A:
      /* Основная программа */ 
      if ( max_data_size <= sizeof(pos_tag_rma_stm_t ) ) return 255;
      return ReportTagStmTable( (pos_tag_rma_stm_t*)tag_box, index_tag_report );  
      break;
    default:
      /* Oбработанных данных нет */
      return 0;
    }
    break;
  case RF_TYPE_ABON_RSN_pv3:
  case RF_TYPE_ABON_RSN_SE2435L_pv3:    
  case RF_TYPE_ABON_RSS_SE2435L_pv3:      
  case RF_TYPE_ABON_RSNP_pv3:       
    /* PCH-У/PCH/PCC/PCН-П */
    switch ( tag_base[index_tag_report].DATA_ID )
    {
    case 0x01:
      /* Загрузчик */ 
      if ( max_data_size <= sizeof(pos_tag_rsn_loader_t ) ) return 255;
      return ReportTagRSNLoaderTable( (pos_tag_rsn_loader_t*)tag_box, index_tag_report );  
    case 0x0A:
      /* Основная программа */ 
      if ( max_data_size <= sizeof(pos_tag_rsn_t ) ) return 255;
      return ReportTagRSNTable( (pos_tag_rsn_t*)tag_box, index_tag_report );  
    default:
      /* Oбработанных данных нет */
      return 0;
    }
    break;
  default:
    /* Oбработанных даных нет */
    return 0;
  }
}

/*____________________________________________________________________________*/

/**
  * @brief Подготовка и отправка пакета отчета позиционирования в RS.
  * @param radio_struct_t* PortRF - указатель на структуру радио шлюза
  * @param router_box_t RsBOX_t - указатель на фрейм который нужно отправить
  * @param uint8_t size_data_box - размер данных в пакете
  * @retval none
  */
void TxReportPositionFrame( core_struct_t* core_st, RSFrameTagNewPosV3_t* report_box , uint8_t size_data_box )
{

  report_box->frame_tag_pos.VERSION_ID = 3;                                     /* Версия пакета                                       */
  report_box->frame_tag_pos.own_physical_addr = DataLoaderSto.Settings.phy_adr; /* Собсвенный физический адрес                         */
  report_box->frame_tag_pos.Number_BPM = DataSto.Settings.number_position_vrm;  /* Число лучших ВРМ  - для данной структуры            */
  
  report_box->frame_tag_pos.size = size_data_box + sizeof(NwkFrameTagNewPosV3_t);/* длинна пакета  */
  /* Формирование шапки пакета по RF установить настройки фрейма на mac уровне (для всех фреймов одинаковые) и заголовок MAC в посылке */  
  report_box->frame_tag_pos.header.macSeq = core_st->contic_tx_box;             /* sequence number */
  /* без аппаратного подтверждения фрейма                                       */
  report_box->frame_tag_pos.header.macFcf = 0x8841;                             /* frame control   */
  /* установить PANID уст. в собственных настройках                             */
  report_box->frame_tag_pos.header.macDstPanId = 0xABCD;
  /* Установить mac адрес устройства назначения = широковещательный             */
  report_box->frame_tag_pos.header.macDstAddr = 0xFFFF;
  /*  Записать mac адрес источника данных = собственному адресу роутера         */
  report_box->frame_tag_pos.header.macSrcAddr = DataSto.Settings.mac_adr;  
  /*  заголовок NWK в посылке (входит в PAYLOAD посылки по IEEE  802.15.4)      */
  report_box->frame_tag_pos.header.id1 = _RF_DATA_VRM_TX_POSITIONING_;                         /* ID1 пакета                                                     */
  report_box->frame_tag_pos.header.id2 = (uint8_t)~(unsigned)_RF_DATA_VRM_TX_POSITIONING_;     /* ID2 пакета                                                     */
                                                                                               /*                                                                */
  report_box->frame_tag_pos.header.nwkFcf_u.val = 0x16;                                        /* Сбросить все флаги настроек, после установить нужные           */
  report_box->frame_tag_pos.header.nwkSeq = core_st->contic_tx_box;                            /* идентификатор последовательности кадра                         */
  report_box->frame_tag_pos.header.nwkSrcPanId = 1;                                            /* Сетевой PANID источник (он же зона для ретрасляции)            */
  report_box->frame_tag_pos.header.nwkDstPanId = 1;                                            /* Сетевой PANID назначения (зона для ретрасляции)                */
  report_box->frame_tag_pos.header.nwkDstRouterAddr = 0xff;                                    /* Установить адрес роутера кому предназнаются данные             */
                                                                                               /*                                                                */
  report_box->frame_tag_pos.header.nwkSrcHop = 0xff;                                           /* Хоп источника данных                                           */
  report_box->frame_tag_pos.header.nwkOwnHop = 0xff;                                           /* Собственный хоп                                                */
  report_box->frame_tag_pos.header.nwkSrcAddr = DataSto.Settings.mac_adr;                      /* Записать адрес источника данных                                */
  report_box->frame_tag_pos.header.nwkDstAddr = 0xFFFF;                                        /* Записать короткий адрес назначения                             */
  report_box->frame_tag_pos.header.nwkDstEndpoint = 0x01;                                      /* Установить EndPoint назначения                                 */
  report_box->frame_tag_pos.header.nwkSrcEndpoint = 0x01;                                      /* Установить EndPoint источника                                  */
  report_box->frame_tag_pos.header.nwk_count_routing = 0;                                      /* Установить начальное значение счётчика ретрасляции             */
  report_box->frame_tag_pos.header.nwk_src_factory_addr = DataLoaderSto.Settings.phy_adr;      /* Установить заводской адрес истоника текущей посылки            */
  report_box->frame_tag_pos.header.nwk_own_factory_addr = DataLoaderSto.Settings.phy_adr;      /* Установить заводской адрес инициатора посылки                  */
                                                                                               /*                                                                */
  report_box->frame_tag_pos.header.reserv1 = 0x00;                                             /* резерв 1                                                       */
  report_box->frame_tag_pos.header.reserv2 = 0x0000;                                           /* резерв 2                                                       */
  
  /* =================== Формирование шапки пакета по RS ==================== */                                                                                                                          
  report_box->rs_head.pre = 0xAA55;                                                            /* Преамбула  0x55 0xAA                                           */
  report_box->rs_head.lenght = report_box->frame_tag_pos.size + sizeof(head_box_t) - SIZE_PRE - 1; /* Длина пакета (включая контрольную сумму, не включая преамбулу) */
  report_box->rs_head.id = ID_NEW_POSITION;                                                    /* Идентификатор пакета                                           */
  report_box->rs_head.dest = 0xFFFF;                                                           /* Физический адрес получателя                                    */
  report_box->rs_head.src = DataLoaderSto.Settings.phy_adr;                                    /* Устанавливаем свой физический адрес источника                  */
  
  /* Обновление RS пакета перед отправкой */
  update_rs_tx_box( (router_box_t*)report_box,&(core_st->contic_tx_box));
  
  /* Отправляетм пакет в роутер */
  if (core_st->xQueue_core_router != NULL)
  {
    /* отправить пакет в роутер */
    xQueueSend ( core_st->xQueue_core_router, report_box , ( TickType_t ) 0 );
  }   
}

/**
  * @brief Функция формирования пакета отчета о позиционировании
  * @param core_struct_t*  core_st - указатель на структуру ядра
  * @retval None
  */
void ProcessingPositionReport( core_struct_t* core_st )
{
  uint8_t cnt_wr_byte = 0; /* Число записанных байт             */
  uint8_t tag_wr_byte = 0; /* Размер записанного тега в байтах  */
  uint8_t pindex_id_port;  /* индекс порта источника ID  */
  
  /* Объявляем указатель на */
  RSFrameTagNewPosV3_t* ReportFramePosTag = (RSFrameTagNewPosV3_t*)(&(core_st->data_tx));
  
  /* получения индекса порта источника по заданному ID */
  if ( get_index_for_port_scr_tag( PositionID , &pindex_id_port ) ) 
  {
    /* Цикл по таблице тегов */
    for ( uint16_t cntik_tag_report = 0; cntik_tag_report < MAX_NUM_TAG; cntik_tag_report++ )
    {
      /* Если все теги обновлены - выход */
      if ( tag_base[cntik_tag_report].ADDR_Tag == 0x0000 ) break;
      /* Если таg позиционируется в RF - заносим его в отчет */
      if ( tag_base[cntik_tag_report].TOFL[pindex_id_port] > 0 )
      {
        /* Функция формирования отчета регистрации тега в таблице */
        tag_wr_byte = ProcessingTagReport( &(ReportFramePosTag->frame_tag_pos.data_tag[cnt_wr_byte]), cntik_tag_report, MAX_SIZE_DATA - cnt_wr_byte );
        
        /* Анализ отчета по тегу */
        if ( tag_wr_byte == 255 )
        {
          /* Переполнение пакета - отправляем пакет */
          TxReportPositionFrame( core_st, ReportFramePosTag, cnt_wr_byte );
          /* Корректируем счетчики данных*/
          cnt_wr_byte = 0;
          /* Повторная отправка тега на обработку*/
          /* Функция формирования отчета регистрации тега в таблице */
          tag_wr_byte = ProcessingTagReport( &(ReportFramePosTag->frame_tag_pos.data_tag[cnt_wr_byte]), cntik_tag_report, MAX_SIZE_DATA - cnt_wr_byte);
        }       
        
        /* Корректируем счетчики данных*/
        cnt_wr_byte = cnt_wr_byte + tag_wr_byte;
      }  
    }
    /* Анализ отчета по тегу */
    if ( cnt_wr_byte > 0 )
    {
      /* Есть данные в пакете - отправляем пакет */
      TxReportPositionFrame( core_st, ReportFramePosTag, cnt_wr_byte );
    }
  }
}

/*____________________________________________________________________________*/

/**
  * @brief Функция регистрации тега в таблице 
  * @param uint8_t* tаg_box 
  * @param uint8_t number_vrm - число врм для позиционирования
  * @param uint16_t src_phy_addr
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - размер обработанных данных
  */
uint8_t ProcessingNewPositionBoxV3( uint8_t* tag_box, uint8_t number_vrm, uint16_t src_phy_addr, uint8_t src_port_ID )
{
  /* Объявляем указатель на шапку пакета */
  head_tag_new_pos_v3_t  *head_tag = (head_tag_new_pos_v3_t*)tag_box;
  
  switch ( head_tag->TypeTag )
  {
  case 0x50:
    /* RMA - спутник */
    switch ( head_tag->DATA_ID )
    {
    case 0x01:
      /* Загрузчик */
      return RegTagSputnicLoaderTable( (pos_tag_sputnic_loader_t*)tag_box, number_vrm, src_phy_addr, src_port_ID );      
      break;
    case 0x09:
      /* Загрузчик */
      return RegTagSputnicLoaderNoProtocolTable( (pos_tag_sputnic_loader_no_protocol_t*)tag_box, number_vrm, src_phy_addr, src_port_ID );      
      break;      
    case 0x0A:
      /* Основная программа */      
      return RegTagSputnicTable( (pos_tag_sputnic_t*)tag_box, number_vrm, src_phy_addr, src_port_ID );
      break;
    case 0x0B:
      /* Основная программа */      
      return RegTagSputnicNoProtocolTable( (pos_tag_sputnic_no_protocol_t*)tag_box, number_vrm, src_phy_addr, src_port_ID );
      break;
    default:
      /* Oбработанных данных нет */
      return 255;
    }
    break;
  case 0x60:
    /* RMA - STM */
    switch ( head_tag->DATA_ID )
    {
    case 0x01:
      /* Загрузчик */ 
      return RegTagStmLoaderTable( (pos_tag_rma_stm_loader_t*)tag_box, number_vrm, src_phy_addr, src_port_ID );  
      break;
    case 0x0A:
      /* Основная программа */ 
      return RegTagStmTable( (pos_tag_rma_stm_t*)tag_box, number_vrm, src_phy_addr, src_port_ID );  
      break;
    default:
      /* Oбработанных данных нет */
      return 255;
    }
    break;
  case RF_TYPE_ABON_RSN_pv3:
  case RF_TYPE_ABON_RSN_SE2435L_pv3:    
  case RF_TYPE_ABON_RSS_SE2435L_pv3:      
  case RF_TYPE_ABON_RSNP_pv3:    
    /* PCH-У/PCH/PCC/PCН-П */
    switch ( head_tag->DATA_ID )
    {
    case 0x01:
      /* Загрузчик */ 
      return RegTagRSNLoaderTable( (pos_tag_rsn_loader_t*)tag_box, number_vrm, src_phy_addr, src_port_ID );  
      break;
    case 0x0A:
      /* Основная программа */ 
      return RegTagRSNTable( (pos_tag_rsn_t*)tag_box, number_vrm, src_phy_addr, src_port_ID );  
      break;
    default:
      /* Oбработанных данных нет */
      return 255;
    }
    break;
  default:
    /* Oбработанных даных нет */
    return 255;
  }
}

uint16_t CntTag = 0;
uint16_t TempCntTag = 0;

/**
  * @brief  Функция формирования статуса зарегистрированных тегов
  * @param  char *pcInsert - указатель на массив pcInsert для формируемой строки
  * @param  enum_type_stage_t stage - режим вызова STAT_TITLE - выдача имени страницы
  * 						   STAT_HEAD - выдача заголовка таблицы
  * 						   STAT_LINE - выдача строки таблицы
  *
  *
  * @retval uint16_t длина формируемой строки
  */
uint16_t TabStatusTag(char *pcInsert, enum_type_stage_t stage)
{
  /* Переменная для отображения типа устройства */
  char type_name[10]; 
  /* Переменая длинны выводимой строки */
  uint16_t temp_size;
  
  switch (stage)
  {
  case STAT_TITLE:
    /* Вывод наименования таблицы */
    return  sprintf(pcInsert,"Таблица зарегистрированных тегов");
  case STAT_HEAD:
    /* Вывод шапки таблицы        */
    CntTag = 0;
    temp_size =  sprintf(pcInsert,"<tr><td>VerID</td><td>SrcAdr</td><td>NumVrm</td><td>TypeTag</td><td>AdrTag</td><td>ED R->V</td><td>DataID</td><td>AdrVRM</td><td>ED V->R</td><td>AdrVRM</td><td>ED V->R</td><td>AdrVRM</td><td>ED V->R</td>");
    /* Дополнение строки выводом наименования таблицы TOFL */
    for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
    {
      temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>%s</td>",  st_core.tag_ch_stat[cnt_tab].ShotNameSrcPortID );  
    }
    return  temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"</tr>");   
  case STAT_LINE:
    /* Вывод строк таблицы        */
    while( (tag_base[CntTag].ADDR_Tag > 0) && ( CntTag < MAX_NUM_TAG ) ) 
    { /* Сохраняем индекс записи       */
      TempCntTag = CntTag;
      /* Инкрементируем счетчик записи */               
      CntTag++;
      
      switch (tag_base[TempCntTag].TypeTag)
      {
      case RF_TYPE_ABON_NO:                      
        sprintf(type_name,"");
        break;
      case RF_TYPE_ABON_RMA:                     
        sprintf(type_name,"    RMA");    
        break; 
      case RF_TYPE_ABON_RMAT:                    
        sprintf(type_name,"  RMA-T");    
        break;
      case RF_TYPE_ABON_RSN:                     
        sprintf(type_name,"    RSN");    
        break;
      case RF_TYPE_ABON_RSN_SE2435L:             
        sprintf(type_name,"  RSN-U");    
        break;        
      case RF_TYPE_ABON_RSNP:                    
        sprintf(type_name,"  RSN-P");    
        break;
      case RF_TYPE_ABON_RMA_METHANE_SPUTNIK_pv3: 
        sprintf(type_name,"RMA-SPU");    
        break;       
      case RF_TYPE_ABON_RMA_MEMS_pv3:            
        sprintf(type_name,"RMA-STM");    
        break;        
      case RF_TYPE_ABON_RSN_pv3:                 
        sprintf(type_name,"    RSN");    
        break;
      case RF_TYPE_ABON_RSN_SE2435L_pv3:         
        sprintf(type_name,"  RSN-U");    
        break; 
      case RF_TYPE_ABON_RSS_SE2435L_pv3:         
        sprintf(type_name,"  RSS-U");    
        break;
      case RF_TYPE_ABON_RSNP_pv3:                
        sprintf(type_name,"  RSN-P");    
        break;
      default:                                   
        sprintf(type_name,"");           
        break;
      };       
      
      temp_size = sprintf(pcInsert,"<tr><td>%.2d</td><td>%.5d</td><td>%.2d</td><td>%.7s</td><td>%.5d</td><td>%.2d</td><td>%.2d</td><td>%.5d</td><td>%.2d</td><td>%.5d</td><td>%.2d</td><td>%.5d</td><td>%.2d</td>",
                      tag_base[TempCntTag].VERSION_ID,
                      tag_base[TempCntTag].src_phy_addr, 
                      tag_base[TempCntTag].Number_BPM,
                      type_name,  
                      tag_base[TempCntTag].ADDR_Tag,                        
                      tag_base[TempCntTag].ED_RMA_VRM,
                      tag_base[TempCntTag].DATA_ID,                       
                      tag_base[TempCntTag].vrm[0].PHY_ADDR_VRM,
                      tag_base[TempCntTag].vrm[0].ED_VRM_RMA,                       
                      tag_base[TempCntTag].vrm[1].PHY_ADDR_VRM,
                      tag_base[TempCntTag].vrm[1].ED_VRM_RMA,                       
                      tag_base[TempCntTag].vrm[2].PHY_ADDR_VRM,
                      tag_base[TempCntTag].vrm[2].ED_VRM_RMA);
      
      /* Дополнение строки выводом таблицы TOFL */
      for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
      {
        temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>%d</td>", tag_base[TempCntTag].TOFL[cnt_tab] );  
      }
      return  temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"</tr>");
      
    }
    /* Печатаем пустое поле */
    pcInsert = "";
    return 0;
  default:
    /* Печатаем пустое поле */
    pcInsert = "";
    return 0;
  }
}

/**
  * @brief  Функция формирования списка зарегистрированных каналов
  * @param  char *pcInsert - указатель на массив pcInsert для формируемой строки
  * @param  enum_type_stage_t stage - режим вызова STAT_TITLE - выдача имени страницы
  * 						   STAT_HEAD - выдача заголовка таблицы
  * 						   STAT_LINE - выдача строки таблицы
  *
  *
  * @retval uint16_t длина формируемой строки
  */
uint16_t TabStatusChanel(char *pcInsert, enum_type_stage_t stage)
{
  uint16_t temp_size = 0;
  
  switch (stage)
  {
  case STAT_TITLE:
    /* Вывод наименования таблицы */
    return  sprintf(pcInsert,"Таблица зарегистрированных каналов голосовых тегов");
  case STAT_HEAD:
    /* Вывод шапки таблицы        */
    CntTag = 1;
    
    temp_size = sprintf((char *)(pcInsert + (uint32_t)temp_size),"<tr><td>Описание</td>");  
    
    for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
    {
      temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>%s</td>", st_core.tag_ch_stat[cnt_tab].NameSrcPortID );  
    }
    return  temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"</tr>");
    
  case STAT_LINE:
    /* Вывод строк таблицы        */
    while(CntTag < MAX_SIZE_TAB_CH_VOICE_TAG) 
    { /* Сохраняем индекс записи       */
      TempCntTag = CntTag;
      /* Инкрементируем счетчик записи */               
      CntTag++;

      /* Проверка каналов по всем портам */
      for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
      {
        if( GetNumVoiceTagChanel ( TempCntTag, st_core.tag_ch_stat[cnt_tab].SrcPortID ) > 0)
        {
          temp_size = sprintf((char *)(pcInsert + (uint32_t)temp_size),"<tr><td>%d</td>",TempCntTag);  
          
          for ( uint8_t cntic_tab = 0; cntic_tab < MAX_TAG_CH_TAB;cntic_tab++ )
          {
            temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>%d</td>",GetNumVoiceTagChanel ( TempCntTag, st_core.tag_ch_stat[cntic_tab].SrcPortID ));  
          }
          return  temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"</tr>");      
        }
      }
    }
    /* Печатаем пустое поле */
    pcInsert = "";
    return 0;
  default:
    /* Печатаем пустое поле */
    pcInsert = "";
    return 0;
  }
}
 
/**
  * @brief  Функция формирования списка число зарегистрированных тегов по типам
  * @param  char *pcInsert - указатель на массив pcInsert для формируемой строки
  * @param  enum_type_stage_t stage - режим вызова STAT_TITLE - выдача имени страницы
  * 						   STAT_HEAD - выдача заголовка таблицы
  * 						   STAT_LINE - выдача строки таблицы
  *
  *
  * @retval uint16_t длина формируемой строки
  */
uint16_t TabStatusRegTag(char *pcInsert, enum_type_stage_t stage)
{
  uint16_t temp_size = 0;  
  
  switch (stage)
  {
  case STAT_TITLE:
    /* Вывод наименования таблицы */
    return  sprintf(pcInsert,"Таблица зарегистрированных тегов по типам");
  case STAT_HEAD:
    /* Вывод шапки таблицы        */
    CntTag = 1;
    temp_size = sprintf(pcInsert,"<tr><td>Тип тега</td>");
    /* Дополнение строки выводом наименования таблицы TOFL */
    for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
    {
      temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>%s</td>", st_core.tag_ch_stat[cnt_tab].ShotNameSrcPortID );  
    }
  return  temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"</tr>");
      
  case STAT_LINE:
    CntTag++;
    switch (CntTag)
    {
    case 2:/* Всего зарегистрировано тегов */
      temp_size = sprintf(pcInsert,"<tr><td>Всего тегов</td>");
      /* Дополнение строки выводом наименования таблицы TOFL */
      for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
      {
        temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>%.2d</td>",  GetRegTagAll( st_core.tag_ch_stat[cnt_tab].SrcPortID ) );  
      }
      return  temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"</tr>");          
      
    case 3:/* Тегов светильника */
      temp_size = sprintf(pcInsert,"<tr><td>Тегов светильника</td>");
      /* Дополнение строки выводом наименования таблицы TOFL */
      for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
      {
        temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>%.2d</td>",  GetRegTagRMA( st_core.tag_ch_stat[cnt_tab].SrcPortID ) );  
      }
      return  temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"</tr>");      
      
    case 4:/* Тегов радиостанции */
      temp_size = sprintf(pcInsert,"<tr><td>Тегов радиостанции</td>");
      /* Дополнение строки выводом наименования таблицы TOFL */
      for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
      {
        temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>%.2d</td>",  GetRegTagRSN( st_core.tag_ch_stat[cnt_tab].SrcPortID ) );  
      }
      return  temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"</tr>");          
      
    case 5:/* Тегов газоанализаторов */
      temp_size = sprintf(pcInsert,"<tr><td>Тегов газоанализаторов</td>");
      /* Дополнение строки выводом наименования таблицы TOFL */
      for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
      {
        temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>%.2d</td>",  GetRegTagGas( st_core.tag_ch_stat[cnt_tab].SrcPortID ) );  
      }
      return  temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"</tr>");        
      
    default:    
      /* Печатаем пустое поле */
      pcInsert = "";
      return 0;
    }   
  default:
    /* Печатаем пустое поле */
    pcInsert = "";
    return 0;
  }
}

/**
  * @brief  Функция формирования статуса зарегистрированных тегов 
  * @param  char *pcInsert - указатель на массив pcInsert для формируемой строки
  * @param  enum_type_stage_t stage - режим вызова STAT_TITLE - выдача имени страницы
  * 						   STAT_HEAD - выдача заголовка таблицы
  * 						   STAT_LINE - выдача строки таблицы
  *
  *
  * @retval uint16_t длина формируемой строки
  */
uint16_t TabStatusTagPos(char *pcInsert, enum_type_stage_t stage)
{
  /* Переменная для отображения типа устройства */
  char type_name[10];   
  /* Переменая длинны выводимой строки */
  uint16_t temp_size;  
  /* индекс порта источника ID  */
  uint8_t pindex_id_port;  
 
  switch (stage)
  {
  case STAT_TITLE:
    /* Вывод наименования таблицы */
    return  sprintf(pcInsert,"Таблица зарегистрированных тегов");
  case STAT_HEAD:
    /* Вывод шапки таблицы        */
    CntTag = 0;
    temp_size = sprintf(pcInsert,"<tr><td>VerID</td><td>SrcAdr</td><td>NumVrm</td><td>TypeTag</td><td>AdrTag</td><td>ED R->V</td><td>DataID</td><td>AdrVRM</td><td>ED V->R</td><td>AdrVRM</td><td>ED V->R</td><td>AdrVRM</td><td>ED V->R</td>");
    /* Дополнение строки выводом наименования таблицы TOFL */
    for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
    {
      temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>%s</td>",  st_core.tag_ch_stat[cnt_tab].ShotNameSrcPortID );  
    }
    return  temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"</tr>");   
  case STAT_LINE:
    /* Вывод строк таблицы        */
    while( (tag_base[CntTag].ADDR_Tag > 0) && ( CntTag < MAX_NUM_TAG ) ) 
    { /* Сохраняем индекс записи       */
      TempCntTag = CntTag;
      /* Инкрементируем счетчик записи */               
      CntTag++;
      /* получения индекса порта источника по заданному ID */
      if ( get_index_for_port_scr_tag( PositionID, &pindex_id_port ) ) 
      {
        if ( ( tag_base[TempCntTag].TOFL[pindex_id_port] ) > 0 ) 
        {
          switch (tag_base[TempCntTag].TypeTag)
          {
          case RF_TYPE_ABON_NO:                      
            sprintf(type_name,"");
            break;
          case RF_TYPE_ABON_RMA:                     
            sprintf(type_name,"    RMA");    
            break; 
          case RF_TYPE_ABON_RMAT:                    
            sprintf(type_name,"  RMA-T");    
            break;
          case RF_TYPE_ABON_RSN:                     
            sprintf(type_name,"    RSN");    
            break;
          case RF_TYPE_ABON_RSN_SE2435L:             
            sprintf(type_name,"  RSN-U");    
            break;        
          case RF_TYPE_ABON_RSNP:                    
            sprintf(type_name,"  RSN-P");    
            break;
          case RF_TYPE_ABON_RMA_METHANE_SPUTNIK_pv3: 
            sprintf(type_name,"RMA-SPU");    
            break;       
          case RF_TYPE_ABON_RMA_MEMS_pv3:            
            sprintf(type_name,"RMA-STM");    
            break;        
          case RF_TYPE_ABON_RSN_pv3:                 
            sprintf(type_name,"    RSN");    
            break;
          case RF_TYPE_ABON_RSN_SE2435L_pv3:         
            sprintf(type_name,"  RSN-U");    
            break; 
          case RF_TYPE_ABON_RSS_SE2435L_pv3:         
            sprintf(type_name,"  RSS-U");    
            break;
          case RF_TYPE_ABON_RSNP_pv3:                
            sprintf(type_name,"  RSN-P");    
            break;
          default:                                   
            sprintf(type_name,"");           
            break;
          };            
          temp_size = sprintf(pcInsert,"<tr><td>%.2d</td><td>%.5d</td><td>%.2d</td><td>%.7s</td><td>%.5d</td><td>%.2d</td><td>%.2d</td><td>%.5d</td><td>%.2d</td><td>%.5d</td><td>%.2d</td><td>%.5d</td><td>%.2d</td>",
                              tag_base[TempCntTag].VERSION_ID,
                              tag_base[TempCntTag].src_phy_addr, 
                              tag_base[TempCntTag].Number_BPM,
                              type_name,  
                              tag_base[TempCntTag].ADDR_Tag,                        
                              tag_base[TempCntTag].ED_RMA_VRM,
                              tag_base[TempCntTag].DATA_ID,                       
                              tag_base[TempCntTag].vrm[0].PHY_ADDR_VRM,
                              tag_base[TempCntTag].vrm[0].ED_VRM_RMA,                       
                              tag_base[TempCntTag].vrm[1].PHY_ADDR_VRM,
                              tag_base[TempCntTag].vrm[1].ED_VRM_RMA,                       
                              tag_base[TempCntTag].vrm[2].PHY_ADDR_VRM,
                              tag_base[TempCntTag].vrm[2].ED_VRM_RMA); 
          
          /* Дополнение строки выводом таблицы TOFL */
          for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
          {
            temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>%d</td>", tag_base[TempCntTag].TOFL[cnt_tab] );  
          }
          return  temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"</tr>");        
        }
      }  
    }
    /* Печатаем пустое поле */
    pcInsert = "";
    return 0;
  default:
    /* Печатаем пустое поле */
    pcInsert = "";
    return 0;
  }
}

/**
  * @brief  Функция формирования статуса зарегистрированных тегов RMA STM
  * @param  char *pcInsert - указатель на массив pcInsert для формируемой строки
  * @param  enum_type_stage_t stage - режим вызова STAT_TITLE - выдача имени страницы
  * 						   STAT_HEAD - выдача заголовка таблицы
  * 						   STAT_LINE - выдача строки таблицы
  *
  *
  * @retval uint16_t длина формируемой строки
  */
uint16_t TabStatusTagRmaSTM(char *pcInsert, enum_type_stage_t stage)
{
  /* Переменая длинны выводимой строки */
  uint16_t temp_size;  
  
  switch (stage)
  {
  case STAT_TITLE:
    /* Вывод наименования таблицы */
    return  sprintf(pcInsert,"Таблица зарегистрированных тегов");
  case STAT_HEAD:
    /* Вывод шапки таблицы        */
    CntTag = 0;
    temp_size = sprintf(pcInsert,"<tr><td>VerID</td><td>SrcAdr</td><td>NumVrm</td><td>TypeTag</td><td>AdrTag</td><td>ED R->V</td><td>DataID</td><td>StatusTag</td><td>ChargeACB</td><td>Accelero</td><td>AdrVRM</td><td>ED V->R</td><td>AdrVRM</td><td>ED V->R</td><td>AdrVRM</td><td>ED V->R</td>");
    /* Дополнение строки выводом наименования таблицы TOFL */
    for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
    {
      temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>%s</td>",  st_core.tag_ch_stat[cnt_tab].ShotNameSrcPortID );  
    }
    return  temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"</tr>");   
  case STAT_LINE:
    /* Вывод строк таблицы        */
    while( (tag_base[CntTag].ADDR_Tag > 0) && ( CntTag < MAX_NUM_TAG ) ) 
    { /* Сохраняем индекс записи       */
      TempCntTag = CntTag;
      /* Инкрементируем счетчик записи */               
      CntTag++;
      /* Вывод только тегов на  RMA STM */
      if ( tag_base[TempCntTag].TypeTag == 0x60) 
      {
        temp_size = sprintf(pcInsert,"<tr><td>%.2d</td><td>%.5d</td><td>%.2d</td><td>%.7s</td><td>%.5d</td><td>%.2d</td><td>%.2d</td><td>0x%.2X</td><td>%.3d</td><td>%.3d</td><td>%.5d</td><td>%.2d</td><td>%.5d</td><td>%.2d</td><td>%.5d</td><td>%.2d</td>",
                        tag_base[TempCntTag].VERSION_ID,
                        tag_base[TempCntTag].src_phy_addr, 
                        tag_base[TempCntTag].Number_BPM,
                        "RMA-STM",  
                        tag_base[TempCntTag].ADDR_Tag,                        
                        tag_base[TempCntTag].ED_RMA_VRM,
                        tag_base[TempCntTag].DATA_ID,
                        (*(data_tag_rma_stm_t*)(&(tag_base[TempCntTag].DATA[0]))).StatusTag,
                        (*(data_tag_rma_stm_t*)(&(tag_base[TempCntTag].DATA[0]))).ChargeACB,
                        (*(data_tag_rma_stm_t*)(&(tag_base[TempCntTag].DATA[0]))).Accelerometer,                        
                        tag_base[TempCntTag].vrm[0].PHY_ADDR_VRM,
                        tag_base[TempCntTag].vrm[0].ED_VRM_RMA,                       
                        tag_base[TempCntTag].vrm[1].PHY_ADDR_VRM,
                        tag_base[TempCntTag].vrm[1].ED_VRM_RMA,                       
                        tag_base[TempCntTag].vrm[2].PHY_ADDR_VRM,
                        tag_base[TempCntTag].vrm[2].ED_VRM_RMA); 
        
        /* Дополнение строки выводом таблицы TOFL */
        for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
        {
          temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>%d</td>", tag_base[TempCntTag].TOFL[cnt_tab] );  
        }
        return  temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"</tr>");    
      }
    }
    /* Печатаем пустое поле */
    pcInsert = "";
    return 0;
  default:
    /* Печатаем пустое поле */
    pcInsert = "";
    return 0;
  }
}

/**
  * @brief  Функция формирования статуса зарегистрированных тегов RMA спутник
  * @param  char *pcInsert - указатель на массив pcInsert для формируемой строки
  * @param  enum_type_stage_t stage - режим вызова STAT_TITLE - выдача имени страницы
  * 						   STAT_HEAD - выдача заголовка таблицы
  * 						   STAT_LINE - выдача строки таблицы
  *
  *
  * @retval uint16_t длина формируемой строки
  */
uint16_t TabStatusTagSputnic(char *pcInsert, enum_type_stage_t stage)
{
  /* Переменая длинны выводимой строки */
  uint16_t temp_size;
 
  switch (stage)
  {
  case STAT_TITLE:
    /* Вывод наименования таблицы */
    return  sprintf(pcInsert,"Таблица зарегистрированных тегов");
  case STAT_HEAD:
    /* Вывод шапки таблицы        */
    CntTag = 0;
    temp_size = sprintf(pcInsert,"<tr><td>VerID</td><td>SrcAdr</td><td>NumVrm</td><td>TypeTag</td><td>AdrTag</td><td>ED R->V</td><td>DataID</td><td>StatusTag</td><td>GazCH4</td><td>MaxGazCH4</td><td>GazO2</td><td>MinGazO2</td><td>GazCO</td><td>MaxGazCO</td><td>GazCO2</td><td>MaxGazCO2</td><td>FlagsWorks</td><td>ChargeACB</td><td>AdrVRM</td><td>ED V->R</td><td>AdrVRM</td><td>ED V->R</td><td>AdrVRM</td><td>ED V->R</td>");
    /* Дополнение строки выводом наименования таблицы TOFL */
    for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
    {
      temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>%s</td>",  st_core.tag_ch_stat[cnt_tab].ShotNameSrcPortID );  
    }
    return  temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"</tr>");   
  case STAT_LINE:
    /* Вывод строк таблицы        */
    while( (tag_base[CntTag].ADDR_Tag > 0) && ( CntTag < MAX_NUM_TAG ) ) 
    { /* Сохраняем индекс записи       */
      TempCntTag = CntTag;
      /* Инкрементируем счетчик записи */               
      CntTag++;
      /* Вывод только тегов на  RMA STM */
      if ( tag_base[TempCntTag].TypeTag == 0x50) 
      {
        switch (tag_base[TempCntTag].DATA_ID)
        { 
        case 0x01:        
          temp_size = sprintf(pcInsert,"<tr><td>%.2d</td><td>%.5d</td><td>%.2d</td><td>%.15s</td><td>%.5d</td><td>%.2d</td><td>%.2d</td><td>0x%.2X</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.3d</td><td>%.5d</td><td>%.2d</td><td>%.5d</td><td>%.2d</td><td>%.5d</td><td>%.2d</td>",
                              tag_base[TempCntTag].VERSION_ID,
                              tag_base[TempCntTag].src_phy_addr, 
                              tag_base[TempCntTag].Number_BPM,
                              "SPU_LDR",  
                              tag_base[TempCntTag].ADDR_Tag,                        
                              tag_base[TempCntTag].ED_RMA_VRM,
                              tag_base[TempCntTag].DATA_ID,
                              (*(data_tag_sputnic_loader_t*)(&(tag_base[TempCntTag].DATA[0]))).StatusTag,
                              0,0,0,0,0,0,0,0,0,0,
                              tag_base[TempCntTag].vrm[0].PHY_ADDR_VRM,
                              tag_base[TempCntTag].vrm[0].ED_VRM_RMA,                       
                              tag_base[TempCntTag].vrm[1].PHY_ADDR_VRM,
                              tag_base[TempCntTag].vrm[1].ED_VRM_RMA,                       
                              tag_base[TempCntTag].vrm[2].PHY_ADDR_VRM,
                              tag_base[TempCntTag].vrm[2].ED_VRM_RMA); 
          break;            
        case 0x09:        
          temp_size = sprintf(pcInsert,"<tr><td>%.2d</td><td>%.5d</td><td>%.2d</td><td>%.7s</td><td>%.5d</td><td>%.2d</td><td>%.2d</td><td>0x%.2X</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.3d</td><td>%.5d</td><td>%.2d</td><td>%.5d</td><td>%.2d</td><td>%.5d</td><td>%.2d</td>",
                              tag_base[TempCntTag].VERSION_ID,
                              tag_base[TempCntTag].src_phy_addr, 
                              tag_base[TempCntTag].Number_BPM,
                              "SPU_LDR_NOPROT",  
                              tag_base[TempCntTag].ADDR_Tag,                        
                              tag_base[TempCntTag].ED_RMA_VRM,
                              tag_base[TempCntTag].DATA_ID,
                              (*(data_tag_sputnic_loader_no_protocol_t*)(&(tag_base[TempCntTag].DATA[0]))).StatusTag,
                              0,0,0,0,0,0,0,0,0,0,
                              tag_base[TempCntTag].vrm[0].PHY_ADDR_VRM,
                              tag_base[TempCntTag].vrm[0].ED_VRM_RMA,                       
                              tag_base[TempCntTag].vrm[1].PHY_ADDR_VRM,
                              tag_base[TempCntTag].vrm[1].ED_VRM_RMA,                       
                              tag_base[TempCntTag].vrm[2].PHY_ADDR_VRM,
                              tag_base[TempCntTag].vrm[2].ED_VRM_RMA); 
          break;          
        case 0x0B:        
          temp_size = sprintf(pcInsert,"<tr><td>%.2d</td><td>%.5d</td><td>%.2d</td><td>%.7s</td><td>%.5d</td><td>%.2d</td><td>%.2d</td><td>0x%.2X</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.3d</td><td>%.5d</td><td>%.2d</td><td>%.5d</td><td>%.2d</td><td>%.5d</td><td>%.2d</td>",
                              tag_base[TempCntTag].VERSION_ID,
                              tag_base[TempCntTag].src_phy_addr, 
                              tag_base[TempCntTag].Number_BPM,
                              "SPU_NOPROT",  
                              tag_base[TempCntTag].ADDR_Tag,                        
                              tag_base[TempCntTag].ED_RMA_VRM,
                              tag_base[TempCntTag].DATA_ID,
                              (*(data_tag_sputnic_no_protocol_t*)(&(tag_base[TempCntTag].DATA[0]))).StatusTag,
                              (*(data_tag_sputnic_no_protocol_t*)(&(tag_base[TempCntTag].DATA[0]))).AverageGazCH4,
                              (*(data_tag_sputnic_no_protocol_t*)(&(tag_base[TempCntTag].DATA[0]))).MaxGazCH4,                              
                              (*(data_tag_sputnic_no_protocol_t*)(&(tag_base[TempCntTag].DATA[0]))).AverageGazO2,
                              (*(data_tag_sputnic_no_protocol_t*)(&(tag_base[TempCntTag].DATA[0]))).MinGazO2, 
                              (*(data_tag_sputnic_no_protocol_t*)(&(tag_base[TempCntTag].DATA[0]))).AverageGazCO,
                              (*(data_tag_sputnic_no_protocol_t*)(&(tag_base[TempCntTag].DATA[0]))).MaxGazCO,
                              (*(data_tag_sputnic_no_protocol_t*)(&(tag_base[TempCntTag].DATA[0]))).AverageGazCO2,
                              (*(data_tag_sputnic_no_protocol_t*)(&(tag_base[TempCntTag].DATA[0]))).MaxGazCO2,
                              0,
                              0,
                              tag_base[TempCntTag].vrm[0].PHY_ADDR_VRM,
                              tag_base[TempCntTag].vrm[0].ED_VRM_RMA,                       
                              tag_base[TempCntTag].vrm[1].PHY_ADDR_VRM,
                              tag_base[TempCntTag].vrm[1].ED_VRM_RMA,                       
                              tag_base[TempCntTag].vrm[2].PHY_ADDR_VRM,
                              tag_base[TempCntTag].vrm[2].ED_VRM_RMA); 
         break;
        case 0x0A:        
          temp_size = sprintf(pcInsert,"<tr><td>%.2d</td><td>%.5d</td><td>%.2d</td><td>%.7s</td><td>%.5d</td><td>%.2d</td><td>%.2d</td><td>0x%.2X</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.3d</td><td>%.5d</td><td>%.2d</td><td>%.5d</td><td>%.2d</td><td>%.5d</td><td>%.2d</td>",
                        tag_base[TempCntTag].VERSION_ID,
                        tag_base[TempCntTag].src_phy_addr, 
                        tag_base[TempCntTag].Number_BPM,
                              "SPUTNIK",  
                        tag_base[TempCntTag].ADDR_Tag,                        
                        tag_base[TempCntTag].ED_RMA_VRM,
                        tag_base[TempCntTag].DATA_ID,
                        (*(data_tag_sputnic_t*)(&(tag_base[TempCntTag].DATA[0]))).StatusTag,
                        (*(data_tag_sputnic_t*)(&(tag_base[TempCntTag].DATA[0]))).GazCH4,
                              0,                              
                        (*(data_tag_sputnic_t*)(&(tag_base[TempCntTag].DATA[0]))).GazO2, 
                              0, 
                        (*(data_tag_sputnic_t*)(&(tag_base[TempCntTag].DATA[0]))).GazCO,
                              0,
                        (*(data_tag_sputnic_t*)(&(tag_base[TempCntTag].DATA[0]))).GazCO2,
                              0,
                        (*(data_tag_sputnic_t*)(&(tag_base[TempCntTag].DATA[0]))).FlagsWorks,
                        (*(data_tag_sputnic_t*)(&(tag_base[TempCntTag].DATA[0]))).ChargeACB,
                        tag_base[TempCntTag].vrm[0].PHY_ADDR_VRM,
                        tag_base[TempCntTag].vrm[0].ED_VRM_RMA,                       
                        tag_base[TempCntTag].vrm[1].PHY_ADDR_VRM,
                        tag_base[TempCntTag].vrm[1].ED_VRM_RMA,                       
                        tag_base[TempCntTag].vrm[2].PHY_ADDR_VRM,
                        tag_base[TempCntTag].vrm[2].ED_VRM_RMA); 
          break;
          
        default:
          temp_size = sprintf(pcInsert,"<tr><td>%.2d</td><td>%.5d</td><td>%.2d</td><td>%.7s</td><td>%.5d</td><td>%.2d</td><td>%.2d</td><td>0x%.2X</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.3d</td><td>%.5d</td><td>%.2d</td><td>%.5d</td><td>%.2d</td><td>%.5d</td><td>%.2d</td>",
                              0,0,0,"RMA-SPU",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0); 
          break;          
        }     
        
        /* Дополнение строки выводом таблицы TOFL */
        for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
        {
          temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>%d</td>", tag_base[TempCntTag].TOFL[cnt_tab] );  
        }
        return  temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"</tr>");  
      }
    }
    /* Печатаем пустое поле */
    pcInsert = "";
    return 0;
  default:
    /* Печатаем пустое поле */
    pcInsert = "";
    return 0;
  }
}

/**
  * @brief  Функция формирования статуса зарегистрированных тегов RMA STM
  * @param  char *pcInsert - указатель на массив pcInsert для формируемой строки
  * @param  enum_type_stage_t stage - режим вызова STAT_TITLE - выдача имени страницы
  * 						   STAT_HEAD - выдача заголовка таблицы
  * 						   STAT_LINE - выдача строки таблицы
  *
  *
  * @retval uint16_t длина формируемой строки
  */
uint16_t TabStatusTagRSN(char *pcInsert, enum_type_stage_t stage)
{
  /* Переменная для отображения типа устройства */
  char type_name[10];
  /* Переменая длинны выводимой строки */
  uint16_t temp_size;   
  
  switch (stage)
  {
  case STAT_TITLE:
    /* Вывод наименования таблицы */
    return  sprintf(pcInsert,"Таблица зарегистрированных тегов");
  case STAT_HEAD:
    /* Вывод шапки таблицы        */
    CntTag = 0;
    temp_size = sprintf(pcInsert,"<tr><td>VerID</td><td>SrcAdr</td><td>NumVrm</td><td>TypeTag</td><td>AdrTag</td><td>ED R->V</td><td>DataID</td><td>ZoneTag</td><td>StatusTag</td><td>ChargeACB</td><td>AdrVRM</td><td>ED V->R</td><td>AdrVRM</td><td>ED V->R</td><td>AdrVRM</td><td>ED V->R</td>");
    /* Дополнение строки выводом наименования таблицы TOFL */
    for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
    {
      temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>%s</td>",  st_core.tag_ch_stat[cnt_tab].ShotNameSrcPortID );  
    }
    return  temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"</tr>");   
  
  case STAT_LINE:
    /* Вывод строк таблицы        */
    while( (tag_base[CntTag].ADDR_Tag > 0) && ( CntTag < MAX_NUM_TAG ) ) 
    { /* Сохраняем индекс записи       */
      TempCntTag = CntTag;
      /* Инкрементируем счетчик записи */               
      CntTag++;
      /* Вывод только тегов на  RMA RSN */
      if ( ( tag_base[TempCntTag].TypeTag == 112 ) || ( tag_base[TempCntTag].TypeTag == 113 ) || ( tag_base[TempCntTag].TypeTag == 114 ) || ( tag_base[TempCntTag].TypeTag == 115 ) )
      {
        
        switch (tag_base[TempCntTag].TypeTag)
        {
        case RF_TYPE_ABON_NO:                      
          sprintf(type_name,"");
          break;
        case RF_TYPE_ABON_RMA:                     
          sprintf(type_name,"    RMA");    
          break; 
        case RF_TYPE_ABON_RMAT:                    
          sprintf(type_name,"  RMA-T");    
          break;
        case RF_TYPE_ABON_RSN:                     
          sprintf(type_name,"    RSN");    
          break;
        case RF_TYPE_ABON_RSN_SE2435L:             
          sprintf(type_name,"  RSN-U");    
          break;        
        case RF_TYPE_ABON_RSNP:                    
          sprintf(type_name,"  RSN-P");    
          break;
        case RF_TYPE_ABON_RMA_METHANE_SPUTNIK_pv3: 
          sprintf(type_name,"RMA-SPU");    
          break;       
        case RF_TYPE_ABON_RMA_MEMS_pv3:            
          sprintf(type_name,"RMA-STM");    
          break;        
        case RF_TYPE_ABON_RSN_pv3:                 
          sprintf(type_name,"    RSN");    
          break;
        case RF_TYPE_ABON_RSN_SE2435L_pv3:         
          sprintf(type_name,"  RSN-U");    
          break; 
        case RF_TYPE_ABON_RSS_SE2435L_pv3:         
          sprintf(type_name,"  RSS-U");    
          break;
        case RF_TYPE_ABON_RSNP_pv3:                
          sprintf(type_name,"  RSN-P");    
          break;
        default:                                   
          sprintf(type_name,"");           
          break;
        };      
        
        temp_size = sprintf(pcInsert,"<tr><td>%.2d</td><td>%.5d</td><td>%.2d</td><td>%.7s</td><td>%.5d</td><td>%.2d</td><td>%.2d</td><td>0x%.2X</td><td>%.3d</td><td>%.3d</td><td>%.5d</td><td>%.2d</td><td>%.5d</td><td>%.2d</td><td>%.5d</td><td>%.2d</td>",
                        tag_base[TempCntTag].VERSION_ID,
                        tag_base[TempCntTag].src_phy_addr, 
                        tag_base[TempCntTag].Number_BPM,
                        type_name,  
                        tag_base[TempCntTag].ADDR_Tag,                        
                        tag_base[TempCntTag].ED_RMA_VRM,
                        tag_base[TempCntTag].DATA_ID,
                        (*(data_tag_rsn_t*)(&(tag_base[TempCntTag].DATA[0]))).ZoneTag, 
                        (*(data_tag_rsn_t*)(&(tag_base[TempCntTag].DATA[0]))).StatusTag,
                        (*(data_tag_rsn_t*)(&(tag_base[TempCntTag].DATA[0]))).ChargeACB,                        
                        tag_base[TempCntTag].vrm[0].PHY_ADDR_VRM,
                        tag_base[TempCntTag].vrm[0].ED_VRM_RMA,                       
                        tag_base[TempCntTag].vrm[1].PHY_ADDR_VRM,
                        tag_base[TempCntTag].vrm[1].ED_VRM_RMA,                       
                        tag_base[TempCntTag].vrm[2].PHY_ADDR_VRM,
                        tag_base[TempCntTag].vrm[2].ED_VRM_RMA); 
        
        /* Дополнение строки выводом таблицы TOFL */
        for ( uint8_t cnt_tab = 0; cnt_tab < MAX_TAG_CH_TAB;cnt_tab++ )
        {
          temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>%d</td>", tag_base[TempCntTag].TOFL[cnt_tab] );  
        }
        return  temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"</tr>");   
      }
    }
    /* Печатаем пустое поле */
    pcInsert = "";
    return 0;
  default:
    /* Печатаем пустое поле */
    pcInsert = "";
    return 0;
  }
}

/**
  * @brief Функция регистрации тега из короткого пакета RMA - спутник загрузчик программа в таблице 
  * @param RSFrameShotTagNewPosV3_t *tag_box - указатель на пакет позиционирования
  * @param uint16_t data_size - размер поля даннх тега
  * @param uint8_t src_port_ID - порт ID источника
  * @retval uint8_t - размер обработанных данных
  */
uint8_t RegShotTagNewPosV3( RSFrameShotTagNewPosV3_t *tag_box, uint16_t data_size, uint8_t src_port_ID )
{
  /* Объявляем переменную для индекса */
  uint16_t index_teg = 0;
  /* Переменная статуса записи в таблицу */
  uint8_t stat_record = 0;  
  
  /* Запрос индекса таблицы тегов     */
  stat_record = GetIndexTagTable( tag_box->frame_tag_pos.header.macSrcAddr, &index_teg);
    
  if ( stat_record > 0 )
  {/* Получен индекс для записи в таблицу */
    /* Функция анализа режима обновления времени и регистрации тега */
    if (UpdateTagTimeReg( DataLoaderSto.Settings.phy_adr, tag_box->frame_tag_pos.ed_vrm[0].PHY_ADDR_VRM, index_teg, stat_record, src_port_ID ))
      {
    /* Заполнение таблицы тегов */
    tag_base[index_teg].VERSION_ID = 3;		                             /*	   Версия пакета                      */
      tag_base[index_teg].src_phy_addr = DataLoaderSto.Settings.phy_adr;                       /*	   Физический адрес источника         */
    
    tag_base[index_teg].TypeTag = tag_box->frame_tag_pos.Type_Tag;	     /*	   Тип позиционируемого тега          */
    tag_base[index_teg].ADDR_Tag = tag_box->frame_tag_pos.header.macSrcAddr; /*    Адрес позиционируемого тега        */
    
    tag_base[index_teg].ED_RMA_VRM = ((FrameShotTail_t*)((uint32_t)(tag_box->frame_tag_pos.tag_data) + (uint32_t)data_size))->ed;   /*	   Уровень сигнала от тега к ВРМ      */               
    tag_base[index_teg].DATA_ID = tag_box->frame_tag_pos.SubID;              /*	   ID данных пакета позиционирования  */         
            
    tag_base[index_teg].Number_BPM = 3;		                               /*	     Число лучших ВРМ         */
    tag_base[index_teg].vrm[0].ED_VRM_RMA = tag_box->frame_tag_pos.ed_vrm[0].ED_VRM_RMA;     /*      Поле данных лучших ВРМ         */
    tag_base[index_teg].vrm[0].PHY_ADDR_VRM = tag_box->frame_tag_pos.ed_vrm[0].PHY_ADDR_VRM;    
    tag_base[index_teg].vrm[0].MAC_ADDR_VRM = tag_box->frame_tag_pos.ed_vrm[0].MAC_ADDR_VRM; 
    tag_base[index_teg].vrm[1].ED_VRM_RMA = tag_box->frame_tag_pos.ed_vrm[1].ED_VRM_RMA;       
    tag_base[index_teg].vrm[1].PHY_ADDR_VRM = tag_box->frame_tag_pos.ed_vrm[1].PHY_ADDR_VRM; 
    tag_base[index_teg].vrm[1].MAC_ADDR_VRM = tag_box->frame_tag_pos.ed_vrm[1].MAC_ADDR_VRM; 
    tag_base[index_teg].vrm[2].ED_VRM_RMA = tag_box->frame_tag_pos.ed_vrm[2].ED_VRM_RMA;         
    tag_base[index_teg].vrm[2].PHY_ADDR_VRM = tag_box->frame_tag_pos.ed_vrm[2].PHY_ADDR_VRM;     
    tag_base[index_teg].vrm[2].MAC_ADDR_VRM = tag_box->frame_tag_pos.ed_vrm[2].MAC_ADDR_VRM;     
    
    /* Заполнение поля данных */                                             
    for ( uint8_t contik_data_tag = 0 ; contik_data_tag < MAX_SIZE_DATA_POZ; contik_data_tag++ ) 
    {
      if (contik_data_tag < data_size )
      {
        tag_base[index_teg].DATA[contik_data_tag] = tag_box->frame_tag_pos.tag_data[contik_data_tag];
      }
      else
      {
        tag_base[index_teg].DATA[contik_data_tag] = 0; 
      }  
    }
    }
    /* Обработан 1 тег */
    return 1;
  }
  return 0;
}

/**
  * @brief Функция регистрации тега при получении короткого пакета по RF в общей таблице 
  * @param router_box_t* rs_box - указатель на пакета позиционирования  
  * @retval uint8_t - размер обработанных данных
  */
uint8_t RegShotTable( router_box_t* rs_box )
{
  /* Объявляем указатель на шапку пакета */
  RSFrameShotTagNewPosV3_t  *head_tag = (RSFrameShotTagNewPosV3_t*)rs_box;
   
  switch ( head_tag->frame_tag_pos.Type_Tag )
  {
  case RF_TYPE_ABON_RMA_METHANE_SPUTNIK_pv3:
    /* RMA - спутник */
    switch ( head_tag->frame_tag_pos.SubID )
    {
    case 0x01:
      /* Загрузчик */
      return RegShotTagNewPosV3( head_tag, sizeof(data_tag_sputnic_loader_t), rs_box->SrcPortID );      
      break;
    case 0x0A:
      /* Основная программа */      
      return RegShotTagNewPosV3( head_tag, sizeof(data_tag_sputnic_t), rs_box->SrcPortID );
      break;
    default:
      /* Oбработанных данных нет */
      return 0;
    }
    break;
  case RF_TYPE_ABON_RMA_MEMS_pv3:
    /* RMA - STM */
    switch ( head_tag->frame_tag_pos.SubID )
    {
    case 0x01:
      /* Загрузчик */ 
      return RegShotTagNewPosV3( head_tag, sizeof(data_tag_rma_stm_loader_t), rs_box->SrcPortID );  
      break;
    case 0x0A:
      /* Основная программа */ 
      return RegShotTagNewPosV3( head_tag, sizeof(data_tag_rma_stm_t), rs_box->SrcPortID );  
      break;
    default:
      /* Oбработанных данных нет */
      return 0;
    }
    break;
  case RF_TYPE_ABON_RSN_pv3:
  case RF_TYPE_ABON_RSN_SE2435L_pv3:    
  case RF_TYPE_ABON_RSS_SE2435L_pv3:      
  case RF_TYPE_ABON_RSNP_pv3:      
    /* PCH-У/PCH/PCC/PCН-П */
    switch ( head_tag->frame_tag_pos.SubID )
    {
    case 0x01:
      /* Загрузчик */ 
      return RegShotTagNewPosV3( head_tag, sizeof(data_tag_rsn_loader_t), rs_box->SrcPortID );  
      break;
    case 0x0A:
      /* Основная программа */ 
      return RegShotTagNewPosV3( head_tag, sizeof(data_tag_rsn_t), rs_box->SrcPortID );  
      break;
    default:
      /* Oбработанных данных нет */
      return 0;
    }
    break;
  case RF_TYPE_ABON_RMA:
  case RF_TYPE_ABON_RMAT:
  case RF_TYPE_ABON_RSN:    
  case RF_TYPE_ABON_RSN_SE2435L: 
  case RF_TYPE_ABON_RSNP:    
    /* Oбработанных даных нет */
    return 0;   
    break;
  default:
    /* Oбработанных даных нет */
    return 0;
  }
}

/**
  * @brief Функция парсинга пакета позиционирования 
  * @param router_box_t* rs_box - указатель на пакета позиционирования  
  *
  * @retval bool true  - пакет обработан
  *              false - пакет не обработан
  */
bool ProcessingNewPositionBox( router_box_t* rs_box )
{
  /* Объявляем указатель на шапку пакета */
  RSFrameTagNewPosV3_t  *frame_tag = (RSFrameTagNewPosV3_t*)rs_box;
  
  uint8_t tag_offset = 0;
  uint8_t data_tag = 0;
  uint8_t temp_size_tag;
  
  switch ( frame_tag->rs_head.id )
  {
  case ID_NEW_POSITION:
    /* Новый формат позиционирования */
    switch ( frame_tag->frame_tag_pos.VERSION_ID )
    {
    case 2:
      /* вызов функции парсинга пакета позиционировани */
      return true;
    case 3:
      /* Определяем область данных тега */
      data_tag = frame_tag->frame_tag_pos.size - sizeof(NwkFrameTagNewPosV3_t);  
      /* Инициализация смещения */
      tag_offset = 0;
      /*  */
      while( tag_offset < data_tag )
      {
        /* Парсинг тега */
        temp_size_tag = ProcessingNewPositionBoxV3( &(frame_tag->frame_tag_pos.data_tag[tag_offset]), frame_tag->frame_tag_pos.Number_BPM, frame_tag->frame_tag_pos.own_physical_addr, rs_box->SrcPortID );
        
        /* ошибка в парсинге - выход */
        if ( temp_size_tag == 255 ) return true;
        
        /* Увеличение переменной смещения на размер обработанного тега */
        tag_offset = tag_offset + temp_size_tag;
        /* Функция регистрации тега в таблице  */
      }
      return true;
    default:
      /* Oбработанных даных нет */
      return true;    
    } 
  case ID_POSITION:
    /* Старый формат позиционирования/Или данные о позиционировании с приемопередатчика  */  
    RegShotTable( rs_box );
    
    /* Пакет обработан */
    return true;
  default:
    /* Oбработанных даных нет */
    return false;
  }
}

#if (UDP_RX_POS_DEBUG==1)
/**
  * @brief Функция для формирования диагностического сообщения по рассылке пакета позиционирования.
  * @param const char *header_mess - шапка сообщения 
  * @param uint16_t addr_src - адрес источника врм 
  * @param uint8_t type_tag - тип позиционируемого тега   
  * @param uint16_t addr_tag - адрес позиционируемого тега 
  * @param uint16_t addr_phy - адрес наилучшего врм 
  * @retval None
  */
void diag_reg_rs_pos( const char *header_mess, uint16_t addr_src, uint8_t type_tag, uint16_t addr_tag, uint16_t addr_phy )
{
  printf("|%.16s|0x%.4X|0x%.2X|0x%.4X|0x%.4X|\r\n",
         header_mess,
         addr_src,
         type_tag,  /* Тип позиционируемого тега                         */
         addr_tag,  /* Адрес позиционируемого тега                       */
         addr_phy);  
}
#endif

#if (UDP_TX_POS_DEBUG==1)
/**
  * @brief Функция для формирования диагностического сообщения по рассылке пакета позиционирования.
  * @param const char *header_mess - шапка сообщения 
  * @param uint8_t type_tag - тип позиционируемого тега   
  * @param uint16_t addr_tag - адрес позиционируемого тега 
  * @param uint16_t addr_phy - адрес наилучшего врм 
  * @retval None
  */
void diag_tx_mes_pos( const char *header_mess, uint8_t type_tag, uint16_t addr_tag, uint16_t addr_phy )
{
  printf("|%.16s|0x%.2X|0x%.4X|0x%.4X|\r\n",
         header_mess,
         type_tag, /* Тип позиционируемого тега                         */
         addr_tag, /* Адрес позиционируемого тега                       */
         addr_phy);  
}
#endif

/******************* (C) COPYRIGHT 2019 DataExpress *****END OF FILE****/
