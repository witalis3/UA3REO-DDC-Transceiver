#ifndef CW_DECODER_h
#define CW_DECODER_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "functions.h"

#define CWDECODER_NBTIME 6  // ms noise blanker
#define CWDECODER_STRLEN 30 //длинна декодированной строки
#define CWDECODER_MAGNIFY 16 //во сколько приближаем FFT
#define CWDECODER_FFTSIZE 256 //размер FFT для анализа
#define CWDECODER_FFTSIZE_HALF (CWDECODER_FFTSIZE / 2) //половина размера FFT
#define CWDECODER_NOISEGATE 10.0f //во сколько раз полезный сигнал должен быть больше среднего, чтобы начать работу с ним
#define CWDECODER_AFC_LATENCY 1000 //время сколько ожидаем сигнал на частоте, если он не появится снова - забываем и ищем новый
#define CWDECODER_SPEC_PART (CWDECODER_FFTSIZE_HALF / 8) //ищем сигнал только в первой части сигнала (полоса CW узкая)

//Public variables
extern volatile uint16_t CW_Decoder_WPM;
extern char CW_Decoder_Text[CWDECODER_STRLEN + 1];

//Public methods
extern void CWDecoder_Init(void);                   //инициализация CW декодера
extern void CWDecoder_Process(float32_t *bufferIn); //запуск CW декодера для блока данных

#endif
