#include "vad.h"
#include "arm_const_structs.h"
#include "settings.h"
#include "trx_manager.h"

// https://habr.com/ru/post/192954/

static arm_rfft_fast_instance_f32 VAD_FFT_Inst;
IRAM2 static float32_t window_multipliers[VAD_FFT_SIZE] = {0}; // coefficients for the window function
IRAM2 static float32_t VAD_FFTBuffer[VAD_FFT_SIZE] = {0};		 
IRAM2 static float32_t VAD_FFTBufferCharge[VAD_FFT_SIZE * 2] = {0}; // cumulative buffer
IRAM2 static float32_t InputBuffer[VAD_FFT_SIZE * 2] = {0};		//Input buffer
static float32_t VAD_decimState[VAD_BLOCK_SIZE + 4 - 1];
static arm_fir_decimate_instance_f32 VAD_DECIMATE;
static const arm_fir_decimate_instance_f32 VAD_FirDecimate =
{
	// 48ksps, 3kHz lowpass
	.numTaps = 4,
	.pCoeffs = (float32_t *)(const float32_t[]){0.199820836596682871f, 0.272777397353925699f, 0.272777397353925699f, 0.199820836596682871f},
	.pState = NULL,
};
static float32_t Min_E = 999.0f;
static float32_t Min_MD = 999.0f;
static uint32_t start_counter = 0;
bool VAD_Muting; 				//Muting flag

// initialize VAD
void InitVAD(void)
{
	// initialize RFFT
	arm_rfft_fast_init_f32(&VAD_FFT_Inst, VAD_FFT_SIZE);
	// decimator
	arm_fir_decimate_init_f32(&VAD_DECIMATE, VAD_FirDecimate.numTaps, VAD_MAGNIFY, VAD_FirDecimate.pCoeffs, VAD_decimState, VAD_BLOCK_SIZE);
	// Blackman window function
	for (uint_fast16_t i = 0; i < VAD_FFT_SIZE; i ++)
		window_multipliers [i] = ((1.0f - 0.16f) / 2) - 0.5f * arm_cos_f32 ((2.0f * PI * i) / ((float32_t) VAD_FFT_SIZE - 1.0f)) + (0.16f / 2) * arm_cos_f32 (4.0f * PI * i / ((float32_t) VAD_FFT_SIZE - 1.0f));
}

void resetVAD(void)
{
	//need reset
	Min_E = 999.0f;
	Min_MD = 999.0f;
	start_counter = 0;
}

