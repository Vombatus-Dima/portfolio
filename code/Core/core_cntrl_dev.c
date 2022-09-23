/**
  ******************************************************************************
  * @file    core_cntrl_dev.с
  * @author  Trembach Dmitry
  * @version V1.1.0
  * @date    28-12-2020
  * @brief   файл с функциями ядра для контроля устройств в RS
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 DataExpress</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "router_streams.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Core.h"
#include "core_cntrl_dev.h"
#include "rf_frame_header.h"
#include "settings.h"    
#include "board.h"  
#include "printf_dbg.h"
#include "core_cntrl_settings.h"


/* Таблица устройств RS */
diag_dev_t  dev_base[MAX_TABLE_DEV];

/* Цисло циклов запроса пинга по RS */
uint8_t number_cycle_ping_dev = MAX_NUM_CYCLE_PING_DEV; 
/* Счетчик устройств внутри цикла запроса пинга по RS */
uint16_t cnt_ping_dev = 1; 


#if  TEST_GEN_TAB_DEV == 1

/**
  * @brief Функция заполнения массива устройств для тестирования
  * @param none
  * @retval none
  */
void TestGenTableDev( void )
{
  dev_base[ 0].Type_Device =  VRM_V;   dev_base[ 0].Own_PHY_Addr = 7128;  dev_base[ 0].alarm_flag = 0x0000;  dev_base[ 0].Value_VCC = 2343; dev_base[ 0].Value_VCC_min = 2335;  dev_base[ 0].time_of_life = 290; dev_base[ 0].Near_PHY_Add[0] =  4188;  dev_base[ 0].Near_PHY_Add[1] =     0;  dev_base[ 0].Near_PHY_Add[2] =     0;  dev_base[ 0].Near_PHY_Add[3] =     0;
  dev_base[ 1].Type_Device = VRM_RS;   dev_base[ 1].Own_PHY_Addr = 5348;  dev_base[ 1].alarm_flag = 0x0000;  dev_base[ 1].Value_VCC = 1874; dev_base[ 1].Value_VCC_min = 1787;  dev_base[ 1].time_of_life = 293; dev_base[ 1].Near_PHY_Add[0] =  4938;  dev_base[ 1].Near_PHY_Add[1] = 64758;  dev_base[ 1].Near_PHY_Add[2] =     0;  dev_base[ 1].Near_PHY_Add[3] =     0;
  dev_base[ 2].Type_Device = VRM_RS;   dev_base[ 2].Own_PHY_Addr = 5068;  dev_base[ 2].alarm_flag = 0x0000;  dev_base[ 2].Value_VCC = 1933; dev_base[ 2].Value_VCC_min = 1842;  dev_base[ 2].time_of_life = 294; dev_base[ 2].Near_PHY_Add[0] =  4948;  dev_base[ 2].Near_PHY_Add[1] =  5858;  dev_base[ 2].Near_PHY_Add[2] =     0;  dev_base[ 2].Near_PHY_Add[3] =     0;
  dev_base[ 3].Type_Device = VRM_RS;   dev_base[ 3].Own_PHY_Addr = 4938;  dev_base[ 3].alarm_flag = 0x0000;  dev_base[ 3].Value_VCC = 1874; dev_base[ 3].Value_VCC_min = 1777;  dev_base[ 3].time_of_life = 295; dev_base[ 3].Near_PHY_Add[0] =  4968;  dev_base[ 3].Near_PHY_Add[1] =  5348;  dev_base[ 3].Near_PHY_Add[2] =     0;  dev_base[ 3].Near_PHY_Add[3] =     0;
  dev_base[ 4].Type_Device = VRM_RS;   dev_base[ 4].Own_PHY_Addr = 5558;  dev_base[ 4].alarm_flag = 0x0000;  dev_base[ 4].Value_VCC = 1788; dev_base[ 4].Value_VCC_min = 1703;  dev_base[ 4].time_of_life = 295; dev_base[ 4].Near_PHY_Add[0] =  5808;  dev_base[ 4].Near_PHY_Add[1] =  5058;  dev_base[ 4].Near_PHY_Add[2] =     0;  dev_base[ 4].Near_PHY_Add[3] =     0;
  dev_base[ 5].Type_Device = VRM_RS;   dev_base[ 5].Own_PHY_Addr = 4868;  dev_base[ 5].alarm_flag = 0x0000;  dev_base[ 5].Value_VCC = 1677; dev_base[ 5].Value_VCC_min = 1571;  dev_base[ 5].time_of_life = 296; dev_base[ 5].Near_PHY_Add[0] =  4988;  dev_base[ 5].Near_PHY_Add[1] =  4968;  dev_base[ 5].Near_PHY_Add[2] =     0;  dev_base[ 5].Near_PHY_Add[3] =     0;
  dev_base[ 6].Type_Device =  BPI_A;   dev_base[ 6].Own_PHY_Addr = 64188; dev_base[ 6].alarm_flag = 0x0000;  dev_base[ 6].Value_VCC = 7089; dev_base[ 6].Value_VCC_min = 3487;  dev_base[ 6].time_of_life = 296; dev_base[ 6].Near_PHY_Add[0] =  0000;  dev_base[ 6].Near_PHY_Add[1] =     0;  dev_base[ 6].Near_PHY_Add[2] =     0;  dev_base[ 6].Near_PHY_Add[3] =     0;
  dev_base[ 7].Type_Device = VRM_RS;   dev_base[ 7].Own_PHY_Addr = 5858;  dev_base[ 7].alarm_flag = 0x0000;  dev_base[ 7].Value_VCC = 1870; dev_base[ 7].Value_VCC_min = 1787;  dev_base[ 7].time_of_life = 260; dev_base[ 7].Near_PHY_Add[0] =  5068;  dev_base[ 7].Near_PHY_Add[1] =  5808;  dev_base[ 7].Near_PHY_Add[2] =     0;  dev_base[ 7].Near_PHY_Add[3] =     0;
  dev_base[ 8].Type_Device = VRM_RS;   dev_base[ 8].Own_PHY_Addr = 4988;  dev_base[ 8].alarm_flag = 0x0040;  dev_base[ 8].Value_VCC = 1704; dev_base[ 8].Value_VCC_min = 1613;  dev_base[ 8].time_of_life = 297; dev_base[ 8].Near_PHY_Add[0] =  5538;  dev_base[ 8].Near_PHY_Add[1] =  4868;  dev_base[ 8].Near_PHY_Add[2] =     0;  dev_base[ 8].Near_PHY_Add[3] =     0;
  dev_base[ 9].Type_Device = VRM_RS;   dev_base[ 9].Own_PHY_Addr = 5658;  dev_base[ 9].alarm_flag = 0x0000;  dev_base[ 9].Value_VCC = 1863; dev_base[ 9].Value_VCC_min = 1767;  dev_base[ 9].time_of_life = 298; dev_base[ 9].Near_PHY_Add[0] =  5968;  dev_base[ 9].Near_PHY_Add[1] =  5598;  dev_base[ 9].Near_PHY_Add[2] =     0;  dev_base[ 9].Near_PHY_Add[3] =     0;
  dev_base[10].Type_Device = VRM_RS;   dev_base[10].Own_PHY_Addr = 808;   dev_base[10].alarm_flag = 0x0000;  dev_base[10].Value_VCC = 2113; dev_base[10].Value_VCC_min = 2058;  dev_base[10].time_of_life = 298; dev_base[10].Near_PHY_Add[0] =  4858;  dev_base[10].Near_PHY_Add[1] =     0;  dev_base[10].Near_PHY_Add[2] =     0;  dev_base[10].Near_PHY_Add[3] =     0;
  dev_base[11].Type_Device = VRM_RS;   dev_base[11].Own_PHY_Addr = 5968;  dev_base[11].alarm_flag = 0x0000;  dev_base[11].Value_VCC = 1755; dev_base[11].Value_VCC_min = 1653;  dev_base[11].time_of_life = 299; dev_base[11].Near_PHY_Add[0] =  5058;  dev_base[11].Near_PHY_Add[1] =  5658;  dev_base[11].Near_PHY_Add[2] =     0;  dev_base[11].Near_PHY_Add[3] =     0;
  dev_base[12].Type_Device = VRM_RS;   dev_base[12].Own_PHY_Addr = 5538;  dev_base[12].alarm_flag = 0x0000;  dev_base[12].Value_VCC = 1754; dev_base[12].Value_VCC_min = 1656;  dev_base[12].time_of_life = 300; dev_base[12].Near_PHY_Add[0] =  5618;  dev_base[12].Near_PHY_Add[1] =  4988;  dev_base[12].Near_PHY_Add[2] =     0;  dev_base[12].Near_PHY_Add[3] =     0;
  dev_base[13].Type_Device = PI_SPL;   dev_base[13].Own_PHY_Addr = 64998; dev_base[13].alarm_flag = 0x0010;  dev_base[13].Value_VCC = 2232; dev_base[13].Value_VCC_min = 2230;  dev_base[13].time_of_life = 270; dev_base[13].Near_PHY_Add[0] =  5838;  dev_base[13].Near_PHY_Add[1] =  4838;  dev_base[13].Near_PHY_Add[2] =     0;  dev_base[13].Near_PHY_Add[3] = 64068;
  dev_base[14].Type_Device = VRM_RS;   dev_base[14].Own_PHY_Addr = 4948;  dev_base[14].alarm_flag = 0x0000;  dev_base[14].Value_VCC = 2010; dev_base[14].Value_VCC_min = 1922;  dev_base[14].time_of_life = 270; dev_base[14].Near_PHY_Add[0] = 64978;  dev_base[14].Near_PHY_Add[1] =  5068;  dev_base[14].Near_PHY_Add[2] =     0;  dev_base[14].Near_PHY_Add[3] =     0;
  dev_base[15].Type_Device = VRM_RS;   dev_base[15].Own_PHY_Addr = 4188;  dev_base[15].alarm_flag = 0x0008;  dev_base[15].Value_VCC = 2167; dev_base[15].Value_VCC_min = 2086;  dev_base[15].time_of_life = 270; dev_base[15].Near_PHY_Add[0] =  5118;  dev_base[15].Near_PHY_Add[1] =  7128;  dev_base[15].Near_PHY_Add[2] =     0;  dev_base[15].Near_PHY_Add[3] =     0;
  dev_base[16].Type_Device = VRM_RS;   dev_base[16].Own_PHY_Addr = 4858;  dev_base[16].alarm_flag = 0x0000;  dev_base[16].Value_VCC = 2102; dev_base[16].Value_VCC_min = 2020;  dev_base[16].time_of_life = 270; dev_base[16].Near_PHY_Add[0] = 64978;  dev_base[16].Near_PHY_Add[1] =   808;  dev_base[16].Near_PHY_Add[2] =     0;  dev_base[16].Near_PHY_Add[3] =     0;
  dev_base[17].Type_Device = VRM_RS;   dev_base[17].Own_PHY_Addr = 5308;  dev_base[17].alarm_flag = 0x0000;  dev_base[17].Value_VCC = 1907; dev_base[17].Value_VCC_min = 1808;  dev_base[17].time_of_life = 270; dev_base[17].Near_PHY_Add[0] = 64758;  dev_base[17].Near_PHY_Add[1] =  5118;  dev_base[17].Near_PHY_Add[2] =     0;  dev_base[17].Near_PHY_Add[3] =     0;
  dev_base[18].Type_Device = VRM_RS;   dev_base[18].Own_PHY_Addr = 5918;  dev_base[18].alarm_flag = 0x0000;  dev_base[18].Value_VCC = 1880; dev_base[18].Value_VCC_min = 1781;  dev_base[18].time_of_life = 270; dev_base[18].Near_PHY_Add[0] =  4838;  dev_base[18].Near_PHY_Add[1] =  5618;  dev_base[18].Near_PHY_Add[2] =     0;  dev_base[18].Near_PHY_Add[3] =     0;
  dev_base[19].Type_Device = VRM_RS;   dev_base[19].Own_PHY_Addr = 5058;  dev_base[19].alarm_flag = 0x0000;  dev_base[19].Value_VCC = 1748; dev_base[19].Value_VCC_min = 1650;  dev_base[19].time_of_life = 271; dev_base[19].Near_PHY_Add[0] =  5558;  dev_base[19].Near_PHY_Add[1] =  5968;  dev_base[19].Near_PHY_Add[2] =     0;  dev_base[19].Near_PHY_Add[3] =     0;
  dev_base[20].Type_Device = VRM_RS;   dev_base[20].Own_PHY_Addr = 5808;  dev_base[20].alarm_flag = 0x0000;  dev_base[20].Value_VCC = 1813; dev_base[20].Value_VCC_min = 1718;  dev_base[20].time_of_life = 271; dev_base[20].Near_PHY_Add[0] =  5858;  dev_base[20].Near_PHY_Add[1] =  5558;  dev_base[20].Near_PHY_Add[2] =     0;  dev_base[20].Near_PHY_Add[3] =     0;
  dev_base[21].Type_Device = VRM_RS;   dev_base[21].Own_PHY_Addr = 5698;  dev_base[21].alarm_flag = 0x0000;  dev_base[21].Value_VCC = 1983; dev_base[21].Value_VCC_min = 1937;  dev_base[21].time_of_life = 272; dev_base[21].Near_PHY_Add[0] = 64818;  dev_base[21].Near_PHY_Add[1] =     0;  dev_base[21].Near_PHY_Add[2] =     0;  dev_base[21].Near_PHY_Add[3] =     0;
  dev_base[22].Type_Device = VRM_RS;   dev_base[22].Own_PHY_Addr = 5118;  dev_base[22].alarm_flag = 0x0040;  dev_base[22].Value_VCC = 2040; dev_base[22].Value_VCC_min = 1953;  dev_base[22].time_of_life = 273; dev_base[22].Near_PHY_Add[0] =  5308;  dev_base[22].Near_PHY_Add[1] =  4188;  dev_base[22].Near_PHY_Add[2] =     0;  dev_base[22].Near_PHY_Add[3] =     0;
  dev_base[23].Type_Device = VRM_RS;   dev_base[23].Own_PHY_Addr = 4968;  dev_base[23].alarm_flag = 0x0000;  dev_base[23].Value_VCC = 1636; dev_base[23].Value_VCC_min = 1552;  dev_base[23].time_of_life = 292; dev_base[23].Near_PHY_Add[0] =  4868;  dev_base[23].Near_PHY_Add[1] =  4938;  dev_base[23].Near_PHY_Add[2] =     0;  dev_base[23].Near_PHY_Add[3] =     0;
  dev_base[24].Type_Device =  BPI_A;   dev_base[24].Own_PHY_Addr = 64068; dev_base[24].alarm_flag = 0x0000;  dev_base[24].Value_VCC = 1439; dev_base[24].Value_VCC_min =  665;  dev_base[24].time_of_life = 270; dev_base[24].Near_PHY_Add[0] =     0;  dev_base[24].Near_PHY_Add[1] =  0000;  dev_base[24].Near_PHY_Add[2] =     0;  dev_base[24].Near_PHY_Add[3] =     0;
  dev_base[25].Type_Device = VRM_RS;   dev_base[25].Own_PHY_Addr = 4838;  dev_base[25].alarm_flag = 0x0000;  dev_base[25].Value_VCC = 1987; dev_base[25].Value_VCC_min = 1898;  dev_base[25].time_of_life = 289; dev_base[25].Near_PHY_Add[0] = 64998;  dev_base[25].Near_PHY_Add[1] =  5918;  dev_base[25].Near_PHY_Add[2] =     0;  dev_base[25].Near_PHY_Add[3] =     0;
  dev_base[26].Type_Device =  BPI_A;   dev_base[26].Own_PHY_Addr = 64048; dev_base[26].alarm_flag = 0x0000;  dev_base[26].Value_VCC = 1103; dev_base[26].Value_VCC_min =  654;  dev_base[26].time_of_life = 280; dev_base[26].Near_PHY_Add[0] =     0;  dev_base[26].Near_PHY_Add[1] =     0;  dev_base[26].Near_PHY_Add[2] =     0;  dev_base[26].Near_PHY_Add[3] =     0;
  dev_base[27].Type_Device = PI_SPL;   dev_base[27].Own_PHY_Addr = 64978; dev_base[27].alarm_flag = 0x000C;  dev_base[27].Value_VCC = 2300; dev_base[27].Value_VCC_min = 2297;  dev_base[27].time_of_life = 281; dev_base[27].Near_PHY_Add[0] =  4858;  dev_base[27].Near_PHY_Add[1] =  7238;  dev_base[27].Near_PHY_Add[2] =  4948;  dev_base[27].Near_PHY_Add[3] = 64018;
  dev_base[28].Type_Device =  BPI_A;   dev_base[28].Own_PHY_Addr = 64018; dev_base[28].alarm_flag = 0x0000;  dev_base[28].Value_VCC = 1178; dev_base[28].Value_VCC_min =  662;  dev_base[28].time_of_life = 286; dev_base[28].Near_PHY_Add[0] =     0;  dev_base[28].Near_PHY_Add[1] =     0;  dev_base[28].Near_PHY_Add[2] =     0;  dev_base[28].Near_PHY_Add[3] =     0;
  dev_base[29].Type_Device = VRM_RS;   dev_base[29].Own_PHY_Addr = 5618;  dev_base[29].alarm_flag = 0x0000;  dev_base[29].Value_VCC = 1796; dev_base[29].Value_VCC_min = 1703;  dev_base[29].time_of_life = 275; dev_base[29].Near_PHY_Add[0] =  5918;  dev_base[29].Near_PHY_Add[1] =  5538;  dev_base[29].Near_PHY_Add[2] =     0;  dev_base[29].Near_PHY_Add[3] =     0;
  dev_base[30].Type_Device = PI_SPL;   dev_base[30].Own_PHY_Addr = 64758; dev_base[30].alarm_flag = 0x0020;  dev_base[30].Value_VCC = 2086; dev_base[30].Value_VCC_min = 2084;  dev_base[30].time_of_life = 286; dev_base[30].Near_PHY_Add[0] =  5348;  dev_base[30].Near_PHY_Add[1] =  5598;  dev_base[30].Near_PHY_Add[2] =  5308;  dev_base[30].Near_PHY_Add[3] = 64048;
  dev_base[31].Type_Device = VRM_RS;   dev_base[31].Own_PHY_Addr = 5838;  dev_base[31].alarm_flag = 0x0008;  dev_base[31].Value_VCC = 2086; dev_base[31].Value_VCC_min = 2000;  dev_base[31].time_of_life = 287; dev_base[31].Near_PHY_Add[0] =  5078;  dev_base[31].Near_PHY_Add[1] = 64998;  dev_base[31].Near_PHY_Add[2] =     0;  dev_base[31].Near_PHY_Add[3] =     0;
  dev_base[32].Type_Device = VRM_RS;   dev_base[32].Own_PHY_Addr = 5598;  dev_base[32].alarm_flag = 0x0000;  dev_base[32].Value_VCC = 1861; dev_base[32].Value_VCC_min = 1764;  dev_base[32].time_of_life = 288; dev_base[32].Near_PHY_Add[0] =  5658;  dev_base[32].Near_PHY_Add[1] = 64758;  dev_base[32].Near_PHY_Add[2] =     0;  dev_base[32].Near_PHY_Add[3] =     0;
  dev_base[33].Type_Device = PI_SPL;   dev_base[33].Own_PHY_Addr = 64818; dev_base[33].alarm_flag = 0x0008;  dev_base[33].Value_VCC = 2165; dev_base[33].Value_VCC_min = 2163;  dev_base[33].time_of_life = 280; dev_base[33].Near_PHY_Add[0] =  5078;  dev_base[33].Near_PHY_Add[1] =  7268;  dev_base[33].Near_PHY_Add[2] =  5698;  dev_base[33].Near_PHY_Add[3] = 64188;
  dev_base[34].Type_Device = VRM_RS;   dev_base[34].Own_PHY_Addr = 5078;  dev_base[34].alarm_flag = 0x0000;  dev_base[34].Value_VCC = 2051; dev_base[34].Value_VCC_min = 1962;  dev_base[34].time_of_life = 208; dev_base[34].Near_PHY_Add[0] = 64818;  dev_base[34].Near_PHY_Add[1] =  5838;  dev_base[34].Near_PHY_Add[2] =     0;  dev_base[34].Near_PHY_Add[3] =     0;

/*
  dev_base[ 0].Type_Device =  VRM_V;   dev_base[ 0].Own_PHY_Addr = 0x1BD8;  dev_base[ 0].alarm_flag = 0x0000;  dev_base[ 0].Value_VCC = 2342; dev_base[ 0].Value_VCC_min = 2332;  dev_base[ 0].time_of_life = 290; dev_base[ 0].Near_PHY_Add[0] = 0x105C;  dev_base[ 0].Near_PHY_Add[1] = 0x0000;  dev_base[ 0].Near_PHY_Add[2] = 0x0000;  dev_base[ 0].Near_PHY_Add[3] = 0x0000;
  dev_base[ 1].Type_Device = VRM_RS;   dev_base[ 1].Own_PHY_Addr = 0x15DE;  dev_base[ 1].alarm_flag = 0x0000;  dev_base[ 1].Value_VCC = 1861; dev_base[ 1].Value_VCC_min = 1767;  dev_base[ 1].time_of_life = 293; dev_base[ 1].Near_PHY_Add[0] = 0x161A;  dev_base[ 1].Near_PHY_Add[1] = 0xFCF6;  dev_base[ 1].Near_PHY_Add[2] = 0x0000;  dev_base[ 1].Near_PHY_Add[3] = 0x0000;
  dev_base[ 2].Type_Device = VRM_RS;   dev_base[ 2].Own_PHY_Addr = 0x13D6;  dev_base[ 2].alarm_flag = 0x0000;  dev_base[ 2].Value_VCC = 2052; dev_base[ 2].Value_VCC_min = 1960;  dev_base[ 2].time_of_life = 294; dev_base[ 2].Near_PHY_Add[0] = 0xFD32;  dev_base[ 2].Near_PHY_Add[1] = 0x16CE;  dev_base[ 2].Near_PHY_Add[2] = 0x0000;  dev_base[ 2].Near_PHY_Add[3] = 0x0000;
  dev_base[ 3].Type_Device = VRM_RS;   dev_base[ 3].Own_PHY_Addr = 0x13CC;  dev_base[ 3].alarm_flag = 0x0000;  dev_base[ 3].Value_VCC = 1942; dev_base[ 3].Value_VCC_min = 1847;  dev_base[ 3].time_of_life = 295; dev_base[ 3].Near_PHY_Add[0] = 0x1354;  dev_base[ 3].Near_PHY_Add[1] = 0x16E2;  dev_base[ 3].Near_PHY_Add[2] = 0x0000;  dev_base[ 3].Near_PHY_Add[3] = 0x0000;
  dev_base[ 4].Type_Device = VRM_RS;   dev_base[ 4].Own_PHY_Addr = 0x14E4;  dev_base[ 4].alarm_flag = 0x0000;  dev_base[ 4].Value_VCC = 1876; dev_base[ 4].Value_VCC_min = 1787;  dev_base[ 4].time_of_life = 295; dev_base[ 4].Near_PHY_Add[0] = 0x134A;  dev_base[ 4].Near_PHY_Add[1] = 0xFCF6;  dev_base[ 4].Near_PHY_Add[2] = 0x0000;  dev_base[ 4].Near_PHY_Add[3] = 0x0000;
  dev_base[ 5].Type_Device = VRM_RS;   dev_base[ 5].Own_PHY_Addr = 0x134A;  dev_base[ 5].alarm_flag = 0x0000;  dev_base[ 5].Value_VCC = 1884; dev_base[ 5].Value_VCC_min = 1780;  dev_base[ 5].time_of_life = 296; dev_base[ 5].Near_PHY_Add[0] = 0x1368;  dev_base[ 5].Near_PHY_Add[1] = 0x14E4;  dev_base[ 5].Near_PHY_Add[2] = 0x0000;  dev_base[ 5].Near_PHY_Add[3] = 0x0000;
  dev_base[ 6].Type_Device = VRM_RS;   dev_base[ 6].Own_PHY_Addr = 0x15B6;  dev_base[ 6].alarm_flag = 0x0000;  dev_base[ 6].Value_VCC = 1804; dev_base[ 6].Value_VCC_min = 1693;  dev_base[ 6].time_of_life = 296; dev_base[ 6].Near_PHY_Add[0] = 0x16B0;  dev_base[ 6].Near_PHY_Add[1] = 0x13C2;  dev_base[ 6].Near_PHY_Add[2] = 0x0000;  dev_base[ 6].Near_PHY_Add[3] = 0x0000;
  dev_base[ 7].Type_Device =  BPI_A;   dev_base[ 7].Own_PHY_Addr = 0xFA44;  dev_base[ 7].alarm_flag = 0x0000;  dev_base[ 7].Value_VCC = 1430; dev_base[ 7].Value_VCC_min = 1405;  dev_base[ 7].time_of_life = 260; dev_base[ 7].Near_PHY_Add[0] = 0x0000;  dev_base[ 7].Near_PHY_Add[1] = 0x0000;  dev_base[ 7].Near_PHY_Add[2] = 0x0000;  dev_base[ 7].Near_PHY_Add[3] = 0x0000;
  dev_base[ 8].Type_Device = VRM_RS;   dev_base[ 8].Own_PHY_Addr = 0x13FE;  dev_base[ 8].alarm_flag = 0x0040;  dev_base[ 8].Value_VCC = 2046; dev_base[ 8].Value_VCC_min = 1956;  dev_base[ 8].time_of_life = 297; dev_base[ 8].Near_PHY_Add[0] = 0x14BC;  dev_base[ 8].Near_PHY_Add[1] = 0x105C;  dev_base[ 8].Near_PHY_Add[2] = 0x0000;  dev_base[ 8].Near_PHY_Add[3] = 0x0000;
  dev_base[ 9].Type_Device = VRM_RS;   dev_base[ 9].Own_PHY_Addr = 0x16E2;  dev_base[ 9].alarm_flag = 0x0000;  dev_base[ 9].Value_VCC = 1871; dev_base[ 9].Value_VCC_min = 1783;  dev_base[ 9].time_of_life = 298; dev_base[ 9].Near_PHY_Add[0] = 0x13CC;  dev_base[ 9].Near_PHY_Add[1] = 0x16B0;  dev_base[ 9].Near_PHY_Add[2] = 0x0000;  dev_base[ 9].Near_PHY_Add[3] = 0x0000;
  dev_base[10].Type_Device = PI_SPL;   dev_base[10].Own_PHY_Addr = 0xFDE6;  dev_base[10].alarm_flag = 0x0010;  dev_base[10].Value_VCC = 2235; dev_base[10].Value_VCC_min = 2233;  dev_base[10].time_of_life = 298; dev_base[10].Near_PHY_Add[0] = 0x16CE;  dev_base[10].Near_PHY_Add[1] = 0x12E6;  dev_base[10].Near_PHY_Add[2] = 0x0000;  dev_base[10].Near_PHY_Add[3] = 0xFA44;
  dev_base[11].Type_Device = VRM_RS;   dev_base[11].Own_PHY_Addr = 0x161A;  dev_base[11].alarm_flag = 0x0000;  dev_base[11].Value_VCC = 1874; dev_base[11].Value_VCC_min = 1772;  dev_base[11].time_of_life = 299; dev_base[11].Near_PHY_Add[0] = 0x1750;  dev_base[11].Near_PHY_Add[1] = 0x15DE;  dev_base[11].Near_PHY_Add[2] = 0x0000;  dev_base[11].Near_PHY_Add[3] = 0x0000;
  dev_base[12].Type_Device = VRM_RS;   dev_base[12].Own_PHY_Addr = 0x15A2;  dev_base[12].alarm_flag = 0x0000;  dev_base[12].Value_VCC = 1749; dev_base[12].Value_VCC_min = 1649;  dev_base[12].time_of_life = 300; dev_base[12].Near_PHY_Add[0] = 0x15F2;  dev_base[12].Near_PHY_Add[1] = 0x137C;  dev_base[12].Near_PHY_Add[2] = 0x0000;  dev_base[12].Near_PHY_Add[3] = 0x0000;
  dev_base[13].Type_Device = VRM_RS;   dev_base[13].Own_PHY_Addr = 0x105C;  dev_base[13].alarm_flag = 0x0008;  dev_base[13].Value_VCC = 2171; dev_base[13].Value_VCC_min = 2086;  dev_base[13].time_of_life = 270; dev_base[13].Near_PHY_Add[0] = 0x13FE;  dev_base[13].Near_PHY_Add[1] = 0x1BD8;  dev_base[13].Near_PHY_Add[2] = 0x0000;  dev_base[13].Near_PHY_Add[3] = 0x0000;
  dev_base[14].Type_Device = VRM_RS;   dev_base[14].Own_PHY_Addr = 0x1750;  dev_base[14].alarm_flag = 0x0000;  dev_base[14].Value_VCC = 1744; dev_base[14].Value_VCC_min = 1657;  dev_base[14].time_of_life = 270; dev_base[14].Near_PHY_Add[0] = 0x13C2;  dev_base[14].Near_PHY_Add[1] = 0x161A;  dev_base[14].Near_PHY_Add[2] = 0x0000;  dev_base[14].Near_PHY_Add[3] = 0x0000;
  dev_base[15].Type_Device = VRM_RS;   dev_base[15].Own_PHY_Addr = 0x1304;  dev_base[15].alarm_flag = 0x0000;  dev_base[15].Value_VCC = 1659; dev_base[15].Value_VCC_min = 1569;  dev_base[15].time_of_life = 270; dev_base[15].Near_PHY_Add[0] = 0x137C;  dev_base[15].Near_PHY_Add[1] = 0x1368;  dev_base[15].Near_PHY_Add[2] = 0x0000;  dev_base[15].Near_PHY_Add[3] = 0x0000;
  dev_base[16].Type_Device = VRM_RS;   dev_base[16].Own_PHY_Addr = 0x14BC;  dev_base[16].alarm_flag = 0x0000;  dev_base[16].Value_VCC = 1903; dev_base[16].Value_VCC_min = 1817;  dev_base[16].time_of_life = 270; dev_base[16].Near_PHY_Add[0] = 0xFCF6;  dev_base[16].Near_PHY_Add[1] = 0x13FE;  dev_base[16].Near_PHY_Add[2] = 0x0000;  dev_base[16].Near_PHY_Add[3] = 0x0000;
  dev_base[17].Type_Device = VRM_RS;   dev_base[17].Own_PHY_Addr = 0x1354;  dev_base[17].alarm_flag = 0x0000;  dev_base[17].Value_VCC = 2020; dev_base[17].Value_VCC_min = 1924;  dev_base[17].time_of_life = 270; dev_base[17].Near_PHY_Add[0] = 0xFDD2;  dev_base[17].Near_PHY_Add[1] = 0x13CC;  dev_base[17].Near_PHY_Add[2] = 0x0000;  dev_base[17].Near_PHY_Add[3] = 0x0000;
  dev_base[18].Type_Device = VRM_RS;   dev_base[18].Own_PHY_Addr = 0x12FA;  dev_base[18].alarm_flag = 0x0000;  dev_base[18].Value_VCC = 2105; dev_base[18].Value_VCC_min = 2019;  dev_base[18].time_of_life = 270; dev_base[18].Near_PHY_Add[0] = 0xFDD2;  dev_base[18].Near_PHY_Add[1] = 0x0328;  dev_base[18].Near_PHY_Add[2] = 0x0000;  dev_base[18].Near_PHY_Add[3] = 0x0000;
  dev_base[19].Type_Device = VRM_RS;   dev_base[19].Own_PHY_Addr = 0x171E;  dev_base[19].alarm_flag = 0x0000;  dev_base[19].Value_VCC = 1879; dev_base[19].Value_VCC_min = 1777;  dev_base[19].time_of_life = 271; dev_base[19].Near_PHY_Add[0] = 0x12E6;  dev_base[19].Near_PHY_Add[1] = 0x15F2;  dev_base[19].Near_PHY_Add[2] = 0x0000;  dev_base[19].Near_PHY_Add[3] = 0x0000;
  dev_base[20].Type_Device = VRM_RS;   dev_base[20].Own_PHY_Addr = 0x13C2;  dev_base[20].alarm_flag = 0x0000;  dev_base[20].Value_VCC = 1752; dev_base[20].Value_VCC_min = 1648;  dev_base[20].time_of_life = 271; dev_base[20].Near_PHY_Add[0] = 0x15B6;  dev_base[20].Near_PHY_Add[1] = 0x1750;  dev_base[20].Near_PHY_Add[2] = 0x0000;  dev_base[20].Near_PHY_Add[3] = 0x0000;
  dev_base[21].Type_Device = VRM_RS;   dev_base[21].Own_PHY_Addr = 0x16B0;  dev_base[21].alarm_flag = 0x0000;  dev_base[21].Value_VCC = 1822; dev_base[21].Value_VCC_min = 1720;  dev_base[21].time_of_life = 272; dev_base[21].Near_PHY_Add[0] = 0x16E2;  dev_base[21].Near_PHY_Add[1] = 0x15B6;  dev_base[21].Near_PHY_Add[2] = 0x0000;  dev_base[21].Near_PHY_Add[3] = 0x0000;
  dev_base[22].Type_Device = VRM_RS;   dev_base[22].Own_PHY_Addr = 0x1642;  dev_base[22].alarm_flag = 0x0000;  dev_base[22].Value_VCC = 1987; dev_base[22].Value_VCC_min = 1928;  dev_base[22].time_of_life = 273; dev_base[22].Near_PHY_Add[0] = 0xFD32;  dev_base[22].Near_PHY_Add[1] = 0x0000;  dev_base[22].Near_PHY_Add[2] = 0x0000;  dev_base[22].Near_PHY_Add[3] = 0x0000;
  dev_base[23].Type_Device = PI_SPL;   dev_base[23].Own_PHY_Addr = 0xFD32;  dev_base[23].alarm_flag = 0x0008;  dev_base[23].Value_VCC = 2168; dev_base[23].Value_VCC_min = 2165;  dev_base[23].time_of_life = 292; dev_base[23].Near_PHY_Add[0] = 0x13D6;  dev_base[23].Near_PHY_Add[1] = 0x1C64;  dev_base[23].Near_PHY_Add[2] = 0x1642;  dev_base[23].Near_PHY_Add[3] = 0xFABC;
  dev_base[24].Type_Device =  BPI_A;   dev_base[24].Own_PHY_Addr = 0xFA12;  dev_base[24].alarm_flag = 0x0000;  dev_base[24].Value_VCC = 1172; dev_base[24].Value_VCC_min = 1162;  dev_base[24].time_of_life = 270; dev_base[24].Near_PHY_Add[0] = 0x0000;  dev_base[24].Near_PHY_Add[1] = 0x0000;  dev_base[24].Near_PHY_Add[2] = 0x0000;  dev_base[24].Near_PHY_Add[3] = 0x0000;
  dev_base[25].Type_Device = VRM_RS;   dev_base[25].Own_PHY_Addr = 0x137C;  dev_base[25].alarm_flag = 0x0040;  dev_base[25].Value_VCC = 1701; dev_base[25].Value_VCC_min = 1598;  dev_base[25].time_of_life = 289; dev_base[25].Near_PHY_Add[0] = 0x15A2;  dev_base[25].Near_PHY_Add[1] = 0x1304;  dev_base[25].Near_PHY_Add[2] = 0x0000;  dev_base[25].Near_PHY_Add[3] = 0x0000;
  dev_base[26].Type_Device = PI_SPL;   dev_base[26].Own_PHY_Addr = 0xFABC;  dev_base[26].alarm_flag = 0x0000;  dev_base[26].Value_VCC = 6887; dev_base[26].Value_VCC_min = 3487;  dev_base[26].time_of_life = 280; dev_base[26].Near_PHY_Add[0] = 0x15A2;  dev_base[26].Near_PHY_Add[1] = 0x1304;  dev_base[26].Near_PHY_Add[2] = 0x0000;  dev_base[26].Near_PHY_Add[3] = 0x0000;
  dev_base[27].Type_Device = VRM_RS;   dev_base[27].Own_PHY_Addr = 0x12E6;  dev_base[27].alarm_flag = 0x0000;  dev_base[27].Value_VCC = 1989; dev_base[27].Value_VCC_min = 1899;  dev_base[27].time_of_life = 281; dev_base[27].Near_PHY_Add[0] = 0xFDE6;  dev_base[27].Near_PHY_Add[1] = 0x171E;  dev_base[27].Near_PHY_Add[2] = 0x0000;  dev_base[27].Near_PHY_Add[3] = 0x0000;
  dev_base[28].Type_Device = PI_SPL;   dev_base[28].Own_PHY_Addr = 0xFDD2;  dev_base[28].alarm_flag = 0x0008;  dev_base[28].Value_VCC = 2302; dev_base[28].Value_VCC_min = 2298;  dev_base[28].time_of_life = 286; dev_base[28].Near_PHY_Add[0] = 0x12FA;  dev_base[28].Near_PHY_Add[1] = 0x1C46;  dev_base[28].Near_PHY_Add[2] = 0x1354;  dev_base[28].Near_PHY_Add[3] = 0xFA12;
  dev_base[29].Type_Device = VRM_RS;   dev_base[29].Own_PHY_Addr = 0x0328;  dev_base[29].alarm_flag = 0x0000;  dev_base[29].Value_VCC = 2115; dev_base[29].Value_VCC_min = 2063;  dev_base[29].time_of_life = 275; dev_base[29].Near_PHY_Add[0] = 0x12FA;  dev_base[29].Near_PHY_Add[1] = 0x0000;  dev_base[29].Near_PHY_Add[2] = 0x0000;  dev_base[29].Near_PHY_Add[3] = 0x0000;
  dev_base[30].Type_Device = PI_SPL;   dev_base[30].Own_PHY_Addr = 0xFCF6;  dev_base[30].alarm_flag = 0x0020;  dev_base[30].Value_VCC = 2096; dev_base[30].Value_VCC_min = 2094;  dev_base[30].time_of_life = 286; dev_base[30].Near_PHY_Add[0] = 0x14E4;  dev_base[30].Near_PHY_Add[1] = 0x15DE;  dev_base[30].Near_PHY_Add[2] = 0x14BC;  dev_base[30].Near_PHY_Add[3] = 0xFA30;
  dev_base[31].Type_Device = VRM_RS;   dev_base[31].Own_PHY_Addr = 0x15F2;  dev_base[31].alarm_flag = 0x0000;  dev_base[31].Value_VCC = 1807; dev_base[31].Value_VCC_min = 1697;  dev_base[31].time_of_life = 287; dev_base[31].Near_PHY_Add[0] = 0x171E;  dev_base[31].Near_PHY_Add[1] = 0x15A2;  dev_base[31].Near_PHY_Add[2] = 0x0000;  dev_base[31].Near_PHY_Add[3] = 0x0000;
  dev_base[32].Type_Device = VRM_RS;   dev_base[32].Own_PHY_Addr = 0x16CE;  dev_base[32].alarm_flag = 0x0008;  dev_base[32].Value_VCC = 2084; dev_base[32].Value_VCC_min = 2000;  dev_base[32].time_of_life = 288; dev_base[32].Near_PHY_Add[0] = 0x13D6;  dev_base[32].Near_PHY_Add[1] = 0xFDE6;  dev_base[32].Near_PHY_Add[2] = 0x0000;  dev_base[32].Near_PHY_Add[3] = 0x0000;
  dev_base[33].Type_Device = VRM_RS;   dev_base[33].Own_PHY_Addr = 0x1368;  dev_base[33].alarm_flag = 0x0000;  dev_base[33].Value_VCC = 1653; dev_base[33].Value_VCC_min = 1557;  dev_base[33].time_of_life = 280; dev_base[33].Near_PHY_Add[0] = 0x1304;  dev_base[33].Near_PHY_Add[1] = 0x134A;  dev_base[33].Near_PHY_Add[2] = 0x0000;  dev_base[33].Near_PHY_Add[3] = 0x0000;
  dev_base[34].Type_Device =  BPI_A;   dev_base[34].Own_PHY_Addr = 0xFA30;  dev_base[34].alarm_flag = 0x0000;  dev_base[34].Value_VCC = 2168; dev_base[34].Value_VCC_min = 2165;  dev_base[34].time_of_life = 208; dev_base[34].Near_PHY_Add[0] = 0x0000;  dev_base[34].Near_PHY_Add[1] = 0x0000;  dev_base[34].Near_PHY_Add[2] = 0x0000;  dev_base[34].Near_PHY_Add[3] = 0x0000;
  dev_base[35].Type_Device =  VRM_V;   dev_base[35].Own_PHY_Addr = 0x1C64;  dev_base[35].alarm_flag = 0x0000;  dev_base[35].Value_VCC = 2168; dev_base[35].Value_VCC_min = 2165;  dev_base[35].time_of_life = 290; dev_base[35].Near_PHY_Add[0] = 0xFD32;  dev_base[35].Near_PHY_Add[1] = 0x0000;  dev_base[35].Near_PHY_Add[2] = 0x0000;  dev_base[35].Near_PHY_Add[3] = 0x0000;
  dev_base[36].Type_Device =  VRM_V;   dev_base[36].Own_PHY_Addr = 0x1C46;  dev_base[36].alarm_flag = 0x0000;  dev_base[36].Value_VCC = 2302; dev_base[36].Value_VCC_min = 2298;  dev_base[36].time_of_life = 290; dev_base[36].Near_PHY_Add[0] = 0xFDD2;  dev_base[36].Near_PHY_Add[1] = 0x0000;  dev_base[36].Near_PHY_Add[2] = 0x0000;  dev_base[36].Near_PHY_Add[3] = 0x0000;
*/
}                            

