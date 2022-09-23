/**
  ******************************************************************************
  * @file    core_cntrl_settings.с
  * @author  Trembach Dmitry
  * @version V1.1.0
  * @date    18-04-2019
  * @brief   файл с функциями ядра для контроля настроек
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
#include "core_cntrl_settings.h"
#include "config_var.h"
#include "loader_settings.h"
#include "EEPROM_Flash.h" 
#if (RS_GATE_ENABLE == 1)
#include "pre_set_rs.h" 
#endif
// Индекс переменной конфигурирования
uint16_t index_var;

//==============
uint16_t  cnt_update_time_test;         // счетчик таймера обновления теста
#define MAX_UPDATE_TIME_TEST (2000)     // период таймера обновления теста
uint8_t contic_test;                    // Счетчик неприрырности теста
//==============

/**
  * @brief Функция поиска переменной по заданному идентификатору
  * @param  uint32_t   id_var;  //идентификатор переменной  (Type<<24(Тип ресурса),N_port<<16(Номер порта), SETUP_ID<<8(Идентификатор настроеки, Num(номер параметра внутри переменной)))
  * @param  uint16_t*  index_var указатель на индекс найденой переменной
  * @retval bool   true  - переменная найдена
  *                false - переменная не найдена
  */
bool GetIndexVar( uint32_t id_var , uint16_t *index_var )
{
  uint16_t cnt_index_var;
  // Начинаем с нулевого индекса
  cnt_index_var = 0;
  // Цикл по всем заявленым переменным  
  while(SetVarArray[cnt_index_var].number_section != STOP_SECT) 
  {
    if (SetVarArray[cnt_index_var].id_var == id_var)
    {
      // Есть совпадение
      *index_var = cnt_index_var;
      // Переменная найдена
      return true;
    }
    cnt_index_var++;
  }
  //Переменная не найдена
  return false;
}

/**
  * @brief  Функция обработки конфигурировани переменной длинной 1 байт
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  * @param  setup_prm_t* ReqConfig - указатель на буфер запроса конфигурации
  * @param  setup_prm_t* RespConfig - указатель на буфер для формирования ответа на запрос конфигурации
  * @param  uint16_t index_var - индекс найденой переменной
  * @param  uint8_t index_resp_buf - индекс буфера данных по переменной
  * @retval uint8_t длинна записи переменной 
  */
uint8_t resp_ui8( core_struct_t* core_st,  setup_prm_t* ReqConfig, setup_prm_t* RespConfig, uint16_t index_var, uint8_t index_resp_buf )
{
  // Проверка операции чтение или запись
  if ( (ReqConfig->setup_head.SETUP_ID & 0x01 ) == 0x01)
  {
    // Записать значение в оперативную память 
    *((uint8_t*)SetVarArray[index_var].new_var) = ReqConfig->wr_req_ui8[index_resp_buf];
    
    //================Сформировать ответ о результатах сохранения===============
    
    // Загрузка значений   
    RespConfig->wr_resp_ui8[index_resp_buf].rd_param_rom = *((uint8_t*)SetVarArray[index_var].store_var);         
    RespConfig->wr_resp_ui8[index_resp_buf].rd_param_ram = *((uint8_t*)SetVarArray[index_var].new_var);
    
    // Проверяем корректности записи в оперативную память                                                            
    if ((RespConfig->wr_resp_ui8[index_resp_buf].rd_param_ram) == (ReqConfig->wr_req_ui8[index_resp_buf]))
    {
      RespConfig->wr_resp_ui8[index_resp_buf].status_wr_rom = 0x01;
    }
    else
    {
      RespConfig->wr_resp_ui8[index_resp_buf].status_wr_rom = 0x00;     
    } 
    
    // Проверяем соответствие содержимого флеш и настроек                                                            
    if ((RespConfig->wr_resp_ui8[index_resp_buf].rd_param_rom) == (*((uint8_t*)SetVarArray[index_var].flash_var)))
    {
      RespConfig->wr_resp_ui8[index_resp_buf].status_rd_rom = 0x01;
    }
    else
    {
      RespConfig->wr_resp_ui8[index_resp_buf].status_rd_rom = 0x00;     
    }    
    // Проверяем соответствие содержимого флеш и записи в оперативную память                                                            
    if ((RespConfig->wr_resp_ui8[index_resp_buf].rd_param_ram) == (*((uint8_t*)SetVarArray[index_var].flash_var)))
    {
      RespConfig->wr_resp_ui8[index_resp_buf].status_rd_ram = 0x01;
    }
    else
    {
      RespConfig->wr_resp_ui8[index_resp_buf].status_rd_ram = 0x00;     
    }           
    // Установка размера ответа 
    return  sizeof(wr_resp_ui8_box_t);
  }
  else
  {
    // Формирование ответа 
    // Загрузка значений   
    RespConfig->rd_resp_ui8[index_resp_buf].rd_param_rom = *((uint8_t*)SetVarArray[index_var].store_var);         
    RespConfig->rd_resp_ui8[index_resp_buf].rd_param_ram = *((uint8_t*)SetVarArray[index_var].new_var);
    
    // Проверяем соответствие содержимого флеш и настроек                                                            
    if ((RespConfig->rd_resp_ui8[index_resp_buf].rd_param_rom) == (*((uint8_t*)SetVarArray[index_var].flash_var)))
    {
      RespConfig->rd_resp_ui8[index_resp_buf].status_rd_rom = 0x01;
    }
    else
    {
      RespConfig->rd_resp_ui8[index_resp_buf].status_rd_rom = 0x00;     
    }    
    // Проверяем соответствие содержимого флеш и записи в оперативную память                                                            
    if ((RespConfig->rd_resp_ui8[index_resp_buf].rd_param_ram) == (*((uint8_t*)SetVarArray[index_var].flash_var)))
    {
      RespConfig->rd_resp_ui8[index_resp_buf].status_rd_ram = 0x01;
    }
    else
    {
      RespConfig->rd_resp_ui8[index_resp_buf].status_rd_ram = 0x00;     
    }           
    // Установка размера ответа 
    return  sizeof(rd_resp_ui8_box_t);
  }  
}

