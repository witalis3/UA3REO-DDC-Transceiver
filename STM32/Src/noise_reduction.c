#include "noise_reduction.h"
#include "stm32h7xx_hal.h"
#include "arm_const_structs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include "arm_math.h"
#include "settings.h"
#include "functions.h"

static arm_lms_norm_instance_f32 RX1_lms2_Norm_instance;
static arm_lms_norm_instance_f32 RX2_lms2_Norm_instance;
static float32_t RX1_lms2_stateF32[NOISE_REDUCTION_TAPS + NOISE_REDUCTION_BLOCK_SIZE - 1];
static float32_t RX2_lms2_stateF32[NOISE_REDUCTION_TAPS + NOISE_REDUCTION_BLOCK_SIZE - 1];
static float32_t RX1_lms2_normCoeff_f32[NOISE_REDUCTION_TAPS];
static float32_t RX2_lms2_normCoeff_f32[NOISE_REDUCTION_TAPS];
static float32_t RX1_lms2_reference[NOISE_REDUCTION_REFERENCE_SIZE];
static float32_t RX2_lms2_reference[NOISE_REDUCTION_REFERENCE_SIZE];
static float32_t RX1_lms2_errsig2[NOISE_REDUCTION_BLOCK_SIZE];
static float32_t RX2_lms2_errsig2[NOISE_REDUCTION_BLOCK_SIZE];

static uint_fast16_t RX1_reference_index_old = 0;
static uint_fast16_t RX2_reference_index_old = 0;
static uint_fast16_t RX1_reference_index_new = 0;
static uint_fast16_t RX2_reference_index_new = 0;

void InitNoiseReduction(void)
{
	arm_lms_norm_init_f32(&RX1_lms2_Norm_instance, NOISE_REDUCTION_TAPS, RX1_lms2_normCoeff_f32, RX1_lms2_stateF32, NOISE_REDUCTION_STEP, NOISE_REDUCTION_BLOCK_SIZE);
	arm_lms_norm_init_f32(&RX2_lms2_Norm_instance, NOISE_REDUCTION_TAPS, RX2_lms2_normCoeff_f32, RX2_lms2_stateF32, NOISE_REDUCTION_STEP, NOISE_REDUCTION_BLOCK_SIZE);
	arm_fill_f32(0.0f, RX1_lms2_reference, NOISE_REDUCTION_REFERENCE_SIZE);
	arm_fill_f32(0.0f, RX2_lms2_reference, NOISE_REDUCTION_REFERENCE_SIZE);
	arm_fill_f32(0.0f, RX1_lms2_normCoeff_f32, NOISE_REDUCTION_TAPS);
	arm_fill_f32(0.0f, RX2_lms2_normCoeff_f32, NOISE_REDUCTION_TAPS);
}

void processNoiseReduction(float32_t *bufferIn, float32_t *bufferOut, AUDIO_PROC_RX_NUM rx_id)
{
	if(rx_id==AUDIO_RX1)
	{
		arm_copy_f32(bufferIn, &RX1_lms2_reference[RX1_reference_index_new], NOISE_REDUCTION_BLOCK_SIZE);
		arm_lms_norm_f32(&RX1_lms2_Norm_instance, bufferIn, &RX1_lms2_reference[RX1_reference_index_old], bufferOut, RX1_lms2_errsig2, NOISE_REDUCTION_BLOCK_SIZE);
		RX1_reference_index_old += NOISE_REDUCTION_BLOCK_SIZE;
		if (RX1_reference_index_old >= NOISE_REDUCTION_REFERENCE_SIZE)
			RX1_reference_index_old = 0;
		RX1_reference_index_new = RX1_reference_index_old + NOISE_REDUCTION_BLOCK_SIZE;
		if (RX1_reference_index_new >= NOISE_REDUCTION_REFERENCE_SIZE)
			RX1_reference_index_new = 0;
	}
	else if(rx_id==AUDIO_RX2)
	{
		arm_copy_f32(bufferIn, &RX2_lms2_reference[RX2_reference_index_new], NOISE_REDUCTION_BLOCK_SIZE);
		arm_lms_norm_f32(&RX2_lms2_Norm_instance, bufferIn, &RX2_lms2_reference[RX2_reference_index_old], bufferOut, RX2_lms2_errsig2, NOISE_REDUCTION_BLOCK_SIZE);
		RX2_reference_index_old += NOISE_REDUCTION_BLOCK_SIZE;
		if (RX2_reference_index_old >= NOISE_REDUCTION_REFERENCE_SIZE)
			RX2_reference_index_old = 0;
		RX2_reference_index_new = RX2_reference_index_old + NOISE_REDUCTION_BLOCK_SIZE;
		if (RX2_reference_index_new >= NOISE_REDUCTION_REFERENCE_SIZE)
			RX2_reference_index_new = 0;
	}
}
