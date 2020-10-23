#include "auto_notch.h"

// Private variables
static AN_Instance RX1_AN_instance = {0}; // filter instances for two receivers
static AN_Instance RX2_AN_instance = {0};

// initialize the automatic notch filter
void InitAutoNotchReduction(void)
{
	memset(&RX1_AN_instance, 0x00, sizeof RX1_AN_instance);
	memset(&RX2_AN_instance, 0x00, sizeof RX2_AN_instance);
	arm_lms_norm_init_f32(&RX1_AN_instance.lms2_Norm_instance, AUTO_NOTCH_TAPS, RX1_AN_instance.lms2_normCoeff_f32, RX1_AN_instance.lms2_stateF32, AUTO_NOTCH_STEP, AUTO_NOTCH_BLOCK_SIZE);
	arm_lms_norm_init_f32(&RX2_AN_instance.lms2_Norm_instance, AUTO_NOTCH_TAPS, RX2_AN_instance.lms2_normCoeff_f32, RX2_AN_instance.lms2_stateF32, AUTO_NOTCH_STEP, AUTO_NOTCH_BLOCK_SIZE);
}

// start automatic notch filter
void processAutoNotchReduction(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id)
{
	AN_Instance *instance = &RX1_AN_instance;
	if (rx_id == AUDIO_RX2)
		instance = &RX2_AN_instance;

	memcpy(&instance->lms2_reference[instance->reference_index_new], buffer, sizeof(float32_t) * AUTO_NOTCH_BLOCK_SIZE);												// save the data to the reference buffer
	
	arm_lms_norm_f32(&instance->lms2_Norm_instance, buffer, &instance->lms2_reference[instance->reference_index_old], instance->lms2_errsig2, buffer, AUTO_NOTCH_BLOCK_SIZE); // start LMS filter
	
	if (__ARM_isnanf(buffer[0]) || __ARM_isinff(buffer[0]) || (buffer[0] > 2.0f) || (buffer[0] < -2.0f))
	{
		sendToDebug_str("auto notch err ");
		sendToDebug_float32(buffer[0],false);
		memset(buffer, 0x00, sizeof sizeof(float32_t) * AUTO_NOTCH_BLOCK_SIZE);
		InitAutoNotchReduction();
	}
	
	instance->reference_index_old += AUTO_NOTCH_BLOCK_SIZE;																												  // move along the reference buffer
	if (instance->reference_index_old >= AUTO_NOTCH_REFERENCE_SIZE)
		instance->reference_index_old = 0;
	instance->reference_index_new = instance->reference_index_old + AUTO_NOTCH_BLOCK_SIZE;
	if (instance->reference_index_new >= AUTO_NOTCH_REFERENCE_SIZE)
		instance->reference_index_new = 0;
}