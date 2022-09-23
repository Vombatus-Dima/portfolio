/**
  ******************************************************************************
  * @file    codec_i2c.c
  * @author  Trembach Dmitry
  * @date    20-07-2020
  * @brief   Функции инициализации интерфейса i2c управления кодеком UDA1380
  *
  ******************************************************************************
  * @attention
  * 
  *
  * <h2><center>&copy; COPYRIGHT 2020 DataExpress</center></h2>
  ******************************************************************************
  */

#include "codec_i2c.h"

/**
 * @brief  Функция задержки с 100 нс шагом. 
 * @param  uint16_t time_val
 * @retval None
  ******************************************************************************
  * @attention
  * результаты измерения
  *  time_val=3     - 0.29 мкс 
  *  time_val=100   - 10.1 мкс
  *     ошибка ~ 1%
  ******************************************************************************
 */
#pragma optimize=none 
void i2c_ns100_delay(uint16_t time_val)
{
  for(uint16_t cnt_time = 1;cnt_time < time_val;cnt_time++)
  {
    asm("nop");  asm("nop");   asm("nop");  asm("nop");       
    asm("nop");  asm("nop");   asm("nop");  asm("nop");    
  }
}

/* This function issues a start condition and 
 * transmits the slave address + R/W bit
 * 
 * Parameters:
 * 		I2Cx --> the I2C peripheral e.g. I2C1
 * 		address --> the 7 bit slave address
 * 		direction --> the tranmission direction can be:
 * 						I2C_Direction_Tranmitter for Master transmitter mode
 * 						I2C_Direction_Receiver for Master receiver
 */
void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction){
  // wait until I2C1 is not busy anymore
  while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
  
  // Send I2C1 START condition 
  I2C_GenerateSTART(I2Cx, ENABLE);
  
  // wait for I2C1 EV5 --> Slave has acknowledged start condition
  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
  
  // Send slave Address for write 
  I2C_Send7bitAddress(I2Cx, address, direction);
  
  /* wait for I2C1 EV6, check if 
  * either Slave has acknowledged Master transmitter or
  * Master receiver mode, depending on the transmission
  * direction
  */ 
  if(direction == I2C_Direction_Transmitter){
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
  }
  else if(direction == I2C_Direction_Receiver){
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
  }
}

/* This function transmits one byte to the slave device
 * Parameters:
 *		I2Cx --> the I2C peripheral e.g. I2C1 
 *		data --> the data byte to be transmitted
 */
void I2C_write(I2C_TypeDef* I2Cx, uint8_t data)
{
  I2C_SendData(I2Cx, data);
  // wait for I2C1 EV8_2 --> byte has been transmitted
  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

/* This function reads one byte from the slave device 
 * and acknowledges the byte (requests another byte)
 */
uint8_t I2C_read_ack(I2C_TypeDef* I2Cx){
  // enable acknowledge of recieved data
  I2C_AcknowledgeConfig(I2Cx, ENABLE);
  // wait until one byte has been received
  while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) );
  // read data from I2C data register and return data byte
  uint8_t data = I2C_ReceiveData(I2Cx);
  return data;
}

/* This function reads one byte from the slave device
 * and doesn't acknowledge the recieved data 
 */
uint8_t I2C_read_nack(I2C_TypeDef* I2Cx){
  // disabe acknowledge of received data
  // nack also generates stop condition after last byte received
  // see reference manual for more info
  I2C_AcknowledgeConfig(I2Cx, DISABLE);
  I2C_GenerateSTOP(I2Cx, ENABLE);
  // wait until one byte has been received
  while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) );
  // read data from I2C data register and return data byte
  uint8_t data = I2C_ReceiveData(I2Cx);
  return data;
}

/* This funtion issues a stop condition and therefore
 * releases the bus
 */
void I2C_stop(I2C_TypeDef* I2Cx){
  // Send I2C1 STOP Condition 
  I2C_GenerateSTOP(I2Cx, ENABLE);
}

/**
  * @brief  Функция инициализации порта I2C 
  * @param  None
  * @retval None
  */
