#ifndef AUTO_NOTCH_h
#define AUTO_NOTCH_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "audio_processor.h"

#define AUTO_NOTCH_BLOCK_SIZE (AUDIO_BUFFER_HALF_SIZE / 3) //размер блока для обработки
#define AUTO_NOTCH_TAPS AUTO_NOTCH_BLOCK_SIZE					//порядок фильтра
#define AUTO_NOTCH_REFERENCE_SIZE (AUTO_NOTCH_BLOCK_SIZE * 2)	//размер опорного буффера
#define AUTO_NOTCH_STEP 0.0001f									//шаг LMS алгоритма

typedef struct //инстанс фильтра
{
	arm_lms_norm_instance_f32 lms2_Norm_instance;
	float32_t lms2_stateF32[AUTO_NOTCH_TAPS + AUTO_NOTCH_BLOCK_SIZE - 1];
	float32_t lms2_normCoeff_f32[AUTO_NOTCH_TAPS];
	float32_t lms2_reference[AUTO_NOTCH_REFERENCE_SIZE];
	float32_t lms2_errsig2[AUTO_NOTCH_BLOCK_SIZE];
	uint_fast16_t reference_index_old;
	uint_fast16_t reference_index_new;
} AN_Instance;

//Public methods
extern void InitAutoNotchReduction(void);										   //инициализация автоматического нотч-фильтра
extern void processAutoNotchReduction(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id); //запуск автоматического нотч-фильтра

#endif