/**
  * @brief  Функция обработки конфигурировани переменной длиной 2 байта
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  * @param  setup_prm_t* ReqConfig - указатель на буфер запроса конфигурации
  * @param  setup_prm_t* RespConfig - указатель на буфер для формирования ответа на запрос конфигурации
  * @param  uint16_t index_var - индекс найденой переменной
  * @param  uint8_t index_resp_buf - индекс буфера данных по переменной
  * @retval uint8_t длинна записи переменной 
  */
uint8_t resp_ui16( core_struct_t* core_st,  setup_prm_t* ReqConfig, setup_prm_t* RespConfig, uint16_t index_var , uint8_t index_resp_buf )
{
  // Проверка операции чтение или запись
  if ( (ReqConfig->setup_head.SETUP_ID & 0x01 ) == 0x01)
  {
    // Записать значение в оперативную память 
    *((uint16_t*)SetVarArray[index_var].new_var) = ReqConfig->wr_req_ui16[index_resp_buf];
    
    // Формирование ответа 
    
    // Загрузка значений   
    RespConfig->wr_resp_ui16[index_resp_buf].rd_param_rom = *((uint16_t*)SetVarArray[index_var].store_var);         
    RespConfig->wr_resp_ui16[index_resp_buf].rd_param_ram = *((uint16_t*)SetVarArray[index_var].new_var);
    
    // Проверяем корректности записи в оперативную память                                                            
    if ((RespConfig->wr_resp_ui16[index_resp_buf].rd_param_ram) == (ReqConfig->wr_req_ui16[index_resp_buf]))
    {
      RespConfig->wr_resp_ui16[index_resp_buf].status_wr_rom = 0x01;
    }
    else
    {
      RespConfig->wr_resp_ui16[index_resp_buf].status_wr_rom = 0x00;     
    } 
    
    // Проверяем соответствие содержимого флеш и настроек                                                            
    if ((RespConfig->wr_resp_ui16[index_resp_buf].rd_param_rom) == (*((uint16_t*)SetVarArray[index_var].flash_var)))
    {
      RespConfig->wr_resp_ui16[index_resp_buf].status_rd_rom = 0x01;
    }
    else
    {
      RespConfig->wr_resp_ui16[index_resp_buf].status_rd_rom = 0x00;     
    }    
    // Проверяем соответствие содержимого флеш и записи в оперативную память                                                            
    if ((RespConfig->wr_resp_ui16[index_resp_buf].rd_param_ram) == (*((uint16_t*)SetVarArray[index_var].flash_var)))
    {
      RespConfig->wr_resp_ui16[index_resp_buf].status_rd_ram = 0x01;
    }
    else
    {
      RespConfig->wr_resp_ui16[index_resp_buf].status_rd_ram = 0x00;     
    }           
    // Установка размера ответа 
    return  sizeof(wr_resp_ui16_box_t);
  }
  else
  {
    // Формирование ответа 
    // Загрузка значений   
    RespConfig->rd_resp_ui16[index_resp_buf].rd_param_rom = *((uint16_t*)SetVarArray[index_var].store_var);         
    RespConfig->rd_resp_ui16[index_resp_buf].rd_param_ram = *((uint16_t*)SetVarArray[index_var].new_var);
    
    // Проверяем соответствие содержимого флеш и настроек                                                            
    if ((RespConfig->rd_resp_ui16[index_resp_buf].rd_param_rom) == (*((uint16_t*)SetVarArray[index_var].flash_var)))
    {
      RespConfig->rd_resp_ui16[index_resp_buf].status_rd_rom = 0x01;
    }
    else
    {
      RespConfig->rd_resp_ui16[index_resp_buf].status_rd_rom = 0x00;     
    }    
    // Проверяем соответствие содержимого флеш и записи в оперативную память                                                            
    if ((RespConfig->rd_resp_ui16[index_resp_buf].rd_param_ram) == (*((uint16_t*)SetVarArray[index_var].flash_var)))
    {
      RespConfig->rd_resp_ui16[index_resp_buf].status_rd_ram = 0x01;
    }
    else
    {
      RespConfig->rd_resp_ui16[index_resp_buf].status_rd_ram = 0x00;     
    }           
    // Установка размера ответа 
    return  sizeof(rd_resp_ui16_box_t);
  }  
}
/**
  * @brief  Функция обработки конфигурирования переменной длиной 4 байта
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  * @param  setup_prm_t* ReqConfig - указатель на буфер запроса конфигурации
  * @param  setup_prm_t* RespConfig - указатель на буфер для формирования ответа на запрос конфигурации
  * @param  uint16_t index_var - индекс найденой переменной
  * @param  uint8_t index_resp_buf - индекс буфера данных по переменной
  * @retval uint8_t длинна записи переменной 
  */
