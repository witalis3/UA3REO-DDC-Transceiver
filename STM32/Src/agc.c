#include "stm32h7xx_hal.h"
#include "agc.h"
#include "settings.h"
#include "audio_filters.h"
#include "vad.h"

//Private variables
static float32_t AGC_RX1_need_gain_db = 0.0f;
static float32_t AGC_RX2_need_gain_db = 0.0f;
static float32_t AGC_RX1_need_gain_db_old = 0.0f;
static float32_t AGC_RX2_need_gain_db_old = 0.0f;
static float32_t AGC_RX1_agcBuffer_kw[AUDIO_BUFFER_HALF_SIZE] = {0};
static float32_t AGC_RX2_agcBuffer_kw[AUDIO_BUFFER_HALF_SIZE] = {0};
static float32_t AGC_RX1_ringbuffer[AGC_RINGBUFFER_TAPS_SIZE * AUDIO_BUFFER_HALF_SIZE] = {0};
IRAM2 static float32_t AGC_RX2_ringbuffer[AGC_RINGBUFFER_TAPS_SIZE * AUDIO_BUFFER_HALF_SIZE] = {0};

//Run AGC on data block
void DoRxAGC(float32_t *agcBuffer, uint_fast16_t blockSize, AUDIO_PROC_RX_NUM rx_id, uint_fast8_t mode)
{
	//RX1 or RX2
	float32_t *AGC_need_gain_db = &AGC_RX1_need_gain_db;
	float32_t *AGC_need_gain_db_old = &AGC_RX1_need_gain_db_old;
	float32_t *agcBuffer_kw = (float32_t *)&AGC_RX1_agcBuffer_kw;
	float32_t *agc_ringbuffer = (float32_t *)&AGC_RX1_ringbuffer;
	if (rx_id == AUDIO_RX2)
	{
		AGC_need_gain_db = &AGC_RX2_need_gain_db;
		AGC_need_gain_db_old = &AGC_RX2_need_gain_db_old;
		agcBuffer_kw = (float32_t *)&AGC_RX2_agcBuffer_kw;
		agc_ringbuffer = (float32_t *)&AGC_RX2_ringbuffer;
	}

	//higher speed in settings - higher speed of AGC processing
	float32_t RX_AGC_STEPSIZE_UP = 0.0f;
	float32_t RX_AGC_STEPSIZE_DOWN = 0.0f;
	if (mode == TRX_MODE_CW_L || mode == TRX_MODE_CW_U)
	{
		RX_AGC_STEPSIZE_UP = 200.0f / (float32_t)TRX.RX_AGC_CW_speed;
		RX_AGC_STEPSIZE_DOWN = 20.0f / (float32_t)TRX.RX_AGC_CW_speed;
	}
	else
	{
		RX_AGC_STEPSIZE_UP = 200.0f / (float32_t)TRX.RX_AGC_SSB_speed;
		RX_AGC_STEPSIZE_DOWN = 20.0f / (float32_t)TRX.RX_AGC_SSB_speed;
	}

	//do k-weighting (for LKFS)
	if (rx_id == AUDIO_RX1)
	{
		arm_biquad_cascade_df2T_f32(&AGC_RX1_KW_HSHELF_FILTER, agcBuffer, agcBuffer_kw, blockSize);
		arm_biquad_cascade_df2T_f32(&AGC_RX1_KW_HPASS_FILTER, agcBuffer, agcBuffer_kw, blockSize);
	}
	else if (rx_id == AUDIO_RX2)
	{
		arm_biquad_cascade_df2T_f32(&AGC_RX2_KW_HSHELF_FILTER, agcBuffer, agcBuffer_kw, blockSize);
		arm_biquad_cascade_df2T_f32(&AGC_RX2_KW_HPASS_FILTER, agcBuffer, agcBuffer_kw, blockSize);
	}

	//do ring buffer
	static uint32_t ring_position = 0;
	//save new data to ring buffer
	memcpy(&agc_ringbuffer[ring_position * blockSize], agcBuffer, sizeof(float32_t) * blockSize);
	//move ring buffer index
	ring_position++;
	if (ring_position >= AGC_RINGBUFFER_TAPS_SIZE)
		ring_position = 0;
	//get old data to process
	memcpy(agcBuffer, &agc_ringbuffer[ring_position * blockSize], sizeof(float32_t) * blockSize);

	//calculate the magnitude in dBFS
	float32_t AGC_RX_magnitude = 0;
	arm_rms_f32(agcBuffer_kw, blockSize, &AGC_RX_magnitude);
	if (AGC_RX_magnitude == 0.0f)
		AGC_RX_magnitude = 0.001f;
	float32_t full_scale_rate = AGC_RX_magnitude / FLOAT_FULL_SCALE_POW;
	float32_t AGC_RX_dbFS = rate2dbV(full_scale_rate);

	//move the gain one step
	if (!WM8731_Muting && !VAD_Muting)
	{
		float32_t diff = ((float32_t)TRX.AGC_GAIN_TARGET - (AGC_RX_dbFS + *AGC_need_gain_db));
		if (diff > 0)
			*AGC_need_gain_db += diff / RX_AGC_STEPSIZE_UP;
		else
			*AGC_need_gain_db += diff / RX_AGC_STEPSIZE_DOWN;

		//overload (clipping), sharply reduce the gain
		if ((AGC_RX_dbFS + *AGC_need_gain_db) > ((float32_t)TRX.AGC_GAIN_TARGET + AGC_CLIPPING))
		{
			*AGC_need_gain_db = (float32_t)TRX.AGC_GAIN_TARGET - AGC_RX_dbFS;
			//sendToDebug_float32(diff,false);
		}
	}

	//noise threshold, below it - do not amplify
	/*if (AGC_RX_dbFS < AGC_NOISE_GATE)
		*AGC_need_gain_db = 1.0f;*/

	//AGC off, not adjustable
	if ((rx_id == AUDIO_RX1 && !CurrentVFO()->AGC) || (rx_id == AUDIO_RX2 && !SecondaryVFO()->AGC))
		*AGC_need_gain_db = 1.0f;

	//Muting if need
	if (WM8731_Muting || VAD_Muting)
		*AGC_need_gain_db = -200.0f;

	//gain limitation
	if (*AGC_need_gain_db > AGC_MAX_GAIN)
		*AGC_need_gain_db = AGC_MAX_GAIN;

	//apply gain
	if (fabsf(*AGC_need_gain_db_old - *AGC_need_gain_db) > 0.0f) //gain changed
	{
		float32_t gainApplyStep = 0;
		if (*AGC_need_gain_db_old > *AGC_need_gain_db)
			gainApplyStep = -(*AGC_need_gain_db_old - *AGC_need_gain_db) / (float32_t)blockSize;
		if (*AGC_need_gain_db_old < *AGC_need_gain_db)
			gainApplyStep = (*AGC_need_gain_db - *AGC_need_gain_db_old) / (float32_t)blockSize;
		float32_t val_prev = 0.0f;
		bool zero_cross = false;
		for (uint_fast16_t i = 0; i < blockSize; i++)
		{
			if (val_prev < 0.0f && agcBuffer[i] > 0.0f)
				zero_cross = true;
			else if (val_prev > 0.0f && agcBuffer[i] < 0.0f)
				zero_cross = true;
			if (zero_cross)
				*AGC_need_gain_db_old += gainApplyStep;

			agcBuffer[i] = agcBuffer[i] * db2rateV(*AGC_need_gain_db_old);
			val_prev = agcBuffer[i];
		}
	}
	else //gain did not change, apply gain across all samples
	{
		arm_scale_f32(agcBuffer, db2rateV(*AGC_need_gain_db), agcBuffer, blockSize);
	}
}

void ResetAGC(void)
{
	AGC_RX1_need_gain_db = 0.0f;
	AGC_RX2_need_gain_db = 0.0f;
	memset(AGC_RX1_ringbuffer, 0x00, sizeof(AGC_RX1_ringbuffer));
	memset(AGC_RX2_ringbuffer, 0x00, sizeof(AGC_RX2_ringbuffer));
}