#endif /* TEST_GEN_TAB_DEV == 1 */

/**
  * @brief Функция обнуления записи в таблице устройств
  * @param diag_dev_t *dev_record - указатель на запись в таблице устройств
  * @retval none
  */
void ClearDev(diag_dev_t *dev_record)
{
  for ( uint8_t cntik_dev_byte = 0; cntik_dev_byte < sizeof(diag_dev_t); cntik_dev_byte++ )
  {
    ((uint8_t*)dev_record)[cntik_dev_byte] = 0;
  }
}

/**
  * @brief Функция обнуления всей таблицы устройств
  * @param none
  * @retval none
  */
void ClearTableDev( void )
{
  for ( uint16_t cntik_dev_rec = 0; cntik_dev_rec < MAX_TABLE_DEV; cntik_dev_rec++ )
  {
    ClearDev(&(dev_base[cntik_dev_rec]));
  }
}

/**
  * @brief Функция обнуления динамических параметров в таблице устройств
  * @param none
  * @retval none
  */
void ClearParamTableDev( void )
{
  /* Обнуление параметров у всех устройств кроме себя  */
  for ( uint16_t cntik_dev_rec = 1; cntik_dev_rec < MAX_TABLE_DEV; cntik_dev_rec++ )
  {
    if ( dev_base[cntik_dev_rec].Own_PHY_Addr != 0x0000 )
    {
      dev_base[cntik_dev_rec].Value_VCC = 0;       /* Напряжение питания  (за время *)            */
      dev_base[cntik_dev_rec].Value_VCC_min = 0;   /* Минимальное напряжение питания (за время *) */  
      dev_base[cntik_dev_rec].Near_PHY_Add[0] = 0; /* Адрес сосед по порту RS 1                   */
      dev_base[cntik_dev_rec].Near_PHY_Add[1] = 0; /* Адрес сосед по порту RS 2                   */
      dev_base[cntik_dev_rec].Near_PHY_Add[2] = 0; /* Адрес сосед по порту RS 3                   */
      dev_base[cntik_dev_rec].Near_PHY_Add[3] = 0; /* Адрес сосед по порту RS 4                   */
      dev_base[cntik_dev_rec].time_ping_req = 0;   /* Время запроса пинга                         */
      dev_base[cntik_dev_rec].time_ping_resp = 0;  /* Время ответа пинга                          */  
    }
  }
}