uint8_t resp_ui32( core_struct_t* core_st,  setup_prm_t* ReqConfig, setup_prm_t* RespConfig, uint16_t index_var , uint8_t index_resp_buf )
{
  // Проверка операции чтение или запись
  if ( (ReqConfig->setup_head.SETUP_ID & 0x01 ) == 0x01)
  {
    // Записать значение в оперативную память 
    *((uint32_t*)SetVarArray[index_var].new_var) = ReqConfig->wr_req_ui32[index_resp_buf];
    
    // Формирование ответа 
    
    // Загрузка значений   
    RespConfig->wr_resp_ui32[index_resp_buf].rd_param_rom = *((uint32_t*)SetVarArray[index_var].store_var);         
    RespConfig->wr_resp_ui32[index_resp_buf].rd_param_ram = *((uint32_t*)SetVarArray[index_var].new_var);
    
    // Проверяем корректности записи в оперативную память                                                            
    if ((RespConfig->wr_resp_ui32[index_resp_buf].rd_param_ram) == (ReqConfig->wr_req_ui32[index_resp_buf]))
    {
      RespConfig->wr_resp_ui32[index_resp_buf].status_wr_rom = 0x01;
    }
    else
    {
      RespConfig->wr_resp_ui32[index_resp_buf].status_wr_rom = 0x00;     
    } 
    
    // Проверяем соответствие содержимого флеш и настроек                                                            
    if ((RespConfig->wr_resp_ui32[index_resp_buf].rd_param_rom) == (*((uint32_t*)SetVarArray[index_var].flash_var)))
    {
      RespConfig->wr_resp_ui32[index_resp_buf].status_rd_rom = 0x01;
    }
    else
    {
      RespConfig->wr_resp_ui32[index_resp_buf].status_rd_rom = 0x00;     
    }    
    // Проверяем соответствие содержимого флеш и записи в оперативную память                                                            
    if ((RespConfig->wr_resp_ui32[index_resp_buf].rd_param_ram) == (*((uint32_t*)SetVarArray[index_var].flash_var)))
    {
      RespConfig->wr_resp_ui32[index_resp_buf].status_rd_ram = 0x01;
    }
    else
    {
      RespConfig->wr_resp_ui32[index_resp_buf].status_rd_ram = 0x00;     
    }           
    // Установка размера ответа 
    return  sizeof(wr_resp_ui32_box_t);
  }
  else
  {
    // Формирование ответа 
    // Загрузка значений   
    RespConfig->rd_resp_ui32[index_resp_buf].rd_param_rom = *((uint32_t*)SetVarArray[index_var].store_var);         
    RespConfig->rd_resp_ui32[index_resp_buf].rd_param_ram = *((uint32_t*)SetVarArray[index_var].new_var);
    
    // Проверяем соответствие содержимого флеш и настроек                                                            
    if ((RespConfig->rd_resp_ui32[index_resp_buf].rd_param_rom) == (*((uint32_t*)SetVarArray[index_var].flash_var)))
    {
      RespConfig->rd_resp_ui32[index_resp_buf].status_rd_rom = 0x01;
    }
    else
    {
      RespConfig->rd_resp_ui32[index_resp_buf].status_rd_rom = 0x00;     
    }    
    // Проверяем соответствие содержимого флеш и записи в оперативную память                                                            
    if ((RespConfig->rd_resp_ui32[index_resp_buf].rd_param_ram) == (*((uint32_t*)SetVarArray[index_var].flash_var)))
    {
      RespConfig->rd_resp_ui32[index_resp_buf].status_rd_ram = 0x01;
    }
    else
    {
      RespConfig->rd_resp_ui32[index_resp_buf].status_rd_ram = 0x00;     
    }           
    // Установка размера ответа 
    return  sizeof(rd_resp_ui32_box_t);
  }  
}

/**
  * @brief  Функция обработки конфигурирования переменной длиной mask_type
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  * @param  setup_prm_t* ReqConfig - указатель на буфер запроса конфигурации
  * @param  setup_prm_t* RespConfig - указатель на буфер для формирования ответа на запрос конфигурации
  * @param  uint16_t index_var - индекс найденой переменной
  * @param  uint8_t index_resp_buf - индекс буфера данных по переменной
  * @retval uint8_t длинна записи переменной 
  */
uint8_t resp_mask_type( core_struct_t* core_st,  setup_prm_t* ReqConfig, setup_prm_t* RespConfig, uint16_t index_var , uint8_t index_resp_buf )
{
  // Переменна длинны ответа
  uint8_t data_lenght_resp;
  // Обнуляем переменную длинны ответа
  data_lenght_resp = 0;
  // Проверка операции чтение или запись
  if ( (ReqConfig->setup_head.SETUP_ID & 0x01 ) == 0x01)
  {
    // Отработка записи
    for (uint8_t cntik_index = 0; cntik_index < 16; cntik_index++)
    {  
      // Записать значение в оперативную память 
      ((mask_box_id_t*)SetVarArray[index_var].new_var)->mask_id[cntik_index] = ReqConfig->wr_req_ui16[cntik_index];
      //================Сформировать ответ о результатах сохранения===============
      // Загрузка значений   
      RespConfig->wr_resp_ui16[cntik_index].rd_param_rom = ((mask_box_id_t*)SetVarArray[index_var].store_var)->mask_id[cntik_index];         
      RespConfig->wr_resp_ui16[cntik_index].rd_param_ram = ((mask_box_id_t*)SetVarArray[index_var].new_var)->mask_id[cntik_index];
      
      // Проверяем корректности записи в оперативную память                                                            
      if ((RespConfig->wr_resp_ui16[cntik_index].rd_param_ram) == (ReqConfig->wr_req_ui16[cntik_index]))
      {
        RespConfig->wr_resp_ui16[cntik_index].status_wr_rom = 0x01;
      }
      else
      {
        RespConfig->wr_resp_ui16[cntik_index].status_wr_rom = 0x00;     
      } 
      
      // Проверяем соответствие содержимого флеш и настроек                                                            
      if ((RespConfig->wr_resp_ui16[cntik_index].rd_param_rom) == ((mask_box_id_t*)SetVarArray[index_var].flash_var)->mask_id[cntik_index])
      {
        RespConfig->wr_resp_ui16[cntik_index].status_rd_rom = 0x01;
      }
      else
      {
        RespConfig->wr_resp_ui16[cntik_index].status_rd_rom = 0x00;     
      }    
      // Проверяем соответствие содержимого флеш и записи в оперативную память                                                            
      if ((RespConfig->wr_resp_ui16[cntik_index].rd_param_ram) == ((mask_box_id_t*)SetVarArray[index_var].flash_var)->mask_id[cntik_index])
      {
        RespConfig->wr_resp_ui16[cntik_index].status_rd_ram = 0x01;
      }
      else
      {
        RespConfig->wr_resp_ui16[cntik_index].status_rd_ram = 0x00;     
      }           
      // Установка размера ответа 
      data_lenght_resp = data_lenght_resp + sizeof(wr_resp_ui16_box_t);
    }
  }
  else
  {
    // Отработка чтения
    for (uint8_t cntik_index = 0; cntik_index < 16; cntik_index++)
    {
      // Формирование ответа 
      // Загрузка значений   
      RespConfig->rd_resp_ui16[cntik_index].rd_param_rom = ((mask_box_id_t*)SetVarArray[index_var].store_var)->mask_id[cntik_index];      
      RespConfig->rd_resp_ui16[cntik_index].rd_param_ram = ((mask_box_id_t*)SetVarArray[index_var].new_var)->mask_id[cntik_index];
      
      // Проверяем соответствие содержимого флеш и настроек                                                            
      if ((RespConfig->rd_resp_ui16[cntik_index].rd_param_rom) == ((mask_box_id_t*)SetVarArray[index_var].flash_var)->mask_id[cntik_index])
      {
        RespConfig->rd_resp_ui16[cntik_index].status_rd_rom = 0x01;
      }
      else
      {
        RespConfig->rd_resp_ui16[cntik_index].status_rd_rom = 0x00;     
      }    
      // Проверяем соответствие содержимого флеш и записи в оперативную память                                                            
      if ((RespConfig->rd_resp_ui16[cntik_index].rd_param_ram) == ((mask_box_id_t*)SetVarArray[index_var].flash_var)->mask_id[cntik_index])
      {
        RespConfig->rd_resp_ui16[cntik_index].status_rd_ram = 0x01;
      }
      else
      {
        RespConfig->rd_resp_ui16[cntik_index].status_rd_ram = 0x00;     
      }           
      // Установка размера ответа 
      data_lenght_resp = data_lenght_resp + sizeof(rd_resp_ui16_box_t);
    }  
  }
  // Возвращаем длинну данных
  return data_lenght_resp;
}

