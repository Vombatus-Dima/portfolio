/**
  ******************************************************************************
  * @file    cmx618_reg.h
  * @author  Trembach Dmitry
  * @date    26-04-2020
  * @brief   Инициализация задачи сопряжения кодека и мультиплексора
  *
  ******************************************************************************
  * @attention
  * <h2><center>&copy; COPYRIGHT 2020 DataExpress</center></h2>
  ******************************************************************************
  */

#ifndef _CMX618_REG_H
#define _CMX618_REG_H



#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "main.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "event_groups.h"
 
// Регистры CMX618
#define RESETN 		        (0x01)
#define SYNC		        (0x02)
#define SYNCCTRL	        (0x04)
#define AIG		        (0x05)
#define AOG		        (0x06)
#define VCFG		        (0x07)
#define MVCFG		        (0x2C)
#define SDTMF		        (0x08)
#define POWERSAVE	        (0x09)
#define DTMFATTEN	        (0x0A)
#define EXCODECCONT	        (0x0B)
#define IDD		        (0x0C)
#define SVCREQ		        (0x0E)
#define FRAMETYPEW	        (0x0F)
#define DECFRAME	        (0x10)
#define ECFIFO		        (0x24)
#define SVCACK		        (0x2E)
#define FRAMETYPER	        (0x2F)
#define ENCFRAME	        (0x30)
#define VCTRL		        (0x11)
#define MVCTRL		        (0x3C)
#define EXCODECCMD	        (0x12)
#define CLOCK		        (0x1D)
#define VDWHLWM		        (0x1E)
#define IRQENAB		        (0x1F)
#define DFRAMEDATA	        (0x31)
#define PLEVEL		        (0x37)
#define EFRAMEDATA	        (0x38)
#define STATUS		        (0x40)

// CMX618 SYNCCTRL Register Definitions
#define SYNC_PERIOD_60MS        (0x0c)        // Pulse every 60ms
#define SYNC_PERIOD_40MS        (0x08)        // Pulse every 40ms
#define SYNC_PERIOD_20MS        (0x04)        // Pulse every 20ms
#define SYNC_PERIOD_125MS       (0x00)        // 8kHz square wave

// External synchronisation. SYNC pin 25 is set as an input. A sync pulse
// should be applied to this pin or this pin should be tied low and a write to the
// SYNC register ($02) will act as a sync pulse.
#define SYNC_INPUT	 	(0x03)
// Internal synchronisation. SYNC pin 25 is set as an output, and a sync pulse
// is produced to allow an external device to synchronise with this one.
#define SYNC_OUTPUT	 	(0x01)
// Internal synchronisation. SYNC pin 25 is set as an input but unused. This is
// the default setting for this pin. This pin must be tied to '0' or '1' by the user.
#define SYNC_UNUSED	 	(0x00)
    
// CMX618 AIG Register Definitions
#define MIC_AMPL_20dB	 	(0x80)            // '0' in this bit sets the microphone amplifier gain to 0dB. A '1' in this bit sets the
#define MIC_AMPL_0dB	 	(0x00) 						// microphone amplifier gain to +20dB.

#define INPUT_GAIN_0	 	(0x00)            //  0	  Gain (dB)
#define INPUT_GAIN_1_5	 	(0x01)            //  1.5	  Gain (dB)
#define INPUT_GAIN_3_0	 	(0x02)            //  3.0	  Gain (dB)
#define INPUT_GAIN_4_5	 	(0x03)            //  4.5	  Gain (dB)
#define INPUT_GAIN_6_0	 	(0x04)            //  6.0	  Gain (dB)
#define INPUT_GAIN_7_5	 	(0x05)            //  7.5	  Gain (dB)
#define INPUT_GAIN_9_0	 	(0x06)            //  9.0	  Gain (dB)
#define INPUT_GAIN_10_5	 	(0x07)            //  10.5  Gain (dB)
#define INPUT_GAIN_12_0	 	(0x08)            //  12.0  Gain (dB)
#define INPUT_GAIN_13_5	 	(0x09)            //  13.5  Gain (dB)
#define INPUT_GAIN_15_0	 	(0x0A)            //  15.0  Gain (dB)
#define INPUT_GAIN_16_5	 	(0x0B)            //  16.5  Gain (dB)
#define INPUT_GAIN_18_0	 	(0x0C)            //  18.0  Gain (dB)
#define INPUT_GAIN_19_5	 	(0x0D)            //  19.5  Gain (dB)
#define INPUT_GAIN_21_0	 	(0x0E)            //  21.0  Gain (dB)
#define INPUT_GAIN_22_5	 	(0x0F)            //  22.5  Gain (dB)
   
//CMX618 A0G Register Definitions
#define OUTPUT_SW_6dB	        (0x80)            // A '0' in this bit sets the earpiece gain to 0dB. A '1' in this bit sets the earpiece gain
#define OUTPUT_SW_0dB	        (0x00)						// to +6dB. A further 6dB of gain can be achieved by using both audio outputs.
  