/**
  * @brief Функция запроса пинг устройства
  * @param uint16_t PHY_Addr - физический адрес  
  * @retval none
  */
void GetPingDev( uint16_t PHY_Addr )
{
  /* Формирование пакета */
  st_core.req_ping.rs_head.pre = 0xAA55;                                                 /* Преамбула  0x55 0xAA                          */
  st_core.req_ping.rs_head.lenght = sizeof(rs_req_ping_t) - SIZE_LENGHT - SIZE_PRE;      /* Длина пакета данных                           */
  st_core.req_ping.rs_head.id = ID_SETUP_REQ;                                            /* ID ответ команды конфигурации                 */
  st_core.req_ping.rs_head.dest = 0xFFFF;                                                /* Широковещательный  адрес получателя           */
  st_core.req_ping.rs_head.src = DataLoaderSto.Settings.phy_adr;                         /* Устанавливаем свой физический адрес источника */
  /* Физический адрес источника и Cчетчик неприрывности пакетов 0..255 заполняется функцией update_rs_tx_box */
  st_core.req_ping.rs_head.reserv = 0x00;      
  st_core.req_ping.rs_head.flag_rs_to_eth = 1;                                           /* Блокировка отправки в ETh */
  /* Заполнение тела пакета */
  st_core.req_ping.setup_head.factory_addr = PHY_Addr; 
  st_core.req_ping.setup_head.SETUP_ID = PING_CORE_PHY_ADDR;         
  st_core.req_ping.setup_head.N_port = Core_N_port;         
  st_core.req_ping.setup_head.Type = Core_Type;          
  
  /* Обновление RS пакета перед отправкой */
  update_rs_tx_box((void *)&(st_core.req_ping),&(st_core.contic_tx_box));
  
  /* Отправляетм пакет в роутер */
  if (st_core.xQueue_core_router != NULL)
  {
    /* отправить пакет в роутер */
    xQueueSend ( st_core.xQueue_core_router, ( void * )&(st_core.req_ping) , ( TickType_t ) 0 );
  }   
}

