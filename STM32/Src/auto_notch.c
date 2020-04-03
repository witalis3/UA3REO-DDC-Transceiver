#include "auto_notch.h"
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

static AN_Instance RX1_AN_instance = {0};
static AN_Instance RX2_AN_instance = {0};

void InitAutoNotchReduction(void)
{
	arm_lms_norm_init_f32(&RX1_AN_instance.lms2_Norm_instance, AUTO_NOTCH_TAPS, RX1_AN_instance.lms2_normCoeff_f32, RX1_AN_instance.lms2_stateF32, AUTO_NOTCH_STEP, AUTO_NOTCH_BLOCK_SIZE);
	arm_lms_norm_init_f32(&RX2_AN_instance.lms2_Norm_instance, AUTO_NOTCH_TAPS, RX2_AN_instance.lms2_normCoeff_f32, RX2_AN_instance.lms2_stateF32, AUTO_NOTCH_STEP, AUTO_NOTCH_BLOCK_SIZE);
	arm_fill_f32(0.0f, RX1_AN_instance.lms2_reference, AUTO_NOTCH_REFERENCE_SIZE);
	arm_fill_f32(0.0f, RX2_AN_instance.lms2_reference, AUTO_NOTCH_REFERENCE_SIZE);
	arm_fill_f32(0.0f, RX1_AN_instance.lms2_normCoeff_f32, AUTO_NOTCH_TAPS);
	arm_fill_f32(0.0f, RX2_AN_instance.lms2_normCoeff_f32, AUTO_NOTCH_TAPS);
}

void processAutoNotchReduction(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id)
{
	AN_Instance* instance = &RX1_AN_instance;
	if(rx_id==AUDIO_RX2)
		instance = &RX2_AN_instance;

	arm_copy_f32(buffer, &instance->lms2_reference[instance->reference_index_new], AUTO_NOTCH_BLOCK_SIZE);
	arm_lms_norm_f32(&instance->lms2_Norm_instance, buffer, &instance->lms2_reference[instance->reference_index_old], instance->lms2_errsig2, buffer , AUTO_NOTCH_BLOCK_SIZE);
	instance->reference_index_old += AUTO_NOTCH_BLOCK_SIZE;
	if (instance->reference_index_old >= AUTO_NOTCH_REFERENCE_SIZE)
		instance->reference_index_old = 0;
	instance->reference_index_new = instance->reference_index_old + AUTO_NOTCH_BLOCK_SIZE;
	if (instance->reference_index_new >= AUTO_NOTCH_REFERENCE_SIZE)
		instance->reference_index_new = 0;
}