/**
  * @brief  Функция обработки конфигурирования переменной длиной mac_type
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  * @param  setup_prm_t* ReqConfig - указатель на буфер запроса конфигурации
  * @param  setup_prm_t* RespConfig - указатель на буфер для формирования ответа на запрос конфигурации
  * @param  uint16_t index_var - индекс найденой переменной
  * @param  uint8_t index_resp_buf - индекс буфера данных по переменной
  * @retval uint8_t длинна записи переменной 
  */
uint8_t resp_mac_type( core_struct_t* core_st,  setup_prm_t* ReqConfig, setup_prm_t* RespConfig, uint16_t index_var , uint8_t index_resp_buf )
{
  // Переменна длинны ответа
  uint8_t data_lenght_resp;
  // Обнуляем переменную длинны ответа
  data_lenght_resp = 0;
  // загрузка mac адреса как 6 отдельных байт 
  for (uint8_t cntik_index = 0; cntik_index < 6; cntik_index++)
  {
    if (GetIndexVar( VarID(ReqConfig->setup_head.Type, ReqConfig->setup_head.N_port , ReqConfig->setup_head.SETUP_ID , cntik_index ) , &index_var ))
    {
      data_lenght_resp = data_lenght_resp + resp_ui8( core_st, ReqConfig, RespConfig, index_var , cntik_index );
    }
    else
    {
      return 0;
    }  
  }
  // Возвращаем длинну данных
  return data_lenght_resp;
}

#if DEVICE_TYPE == PDH_B

/**
  * @brief  Функция обработки конфигурирования переменной настроек клавиатуры
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  * @param  setup_prm_t* ReqConfig - указатель на буфер запроса конфигурации
  * @param  setup_prm_t* RespConfig - указатель на буфер для формирования ответа на запрос конфигурации
  * @retval uint8_t длинна записи переменной 
  */
uint8_t KeyConfigBox( core_struct_t* core_st,  setup_prm_t* ReqConfig, setup_prm_t* RespConfig)
{
  // Переменна длинны ответа
  uint8_t data_lenght_resp;
  // Обнуляем переменную длинны ответа
  data_lenght_resp = 0;  
  
  // Анализ типа ресурса конфигурации
  if (GetIndexVar( VarID(ReqConfig->setup_head.Type, KEY_N_port , ReqConfig->setup_head.SETUP_ID , 0 ) , &index_var ))
  {
    // Анализ переменной
    switch(SetVarArray[index_var].type_var)
    {
    case UINT8:
    case INT8:
      // Обрабока переменных типа UINT8 INT8
      data_lenght_resp = resp_ui8( core_st, ReqConfig, RespConfig, index_var , ReqConfig->setup_head.N_port );
      break;      
    case UINT16:
    case INT16:
      // Обрабока переменных типа UINT16 INT16
      data_lenght_resp = resp_ui16( core_st, ReqConfig, RespConfig, index_var , ReqConfig->setup_head.N_port );      
      break;      
    default:
      // Переменной с данным идентификатором нет
      RespConfig->data[0] = 0xEE;
      // Установка длинны ответа
      data_lenght_resp = 1;
      break;    
    }
  }
  else
  {
    // Переменной с данным идентификатором нет
    RespConfig->data[0] = 0xEE;
    // Установка длинны ответа
    data_lenght_resp = 1;
  }  
  return  data_lenght_resp;
}
#endif

/**
  * @brief  Функция обработки конфигурирования переменной длиной ip_type
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  * @param  setup_prm_t* ReqConfig - указатель на буфер запроса конфигурации
  * @param  setup_prm_t* RespConfig - указатель на буфер для формирования ответа на запрос конфигурации
  * @param  uint16_t index_var - индекс найденой переменной
  * @param  uint8_t index_resp_buf - индекс буфера данных по переменной
  * @retval uint8_t длинна записи переменной 
  */
uint8_t resp_ip_type( core_struct_t* core_st,  setup_prm_t* ReqConfig, setup_prm_t* RespConfig, uint16_t index_var , uint8_t index_resp_buf )
{
  // Переменна длинны ответа
  uint8_t data_lenght_resp;
  // Обнуляем переменную длинны ответа
  data_lenght_resp = 0;
  // загрузка ip адреса как 4 отдельных байта 
  for (uint8_t cntik_index = 0; cntik_index < 4; cntik_index++)
  {
    if (GetIndexVar( VarID(ReqConfig->setup_head.Type, ReqConfig->setup_head.N_port , ReqConfig->setup_head.SETUP_ID , cntik_index ) , &index_var ))
    {
      data_lenght_resp = data_lenght_resp + resp_ui8( core_st, ReqConfig, RespConfig, index_var , cntik_index );
    }
    else
    {
      return 0;
    }  
  }
  // Возвращаем длинну данных
  return data_lenght_resp;
}

/**
  * @brief Функция обработка запросов конфигурирования
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  * @param  setup_prm_t* ReqConfig - указатель на буфер запроса конфигурации
  * @param  setup_prm_t* RespConfig - указатель на буфер для формирования ответа на запрос конфигурации
  * @param  uint8_t* data_lenght_resp - указатель на переменую длинны ответа
  *
  * @retval none
  */