/**
  * @brief Функция управления запуска построения сети 
  * @param none
  * @retval none
  */
void UpdateProcDiagramDev( void )
{
  /* Отсчет счетчика построения диаграммы устройств */
  if (number_cycle_ping_dev == 0) return;
  
  /* Проверка выхода индексов за границы */
  if ( number_cycle_ping_dev > MAX_NUM_CYCLE_PING_DEV )
  {
    cnt_ping_dev = 1;
    number_cycle_ping_dev = 0;
    return;
  }
  
  if ( ( dev_base[cnt_ping_dev].Own_PHY_Addr == 0 ) || ( cnt_ping_dev >= MAX_TABLE_DEV ) )
  {
    /* Переход на следующий цикл */
    number_cycle_ping_dev--;
    cnt_ping_dev = 1;
    
    xTimerStop( st_core.xSoftTimerPing , 0 );
    xTimerChangePeriod( st_core.xSoftTimerPing, MAX_INTERAL_TEST_TABLE_DEV, 0 );
    xTimerStart( st_core.xSoftTimerPing , 0 ); 
    
  }
  else
  {
    if ( cnt_ping_dev == 1 )
  {
      xTimerStop( st_core.xSoftTimerPing , 0 );
      xTimerChangePeriod( st_core.xSoftTimerPing, MAX_INTERAL_TEST_DEV, 0 );
      xTimerStart( st_core.xSoftTimerPing , 0 ); 
    }
    
    /* Формируем пинг */
    GetPingDev(dev_base[cnt_ping_dev].Own_PHY_Addr);
              
    /* Сохранить врема запроса */    
    dev_base[cnt_ping_dev].time_ping_req = ulGetRuntimeCounterValue();
    
#if ( RS_PING_DEV_DEBUG == 1 )   
    printf( "|>> REQ_PING_DEV |%.2d|%.5d|%.6d|\r\n",
           cnt_ping_dev,
           dev_base[cnt_ping_dev].Own_PHY_Addr,
           dev_base[cnt_ping_dev].time_ping_req );
#endif      
   
    /* увеличиваем индекс */
    cnt_ping_dev++;
  }
}

