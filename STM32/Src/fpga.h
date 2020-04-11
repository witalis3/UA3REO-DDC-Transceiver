#ifndef FPGA_h
#define FPGA_h

#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include "fft.h"
#include "audio_processor.h"
#include "settings.h"

//Public variables
extern volatile uint32_t FPGA_samples;                                     //счетчик числа семплов при обмене с FPGA
extern volatile bool FPGA_Buffer_underrun;                                 //флаг недостатка данных из FPGA
extern volatile bool FPGA_NeedSendParams;                                  //флаг необходимости отправить параметры в FPGA
extern volatile bool FPGA_NeedGetParams;                                   //флаг необходимости получить параметры из FPGA
extern volatile bool FPGA_NeedRestart;                                     //флаг необходимости рестарта модулей FPGA
extern volatile float32_t FPGA_Audio_Buffer_RX1_Q[FPGA_AUDIO_BUFFER_SIZE]; //буфферы FPGA
extern volatile float32_t FPGA_Audio_Buffer_RX1_I[FPGA_AUDIO_BUFFER_SIZE];
extern volatile float32_t FPGA_Audio_Buffer_RX2_Q[FPGA_AUDIO_BUFFER_SIZE];
extern volatile float32_t FPGA_Audio_Buffer_RX2_I[FPGA_AUDIO_BUFFER_SIZE];
extern volatile float32_t FPGA_Audio_SendBuffer_Q[FPGA_AUDIO_BUFFER_SIZE];
extern volatile float32_t FPGA_Audio_SendBuffer_I[FPGA_AUDIO_BUFFER_SIZE];
extern uint_fast16_t FPGA_Audio_Buffer_Index; //текущий индекс в буфферах FPGA
extern bool FPGA_Audio_Buffer_State;          //состояние буффера, заполнена половина или целиком true - compleate ; false - half

//Public methods
extern void FPGA_Init(void);                //инициализация обмена с FPGA
extern void FPGA_fpgadata_iqclock(void);    //обмен IQ данными с FPGA
extern void FPGA_fpgadata_stuffclock(void); //обмен параметрами с FPGA
extern void FPGA_restart(void);             //перезапуск модулей FPGA

#endif
