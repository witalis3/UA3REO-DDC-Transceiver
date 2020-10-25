#include "vad.h"
#include "arm_const_structs.h"
#include "settings.h"
#include "trx_manager.h"
#include "fft.h"

// https://habr.com/ru/post/192954/

const static arm_cfft_instance_f32 *VAD_FFT_Inst = &arm_cfft_sR_f32_len128;
IRAM2 static float32_t window_multipliers[VAD_FFT_SIZE] = {0}; // coefficients for the window function
IRAM2 static float32_t VAD_FFTBuffer[VAD_FFT_SIZE * 2] = {0};		 
IRAM2 static float32_t VAD_FFTBufferCharge[VAD_FFT_SIZE * 2] = {0}; // cumulative buffer
IRAM2 static float32_t InputBuffer[VAD_BLOCK_SIZE] = {0};		//Input buffer
//IRAM2 float32_t VAD_FFTBuffer_Export [VAD_FFT_SIZE] = {0};
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
bool VAD_Muting = false; 				//Muting flag
float32_t VAD_Equability = 1.0f;	//Bandwidth Equability

// initialize VAD
void InitVAD(void)
{
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
	
	#define debug true
	#define debug_th true
		
	// clear the old FFT buffer
	memset(VAD_FFTBuffer, 0x00, sizeof(VAD_FFTBuffer));
	// copy the incoming data for the next work
	memcpy(InputBuffer, buffer, sizeof(InputBuffer));
	// Decimator
	arm_fir_decimate_f32(&VAD_DECIMATE, InputBuffer, InputBuffer, VAD_BLOCK_SIZE);
	// Fill the unnecessary part of the buffer with zeros
	for (uint_fast16_t i = 0; i < VAD_FFT_SIZE; i++)
	{
		if (i < (VAD_FFT_SIZE - VAD_ZOOMED_SAMPLES)) // offset old data
			VAD_FFTBufferCharge[i] = VAD_FFTBufferCharge[(i + VAD_ZOOMED_SAMPLES)];
		else // Add new data to the FFT buffer for calculation
			VAD_FFTBufferCharge[i] = InputBuffer[i - (VAD_FFT_SIZE - VAD_ZOOMED_SAMPLES)];
		
		VAD_FFTBuffer[i * 2] = window_multipliers[i] * VAD_FFTBufferCharge[i]; // + Window function for FFT
		VAD_FFTBuffer[i * 2 + 1] = 0.0f;
	}
	
	// Do FFT
	arm_cfft_f32(VAD_FFT_Inst, VAD_FFTBuffer, 0, 1);
	arm_cmplx_mag_squared_f32(VAD_FFTBuffer, VAD_FFTBuffer, VAD_FFT_SIZE);

	//DEBUG VAD
	/*if(NeedFFTInputBuffer)
	{
		memset(VAD_FFTBuffer_Export, 0x00, sizeof VAD_FFTBuffer_Export);
		for (uint_fast16_t i = 0; i < VAD_FFT_SIZE_HALF; i ++)
			VAD_FFTBuffer_Export[i] = VAD_FFTBuffer[i];
		for (uint_fast16_t i = 0; i < VAD_FFT_SIZE_HALF; i ++)
			VAD_FFTBuffer_Export[i + VAD_FFT_SIZE_HALF] = VAD_FFTBuffer[i];
		NeedFFTInputBuffer = false;
	}
	return;*/
	
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
	float32_t Res_MD_IDX = fabsf(8 - (float32_t)MD_index); //8 ~190hz, voice dominant
	
	static uint32_t prevPrint = 0;
	
	uint8_t points = 0;
	if(Res_MD > 10.0f)
	{
		points++;
		if(debug_th && (HAL_GetTick() - prevPrint) > 100) sendToDebug_str("m");
		if(Res_MD_IDX < 10)
		{
			points++;
			if(debug_th && (HAL_GetTick() - prevPrint) > 100) sendToDebug_str("i");
		}
	}
	if(SMFdb < -13.0f)
	{
		points++;
		if(debug_th && (HAL_GetTick() - prevPrint) > 100) sendToDebug_str("s");
	}
	if(Res_E > (Min_E * 100.0f))
	{
		points++;
		if(debug_th && (HAL_GetTick() - prevPrint) > 100) sendToDebug_str("e");
	}
	if(debug_th && (HAL_GetTick() - prevPrint) > 100) sendToDebug_str(" ");
	
	//calculate result state
	static bool state = false;
	static uint16_t state_no_counter = 0;
	static uint16_t state_yes_counter = 0;
	if(points > 1)
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
		VAD_Equability = 1.0f;
	}
	if(state && state_no_counter > 700)
	{
		state_yes_counter = 0;
		state_no_counter = 0;
		state = false;
		VAD_Equability = 1.0f;
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
	
	//calc bw equability
	if(state)
	{
		uint32_t fft_bw = (TRX_SAMPLERATE / VAD_MAGNIFY / 2);
		uint32_t fft_bins = VAD_FFT_SIZE / 2;
		uint32_t trx_hpf = CurrentVFO()->HPF_Filter_Width;
		uint32_t trx_lpf = CurrentVFO()->LPF_Filter_Width;
		uint32_t fft_hpf_bin = fft_bins * trx_hpf / fft_bw;
		uint32_t fft_lpf_bin = fft_bins * trx_lpf / fft_bw;
		if(fft_hpf_bin > fft_bw)
			fft_hpf_bin = fft_bw;
		if(fft_lpf_bin > fft_bw)
			fft_lpf_bin = fft_bw;
		uint32_t fft_center_bin = (fft_hpf_bin + fft_lpf_bin) / 2;
		float32_t low_sum = 0;
		float32_t high_sum = 0;
		for(uint32_t bin = fft_hpf_bin ; bin < fft_center_bin ; bin++)
			low_sum += VAD_FFTBuffer[bin];
		for(uint32_t bin = fft_center_bin ; bin < fft_lpf_bin ; bin++)
			high_sum += VAD_FFTBuffer[bin];
		VAD_Equability = 0.99f * VAD_Equability + 0.01f * (low_sum / high_sum);
	}
	if(VAD_Equability < 0.1f)
		state = false;
	
	//set mute state
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
		sendToDebug_str(" EQU: ");
		sendToDebug_float32(VAD_Equability, true);
		sendToDebug_str(" State: ");
		sendToDebug_float32(state, true);
		sendToDebug_newline();
		
		prevPrint = HAL_GetTick();
	}
}