/**
  * @brief Функция установки запуска построения сети 
  * @param none
  * @retval none
  */
void SetProcDiagramDev( void )
{
  /* Число циклов запроса пинга по RS */
  number_cycle_ping_dev = MAX_NUM_CYCLE_PING_DEV; 
  /* Сброс счетчика устройств внутри цикла запроса пинга по RS */
  cnt_ping_dev = 1; 
  /* Перезапуск таймера */
  xTimerStop( st_core.xSoftTimerPing , 0 );
  xTimerChangePeriod( st_core.xSoftTimerPing, MAX_INTERAL_TEST_DEV, 0 );
  xTimerStart( st_core.xSoftTimerPing , 0 ); 
}

/**
  * @brief Функция записи собственных параметров в таблицу устройств
  * @param uint16_t alarm_flag - флаги аварийных режимов                      
  * @param uint16_t Value_VCC - напряжение питания  (за время *)            
  * @param uint16_t Value_VCC_min - минимальное напряжение питания (за время *)  
  * @param uint16_t NearPhyAdd1 - адрес сосед по порту RS 1                       
  * @param uint16_t NearPhyAdd2 - адрес сосед по порту RS 2                       
  * @param uint16_t NearPhyAdd3 - адрес сосед по порту RS 3                       
  * @param uint16_t NearPhyAdd4 - адрес сосед по порту RS 4                       
  * @retval none
  */
void SetOwnDevTable( uint16_t alarm_flag, uint16_t Value_VCC, uint16_t Value_VCC_min, uint16_t NearPhyAdd1, uint16_t NearPhyAdd2, uint16_t NearPhyAdd3, uint16_t NearPhyAdd4 )
{
    dev_base[0].Own_PHY_Addr = DataLoaderSto.Settings.phy_adr; /* Собсвенный физический адрес                 */
    dev_base[0].Type_Device = DEVICE_TYPE;                     /* Тип устройсва                               */
    dev_base[0].alarm_flag = alarm_flag;                       /* Флаги аварийных режимов                     */
    dev_base[0].time_of_life = MAX_TIME_OF_LIFE_DEV;           /* Время жизни                                 */
    dev_base[0].Value_VCC = Value_VCC;                         /* Напряжение питания  (за время *)            */
    dev_base[0].Value_VCC_min = Value_VCC_min;                 /* Минимальное напряжение питания (за время *) */  
    dev_base[0].Near_PHY_Add[0] = NearPhyAdd1;                 /* Адрес сосед по порту RS 1                   */
    dev_base[0].Near_PHY_Add[1] = NearPhyAdd2;                 /* Адрес сосед по порту RS 2                   */
    dev_base[0].Near_PHY_Add[2] = NearPhyAdd3;                 /* Адрес сосед по порту RS 3                   */
    dev_base[0].Near_PHY_Add[3] = NearPhyAdd4;                 /* Адрес сосед по порту RS 4                   */    
    dev_base[0].time_ping_req = 0;                             /* Время запроса пинга                         */
    dev_base[0].time_ping_resp = 0;                            /* Время ответа пинга                          */      
}

/**
  * @brief Функция проверяет таблицу на наличие зарегистрированного устройства, если в таблице уже
  *        такой адрес устройства есть - заносим индекс, если нет, но есть место
  *        в таблице заносим индекс, если места нет - ответ нет места индекс нулевой
  *
  * @param uint16_t  AddrDev  - адрес позиционируемого устройства
  * @param uint8_t  *IndexTag  - указатель на индекс устройства в таблице
  * @retval uint8_t - статус возвращенного индекса
  *                   0 - нет индекса нет места.
  *                   1 - есть уже в таблице.
  *                   2 - нет идекса есть место в таблице 
  */
