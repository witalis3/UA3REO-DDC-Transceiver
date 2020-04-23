#ifndef DECODER_h
#define DECODER_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "functions.h"
#include "settings.h"

#define DECODER_FREQ	1000				 //частота срабатывания декодера (1 мсек)
#define DECODER_PACKET_SIZE	(TRX_SAMPLERATE / DECODER_FREQ)				 //данных в одном пакете (48 байт)
#define DECODER_BUFF_SIZE (DECODER_PACKET_SIZE * 8 * 2)       //размер буффера декодера

//Public methods
extern void DECODER_Init(void); //инициализация декодера и его дочерних процессов
extern void DECODER_Process(void); //запуск подходящего декодера
extern void DECODER_PutSamples(float32_t *bufferIn, uint32_t size); //добавление в буффер декодера новых данных

#endif