void ProcessingConfig(core_struct_t* core_st, setup_prm_t* ReqConfig, setup_prm_t* RespConfig , uint8_t* data_lenght_resp)
{
  // проверка на пинг
  if ( (ReqConfig->setup_head.Type == Core_Type) && (ReqConfig->setup_head.N_port == Core_N_port) && (ReqConfig->setup_head.SETUP_ID == PING_CORE_PHY_ADDR ))
  {
    // Отработка пинга PING_ID
    RespConfig->data[0] = 0x4F;
    RespConfig->data[1] = 0x4B;      
    // Установка длинны ответа
    *data_lenght_resp = 2;
  }
  else  
  {
    // отработка команд прочитать версию устройства и его ID
    if ( (ReqConfig->setup_head.Type == Core_Type) && (ReqConfig->setup_head.N_port == Core_N_port) && (ReqConfig->setup_head.SETUP_ID == GET_CORE_VERSION))
    {
      //отработка команд версии устройства и его ID
      // Анализ переменной
      switch(ReqConfig->data[0])
      {
      case ID_VER:// Чтение версии проекта
        // Заполнение шапки ответа версии устройства и его ID
        RespConfig->rd_ver_id.VERSION_ID = ID_VER; // ID настройки
        RespConfig->rd_ver_id.TYPE = 0;            //Тип ответа (0 - строковый; 1 - численный)
        // Установка длинны ответа и содержимого ответа
        *data_lenght_resp = 2 + sprintf((char *)&RespConfig->rd_ver_id.data,"%s",__version__);
        break;
      case ID_DATE:// Чтение Даты компиляц  
        // Заполнение шапки ответа версии устройства и его ID
        RespConfig->rd_ver_id.VERSION_ID = ID_DATE; // ID настройки
        RespConfig->rd_ver_id.TYPE = 0;            //Тип ответа (0 - строковый; 1 - численный)
        // Установка длинны ответа и содержимого ответа
        *data_lenght_resp = 2 + sprintf((char *)&RespConfig->rd_ver_id.data,"%s",__TIME__);        
        break;      
      case ID_TIME:// Чтение Времени компи    
        // Заполнение шапки ответа версии устройства и его ID
        RespConfig->rd_ver_id.VERSION_ID = ID_TIME; // ID настройки
        RespConfig->rd_ver_id.TYPE = 0;            //Тип ответа (0 - строковый; 1 - численный)
        // Установка длинны ответа и содержимого ответа
        *data_lenght_resp = 2 + sprintf((char *)&RespConfig->rd_ver_id.data,"%s",__DATE__);        
        break;      
      case ID_MCU_SIGNATURE:// Чтение сигнатуры  
        // Заполнение шапки ответа версии устройства и его ID
        RespConfig->rd_ver_id.VERSION_ID = ID_MCU_SIGNATURE; // ID настройки
        RespConfig->rd_ver_id.TYPE = 1;            //Тип ответа (0 - строковый; 1 - численный)
        // Установка длинны ответа
        *data_lenght_resp = 2 + 2;
        // Установка содержимого ответа
        // Device signature, bits 11:0 are valid, 15:12 are always 0.
        //           - 0x0413: STM32F405xx/07xx and STM32F415xx/17xx)
        //           - 0x0419: STM32F42xxx and STM32F43xxx
        //           - 0x0423: STM32F401xB/C
        //           - 0x0433: STM32F401xD/E
        //           - 0x0431: STM32F411xC/E
        *(uint16_t*)&RespConfig->rd_ver_id.data = GetSignature(); 
        break;      
      case ID_MCU_REVISION:// Чтение Ревизии мк#define Core_Type  
        // Заполнение шапки ответа версии устройства и его ID
        RespConfig->rd_ver_id.VERSION_ID = ID_MCU_REVISION; // ID настройки
        RespConfig->rd_ver_id.TYPE = 1;            //Тип ответа (0 - строковый; 1 - численный)
        // Установка длинны ответа
        *data_lenght_resp = 2 + 2;
        // Установка содержимого ответа
        // Device revision value
        //   - 0x1000: Revision A
        //   - 0x1001: Revision Z
        //   - 0x1003: Revision Y
        //   - 0x1007: Revision 1
        //   - 0x2001: Revision 3
        *(uint16_t*)&RespConfig->rd_ver_id.data = GetRevision(); 
        break;      
      case ID_MCU_FLASH_SIZE:// Чтение flash памяти в килобайтах#define Core_N_port 
        // Заполнение шапки ответа версии устройства и его ID
        RespConfig->rd_ver_id.VERSION_ID = ID_MCU_FLASH_SIZE; // ID настройки
        RespConfig->rd_ver_id.TYPE = 1;            //Тип ответа (0 - строковый; 1 - численный)
        // Установка длинны ответа
        *data_lenght_resp = 2 + 2;
        // Установка содержимого ответа
        // Flash size in kilo bytes
        *(uint16_t*)&RespConfig->rd_ver_id.data = GetFlashSize(); 
        break;      
      case ID_MCU_UNIQUE:// Чтение 96-битного ID 
        // Заполнение шапки ответа версии устройства и его ID
        RespConfig->rd_ver_id.VERSION_ID = ID_MCU_UNIQUE; // ID настройки
        RespConfig->rd_ver_id.TYPE = 1;            //Тип ответа (0 - строковый; 1 - численный)
        // Установка длинны ответа
        *data_lenght_resp = 2 + 12;
        // Установка содержимого ответа
        for (uint8_t cnt_mcu_unique = 0 ; cnt_mcu_unique < 12 ; cnt_mcu_unique++ )
        {
          // Unique ID address
          ((uint8_t*)&RespConfig->rd_ver_id.data)[cnt_mcu_unique] = GetUnique8(cnt_mcu_unique);
        }
        break;    
        break;      
      default:
        // Переменной с данным идентификатором нет
        RespConfig->data[0] = 0xEE;
        // Установка длинны ответа
        *data_lenght_resp = 1;
        break;    
      }    
    }  
    else
    { 
      /* Отработка команд обнуления счетчиков событий */
      if ( ReqConfig->setup_head.SETUP_ID == CLEAR_EVENTS_COUNTER )      
      {
        /* Анализ типа ресурса */
        switch(SetVarArray[index_var].type_var)
        {
#if (RS_GATE_ENABLE == 1)
        case RS_Type:
          if ( ReqConfig->setup_head.N_port == RS_A_port ) 
          {
            ResetEventCountersRSA();            
            RespConfig->data[0] = 0x4F;
            RespConfig->data[1] = 0x4B;      
            /* Установка длинны ответа */
            *data_lenght_resp = 2;           
          }
          else
          {  
            if ( ReqConfig->setup_head.N_port == RS_B_port )
            {
              ResetEventCountersRSB();
              RespConfig->data[0] = 0x4F;
              RespConfig->data[1] = 0x4B;      
              /* Установка длинны ответа */
              *data_lenght_resp = 2;   
            }
          }
          break;               
#endif
          
        case ETH_Type:
        case Core_Type:
        case ANALOG_Type:
        case CODEC_Type:
        default:
          // Ресурс с данным идентификатором недоступен
          RespConfig->data[0] = 0xEE;
          // Установка длинны ответа
          *data_lenght_resp = 1;
          break;    
        }
      }
      else
      {
        // Отработка команд конфигурирования
#if  DEVICE_TYPE == PDH_B
        // Проверяем порт ресурса - если это КеуBoard - отдельная обработка
        if (ReqConfig->setup_head.Type == KEY_Type)
        {
          // Функция обработки настроек клавиатуры
          *data_lenght_resp = KeyConfigBox(core_st, ReqConfig, RespConfig );
        }  
        else
#endif	  
        {
          // Анализ типа ресурса конфигурации
          if (GetIndexVar( VarID(ReqConfig->setup_head.Type, ReqConfig->setup_head.N_port , ReqConfig->setup_head.SETUP_ID , 0 ) , &index_var ))
          {
            // Анализ переменной
            switch(SetVarArray[index_var].type_var)
            {
            case UINT8:
            case INT8:
              // Обрабока переменных типа UINT8 INT8
              *data_lenght_resp = resp_ui8( core_st, ReqConfig, RespConfig, index_var , 0 );
              break;      
            case UINT16:
            case INT16:
              // Обрабока переменных типа UINT16 INT16
              *data_lenght_resp = resp_ui16( core_st, ReqConfig, RespConfig, index_var , 0 );      
              break;      
            case UINT32:
            case INT32:
              // Обрабока переменных типа UINT32 INT32
              *data_lenght_resp = resp_ui32( core_st, ReqConfig, RespConfig, index_var , 0 );        
              break; 
            case MAC_TYPE:
              // Обрабока переменных типа MAC_TYPE
              *data_lenght_resp = resp_mac_type( core_st, ReqConfig, RespConfig, index_var , 0 );        
              break;       
            case IP_TYPE:
              // Обрабока переменных типа IP_TYPE
              *data_lenght_resp = resp_ip_type( core_st, ReqConfig, RespConfig, index_var , 0 );        
              break;       
              
            case MASK_TYPE:
              // Обрабока переменных типа MASK_TYPE
              *data_lenght_resp = resp_mask_type( core_st, ReqConfig, RespConfig, index_var , 0 );        
              break;        
              
            default:
              // Переменной с данным идентификатором нет
              RespConfig->data[0] = 0xEE;
              // Установка длинны ответа
              *data_lenght_resp = 1;
              break;    
            }
          }
          else
          {
            // Переменной с данным идентификатором нет
            RespConfig->data[0] = 0xEE;
            // Установка длинны ответа
            *data_lenght_resp = 1;
          }  
        }
      }
    }
  }
}