// run VAD for the data block
void processVAD(float32_t *buffer)
{
	if(!TRX.VAD_Squelch && !TRX_ScanMode) return;
	
	#define debug false
		
	// clear the old FFT buffer
	memset(VAD_FFTBuffer, 0x00, sizeof(VAD_FFTBuffer));
	// copy the incoming data for the next work
	memcpy(InputBuffer, buffer, sizeof(InputBuffer));
	// Decimator
	arm_fir_decimate_f32(&VAD_DECIMATE, InputBuffer, InputBuffer, VAD_BLOCK_SIZE);
	// Fill the unnecessary part of the buffer with zeros
	for (uint_fast16_t i = 0; i < VAD_FFT_SIZE; i++)
	{
		if (i < VAD_FFT_SAMPLES)
		{
			if (i < (VAD_FFT_SAMPLES - VAD_ZOOMED_SAMPLES)) // offset old data
				VAD_FFTBufferCharge[i] = VAD_FFTBufferCharge[(i + VAD_ZOOMED_SAMPLES)];
			else // Add new data to the FFT buffer for calculation
				VAD_FFTBufferCharge[i] = InputBuffer[i - (VAD_FFT_SAMPLES - VAD_ZOOMED_SAMPLES)];
			VAD_FFTBuffer[i] = window_multipliers[i] * VAD_FFTBufferCharge[i]; // + Window function for FFT
		}
		else
		{
			VAD_FFTBuffer[i] = 0.0f;
		}
	}

	// Do FFT
	arm_rfft_fast_f32(&VAD_FFT_Inst, VAD_FFTBuffer, VAD_FFTBuffer, 0);
	arm_cmplx_mag_f32(VAD_FFTBuffer, VAD_FFTBuffer, VAD_FFT_SIZE);

	//calc power
	float32_t power = 0;
	arm_power_f32(InputBuffer, VAD_BLOCK_SIZE, &power);

	//calc SFM — Spectral Flatness Measure
	float32_t Amean = 0;
	arm_mean_f32(VAD_FFTBuffer, VAD_FFT_SIZE_HALF, &Amean);
	float32_t Gmean = 0;
	for(uint32_t i = 0; i < VAD_FFT_SIZE_HALF ; i++)
		if(VAD_FFTBuffer[i] != 0)
			Gmean += log10f_fast(VAD_FFTBuffer[i]);
	Gmean = Gmean / VAD_FFT_SIZE_HALF;
	Gmean = powf(10, Gmean);
	float32_t SMFdb = 10 * log10f_fast(Gmean / Amean);
	
	//find most dominant frequency component
	float32_t MD = 0.0f;
	uint32_t MD_index = 0.0f;
	arm_max_f32(VAD_FFTBuffer, VAD_FFT_SIZE_HALF, &MD, &MD_index);
		
	//minimums
	if(start_counter < 100) //skip first packets
	{
		start_counter++;
		Min_E = 999.0f;
		Min_MD = 999.0f;
	}
	
	if(power < Min_E)
		Min_E = power;
	if(MD < Min_MD)
		Min_MD = MD;
	
	//calc results
	float32_t Res_E = power - Min_E;
	float32_t Res_MD = MD / Min_MD;
	float32_t Res_MD_IDX = fabsf(17 - (float32_t)MD_index); //17 - 190hz, voice dominant
	
	static uint32_t prevPrint = 0;
	
	uint8_t points = 0;
	if(Res_MD > 5.0f)
	{
		points++;
		if(debug && (HAL_GetTick() - prevPrint) > 100) sendToDebug_str("m");
	}
	if(SMFdb < -5.0f)
	{
		points++;
		if(debug && (HAL_GetTick() - prevPrint) > 100) sendToDebug_str("s");
	}
	if(Res_E > (Min_E * 100.0f))
	{
		points++;
		if(debug && (HAL_GetTick() - prevPrint) > 100) sendToDebug_str("e");
		if(Res_MD_IDX < 20)
		{
			points++;
			if(debug && (HAL_GetTick() - prevPrint) > 100) sendToDebug_str("i");
		}
	}
	if(debug && (HAL_GetTick() - prevPrint) > 100) sendToDebug_str(" ");
	
	//calculate result state
	static bool state = false;
	static uint16_t state_no_counter = 0;
	static uint16_t state_yes_counter = 0;
	if(points > 0)
	{
		state_yes_counter++;
		if(state_no_counter > 0)
			state_no_counter--;
	}
	else
	{
		state_no_counter++;
		if(state_yes_counter > 0)
			state_yes_counter--;
	}
	
	if(!state && state_yes_counter > 10)
	{
		state_yes_counter = 0;
		state_no_counter = 0;
		state = true;
	}
	if(state && state_no_counter > 700)
	{
		state_yes_counter = 0;
		state_no_counter = 0;
		state = false;
	}
	//move min averages
	if(!state && state_no_counter > 500)
	{
		Min_E = 0.9f * Min_E + 0.1f * power;
		Min_MD = 0.9f * Min_MD + 0.1f * MD;
		state_no_counter = 0;
	}
	if(state && state_yes_counter > 500)
	{
		Min_E = 0.999f * Min_E + 0.01f * power;
		Min_MD = 0.999f * Min_MD + 0.01f * MD;
		state_yes_counter = 0;
	}
	VAD_Muting = !state;
	
	//debug
	if(debug && (HAL_GetTick() - prevPrint) > 100)
	{
		sendToDebug_str(" PWR: ");
		sendToDebug_float32(power, true);
		sendToDebug_str(" SMF: ");
		sendToDebug_float32(SMFdb, true);
		sendToDebug_str(" MD: ");
		sendToDebug_float32(MD, true);
		sendToDebug_str(" Min_E: ");
		sendToDebug_float32(Min_E, true);
		sendToDebug_str(" Min_MD: ");
		sendToDebug_float32(Min_MD, true);
		sendToDebug_str(" Res_E: ");
		sendToDebug_float32(Res_E, true);
		sendToDebug_str(" Res_MD: ");
		sendToDebug_float32(Res_MD, true);
		sendToDebug_str(" MD_Idx: ");
		sendToDebug_float32(MD_index, true);
		sendToDebug_str(" Points: ");
		sendToDebug_float32(points, true);
		sendToDebug_str(" State: ");
		sendToDebug_float32(state, true);
		sendToDebug_newline();
		
		prevPrint = HAL_GetTick();
	}
}
