#ifndef WM8731_h
#define WM8731_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include "audio_processor.h"
#include "functions.h"

#define I2C_ADDRESS_WM8731 0x34 //адрес аудио-кодека
#define CODEC_AUDIO_BUFFER_SIZE (FPGA_AUDIO_BUFFER_SIZE * 2) //размер кольцевого буфера в 2 раза больше буфера FPGA, работаем то в первой половиной, то с другой

//Public variables
extern IRAM2 int32_t CODEC_Audio_Buffer_RX[CODEC_AUDIO_BUFFER_SIZE];
extern IRAM2 int32_t CODEC_Audio_Buffer_TX[CODEC_AUDIO_BUFFER_SIZE];
extern bool WM8731_DMA_state; //с какой частью буфера сейчас работаем, true - compleate; false - half
extern bool WM8731_Buffer_underrun; //недостаток данных в буфере из аудио-процессора
extern uint32_t WM8731_DMA_samples; //считаем количество семплов, переданных аудио-кодеку

//Public methods
extern void WM8731_Init(void); //инициализация аудио-кодека по I2C
extern void WM8731_start_i2s_and_dma(void); //запуск шины I2S
extern void WM8731_Beep(void); //сигнал бипера
extern void WM8731_CleanBuffer(void); //очистка буффера аудио-кодека и USB аудио
extern void WM8731_TX_mode(void); //переход в режим TX (глушим динамик и пр.)
extern void WM8731_RX_mode(void); //переход в режим RX (глушим микрофон и пр.)
extern void WM8731_TXRX_mode(void); //переход в смешанный режим RX-TX (для LOOP)

#endif