uint8_t GetIndexTableDev(uint16_t AddrDev, uint8_t *IndexTag )
{
  /* Цикл по таблице устройств */
  for ( uint8_t cntik_dev_rec = 0; cntik_dev_rec < MAX_TABLE_DEV; cntik_dev_rec++ )
  {
    if ( dev_base[cntik_dev_rec].Own_PHY_Addr == 0x0000 )
    {
      /* Установка индека */
      *IndexTag = cntik_dev_rec;
      /* Нет идекса есть место в таблице */
      return 2;
    }
    
    if ( dev_base[cntik_dev_rec].Own_PHY_Addr == AddrDev )
    {
      /* Установка индека */
      *IndexTag = cntik_dev_rec;
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
  * @brief Функция обновляет время жизни зарегистрированного устройства, если время жизни исчерпано
  *        удаляем устройство из таблицы
  *
  * @param none
  * @retval uint16_t - число зарегистрированных устройств в таблице
  */
uint16_t UpdateDevTable( void )
{
  uint16_t cntik_dev_rec = 0;
  uint16_t index_dev_offset_table = 0;  
 
#if  TEST_GEN_TAB_DEV == 1  
  /* Функция заполнения массива устройств для тестирования */
  TestGenTableDev( );  
#endif /* TEST_GEN_TAB_DEV == 1*/  
   
  /* Инициализация индекса обновления */
  cntik_dev_rec = 0;

  /* Выполняем обновление времени жизни рассылок */
  while( cntik_dev_rec < MAX_TABLE_DEV )
  {
    /* Если все активные устройства обновлены  - выход */
    if ( dev_base[cntik_dev_rec].Own_PHY_Addr == 0x0000 )  break;    
    
    /* Время жизни тега вышло */
    if ( ( dev_base[cntik_dev_rec].time_of_life ) > 0 )
    {
      /* Уменьшение время жизни */
      (dev_base[cntik_dev_rec].time_of_life)--;
    }
    else
    {
      /* Поиск последней записи */
      for ( index_dev_offset_table = cntik_dev_rec; index_dev_offset_table < ( MAX_TABLE_DEV - 1 ); index_dev_offset_table++ )
      {
        /* Если следующая запись пустая - выход */
        if ( dev_base[index_dev_offset_table+1].Own_PHY_Addr == 0x0000 ) break; 
      }
      /* Смещение данных идентификации устройства */             
      for ( uint8_t cntik_dev_byte = 0; cntik_dev_byte < sizeof(info_tag_t); cntik_dev_byte++ )
      {
        /* Копирование данных тега */
        ((uint8_t*)&(dev_base[cntik_dev_rec]))[cntik_dev_byte] = ((uint8_t*)&(dev_base[index_dev_offset_table]))[cntik_dev_byte];
        /* Обнуление дубликата записи в таблице */
        ((uint8_t*)&(dev_base[index_dev_offset_table]))[cntik_dev_byte] = 0;
      }
    }  
    /* Инкрементирование индексов тегов */
    cntik_dev_rec++;
  } 
  /* Число тегов в таблице */
  return cntik_dev_rec;
}

/**
  * @brief Функция запроса диагностики устройств
  * @param uint16_t PHY_Addr - физический адрес  
  * @param uint8_t Type - тип ресурса
  * @param uint8_t N_port - номер порта
  * @param uint8_t ID_DIAG - ID команды диагностики
  * @retval none
  */
void GetDiagDev( uint16_t PHY_Addr, uint8_t Type, uint8_t N_port, uint8_t ID_DIAG )
{
  /* Формирование пакета */
  st_core.req_diag.rs_head.pre = 0xAA55;                                                 /* Преамбула  0x55 0xAA                          */
  st_core.req_diag.rs_head.lenght = sizeof(req_diagnostics_t) - SIZE_LENGHT - SIZE_PRE;  /* Длина пакета данных                           */
  st_core.req_diag.rs_head.id = ID_DIAGNOSTICS_REQ;                                      /* ID ответ команды конфигурации                 */
  st_core.req_diag.rs_head.dest = 0xFFFF;                                                /* Широковещательный  адрес получателя           */
  st_core.req_diag.rs_head.src = DataLoaderSto.Settings.phy_adr;                         /* Устанавливаем свой физический адрес источника */
  /* Физический адрес источника и Cчетчик неприрывности пакетов 0..255 заполняется функцией update_rs_tx_box */
  st_core.req_diag.rs_head.reserv = 0x00;      
  st_core.req_diag.rs_head.flag_rs_to_eth = 1;                                           /* Блокировка отправки в ETh */
  /* Заполнение тела пакета */
  st_core.req_diag.Own_PHY_Addr = PHY_Addr;                                              /* Собсвенный физический адрес                   */
  st_core.req_diag.Type = Type;                                                          /* Тип ресурса                                   */
  st_core.req_diag.N_port = N_port;                                                      /* Номер порта                                   */
  st_core.req_diag.ID_DIAG = ID_DIAG;                                                    /* ID команды диагностики                        */
  
  /* Обновление RS пакета перед отправкой */
  update_rs_tx_box((void *)&(st_core.req_diag),&(st_core.contic_tx_box));
  
  /* Отправляетм пакет в роутер */
  if (st_core.xQueue_core_router != NULL)
  {
    /* отправить пакет в роутер */
    xQueueSend ( st_core.xQueue_core_router, ( void * )&(st_core.req_diag) , ( TickType_t ) 0 );
  }    
}

/**
  * @brief Функция парсинга пакета диагностики ID_CORE_WORK_ALARM
  * @param resp_core_work_alarm_t* rs_box - указатель на пакета диагностики  
  * @retval none
  */
void ProcessingDiagCoreWorkDev( resp_core_work_alarm_t* rs_box )
{
  uint8_t  IndexTag;  /* указатель на индекс устройства в таблице */
  
  /* Функция проверяет таблицу на наличие зарегистрированного устройства */
  switch ( GetIndexTableDev( rs_box->Own_PHY_Addr , &IndexTag ) )
  {
  case 1:
    /* 1 - есть уже в таблице. */
    /* Обновление времени жизни */   
    dev_base[IndexTag].time_of_life = MAX_TIME_OF_LIFE_DEV;

    return;
  case 2:
    /* 2 - нет идекса есть место в таблице */         
    /* Заноси данные в таблицу устройств */
    dev_base[IndexTag].Own_PHY_Addr = rs_box->Own_PHY_Addr; /* uint16_t Собсвенный физический адрес                */
    dev_base[IndexTag].Type_Device = rs_box->Type_Device;   /* uint8_t  Тип устройсва                              */
    dev_base[IndexTag].alarm_flag = rs_box->alarm_flag;     /* uint16_t Флаги аварийных режимов                    */
    dev_base[IndexTag].time_of_life = MAX_TIME_OF_LIFE_DEV; /* uint16_t Время жизни                                */
    
    return;
  default:
    /* 0 - нет индекса нет места. */
    /* Пакет обработан */
    return;
  }  
}

/**
  * @brief Функция парсинга и регистрации в таблице ответа на пинг
  * @param rs_resp_ping_t*  resp_ping_box - указатель на пакета пинга  
  * @retval none
  */
void ProcessingPingRespDev( rs_resp_ping_t*  resp_ping_box )
    {
  uint8_t  IndexTag;  /* указатель на индекс устройства в таблице */
  uint32_t time_temp_code;
  
  
  /* Функция проверяет таблицу на наличие зарегистрированного устройства */
  switch ( GetIndexTableDev( resp_ping_box->setup_head.factory_addr , &IndexTag ) )
  {
  case 1:
    /* 1 - есть уже в таблице. */
    if ( dev_base[IndexTag].time_ping_req > 0 )
    {
      /* Сохранение времени получения ответа */
      if ( dev_base[IndexTag].time_ping_req > 0 )
      {
        time_temp_code = ulGetRuntimeCounterValue() - dev_base[IndexTag].time_ping_req;
        if ( time_temp_code < 10000 )
        {
          dev_base[IndexTag].time_ping_resp = time_temp_code;
          dev_base[IndexTag].time_ping_req = 0; 
        }
    }  
    
#if ( RS_PING_DEV_DEBUG == 1 )   
      printf( "%.2d %.5d %.6d\r\n",  //printf( "|>> RESP_PING_DEV|%.2d|%.5d|%.6d|\r\n",
             IndexTag,
             dev_base[IndexTag].Own_PHY_Addr,
             dev_base[IndexTag].time_ping_resp );
#endif       
    }
    
    /* Запрос диагности Ядра */        
    GetDiagDev( dev_base[IndexTag].Own_PHY_Addr, TYPE_CORE_DAIG, N_PORT_CORE_DAIG, ID_CORE_DAIG );   
    
    return;
  default:
    /* Нет записи отправляем без обработки */
    /* Пакет обработан */
    return;
  }  
}

/**
  * @brief Функция парсинга пакета диагностики ID_CORE_DAIG
  * @param resp_core_diag_t* rs_box - указатель на пакета диагностики  
  * @retval none
  */
void ProcessingDiagCoreDev( resp_core_diag_t* rs_box )
{
  uint8_t  IndexTag;  /* указатель на индекс устройства в таблице */
  
  /* Функция проверяет таблицу на наличие зарегистрированного устройства */
  switch ( GetIndexTableDev( rs_box->Own_PHY_Addr , &IndexTag ) )
  {
  case 1:
    /* 1 - есть уже в таблице. */
    /* Обновление времени жизни */   
    dev_base[IndexTag].Value_VCC = rs_box->Value_VCC;          /* Напряжение питания  (за время *)            */
    dev_base[IndexTag].Value_VCC_min = rs_box->Value_VCC_min;  /* Минимальное напряжение питания (за время *) */  
    /* Запрос диагности UART */    
    /* В зависимости от типа устройсва - формирование диагности UART */
    switch ( rs_box->Type_Device )
    {
    case VRM_V:
    case BPI_A:  
    case VRM_RS:
    case CODEC:
    case PI_SPL:      
    case PI_ETH:
    case VRM_RS_U:
    case PGLR_CTRL:            
      /* У запрашиваемого типа устройств есть UART */
      break; 
    default:
      /* Нет UART */
      /* Пакет обработан */
      return;
    }   
    /* Запрос диагности UART */        
    GetDiagDev( rs_box->Own_PHY_Addr, TYPE_UART_DAIG, N_PORT_UART_DAIG, ID_UART_DAIG );    
    
    return;
  default:
    /* Нет записи отправляем без обработки */
    /* Пакет обработан */
    return;
  }  
}

/**
  * @brief Функция парсинга пакета диагностики ID_UART_DAIG
  * @param resp_uart_diag_t* rs_box - указатель на пакета диагностики  
  * @retval none
  */
void ProcessingDiagUartDev( resp_uart_diag_t* rs_box )
{
  uint8_t  IndexTag;  /* указатель на индекс устройства в таблице */
  
  /* Функция проверяет таблицу на наличие зарегистрированного устройства */
  switch ( GetIndexTableDev( rs_box->Own_PHY_Addr , &IndexTag ) )
  {
  case 1:
    /* 1 - есть уже в таблице. */
    if ( ( ( rs_box->N_port ) < 5 ) && ( ( rs_box->N_port ) > 0 ) )
    {
      /* Обновление времени жизни */   
      dev_base[IndexTag].Near_PHY_Add[( rs_box->N_port ) - 1] = rs_box->Connect_PHY_Addr;   
      /* В зависимости от типа устройсва - формирование диагности UART */
      switch ( rs_box->Type_Device )
      {
      case VRM_V:
      case BPI_A:  
        /* Один UART и он уже оработан */
        return;
      case VRM_RS:
      case CODEC:
      case PI_ETH:
      case VRM_RS_U:
      case PGLR_CTRL:  
      
        /* У запрашиваемого типа устройств два UART      */ 
        if ( rs_box->N_port > 1 ) return; 
        break;  
      case PI_SPL: 
        /* У запрашиваемого типа устройств четыре UART   */
        if ( rs_box->N_port > 3 ) return; 
        break; 
      default:
        /* Нет UART */
        /* Пакет обработан */
        return;
      }   
      GetDiagDev( rs_box->Own_PHY_Addr, TYPE_UART_DAIG, (rs_box->N_port) + 1, ID_UART_DAIG );   
    }
    return;
  default:
    /* Нет записи отправляем без обработки */
    /* Пакет обработан */
    return;
  }  
}

/**
  * @brief Функция парсинга пакета диагностики ID_UART_DAIG_V2
  * @param resp_uart_diag_v2_t* rs_box - указатель на пакета диагностики  
  * @retval none
  */
void ProcessingDiagUartDevV2( resp_uart_diag_v2_t* rs_box )
{
  uint8_t  IndexTag;  /* указатель на индекс устройства в таблице */
  
  /* Функция проверяет таблицу на наличие зарегистрированного устройства */
  switch ( GetIndexTableDev( rs_box->Own_PHY_Addr , &IndexTag ) )
  {
  case 1:
    /* 1 - есть уже в таблице. */
    if ( ( ( rs_box->N_port ) < 5 ) && ( ( rs_box->N_port ) > 0 ) )
    {
      /* Обновление адреса устройства подключаемого по UART */   
      dev_base[IndexTag].Near_PHY_Add[(rs_box->N_port) - 1] = rs_box->Connect_PHY_Addr;    
      /* В зависимости от типа устройсва - формирование диагности UART */
      switch ( rs_box->Type_Device )
      {
      case VRM_V:
      case BPI_A:  
        /* Один UART и он уже оработан */
        return;
      case VRM_RS:
      case CODEC:
      case PI_ETH:
      case VRM_RS_U:
      case PGLR_CTRL:  
      
        /* У запрашиваемого типа устройств два UART      */ 
        if ( rs_box->N_port > 1 ) return; 
        break;  
      case PI_SPL: 
        /* У запрашиваемого типа устройств четыре UART   */
        if ( rs_box->N_port > 3 ) return; 
        break; 
      default:
        /* Нет UART */
        /* Пакет обработан */
        return;
      }   
      GetDiagDev( rs_box->Own_PHY_Addr, TYPE_UART_DAIG, (rs_box->N_port) + 1, ID_UART_DAIG ); 
 
    }
    return;
  default:
    /* Нет записи отправляем без обработки */
    /* Пакет обработан */
    return;
  }  
}

/**
  * @brief Функция парсинга пакета ответ пинга RS 
  * @param router_box_t* rs_box - указатель на пакета диагностики  
  *
  * @retval bool true  - пакет обработан
  *              false - пакет не обработан
  */
bool ProcessinRespPing( router_box_t* rs_box )
{
  /* Объявляем указатель на шапку пакета */
  rs_resp_ping_t  *frame_resp_ping = (rs_resp_ping_t*)rs_box;
  
  if ( ( frame_resp_ping->rs_head.id ) == ID_SETUP_RESP )
  {
    /* Анализ идентификаторов пакета */
    if ( ( frame_resp_ping->setup_head.Type == Core_Type ) && ( frame_resp_ping->setup_head.N_port == Core_N_port ) && ( frame_resp_ping->setup_head.SETUP_ID == PING_CORE_PHY_ADDR ) ) 
    {
      /* Запрос функции вычисления времени запрос - ответ пинга */
      ProcessingPingRespDev(frame_resp_ping);      

      
      /* Пакет обработан */
      return true;
    } 
  }
  /* Oбработанных даных нет */
  return false;
}

/**
  * @brief Функция парсинга пакета диагностики 
  * @param router_box_t* rs_box - указатель на пакета диагностики  
  *
  * @retval bool true  - пакет обработан
  *              false - пакет не обработан
  */
bool ProcessingDiagDev( router_box_t* rs_box )
{
#if  TEST_GEN_TAB_DEV != 1    
  /* Объявляем указатель на шапку пакета */
  resp_diag_head_t  *frame_diag = (resp_diag_head_t*)rs_box;
 
  if ( ( frame_diag->rs_head.id ) == ID_DIAGNOSTICS_RESP )
  {
    /* Анализ пакета ответа диагностики */
    switch ( frame_diag->Type )
    {
    case TYPE_CORE_DAIG:
      /*  */
      if ( ( frame_diag->VERSION_ID ) == 0x01 )
      {
        /* Анализ пакета ответа диагностики */
        switch ( frame_diag->ID_DIAG )
        {
        case ID_CORE_WORK_ALARM:
          /* парсинга пакета диагностики ID_CORE_WORK_ALARM */ 
          ProcessingDiagCoreWorkDev( (resp_core_work_alarm_t*)rs_box );        
          return true;
        case ID_CORE_DAIG:
          /* Функция парсинга пакета диагностики ID_CORE_DAIG */
          ProcessingDiagCoreDev( (resp_core_diag_t*)rs_box );        
          return true;
        default:
          /* Пакет обработан */
          return true;
        } 
      }
      return true;
    case TYPE_UART_DAIG:
      if ( ( frame_diag->ID_DIAG ) == ID_UART_DAIG ) 
      {
        if ( ( frame_diag->VERSION_ID ) == 0x01 )
        {
          /* Функция парсинга пакета диагностики ID_UART_DAIG */
          ProcessingDiagUartDev( (resp_uart_diag_t*)rs_box );
        }        
        
        if ( ( frame_diag->VERSION_ID ) == 0x02 )
        {
          /* Функция парсинга пакета диагностики ID_UART_DAIG_V2 */
          ProcessingDiagUartDevV2( (resp_uart_diag_v2_t*)rs_box ); 
        }
      }
      return true;
    default:
      /* Пакет обработан */
      return true;
    } 
  }
#endif  /*  TEST_GEN_TAB_DEV == 1  */  
  /* Oбработанных даных нет */
  return false;
}

/* Таблица соединений устройств RS */
con_dev_t  con_base[MAX_TABLE_CON];
uint8_t max_index_stolb;       /* Индекс столба     */

/**
  * @brief Функция обнуления битов обработки таблицы устройств
  * @param none
  * @retval none
  */
void ClearFlagProcDev(void)
{
  for ( uint16_t cntik_dev_rec = 0; cntik_dev_rec < MAX_TABLE_DEV; cntik_dev_rec++ )
  {
    dev_base[cntik_dev_rec].flag_proc[0] = false;
    dev_base[cntik_dev_rec].flag_proc[1] = false;  
    dev_base[cntik_dev_rec].flag_proc[2] = false; 
    dev_base[cntik_dev_rec].flag_proc[3] = false;     
  }
}

/**
  * @brief Функция обнуления таблицы соединения устройств
  * @param none
  * @retval none
  */
void ClearConectTab(void)
{
  for ( uint16_t cntik_dev_con = 0; cntik_dev_con < MAX_TABLE_CON; cntik_dev_con++ )
  {
    con_base[cntik_dev_con].index_dev_1 = 0;            /* физический адрес 1                 */
    con_base[cntik_dev_con].n_port_dev_1 = 0;           /* Тип устройсва                      */      
    con_base[cntik_dev_con].index_dev_2 = 0;            /* физический адрес 1                 */    
    con_base[cntik_dev_con].n_port_dev_2 = 0;           /* Тип устройсва                      */      
    con_base[cntik_dev_con].index_stolb = 0;            /* Индекс строки                      */    
    con_base[cntik_dev_con].index_strok = 0;            /* Индекс столба                      */      
  }
}

/**
  * @brief Функция поиска соседа для указанного порта, и формирование указателя для 
  * @param uint8_t* index_dev - указатель на индекс текущего устройсва
  * @param uint8_t* index_ptr - указатель на индекс текущего порта
  * @param uint8_t* index_stlb - указатель на индекс текущего столбца
  * @param uint8_t* index_strk - указатель на индекс текущей строки 
  * @param uint8_t* index_con - указатель на индекс текущего соединения в таблице соединений 
  * @param uint8_t* flag_update - указатель на флаг обработки таблицы соединений 
  * @retval bool - true цепочка продолжается
  *                false - цепочка прерывается   
  */
bool FindNearDev( uint8_t* index_dev, uint8_t* index_ptr, uint8_t* index_stlb, uint8_t* index_strk, uint8_t* index_con, uint8_t* flag_update )
{                 
  uint8_t new_index_dev;
 
  if (dev_base[*index_dev].flag_proc[*index_ptr] == true) 
  {/* если порт обработан - выход               */
    return false;
  }
  /* установка флага обработки порта */
  dev_base[*index_dev].flag_proc[*index_ptr] = true;
  /* проверяем наличие соседа у порта */
  if ( ( dev_base[*index_dev].Near_PHY_Add[*index_ptr] ) == 0x0000 )
  {/* соседа нет - выход               */
    return false;
  }
  else
  {/* сосед есть - получить индекс соседа */
    if ( ( GetIndexTableDev( dev_base[*index_dev].Near_PHY_Add[*index_ptr], &new_index_dev ) ) != 1 )
    {/* соседа нет в таблице - выход               */
      return false;
    }
    else
    {
      /* Если это блок питания - пропишем подключение к первому порту */
      if  ( ( dev_base[new_index_dev].Type_Device == BPI_A ) || ( dev_base[new_index_dev].Type_Device == VRM_V ) )
      {
          /* Установка флага обработки порта */
          dev_base[new_index_dev].flag_proc[0] = true;
          /* Заносим данные по соединению в таблицу */
          con_base[*index_con].index_dev_1 = *index_dev;      /* физический адрес 1                 */
          con_base[*index_con].n_port_dev_1 = *index_ptr;     /* Тип устройсва                      */ 
          con_base[*index_con].index_dev_2 = new_index_dev;   /* физический адрес 1                 */ 
          con_base[*index_con].n_port_dev_2 = 0;              /* Тип устройсва                      */ 
          con_base[*index_con].index_stolb = *index_stlb;     /* Индекс строки                      */ 
          con_base[*index_con].index_strok = *index_strk;     /* Индекс столба                      */ 
          /* Инкрементируем индекс соединений */
          (*index_con)++;
          /* Инкрементируем строку */         
          (*index_strk)++;
          /* Установка флага обработки данных */
          *flag_update = 1;
          /* Больше у блока питания нет соседей - выход */
          return false;
      }
 
      /* Цикл по всем портам устройства */
      for( uint8_t cntik_con_port = 0;  cntik_con_port < 4; cntik_con_port++ )
      {
        if (  ( dev_base[new_index_dev].Near_PHY_Add[cntik_con_port] ) == dev_base[*index_dev].Own_PHY_Addr )
        {
          /* Установка флага обработки порта */
          dev_base[new_index_dev].flag_proc[cntik_con_port] = true;
          /* Заносим данные по соединению в таблицу */
          con_base[*index_con].index_dev_1 = *index_dev;      /* физический адрес 1                 */
          con_base[*index_con].n_port_dev_1 = *index_ptr;     /* Тип устройсва                      */ 
          con_base[*index_con].index_dev_2 = new_index_dev;   /* физический адрес 1                 */ 
          con_base[*index_con].n_port_dev_2 = cntik_con_port; /* Тип устройсва                      */ 
          con_base[*index_con].index_stolb = *index_stlb;     /* Индекс строки                      */ 
          con_base[*index_con].index_strok = *index_strk;     /* Индекс столба                      */ 
          /* Инкрементируем индекс соединений */
          (*index_con)++;
          /* Инкрементируем строку */         
          (*index_strk)++;
          /* Установка флага обработки данных */
          *flag_update = 1;
          /* Формируем индекс для следующего по цепочке */
          (*index_dev) = new_index_dev;
          /* поиск у соседа необработаный порта с соседом  */
          for( uint8_t cntik_new_port = 0;  cntik_new_port < 4; cntik_new_port++ )
          {
            /* Если есть не обработаный порт с соседом */
            if ( ( ( dev_base[*index_dev].flag_proc[cntik_new_port] ) != true ) && ( ( dev_base[*index_dev].Near_PHY_Add[cntik_new_port] ) != 0x0000 ) && ( GetIndexTableDev( dev_base[*index_dev].Near_PHY_Add[cntik_new_port], &new_index_dev ) == 1 ) )
            {
              /* выдаем данные этого порта как следующего соединения с цепочке */  
              (*index_ptr) = cntik_new_port;
              return true;
            }
          }
        }
      }
    }  
  }  
  return false;
}

/**
  * @brief Функция обработки таблицы соединения устройств
  * @param none
  * @retval none
  */
void GenTableConect( void )
{
  uint8_t index_stolb = 1;       /* Индекс строки     */
  uint8_t index_strok = 0;       /* Индекс столба     */  
  uint8_t index_device = 0;      /* Индекс устройcтва */
  uint8_t index_port = 0;        /* Индекс порта устройcтва */ 
  uint8_t index_connect = 0;     /* Индекс в таблице соединений */  
  uint8_t cntik_con_dev = 0;     /* Счетчик индексов устройств */  
  uint8_t flag_tab_con_proc = 0; /* Флаг обработки данных */
  
  /* Обнуления битов обработки таблицы устройств */
  ClearFlagProcDev();
  /* Обнуления таблицы соединения устройств */
  ClearConectTab();  
  /* Начинаем с первого столба */
  max_index_stolb = index_stolb;
  /* Обрабатываем таблицу пока не отработаем все связи */
  cntik_con_dev = 0;   
  /* Заносим индекс нулевого элемента в таблицу соединений */
  con_base[cntik_con_dev].index_dev_1 = 0;                    /* физический адрес 1                 */
  con_base[cntik_con_dev].n_port_dev_1 = 1;                   /* Тип устройсва                      */
  con_base[cntik_con_dev].index_stolb = index_stolb;          /* Индекс столба                      */
  con_base[cntik_con_dev].index_strok = 1;                    /* Индекс строки                      */  
 
  /* Цикл по всем активным устройствам */  
  while( ( con_base[cntik_con_dev].index_stolb > 0 )&&( cntik_con_dev < MAX_TABLE_CON ) )
  {   
    /* Обнуление флага обработки данных */
    flag_tab_con_proc = 0;
    
    /* Цикл по всем портам устройства */
    for( uint8_t cntik_con_port = 0;  cntik_con_port < 4; cntik_con_port++ )
    {
      if ( ( ( dev_base[con_base[cntik_con_dev].index_dev_1].flag_proc[cntik_con_port] ) == true ) || ( ( dev_base[con_base[cntik_con_dev].index_dev_1].Near_PHY_Add[cntik_con_port] ) == 0x0000 ) )
      {/* Если порт обработан - пропуск */
        
      }
      else
      { 
        /* Поиск индекса строки */
        index_strok = con_base[cntik_con_dev].index_strok; 
        /* Инициализация индекса стартового устройства цепочки */
        index_device = con_base[cntik_con_dev].index_dev_1;
        /* Инициализация номера порта стартового устройства цепочки */
        index_port = cntik_con_port;
        /* Цикл формирования цепочки */ 
        while( ( FindNearDev ( &index_device, &index_port, &index_stolb, &index_strok, &index_connect, &flag_tab_con_proc ) ) == true  );
        /* Проверка флага обработки данных */
        if ( flag_tab_con_proc > 0 )
        {
          /* Переход на новый столбец */
          index_stolb = index_stolb + 1;
        }      
      }
    }
    if ( flag_tab_con_proc > 0)
    {
      /* Если таблица изменялась - начнем снчала */
      cntik_con_dev = 0;
    }
    else
    {  
      /* Инкрементирование индекса */
      cntik_con_dev++;
    }
  }
  /* Сохранение максимального индекса столба */
  max_index_stolb = index_stolb - 1;
}

uint16_t TempCntDev = 0;
uint16_t CntDev = 0;
/* Флаг события при обработке */
uint8_t flag_proc;  
  
/**
  * @brief  Функция формирования статуса зарегистрированных устройств в RS
  * @param  char *pcInsert - указатель на массив pcInsert для формируемой строки
  * @param  enum_type_stage_t stage - режим вызова STAT_TITLE - выдача имени страницы
  * 						   STAT_HEAD - выдача заголовка таблицы
  * 						   STAT_LINE - выдача строки таблицы
  *
  *
  * @retval uint16_t длина формируемой строки
  */
uint16_t TabStatusDevRS(char *pcInsert, enum_type_stage_t stage)
{
  /* Переменная для отображения типа устройства */
  char type_name[10];   

  switch (stage)
  {
  case STAT_TITLE:
    /* Вывод наименования таблицы */
    return  sprintf(pcInsert,"Таблица зарегистрированных устройств в сегменте RS");
  case STAT_HEAD:
    /* Вывод шапки таблицы        */
    CntDev = 0;
    return sprintf(pcInsert,"<tr><td>Type</td><td>PhyAdr</td><td>AlarmFlag</td><td>VCC</td><td>VCCmin</td><td>TOFL</td><td>NearAdd_1</td></td><td>NearAdd_2</td><td>NearAdd_3</td><td>NearAdd_4</td><td>Time Ping</td></td></tr>");
  case STAT_LINE:
    /* Вывод строк таблицы        */
    while( (dev_base[CntDev].Own_PHY_Addr > 0) && ( CntDev < MAX_TABLE_DEV ) ) 
    { /* Сохраняем индекс записи       */
      TempCntDev = CntDev;
      /* Инкрементируем счетчик записи */               
      CntDev++;
         
      switch (dev_base[TempCntDev].Type_Device)
      {
           case VRM_Mesh:                      
              sprintf(type_name,"ВРМ-ME  "); 
              break;
            case VRM_RS:                     
              sprintf(type_name,"VRM-RS  ");    
              break; 
            case VRM_V:                    
              sprintf(type_name,"VRM-V   ");     
              break; 
            case PDH_B:                    
              sprintf(type_name,"PDH-B   ");    
              break;
            case PI_SPL:                      
              sprintf(type_name,"PI-SPL  ");    
              break;
            case BPI_A:             
              sprintf(type_name,"BPI_А   ");    
              break;        
            case VRM_RS_U:                    
              sprintf(type_name,"VRM-AMP ");    
              break;
            case PGLR_CTRL:                    
              sprintf(type_name,"PGLR_CL ");    
              break;       
            case PI_ETH:            
              sprintf(type_name,"PI-Eth  ");    
              break;               
            case CODEC: 
              sprintf(type_name,"CODEC   ");    
              break;       
            default:                                   
              sprintf(type_name,"");           
              break;
      };            
      
      return sprintf(pcInsert,"<tr><td>%.7s</td><td>%.5d</td><td>0x%.4X</td><td>%5.2f</td><td>%5.2f</td><td>%.3d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%.5d</td><td>%5.1f</td></tr>",
                         type_name,  
                         dev_base[TempCntDev].Own_PHY_Addr,         
                         dev_base[TempCntDev].alarm_flag,           
                         ((float)(dev_base[TempCntDev].Value_VCC))/(100.0),            
                         ((float)(dev_base[TempCntDev].Value_VCC_min))/(100.0),                    
                         dev_base[TempCntDev].time_of_life,               
                         dev_base[TempCntDev].Near_PHY_Add[0], 
                         dev_base[TempCntDev].Near_PHY_Add[1],                     
                         dev_base[TempCntDev].Near_PHY_Add[2], 
                     dev_base[TempCntDev].Near_PHY_Add[3],
                     ((float)(dev_base[TempCntDev].time_ping_resp)/(10.0)));       
                     
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
  * @brief  Функция формирования статуса соединений устройств в RS
  * @param  char *pcInsert - указатель на массив pcInsert для формируемой строки
  * @param  enum_type_stage_t stage - режим вызова STAT_TITLE - выдача имени страницы
  * 						   STAT_HEAD - выдача заголовка таблицы
  * 						   STAT_LINE - выдача строки таблицы
  *
  *
  * @retval uint16_t длина формируемой строки
  */
uint16_t TabConDevRS(char *pcInsert, enum_type_stage_t stage)
{
  
  /* обработка таблицы соединения устройств */
  GenTableConect();

  switch (stage)
  {
  case STAT_TITLE:
    /* Вывод наименования таблицы */
    return  sprintf(pcInsert,"Таблица соединений устройств в сегменте RS");
  case STAT_HEAD:
    /* Вывод шапки таблицы        */
    CntDev = 0;
    return sprintf(pcInsert,"<tr><td>IndexDev1</td><td>PortDev1</td><td>IndexDev2</td><td>PortDev2</td><td>IndexStlb</td><td>IndexStrk</td></tr>");
  case STAT_LINE:
    /* Вывод строк таблицы        */
    while( ( con_base[CntDev].index_stolb > 0) && ( CntDev < MAX_TABLE_CON ) ) 
    { /* Сохраняем индекс записи       */
      TempCntDev = CntDev;
      /* Инкрементируем счетчик записи */               
      CntDev++;
    
      return sprintf(pcInsert,"<tr><td>%.3d</td><td>%.3d</td><td>%.3d</td><td>%.3d</td><td>%.3d</td><td>%.3d</td></tr>",
                     con_base[TempCntDev].index_dev_1, 
                     con_base[TempCntDev].n_port_dev_1,
                     con_base[TempCntDev].index_dev_2, 
                     con_base[TempCntDev].n_port_dev_2,    
                     con_base[TempCntDev].index_stolb,                 
                     con_base[TempCntDev].index_strok);
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
  * @brief  Функция формирования схемы соединений устройств в RS
  * @param  char *pcInsert - указатель на массив pcInsert для формируемой строки
  * @param  enum_type_stage_t stage - режим вызова STAT_TITLE - выдача имени страницы
  * 						   STAT_HEAD - выдача заголовка таблицы
  * 						   STAT_LINE - выдача строки таблицы
  *
  *
  * @retval uint16_t длина формируемой строки
  */
uint16_t TabDiagramConDevRS(char *pcInsert, enum_type_stage_t stage)
{
  /* Переменая длинны выводимой строки */
  uint16_t temp_size;
  uint8_t flag_print;  
  /* Переменная для отображения типа устройства */
  char type_name[10];     
  /* обработка таблицы соединения устройств */
  GenTableConect();
  /* Переменная индека */ 
  uint8_t cnt_str;
  
  switch (stage)
  {
  case STAT_TITLE:
    /* Вывод наименования таблицы */
    return  sprintf(pcInsert,"Таблица диаграмма соединений устройств в сегменте RS");
  case STAT_HEAD:
    
    CntDev = 1;
    flag_proc = 1;
    temp_size = sprintf(pcInsert,"<tr>");  
    /* Вывод шапки таблицы        */
    for ( uint8_t cnt_slb = 1; cnt_slb <= max_index_stolb; cnt_slb++ )
    {
      temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>==============================</td>");
    }
    return  temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"</tr>"); 
    
  case STAT_LINE:
    /* Проверка флага события */
    if ( flag_proc > 0 )
    { 
      /* Сброс флага события */
      flag_proc = 0; 
      /* Сохраняем индекс записи       */
      TempCntDev = CntDev;
      /* Инкрементируем счетчик записи */               
      CntDev++;
      temp_size = sprintf(pcInsert,"<tr>");       
      for ( uint8_t cnt_slb = 1; cnt_slb <= max_index_stolb; cnt_slb++ )
      {
        /* Сброс флага события ┐└ ┘┌ */
        flag_print = 0; 
        /* Инициализация индекса цикла */
        cnt_str = 0;
        /*  Цикл по таблице соединений */
        while ( ( con_base[cnt_str].index_stolb > 0 )&& ( cnt_str < MAX_TABLE_CON ) )
        {
          if ( ( con_base[cnt_str].index_stolb == cnt_slb ) && ( con_base[cnt_str].index_strok == TempCntDev ) )
          { 
            switch (dev_base[con_base[cnt_str].index_dev_1].Type_Device)
            {  
           case VRM_Mesh:                      
              sprintf(type_name,"ВРМ-ME  "); 
              break;
            case VRM_RS:                     
              sprintf(type_name,"VRM-RS  ");    
              break; 
            case VRM_V:                    
              sprintf(type_name,"VRM-V   ");     
              break; 
            case PDH_B:                    
              sprintf(type_name,"PDH-B   ");    
              break;
            case PI_SPL:                      
              sprintf(type_name,"PI-SPL  ");    
              break;
            case BPI_A:             
              sprintf(type_name,"BPI_А   ");    
              break;        
            case VRM_RS_U:                    
              sprintf(type_name,"VRM-AMP ");    
              break;
            case PGLR_CTRL:                    
              sprintf(type_name,"PGLR_CL ");    
              break;       
            case PI_ETH:            
              sprintf(type_name,"PI-Eth  ");    
              break;               
            case CODEC: 
              sprintf(type_name,"CODEC   ");    
              break;       
            default:                                   
              sprintf(type_name,"");           
              break;
            };
            /* Формирование текстового обозначения соединения */
            temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>┌%.5d P%.1d %5.1fms %4.2fV %.7s</td>",
                                            dev_base[con_base[cnt_str].index_dev_1].Own_PHY_Addr,
                                            con_base[cnt_str].n_port_dev_1,                                           
                                            ((float)(dev_base[con_base[cnt_str].index_dev_1].time_ping_resp))/(10.0),
                                            ((float)(dev_base[con_base[cnt_str].index_dev_1].Value_VCC))/(100.0),
                                            type_name);
            /* Установка флага события */
            flag_proc = 1; 
            flag_print = 1;             
          }
          /* инкремент индекса */
          cnt_str++;
        }
        if ( flag_print == 0 )
        {
          temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>---</td>");        
        }
      }
      temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"</tr><tr>"); 
      
      for ( uint8_t cnt_slb = 1; cnt_slb <= max_index_stolb; cnt_slb++ )
      {
        /* Сброс флага события ┐└ ┘┌ */
        flag_print = 0; 
        /* Инициализация индекса цикла */
        cnt_str = 0;
        /*  Цикл по таблице соединений */
        while ( ( con_base[cnt_str].index_stolb > 0 )&& ( cnt_str < MAX_TABLE_CON ) )
        {
          if ( ( con_base[cnt_str].index_stolb == cnt_slb ) && ( con_base[cnt_str].index_strok == TempCntDev ) )
          { 
            switch (dev_base[con_base[cnt_str].index_dev_2].Type_Device)
            {  
            case VRM_Mesh:                      
              sprintf(type_name,"ВРМ-ME  "); 
              break;
            case VRM_RS:                     
              sprintf(type_name,"VRM-RS  ");    
              break; 
            case VRM_V:                    
              sprintf(type_name,"VRM-V   ");     
              break; 
            case PDH_B:                    
              sprintf(type_name,"PDH-B   ");    
              break;
            case PI_SPL:                      
              sprintf(type_name,"PI-SPL  ");    
              break;
            case BPI_A:             
              sprintf(type_name,"BPI_А   ");    
              break;        
            case VRM_RS_U:                    
              sprintf(type_name,"VRM-AMP ");    
              break;
            case PGLR_CTRL:                    
              sprintf(type_name,"PGLR_CL ");    
              break;       
            case PI_ETH:            
              sprintf(type_name,"PI-Eth  ");    
              break;               
            case CODEC: 
              sprintf(type_name,"CODEC   ");    
              break;       
            default:                                   
              sprintf(type_name,"");           
              break;
            };
            /* Формирование текстового обозначения соединения */
            temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>└%.5d P%.1d %5.1fms %4.2fV %.7s</td>",
                                            dev_base[con_base[cnt_str].index_dev_2].Own_PHY_Addr,
                                            con_base[cnt_str].n_port_dev_2,                                           
                                            ((float)(dev_base[con_base[cnt_str].index_dev_2].time_ping_resp))/(10.0), 
                                            ((float)(dev_base[con_base[cnt_str].index_dev_2].Value_VCC))/(100.0),
                                            type_name);
            /* Установка флага события */
            flag_proc = 1; 
            flag_print = 1;             
          }
          /* инкремент индекса */
          cnt_str++;
        }
        if ( flag_print == 0 )
        {
          temp_size = temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"<td>---</td>");        
        }
      }
      if ( flag_proc > 0 )
      {
        return  temp_size + sprintf((char *)(pcInsert + (uint32_t)temp_size),"</tr>");      
      }
      else
      {
        /* Печатаем пустое поле */
        pcInsert = "";
        return 0; 
      }
    }
    else
    {
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

/******************* (C) COPYRIGHT 2020 DataExpress *****END OF FILE****/
