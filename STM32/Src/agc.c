#include "stm32h7xx_hal.h"
#include "agc.h"
#include "settings.h"
#include "audio_filters.h"
#include "vad.h"
#include "trx_manager.h"

//Private variables
static float32_t AGC_RX1_need_gain_db = 0.0f;
static float32_t AGC_RX2_need_gain_db = 0.0f;
static float32_t AGC_TX_need_gain_db = 0.0f;
static float32_t AGC_RX1_need_gain_db_old = 0.0f;
static float32_t AGC_RX2_need_gain_db_old = 0.0f;
static float32_t AGC_TX_need_gain_db_old = 0.0f;
static uint32_t AGC_RX1_last_agc_peak_time = 0.0f;
static uint32_t AGC_RX2_last_agc_peak_time = 0.0f;
static float32_t AGC_RX1_agcBuffer_kw[AUDIO_BUFFER_HALF_SIZE] = {0};
IRAM2 static float32_t AGC_RX2_agcBuffer_kw[AUDIO_BUFFER_HALF_SIZE] = {0};
static float32_t AGC_RX1_ringbuffer[AGC_RINGBUFFER_TAPS_SIZE * AUDIO_BUFFER_HALF_SIZE] = {0};
IRAM2 static float32_t AGC_RX2_ringbuffer[AGC_RINGBUFFER_TAPS_SIZE * AUDIO_BUFFER_HALF_SIZE] = {0};
IRAM2 static float32_t AGC_TX_ringbuffer_i[AGC_RINGBUFFER_TAPS_SIZE * AUDIO_BUFFER_HALF_SIZE] = {0};

//Run AGC on data block
void DoRxAGC(float32_t *agcBuffer_i, float32_t *agcBuffer_q, uint_fast16_t blockSize, AUDIO_PROC_RX_NUM rx_id, uint_fast8_t mode, bool stereo)
{
	//RX1 or RX2
	float32_t *AGC_need_gain_db = &AGC_RX1_need_gain_db;
	float32_t *AGC_need_gain_db_old = &AGC_RX1_need_gain_db_old;
	float32_t *agcBuffer_kw = (float32_t *)&AGC_RX1_agcBuffer_kw;
	float32_t *agc_ringbuffer = (float32_t *)&AGC_RX1_ringbuffer;
	uint32_t *last_agc_peak_time = &AGC_RX1_last_agc_peak_time;
	if (rx_id == AUDIO_RX2)
	{
		AGC_need_gain_db = &AGC_RX2_need_gain_db;
		AGC_need_gain_db_old = &AGC_RX2_need_gain_db_old;
		agcBuffer_kw = (float32_t *)&AGC_RX2_agcBuffer_kw;
		agc_ringbuffer = (float32_t *)&AGC_RX2_ringbuffer;
		last_agc_peak_time = &AGC_RX2_last_agc_peak_time;
	}

	//higher speed in settings - higher speed of AGC processing
	float32_t RX_AGC_STEPSIZE_UP = 0.0f;
	float32_t RX_AGC_STEPSIZE_DOWN = 0.0f;
	if (mode == TRX_MODE_CW)
	{
		RX_AGC_STEPSIZE_UP = 200.0f / (float32_t)TRX.RX_AGC_CW_speed;
		RX_AGC_STEPSIZE_DOWN = 20.0f / (float32_t)TRX.RX_AGC_CW_speed;
	}
	else
	{
		RX_AGC_STEPSIZE_UP = 1000.0f / (float32_t)TRX.RX_AGC_SSB_speed;
		RX_AGC_STEPSIZE_DOWN = 20.0f / (float32_t)TRX.RX_AGC_SSB_speed;
	}

	//do k-weighting (for LKFS)
	if (rx_id == AUDIO_RX1)
	{
		arm_biquad_cascade_df2T_f32_rolled(&AGC_RX1_KW_HSHELF_FILTER, agcBuffer_i, agcBuffer_kw, blockSize);
		arm_biquad_cascade_df2T_f32_rolled(&AGC_RX1_KW_HPASS_FILTER, agcBuffer_i, agcBuffer_kw, blockSize);
	}
	else
	{
		arm_biquad_cascade_df2T_f32_rolled(&AGC_RX2_KW_HSHELF_FILTER, agcBuffer_i, agcBuffer_kw, blockSize);
		arm_biquad_cascade_df2T_f32_rolled(&AGC_RX2_KW_HPASS_FILTER, agcBuffer_i, agcBuffer_kw, blockSize);
	}

	//do ring buffer
	static uint32_t ring_position = 0;
	//save new data to ring buffer
	dma_memcpy(&agc_ringbuffer[ring_position * blockSize], agcBuffer_i, sizeof(float32_t) * blockSize);
	//move ring buffer index
	ring_position++;
	if (ring_position >= AGC_RINGBUFFER_TAPS_SIZE)
		ring_position = 0;
	//get old data to process
	if(!stereo)
		dma_memcpy(agcBuffer_i, &agc_ringbuffer[ring_position * blockSize], sizeof(float32_t) * blockSize);

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
		{
			if((HAL_GetTick() - *last_agc_peak_time) > TRX.RX_AGC_Hold)
				*AGC_need_gain_db += diff / RX_AGC_STEPSIZE_UP;
		}
		else
		{
			*AGC_need_gain_db += diff / RX_AGC_STEPSIZE_DOWN;
			*last_agc_peak_time = HAL_GetTick();
		}

		//overload (clipping), sharply reduce the gain
		if ((AGC_RX_dbFS + *AGC_need_gain_db) > ((float32_t)TRX.AGC_GAIN_TARGET + AGC_CLIPPING))
		{
			*AGC_need_gain_db = (float32_t)TRX.AGC_GAIN_TARGET - AGC_RX_dbFS;
			//sendToDebug_float32(diff,false);
		}
	}

	//AGC off, not adjustable
	if ((rx_id == AUDIO_RX1 && !CurrentVFO->AGC) || (rx_id == AUDIO_RX2 && !SecondaryVFO->AGC))
		*AGC_need_gain_db = 1.0f;

	//Muting if need
	if (WM8731_Muting || VAD_Muting)
		*AGC_need_gain_db = -200.0f;

	//gain limitation
	if (*AGC_need_gain_db > (float32_t)TRX.RX_AGC_Max_gain)
		*AGC_need_gain_db = (float32_t)TRX.RX_AGC_Max_gain;

	//apply gain
	//println(*AGC_need_gain_db);
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
			if (val_prev < 0.0f && agcBuffer_i[i] > 0.0f)
				zero_cross = true;
			else if (val_prev > 0.0f && agcBuffer_i[i] < 0.0f)
				zero_cross = true;
			if (zero_cross)
				*AGC_need_gain_db_old += gainApplyStep;

			agcBuffer_i[i] = agcBuffer_i[i] * db2rateV(*AGC_need_gain_db_old);
			if(stereo)
				agcBuffer_q[i] = agcBuffer_q[i] * db2rateV(*AGC_need_gain_db_old);
			val_prev = agcBuffer_i[i];
		}
	}
	else //gain did not change, apply gain across all samples
	{
		arm_scale_f32(agcBuffer_i, db2rateV(*AGC_need_gain_db), agcBuffer_i, blockSize);
		if(stereo)
			arm_scale_f32(agcBuffer_q, db2rateV(*AGC_need_gain_db), agcBuffer_q, blockSize);
	}
}

