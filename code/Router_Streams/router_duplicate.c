/**
  ******************************************************************************
  * @file    router_duplicate.с
  * @author  Trembach D.N.
  * @version V1.0.0
  * @date    30-03-2019
  * @brief   Файл содержит функции удаления дублированных пакетов
  ******************************************************************************
  * @attention
  *  Дубликатор вызываем периодически 10 раз в секунду для обновления времени жизни
  *  Дубликатор обрабатывает только типы пакетов указанные в задданой таблице 
  *  Время хранения дубликатора не более заданного.
  *  Получаем пакет проверяем его ID на совпадение с таблицей ID 
  *  Если такой ID есть - проверяем наличиет такого пакета в таблице
  *  Если такой пакет есть отбрасываем, если нет - записываем в таблицу 
  *  Время жизни записи в таблице не должно превышать 7 сек 
  ******************************************************************************
  */ 


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "router_streams.h"
#include "router_duplicate.h"
#include "rf_frame_header.h"


#define DUPLICATE_TABLE_SIZE     (1000)
#define DUPLICATE_TIME           (7000)
    
// Таблица хранения меток пакета для поиска дубликатов    
Duplicate_t duplicate_table[DUPLICATE_TABLE_SIZE];
// Переменная хранения индекса на первую пустую метку 
static uint16_t MaxIndexDublicateTab = 0;


/**
  * @brief  Функция обновления времени жизни меток фреймов
  * @param  uint16_t time_refresh - время обновления
  * @retval None
  *
  */
void Refresh_Duplicate_Table(uint16_t time_refresh)
{
  // Проверка корректности указателя
  if (MaxIndexDublicateTab > DUPLICATE_TABLE_SIZE) 
  {
    // Обнуление указателя
    MaxIndexDublicateTab = 0;
  }
  else
  {
    // Eсли есть записи в таблице 
    if (MaxIndexDublicateTab > 0)
    {
      // Eсли указатель на последнюю метку в таблице больше 1 
      if (MaxIndexDublicateTab > 1)
      {
        //Цикл по элементам таблицы меток
        for (uint16_t cnt_index_dupl = (MaxIndexDublicateTab - 1); cnt_index_dupl > 0 ; cnt_index_dupl--)
        {
          // Проверка корректности времени жизни
          if (((duplicate_table[cnt_index_dupl].time) < DUPLICATE_TIME) && ((duplicate_table[cnt_index_dupl].time) > time_refresh)) 
          {
            // Уменьшаем время жизни
            duplicate_table[cnt_index_dupl].time =  duplicate_table[cnt_index_dupl].time - time_refresh;           
          }
          else
          {
            // Удаляем метку пакета
            if ((cnt_index_dupl + 1) <  MaxIndexDublicateTab)
            {
              // Это была не последняя метка пакета в таблице 
              // На ее место переносим последнюю метку
              duplicate_table[cnt_index_dupl].factory_addr = duplicate_table[MaxIndexDublicateTab - 1].factory_addr; 
              duplicate_table[cnt_index_dupl].id = duplicate_table[MaxIndexDublicateTab - 1].id;
              duplicate_table[cnt_index_dupl].seq = duplicate_table[MaxIndexDublicateTab - 1].seq;
              duplicate_table[cnt_index_dupl].time = duplicate_table[MaxIndexDublicateTab - 1].time;  
            } 
            // Уменьшаем индекс на первую пустую метку
            MaxIndexDublicateTab--;   
          }
        }
      }
      // Цикл не проверяет нулевой индекс - проверим его     
      // Проверка корректности времени жизни
      if (((duplicate_table[0].time) < DUPLICATE_TIME) && ((duplicate_table[0].time) > time_refresh)) 
      {
        // Уменьшаем время жизни
        duplicate_table[0].time = duplicate_table[0].time - time_refresh;           
      }
      else
      {
        // Удаляем метку пакета
        if (MaxIndexDublicateTab > 1)
        {
          // Это была не последняя метка пакета в таблице 
          // На ее место переносим последнюю метку
          duplicate_table[0].factory_addr = duplicate_table[MaxIndexDublicateTab - 1].factory_addr; 
          duplicate_table[0].id = duplicate_table[MaxIndexDublicateTab - 1].id;
          duplicate_table[0].seq = duplicate_table[MaxIndexDublicateTab - 1].seq;
          duplicate_table[0].time = duplicate_table[MaxIndexDublicateTab - 1].time;  
        } 
        // Уменьшаем индекс на первую пустую метку
        MaxIndexDublicateTab--;   
      }
    }   
  }
}  

/**
  * @brief  Функция определение является ли фрейм дублированным 
  * @param  router_box_t* router_box - указатель на тестируемый пакет
  * @retval bool true  - фрейм дублирован
  *              false - фрейм не дублирован 
  */
bool ControlDuplicateFrame(router_box_t* router_box)
{
  // Функция проверки разрешения трансляции в контроля дубликатов по ID пакета  
  if (permit_index_box(router_box->id, MaskBoxDublicate))
  { 
    if (MaxIndexDublicateTab  > 0)
    {
      // Поиск в таблице метки дубликата
      for (uint16_t cnt_index_dupl = 0; cnt_index_dupl < MaxIndexDublicateTab; cnt_index_dupl++ )
      {
        // Проверка совпадения метки
        if ((duplicate_table[cnt_index_dupl].factory_addr == router_box->NwkFrame.header_t.nwkSrcAddr) &&\
            (duplicate_table[cnt_index_dupl].id == router_box->NwkFrame.header_t.id1) &&\
            (duplicate_table[cnt_index_dupl].seq == router_box->NwkFrame.header_t.nwkSeq))
        {
          // пакет дублирован
          duplicate_table[cnt_index_dupl].time = DUPLICATE_TIME;          
          return true;
        }
      }
    }
    // Проверка корректности указателя
    if (MaxIndexDublicateTab < DUPLICATE_TABLE_SIZE) 
    {
      // Запись метки в таблицу дубликатов
      duplicate_table[MaxIndexDublicateTab].factory_addr = router_box->NwkFrame.header_t.nwkSrcAddr; 
      duplicate_table[MaxIndexDublicateTab].id = router_box->NwkFrame.header_t.id1;
      duplicate_table[MaxIndexDublicateTab].seq = router_box->NwkFrame.header_t.nwkSeq;
      duplicate_table[MaxIndexDublicateTab].time = DUPLICATE_TIME;  
      // Увеличиваем индекс на первую пустую метку
      MaxIndexDublicateTab++;
    }
  }
  // пакет не дублирован
  return false;
}  
/************************ (C) COPYRIGHT DEX 2019 *****END OF FILE**************/