void CodecI2CInit(void)
{
  I2C_InitTypeDef  I2C_InitStructure;
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* Включение тактирования выводов */
  RCC_AHB1PeriphClockCmd(CODEC_I2C_SDA_GPIO_CLK | CODEC_I2C_SCL_GPIO_CLK, ENABLE);
  /* Включение тактирования интерфейса I2C */
  CODEC_CLK_INIT(CODEC_I2C_CLK, ENABLE);
  /* Конфигурирование альтернативной функции выводов */  
  GPIO_PinAFConfig(CODEC_I2C_SDA_GPIO_PORT, CODEC_I2C_SDA_SOURCE, CODEC_I2C_SDA_AF);
  GPIO_PinAFConfig(CODEC_I2C_SCL_GPIO_PORT, CODEC_I2C_SCL_SOURCE, CODEC_I2C_SCL_AF);
  
  /* Инициализация выводов */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  /* Инициализация вывода SDA */
  GPIO_InitStructure.GPIO_Pin =  CODEC_I2C_SDA_PIN;
  GPIO_Init(CODEC_I2C_SDA_GPIO_PORT, &GPIO_InitStructure);
  /* Инициализация вывода SCL */
  GPIO_InitStructure.GPIO_Pin =  CODEC_I2C_SCL_PIN;
  GPIO_Init(CODEC_I2C_SCL_GPIO_PORT, &GPIO_InitStructure);
  
  /* Деинициализация драйвера I2C */
  I2C_DeInit(CODEC_I2C);
  /* Заполнение структурі инициализации драйвера I2C */
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_OwnAddress1 = CODEC_I2C_OWN_ADDRESS7;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_InitStructure.I2C_ClockSpeed = CODEC_I2C_SPEED;
  /* Инициализация драйвера I2C */
  I2C_Init(CODEC_I2C, &I2C_InitStructure);
  /* Включение драйвера I2C */
  I2C_Cmd(CODEC_I2C, ENABLE);
}

/**
  * @brief  Функция записи данных в порта I2C 
  * @param  uint8_t addr    - адрес
  * @param  uint8_t *p_data - указатель на нассив данных
  * @param  uint8_t len     - длинна данных
  * @retval uint8_t - результат операции
  */
uint8_t CodecI2CWrite(uint8_t addr, uint8_t *p_data, uint8_t len)
{
  /* This function issues a start condition and transmits the slave address + R/W bit */
  I2C_start(CODEC_I2C, addr, I2C_Direction_Transmitter);
  /* Delay 50 us */
  i2c_ns100_delay(500);
  
  /* Цикл записи данных */
  for(uint8_t i = 0; i < len; i++)
  {
    /* This function transmits one byte to the slave device */
    I2C_write(CODEC_I2C, *p_data);
    p_data++;
    /* Delay 50 us */
    i2c_ns100_delay(500);    
  }
  /* Формирование стоповой последовательности */
  I2C_GenerateSTOP(CODEC_I2C, ENABLE);
  /* Delay 200 us */
  i2c_ns100_delay(2000);      
  /* Возвращаем статус операции */
  return CODEC_I2C_OK;
}


#define SLAVE_ADDRESS 0x3D // the slave address (example)

void init_I2C1(void){
  
  GPIO_InitTypeDef GPIO_InitStruct;
  I2C_InitTypeDef I2C_InitStruct;
  
  // enable APB1 peripheral clock for I2C1
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
  // enable clock for SCL and SDA pins
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  
  /* setup SCL and SDA pins
  * You can connect I2C1 to two different
  * pairs of pins:
  * 1. SCL on PB6 and SDA on PB7 
  * 2. SCL on PB8 and SDA on PB9
  */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; // we are going to use PB6 and PB7
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;			// set pins to alternate function
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		// set GPIO speed
  GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;			// set output to open drain --> the line has to be only pulled low, not driven high
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;			// enable pull up resistors
  GPIO_Init(GPIOB, &GPIO_InitStruct);					// init GPIOB
  
  // Connect I2C1 pins to AF  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);	// SCL
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1); // SDA
  
  // configure I2C1 
  I2C_InitStruct.I2C_ClockSpeed = 100000; 		// 100kHz
  I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;			// I2C mode
  I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;	// 50% duty cycle --> standard
  I2C_InitStruct.I2C_OwnAddress1 = 0x00;			// own address, not relevant in master mode
  I2C_InitStruct.I2C_Ack = I2C_Ack_Disable;		// disable acknowledge when reading (can be changed later on)
  I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; // set address length to 7 bit addresses
  I2C_Init(I2C1, &I2C_InitStruct);				// init I2C1
  
  // enable I2C1
  I2C_Cmd(I2C1, ENABLE);
}

/************** (C) COPYRIGHT 2020 DataExpress ***** END OF FILE **************/