#define OUTPUT_ATTN_14          (0x00)            // -14 	Attn (dB)
#define OUTPUT_ATTN_12          (0x01)            // -12	Attn (dB)
#define OUTPUT_ATTN_10          (0x02)            // -10	Attn (dB)
#define OUTPUT_ATTN_8           (0x03)            // -8	Attn (dB)
#define OUTPUT_ATTN_6           (0x04)            // -6	Attn (dB)
#define OUTPUT_ATTN_4           (0x05)            // -4	Attn (dB)
#define OUTPUT_ATTN_2           (0x06)            // -2	Attn (dB)
#define OUTPUT_GAIN_0           (0x07)            // 0 	Gain (dB)
#define OUTPUT_GAIN_2           (0x08)            // 2 	Gain (dB)
#define OUTPUT_GAIN_4           (0x09)            // 4 	Gain (dB)
#define OUTPUT_GAIN_6           (0x0A)            // 6 	Gain (dB)
#define OUTPUT_GAIN_8           (0x0B)            // 8 	Gain (dB)
#define OUTPUT_GAIN_10          (0x0C)            // 10	Gain (dB)
#define OUTPUT_GAIN_12          (0x0D)            // 12	Gain (dB)
#define OUTPUT_GAIN_14          (0x0E)            // 14	Gain (dB)
#define OUTPUT_GAIN_16          (0x0F)            // 16	Gain (dB)

    
// CMX618 VCFG Register Definitions
#define DTMFF 		        (0x80)		   // This bit controls the format of the 4-bit DTMF code
#define DTX		        (0x40)		   // This bit controls the DTX (Discontinuous Transmission) feature.
#define HDD 		        (0x20)		   // selects "hard bits" for decoding
#define FEC		        (0x10)		   // enables FEC
#define _80MSEC		        (0x00)
#define _60MSEC		        (0x03)
#define _40MSEC		        (0x02)
#define _20MSEC		        (0x01)
#define _2750BPS                (0x08)
#define _2400BPS 	        (0x04)
#define _2050BPS	        (0x00)

// CMX618 POWERSAVE Register Definitions
// Setting this bit to '1' will turn on the analogue bias. The host should wait for
// approximately 100ms to allow the bias capacitors to charge.
// Clearing this bit to '0' will turn off the analogue bias.
#define POWER_BIAS	        (0x01)
// Setting this bit to '1' will turn on the CODEC master clock and set it up for use with
// CODEC the vocoding function.
// Clearing this bit to '0' will turn off the master clock of the CODEC.
#define POWER_CODEC             (0x02)
// Setting this bit to '1' will prevent the DAC from being power-saved when the
// DACON device is not decoding.
// Clearing this bit to '0' will cause DAC to be power-saved whenever the device is not decoding.
#define POWER_DACON             (0x04)
// Setting this bit to '1' will prevent the ADC from being power-saved when the
// ADCON device is not encoding.
// Clearing this bit to '0' will cause the ADC to be power-saved whenever the device
// is not encoding.
#define POWER_ADCON             (0x08)
// Setting this bit to '1' will reduce the internal clock rate to a quarter of its normal
// frequency when the device is not actively encoding or decoding.
// Clearing this bit to '0' will leave the internal clock rate at its normal frequency.
// This bit should be cleared to '0' in the CMX638, when using full-duplex operation.
#define POWER_THROTTLE          (0x10)

//CMX618 SVCREQ Register Definitions
#define REQ_FUNC_IMAGE		(0x01)            // Request to load a Function Image. 
#define REQ_PLEVEL_ON           (0x09)            // Request to turn on the PLEVEL functionality. 
#define REQ_PLEVEL_OFF		(0x0A)            // Request to turn off the PLEVEL functionality.


// CMX618 IRQENAB Register Definitions
// * Only the hi byte *
#define RDY                     (0x8000)		// Ready (for new commands)
#define VDW                     (0x0100)		// Vocoder Data Wanted (decode mode)
#define SVC                     (0x4000)		// When this bit is set to '1' it indicates that 
                                  // the device has completed a SVC service request
// * Only the low byte *
#define VDA                     (0x0001)		// Vocoder Data Available (encode mode)
#define EFDA                    (0x0002)		// Encoder Frame Data available.*
#define DFDA                    (0x0004)		// Decoder Frame Data available.*
#define PLV                     (0x0040)		// Peak Level sample for the last 20ms frame available.*
      
// VDWHLWM
#define  VDWHL_HI               (0x01)    // Setting the Watermarks for the Vocoder Data Input (Output) FIFOs
#define  VDWHL_LO               (0xe0)  
// * Настройки кодека *
// Входной коэффициент усиления
//#define CMX618_INPUT_GAIN         (MIC_AMPL_0dB | INPUT_GAIN_6_0)
#define CMX618_INPUT_GAIN         (MIC_AMPL_0dB | INPUT_GAIN_0)
// Выходной коэффициент усиления
#define CMX618_OUTPUT_GAIN        (OUTPUT_SW_6dB | OUTPUT_GAIN_16)//(OUTPUT_SW_0dB | OUTPUT_GAIN_10)
//#define CMX618_OUTPUT_GAIN        (OUTPUT_SW_6dB | OUTPUT_GAIN_16)

//------------------------------------------------------------------------------
//       
//------------------------------------------------------------------------------
// Длина счётчика голосовых фреймов
#define CMX618_CNT_LEN          (1)
// Кол-во голосовых байт (длина голосового фрейма)
#define CMX618_VOICE_LEN        (18)

#define Add_VOICE               (0x0010)
#define RX_VOICE                (0)
#define TX_VOICE                (1)


// описание структуры данных кодека
typedef struct CMX618_Data
{
  uint8_t                       cnt;                        // счётчик голосового пакета
  uint8_t                       data[CMX618_VOICE_LEN];     // голосовые данные
} CMX618_Data_t;



#endif /* _CMX618_REG_H */