//Run TX AGC on data block
void DoTxAGC(float32_t *agcBuffer_i, uint_fast16_t blockSize, float32_t target, uint_fast8_t mode)
{
	float32_t *AGC_need_gain_db = &AGC_TX_need_gain_db;
	float32_t *AGC_need_gain_db_old = &AGC_TX_need_gain_db_old;
	//float32_t *agc_ringbuffer_i = (float32_t *)&AGC_TX_ringbuffer_i;

	//higher speed in settings - higher speed of AGC processing
	float32_t TX_AGC_STEPSIZE_UP = 0.0f;
	float32_t TX_AGC_STEPSIZE_DOWN = 0.0f;
	switch(mode)
	{
		case TRX_MODE_LSB:
		case TRX_MODE_USB:
		case TRX_MODE_LOOPBACK:
		default:
			TX_AGC_STEPSIZE_UP = 200.0f / (float32_t)TRX.TX_Compressor_speed_SSB;
			TX_AGC_STEPSIZE_DOWN = 20.0f / (float32_t)TRX.TX_Compressor_speed_SSB;
		break;
		
		case TRX_MODE_NFM:
		case TRX_MODE_WFM:
		case TRX_MODE_AM:
		case TRX_MODE_SAM:
			TX_AGC_STEPSIZE_UP = 200.0f / (float32_t)TRX.TX_Compressor_speed_AMFM;
			TX_AGC_STEPSIZE_DOWN = 20.0f / (float32_t)TRX.TX_Compressor_speed_AMFM;
		break;
	}

	//do ring buffer
	static uint32_t ring_position = 0;
	//save new data to ring buffer
	//dma_memcpy(&agc_ringbuffer_i[ring_position * blockSize], agcBuffer_i, sizeof(float32_t) * blockSize);
	//move ring buffer index
	ring_position++;
	if (ring_position >= AGC_RINGBUFFER_TAPS_SIZE)
		ring_position = 0;
	//get old data to process
	//dma_memcpy(agcBuffer_i, &agc_ringbuffer_i[ring_position * blockSize], sizeof(float32_t) * blockSize);

	//calculate the magnitude
	float32_t AGC_TX_I_magnitude = 0;
	float32_t ampl_max_i = 0.0f;
	float32_t ampl_min_i = 0.0f;
	uint32_t tmp_index;
	arm_max_no_idx_f32(agcBuffer_i, blockSize, &ampl_max_i);
	arm_min_f32(agcBuffer_i, blockSize, &ampl_min_i, &tmp_index);
	if (ampl_max_i > -ampl_min_i)
		AGC_TX_I_magnitude = ampl_max_i;
	else
		AGC_TX_I_magnitude = -ampl_min_i;
	if (AGC_TX_I_magnitude == 0.0f)
		AGC_TX_I_magnitude = 0.000001f;
	
	float32_t AGC_TX_dbFS = rate2dbV(AGC_TX_I_magnitude);
	if(AGC_TX_dbFS < -100.0f)
		AGC_TX_dbFS = -100.0f;
	
	//mic noise threshold (gate), below it - mute
	if (AGC_TX_dbFS < TRX.MIC_NOISE_GATE && TRX.MIC_NOISE_GATE > -120) { //-120db - disabled
		target = 0.0f;
		TRX_MIC_BELOW_NOISEGATE = true;
	} else { 
		//println(AGC_TX_I_magnitude, " ", AGC_TX_dbFS, " ", *AGC_need_gain_db);
		TRX_MIC_BELOW_NOISEGATE = false;
	}
	
	//move the gain one step
	if (target > 0.0f)
	{
		float32_t diff = (target - (AGC_TX_dbFS + *AGC_need_gain_db));
		if (diff > 0)
			*AGC_need_gain_db += diff / TX_AGC_STEPSIZE_UP;
		else
			*AGC_need_gain_db += diff / TX_AGC_STEPSIZE_DOWN;

		//overload (clipping), sharply reduce the gain
		if ((AGC_TX_dbFS + *AGC_need_gain_db) > target)
		{
			*AGC_need_gain_db = target - AGC_TX_dbFS;
			//sendToDebug_float32(diff,false);
		}
	}

	//gain limitation
	switch(mode)
	{
		case TRX_MODE_LSB:
		case TRX_MODE_USB:
		case TRX_MODE_LOOPBACK:
		default:
			if (*AGC_need_gain_db > TRX.TX_Compressor_maxgain_SSB)
				*AGC_need_gain_db = TRX.TX_Compressor_maxgain_SSB;
		break;
		
		case TRX_MODE_NFM:
		case TRX_MODE_WFM:
		case TRX_MODE_AM:
		case TRX_MODE_SAM:
			if (*AGC_need_gain_db > TRX.TX_Compressor_maxgain_AMFM)
				*AGC_need_gain_db = TRX.TX_Compressor_maxgain_AMFM;
		break;
	}
	
	//apply gain
	//println(*AGC_need_gain_db);
	if (target == 0.0f) //zero gain (mute)
	{
		#define zero_gain 200.0f
		float32_t gainApplyStep = 0;
		if (*AGC_need_gain_db_old > zero_gain)
			gainApplyStep = -(*AGC_need_gain_db_old - zero_gain) / (float32_t)blockSize;
		if (*AGC_need_gain_db_old < zero_gain)
			gainApplyStep = (zero_gain - *AGC_need_gain_db_old) / (float32_t)blockSize;
		float32_t val_prev = 0.0f;
		bool zero_cross = false;
		for (uint_fast16_t i = 0; i < blockSize; i++)
		{
			if (val_prev < 0.0f && agcBuffer_i[i] > 0.0f)
				zero_cross = true;
			else if (val_prev > 0.0f && agcBuffer_i[i] < 0.0f)
				zero_cross = true;
			if (zero_cross)
				*AGC_need_gain_db_old += gainApplyStep;

			if(*AGC_need_gain_db_old >= zero_gain)
				agcBuffer_i[i] = agcBuffer_i[i] * db2rateV(*AGC_need_gain_db_old);
			else
				agcBuffer_i[i] = 0.0f;
			
			val_prev = agcBuffer_i[i];
		}
	}
	else if (fabsf(*AGC_need_gain_db_old - *AGC_need_gain_db) > 0.0f) //gain changed
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
			if (val_prev < 0.0f && agcBuffer_i[i] > 0.0f)
				zero_cross = true;
			else if (val_prev > 0.0f && agcBuffer_i[i] < 0.0f)
				zero_cross = true;
			if (zero_cross)
				*AGC_need_gain_db_old += gainApplyStep;

			agcBuffer_i[i] = agcBuffer_i[i] * db2rateV(*AGC_need_gain_db_old);
			val_prev = agcBuffer_i[i];
		}
	}
	else //gain did not change, apply gain across all samples
	{
		arm_scale_f32(agcBuffer_i, db2rateV(*AGC_need_gain_db), agcBuffer_i, blockSize);
	}
}

void ResetAGC(void)
{
	AGC_RX1_need_gain_db = 0.0f;
	AGC_RX2_need_gain_db = 0.0f;
	dma_memset(AGC_RX1_ringbuffer, 0x00, sizeof(AGC_RX1_ringbuffer));
	dma_memset(AGC_RX2_ringbuffer, 0x00, sizeof(AGC_RX2_ringbuffer));
}
