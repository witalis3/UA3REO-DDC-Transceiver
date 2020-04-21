#ifndef CW_DECODER_h
#define CW_DECODER_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "functions.h"

#define CWDECODER_TARGET_FREQ 350.0 //частота сигнала для декодирования
#define CWDECODER_SAMPLES FPGA_AUDIO_BUFFER_HALF_SIZE       //размер блока декодера
#define CWDECODER_HIGH_AVERAGE 100  //пороговые значения
#define CWDECODER_LOW_AVERAGE (CWDECODER_HIGH_AVERAGE * 10)
#define CWDECODER_NBTIME 6  // ms noise blanker
#define CWDECODER_STRLEN 30 //длинна декодированной строки

//Public variables
extern volatile uint16_t CW_Decoder_WPM;
extern char CW_Decoder_Text[CWDECODER_STRLEN + 1];

//Public methods
extern void CWDecoder_Init(void);                   //инициализация CW декодера
extern void CWDecoder_Process(float32_t *bufferIn); //запуск CW декодера для блока данных

#endif