/**
  * @brief Функция формирования шапки ответа на запрос конфигурирования и отправка его в роутер
  * @param uint8_t data_size - размер передаваемых данных
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  *
  * @retval none
  */
void GenRespSetupBox(core_struct_t* core_st, uint8_t data_lenght)
{
  // получаем указатель на буфер запроса конфигурации
  setup_box_t  *ReqConfig = ((setup_box_t*)&(core_st->data_rx)); 
  // получаем указатель на буфер для формирования ответа на запрос конфигурации
  setup_box_t  *RespConfig = ((setup_box_t*)&(core_st->data_tx));  
  
  // Формирование шапки запроса конфигурирования
  RespConfig->rs_head.pre = 0xAA55;                                             // Преамбула  0x55 0xAA
  RespConfig->rs_head.lenght = data_lenght + sizeof(head_box_t) + sizeof(head_setup_t) - SIZE_CRC;       // Длина пакета 
  RespConfig->rs_head.id = ID_SETUP_RESP;                                       // ID ответ команды конфигурации
  RespConfig->rs_head.dest = ReqConfig->rs_head.src;                            // Адрес получателя
  RespConfig->rs_head.src = DataLoaderSto.Settings.phy_adr;                     // Устанавливаем свой физический адрес источника
  // Физический адрес источника и Cчетчик неприрывности пакетов 0..255 заполняется функцией update_rs_tx_box
  RespConfig->rs_head.reserv = 0x00;     //
  RespConfig->rs_head.status_box = 0x00; //
  // Заполнение шапки настроек пакета
  RespConfig->setup_prm.setup_head.factory_addr = DataLoaderSto.Settings.phy_adr;         // Адрес получателя   
  RespConfig->setup_prm.setup_head.Type =         ReqConfig->setup_prm.setup_head.Type;             // Тип ресурса
  RespConfig->setup_prm.setup_head.N_port =       ReqConfig->setup_prm.setup_head.N_port;           // Номер порта
  RespConfig->setup_prm.setup_head.SETUP_ID =     ReqConfig->setup_prm.setup_head.SETUP_ID;         // ID команды 
  
  // Обновление RS пакета перед отправкой
  update_rs_tx_box(&(core_st->data_tx),&(core_st->contic_tx_box));
  
  // Отправляетм пакет в роутер
  if (core_st->xQueue_core_router != NULL)
  {
    // отправить пакет в роутер
    xQueueSend ( core_st->xQueue_core_router, ( void * )&(core_st->data_tx) , ( TickType_t ) 0 );
  } 
}

/**
  * @brief Функция формирования шапки ответа на запрос конфигурирования и отправка его в роутер
  * @param uint8_t data_size - размер передаваемых данных
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  *
  * @retval none
  */
void GenRespRFSetupBox(core_struct_t* core_st, uint8_t data_lenght)
{
  // получаем указатель на буфер запроса конфигурации
  RSFrameSETUP_t  *ReqRFConfig = &(core_st->req_nwk_setup_rx); 
  // получаем указатель на буфер для формирования ответа на запрос конфигурации
  RSFrameSETUP_t  *RespRFConfig = &(core_st->req_nwk_setup_tx);  

  /* Заполнение шапки RS пакета */
  RespRFConfig->rs_head.pre = 0xAA55;                                                              /* Преамбула  0x55 0xAA                                                 */
  /* Длина пакета (включая контрольную сумму, не включая преамбулу)       */
  RespRFConfig->rs_head.lenght = data_lenght + sizeof(head_box_t) + sizeof(head_setup_t) + (1 + sizeof(NwkFrameHeader_t)) + SIZE_ID_BOX - SIZE_CRC;     
  RespRFConfig->rs_head.id = ID_SETUP_RF_RESP;                                                     /* Идентификатор пакета                                                 */
  RespRFConfig->rs_head.dest = 0xFFFF;                                                             /* Физический адрес получателя                                          */
  RespRFConfig->rs_head.src = DataLoaderSto.Settings.phy_adr;                                      /* Физический адрес источника                                           */
  RespRFConfig->rs_head.cnt = 00;                                                                  /* Cчетчик неприрывности пакетов 0..255                                 */
  RespRFConfig->rs_head.reserv = 0x00;     //
  RespRFConfig->rs_head.status_box = 0x00; // 
  /* длинна пакета модбас */
  RespRFConfig->NwkFrameSetup.size = RespRFConfig->rs_head.lenght - sizeof(head_setup_t) - SIZE_PRE - SIZE_CRC;  
  /* Формирование шапки пакета по RF  */
  /* * заголовок MAC в посылке *      */
  RespRFConfig->NwkFrameSetup.header.macFcf = 0x8841;                                             /* frame control                                                        */
  RespRFConfig->NwkFrameSetup.header.macSeq = core_st->contic_tx_box;                            /* sequence number                                                      */
  RespRFConfig->NwkFrameSetup.header.macDstPanId = 0xABCD;                                        /* PANID кому предназначались данные                                    */
  RespRFConfig->NwkFrameSetup.header.macDstAddr = 0xFFFF;                                         /* Адрес кому предназначались данные                                    */
  RespRFConfig->NwkFrameSetup.header.macSrcAddr = DataSto.Settings.mac_adr;                       /* Адрес от кого предавались данные                                     */
    
  /* * заголовок NWK в посылке (входит в PAYLOAD посылки по IEEE  802.15.4) * */        
  RespRFConfig->NwkFrameSetup.header.id1 = _NWK_DATA_SETUP_RESP_;                                 /* ID1 пакета                                                           */
  RespRFConfig->NwkFrameSetup.header.id2 = (uint8_t)~(unsigned)_NWK_DATA_SETUP_RESP_;             /* ID2 пакета                                                           */
  RespRFConfig->NwkFrameSetup.header.nwkFcf_u.val = 0x12;                                         /* * Поле настройки фрейма *                                            */
    
  RespRFConfig->NwkFrameSetup.header.nwkSeq = core_st->contic_tx_box;                            /* идентификатор последовательности кадра                               */
  RespRFConfig->NwkFrameSetup.header.nwkSrcPanId = 0x01;                                          /* Сетевой PANID источника (он же зона для ретрасляции)                 */
  RespRFConfig->NwkFrameSetup.header.nwkDstPanId = 0xFF;                                          /* Сетевой PANID назначения (он же зона для ретрасляции)                */
  RespRFConfig->NwkFrameSetup.header.nwkDstRouterAddr = 0xFF;                                     /* Адрес роутера которому предназначаются данные                        */
  RespRFConfig->NwkFrameSetup.header.nwkSrcHop = 0xFF;                                            /* Хоп источника данных                                                 */
  RespRFConfig->NwkFrameSetup.header.nwkOwnHop = 0xFF;                                            /* Собственный хоп                                                      */
  RespRFConfig->NwkFrameSetup.header.nwkSrcAddr = DataSto.Settings.mac_adr;                       /* Адрес источника                                                      */
  RespRFConfig->NwkFrameSetup.header.nwkDstAddr = 0xFFFF;                                         /* Адрес назначения                                                     */
          
  RespRFConfig->NwkFrameSetup.header.nwkSrcEndpoint = 0x1;                                        /* Endpoint источника                                                   */
  RespRFConfig->NwkFrameSetup.header.nwkDstEndpoint = 0x1;                                        /* Endpoint  назначения                                                 */
    
  RespRFConfig->NwkFrameSetup.header.nwk_count_routing = 0x00;                                    /* Счётчик маршрутизаций                                                */
  RespRFConfig->NwkFrameSetup.header.nwk_src_factory_addr = DataLoaderSto.Settings.phy_adr;       /* Заводской адрес истоника текущей посылки                             */
  RespRFConfig->NwkFrameSetup.header.nwk_own_factory_addr = DataLoaderSto.Settings.phy_adr;       /* Заводской адрес инициатора посылки                                   */
  RespRFConfig->NwkFrameSetup.header.reserv1 = 0x00;                                              /* резерв 1                                                             */
  RespRFConfig->NwkFrameSetup.header.reserv2 = 0x0000;                                            /* резерв 2                                                             */
   
  /* установка  физического адреса получателя пакета модбас */ 
  RespRFConfig->NwkFrameSetup.dest_addr = 0xFFFF;  
  /* установка физического адреса источника пакета модбас   */ 
  RespRFConfig->NwkFrameSetup.src_addr = DataLoaderSto.Settings.phy_adr; 
  /* установка идентификатора соединения */
  RespRFConfig->NwkFrameSetup.index_connect = ReqRFConfig->NwkFrameSetup.index_connect;
  
  /* копируем поле данных пакета *///(tcp_tude_st->buf_eth_rtr_setup.rs_head.lenght + SIZE_CRC - sizeof(head_box_t
  /* Заполнение шапки настроек пакета*/
  RespRFConfig->NwkFrameSetup.setup_prm.setup_head.factory_addr = DataLoaderSto.Settings.phy_adr;         // Адрес получателя   
  RespRFConfig->NwkFrameSetup.setup_prm.setup_head.Type =         ReqRFConfig->NwkFrameSetup.setup_prm.setup_head.Type;             // Тип ресурса
  RespRFConfig->NwkFrameSetup.setup_prm.setup_head.N_port =       ReqRFConfig->NwkFrameSetup.setup_prm.setup_head.N_port;           // Номер порта
  RespRFConfig->NwkFrameSetup.setup_prm.setup_head.SETUP_ID =     ReqRFConfig->NwkFrameSetup.setup_prm.setup_head.SETUP_ID;         // ID команды 
  
  // Обновление RS пакета перед отправкой
  update_rs_tx_box((router_box_t*)RespRFConfig,&(core_st->contic_tx_box));
  
  // Отправляетм пакет в роутер
  if (core_st->xQueue_core_router != NULL)
  {
    // отправить пакет в роутер
    xQueueSend ( core_st->xQueue_core_router, ( void * )&(core_st->data_tx) , ( TickType_t ) 0 );
  } 
}

/**
  * @brief Функция обработка запросов конфигурирования
  * @param  core_struct_t*  core_st - указатель на структуру ядра
  *
  * @retval bool true  - пакет обработан
  *              false - пакет не обработан
  */
bool ProcessingConfigBox(core_struct_t* core_st)
{
  // Переменна длинны ответа
  uint8_t data_lenght_resp = 0;
  // Переменная дляы указателя на буфер запроса конфигурации
  setup_prm_t  *ReqConfig;
  // Переменная дляы указателя на буфер для формирования ответа на запрос конфигурации
  setup_prm_t  *RespConfig;
  
  // Проверка ID RS Запрос команд конфигурации 
  switch(core_st->data_rx.id)
  {
  case ID_SETUP_REQ:
    // получаем указатель на буфер запроса конфигурации
    ReqConfig = &(core_st->req_setup_rx.setup_prm); 
    // получаем указатель на буфер для формирования ответа на запрос конфигурации
    RespConfig = &(core_st->req_setup_tx.setup_prm); 
    // Проверка PHY_Addr
    if ((ReqConfig->setup_head.factory_addr) != DataLoaderSto.Settings.phy_adr) return false;    
    /* Функция обработка запросов конфигурирования*/
    ProcessingConfig(core_st, ReqConfig, RespConfig, &data_lenght_resp);
    // формирования шапки ответа на запрос конфигурирования и отправка его в роутер
    GenRespSetupBox( core_st, data_lenght_resp);   
    /* пакет обработан */
    return true;         
  case ID_SETUP_RF_REQ:
    // получаем указатель на буфер запроса конфигурации
    ReqConfig = &(core_st->req_nwk_setup_rx.NwkFrameSetup.setup_prm); 
    // получаем указатель на буфер для формирования ответа на запрос конфигурации
    RespConfig = &(core_st->req_nwk_setup_tx.NwkFrameSetup.setup_prm); 
    // Проверка PHY_Addr
    if ((ReqConfig->setup_head.factory_addr) != DataLoaderSto.Settings.phy_adr) return false;    
    /* Функция обработка запросов конфигурирования*/
    ProcessingConfig(core_st, ReqConfig, RespConfig, &data_lenght_resp);
    // формирования шапки ответа на запрос конфигурирования и отправка его в роутер
    if (data_lenght_resp < (MAX_SIZE_DATA_NWK_FRAME - SIZE_ID_BOX - sizeof(head_setup_t)))
    {
      GenRespRFSetupBox( core_st, data_lenght_resp); 
    }       
    /* пакет обработан */
    return true;      
  }
  /* пакет не обработан */
  return false;   
}

/**
  * @brief Функция формирования тестового запроса конфигурирования
  * @param QueueHandle_t xQueue_router - указатель на очередб в роутер
  * @param router_box_t  *data_rx - указатель на буфер приема данных
  * @param uint16_t   TimeUpdate - временной интервал с последнего обновления 
  *
  * @retval none
  */
void TestSetupReq(QueueHandle_t xQueue_router, router_box_t *data_rx , uint16_t   TimeUpdate)
{
 
  // Проверяем счетчик периода обновления
  if (cnt_update_time_test > TimeUpdate)
  {
    // Отсчитываем период обновления
    cnt_update_time_test = cnt_update_time_test - TimeUpdate;
  }
  else
  {
    // Инициализация таймера обновления
    cnt_update_time_test = MAX_UPDATE_TIME_TEST;
    // получаем указатель на запрос конфигурации
    setup_box_t  *ReqConfig = ((setup_box_t*)data_rx);
    
    // Формирование шапки запроса конфигурирования
    ReqConfig->rs_head.pre = 0xAA55;      // Преамбула  0x55 0xAA
    ReqConfig->rs_head.lenght = 10 + 5;   // Длина пакета без данных
    ReqConfig->rs_head.id = ID_SETUP_REQ; // ID ответ команды конфигурации
    ReqConfig->rs_head.dest = DataLoaderSto.Settings.phy_adr;     // Адрес получателя
    ReqConfig->rs_head.src = DataLoaderSto.Settings.phy_adr;      // Устанавливаем свой физический адрес источника
    // Физический адрес источника и Cчетчик неприрывности пакетов 0..255 заполняется функцией update_rs_tx_box
    ReqConfig->rs_head.reserv = 0x00;     //
    ReqConfig->rs_head.status_box = 0x00; //
    // Заполнение тела пакета
    
    ReqConfig->setup_prm.setup_head.factory_addr = DataLoaderSto.Settings.phy_adr;   
    ReqConfig->setup_prm.setup_head.Type = Core_Type;                  // Тип ресурса
    ReqConfig->setup_prm.setup_head.N_port = Core_N_port;              // Номер порта
    ReqConfig->setup_prm.setup_head.SETUP_ID = GET_CORE_LOG_ADDR;         //  Прочитать Значение выбранного канала (частота МГц) *    
      
    // Обновление RS пакета перед отправкой
    update_rs_tx_box((router_box_t*)ReqConfig,&contic_test);
    
    // Отправка пакета в роутер
    if (xQueue_router != NULL) 
    {
        // Отправляем пакет
        xQueueSend(xQueue_router, (router_box_t*)ReqConfig, ( TickType_t ) 0 );
    }
  }  
}

/******************* (C) COPYRIGHT 2019 DataExpress *****END OF FILE****/
