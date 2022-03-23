#include "vad.h"
#include "arm_const_structs.h"
#include "settings.h"
#include "trx_manager.h"
#include "fft.h"

// https://habr.com/ru/post/192954/

const static arm_cfft_instance_f32 *VAD_FFT_Inst = &arm_cfft_sR_f32_len128;
static float32_t window_multipliers[VAD_FFT_SIZE] = {0}; // coefficients for the window function
static float32_t VAD_FFTBuffer[VAD_FFT_SIZE * 2] = {0};
static float32_t VAD_FFTBufferCharge[VAD_FFT_SIZE * 2] = {0}; // cumulative buffer
static float32_t InputBuffer[VAD_BLOCK_SIZE] = {0};			  // Input buffer
// IRAM2 float32_t VAD_FFTBuffer_Export [VAD_FFT_SIZE] = {0};
static float32_t VAD_decimState[VAD_BLOCK_SIZE + 4 - 1];
static arm_fir_decimate_instance_f32 VAD_DECIMATE;
static const arm_fir_decimate_instance_f32 VAD_FirDecimate =
	{
		// 48ksps, 3kHz lowpass
		.numTaps = 4,
		.pCoeffs = (float32_t *)(const float32_t[]){0.199820836596682871f, 0.272777397353925699f, 0.272777397353925699f, 0.199820836596682871f},
		.pState = NULL,
};
static float32_t Min_E1 = 999.0f;
static float32_t Min_E2 = 999.0f;
static float32_t Min_MD1 = 999.0f;
static float32_t Min_MD2 = 999.0f;
static uint32_t start_counter = 0;
bool VAD_Muting = false; // Muting flag

// initialize VAD
void InitVAD(void)
{
	// decimator
	arm_fir_decimate_init_f32(&VAD_DECIMATE, VAD_FirDecimate.numTaps, VAD_MAGNIFY, VAD_FirDecimate.pCoeffs, VAD_decimState, VAD_BLOCK_SIZE);
	// Blackman window function
	for (uint_fast16_t i = 0; i < VAD_FFT_SIZE; i++)
		window_multipliers[i] = ((1.0f - 0.16f) / 2) - 0.5f * arm_cos_f32((2.0f * PI * i) / ((float32_t)VAD_FFT_SIZE - 1.0f)) + (0.16f / 2) * arm_cos_f32(4.0f * PI * i / ((float32_t)VAD_FFT_SIZE - 1.0f));
}

void resetVAD(void)
{
	// need reset
	Min_E1 = 999.0f;
	Min_E2 = 999.0f;
	Min_MD1 = 999.0f;
	Min_MD2 = 999.0f;
	start_counter = 0;
}

// run VAD for the data block
void processVAD(float32_t *buffer)
{
	if (!CurrentVFO->SQL && !TRX_ScanMode)
		return;

#define debug false

	// clear the old FFT buffer
	dma_memset(VAD_FFTBuffer, 0x00, sizeof(VAD_FFTBuffer));
	// copy the incoming data for the next work
	dma_memcpy(InputBuffer, buffer, sizeof(InputBuffer));
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

	// DEBUG VAD
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

	// calc bw
	uint32_t fft_bw = (TRX_SAMPLERATE / VAD_MAGNIFY / 2);
	uint32_t fft_bins = VAD_FFT_SIZE / 2;
	uint32_t trx_hpf = CurrentVFO->HPF_RX_Filter_Width;
	uint32_t trx_lpf = CurrentVFO->LPF_RX_Filter_Width;
	if (trx_lpf < trx_hpf || trx_lpf == 0)
		trx_lpf = fft_bw;
	uint32_t fft_hpf_bin = fft_bins * trx_hpf / fft_bw;
	uint32_t fft_lpf_bin = fft_bins * trx_lpf / fft_bw;
	if (fft_hpf_bin > fft_bw)
		fft_hpf_bin = fft_bw;
	if (fft_lpf_bin > fft_bw)
		fft_lpf_bin = fft_bw;
	uint32_t fft_center_bin = (fft_hpf_bin + fft_lpf_bin) / 2;
	uint32_t fft_bin_halflen = fft_lpf_bin - fft_center_bin;

	// calc power
	float32_t power1 = 0;
	float32_t power2 = 0;
	for (uint32_t bin = fft_hpf_bin; bin < fft_center_bin; bin++)
		power1 += VAD_FFTBuffer[bin];
	for (uint32_t bin = fft_center_bin; bin < fft_lpf_bin; bin++)
		power2 += VAD_FFTBuffer[bin];

	// calc SFM — Spectral Flatness Measure
	float32_t Amean1 = 0;
	float32_t Amean2 = 0;
	arm_mean_f32(&VAD_FFTBuffer[fft_hpf_bin], fft_bin_halflen, &Amean1);
	arm_mean_f32(&VAD_FFTBuffer[fft_center_bin], fft_bin_halflen, &Amean2);
	float32_t Gmean1 = 0;
	float32_t Gmean2 = 0;
	for (uint32_t i = 0; i < fft_center_bin; i++)
		if (VAD_FFTBuffer[i] != 0)
			Gmean1 += log10f_fast(VAD_FFTBuffer[i]);
	for (uint32_t i = fft_center_bin; i < fft_lpf_bin; i++)
		if (VAD_FFTBuffer[i] != 0)
			Gmean2 += log10f_fast(VAD_FFTBuffer[i]);
	Gmean1 = Gmean1 / fft_bin_halflen;
	Gmean1 = powf(10, Gmean1);
	float32_t SMFdb1 = 10 * log10f_fast(Gmean1 / Amean1);
	Gmean2 = Gmean2 / fft_bin_halflen;
	Gmean2 = powf(10, Gmean2);
	float32_t SMFdb2 = 10 * log10f_fast(Gmean2 / Amean2);

	// find most dominant frequency component
	float32_t MD1 = 0.0f;
	uint32_t MD1_index = 0.0f;
	arm_max_f32(&VAD_FFTBuffer[fft_hpf_bin], fft_bin_halflen, &MD1, &MD1_index);
	float32_t MD2 = 0.0f;
	uint32_t MD2_index = 0.0f;
	arm_max_f32(&VAD_FFTBuffer[fft_center_bin], fft_bin_halflen, &MD2, &MD2_index);

	// minimums
	if (start_counter < 100) // skip first packets
	{
		start_counter++;
		Min_E1 = 999.0f;
		Min_E2 = 999.0f;
		Min_MD1 = 999.0f;
		Min_MD2 = 999.0f;
	}

	if (power1 < Min_E1)
		Min_E1 = power1;
	if (power2 < Min_E2)
		Min_E2 = power2;
	if (MD1 < Min_MD1)
		Min_MD1 = MD1;
	if (MD2 < Min_MD2)
		Min_MD2 = MD2;

	// calc results
	float32_t Res_E1 = power1 / Min_E1;
	float32_t Res_E2 = power2 / Min_E2;
	float32_t Res_MD1 = MD1 / Min_MD1;
	float32_t Res_MD2 = MD2 / Min_MD2;
	float32_t Res_MD1_IDX = fabsf(5 - (float32_t)MD1_index);  // 8 - voice dominant bin
	float32_t Res_MD2_IDX = fabsf(17 - (float32_t)MD2_index); // 20 - voice dominant bin
	float32_t Res_Equation = Res_E1 / Res_E2;

	// debug
	static uint32_t prevPrint = 0;
	if (debug && (HAL_GetTick() - prevPrint) > 100)
	{
		print(" SMF1: ", SMFdb1, " SMF2: ", SMFdb2, " Res_E1: ", Res_E1, " Res_E2: ", Res_E2, " Res_MD1: ", Res_MD1, " Res_MD2: ", Res_MD2, " MD1_Idx: ", MD1_index, " MD2_Idx: ", MD2_index, " EQU: ", Res_Equation, " ");
	}

	// check thresholds
	uint8_t points1 = 0;
	uint8_t points2 = 0;
	if (Res_MD1 > 15.0f)
	{
		points1++;
		if (debug && (HAL_GetTick() - prevPrint) > 100)
			print("M");
		if (Res_MD1_IDX < 5)
		{
			points1++;
			if (debug && (HAL_GetTick() - prevPrint) > 100)
				print("I");
		}
	}
	if (Res_MD2 > 7.0f)
	{
		points2++;
		if (debug && (HAL_GetTick() - prevPrint) > 100)
			print("m");
		if (Res_MD2_IDX < 10)
		{
			points2++;
			if (debug && (HAL_GetTick() - prevPrint) > 100)
				print("i");
		}
	}
	if (SMFdb1 < -23.0f)
	{
		points1++;
		if (debug && (HAL_GetTick() - prevPrint) > 100)
			print("S");
	}
	if (SMFdb2 < -4.0f)
	{
		points2++;
		if (debug && (HAL_GetTick() - prevPrint) > 100)
			print("s");
	}
	if (Res_E1 > 10.0f)
	{
		points1++;
		if (debug && (HAL_GetTick() - prevPrint) > 100)
			print("E");
	}
	if (Res_E2 > 10.0f)
	{
		points2++;
		if (debug && (HAL_GetTick() - prevPrint) > 100)
			print("e");
	}
	if (Res_Equation > 20.0f)
	{
		points1--;
		if (debug && (HAL_GetTick() - prevPrint) > 100)
			print("Q");
	}
	if (Res_Equation < 0.1f)
	{
		points2--;
		if (debug && (HAL_GetTick() - prevPrint) > 100)
			print("q");
	}
	if (debug && (HAL_GetTick() - prevPrint) > 100)
	{
		println("");
		prevPrint = HAL_GetTick();
	}

	// calculate result state
	static bool state = false;
	static uint16_t state_no_counter = 0;
	static uint16_t state_yes_counter = 0;
	if (points1 > 1 && points2 > 1)
	{
		state_yes_counter++;
		if (state_no_counter > 0)
			state_no_counter--;
	}
	else
	{
		state_no_counter++;
		if (state_yes_counter > 0)
			state_yes_counter--;
	}

	if (!state && state_yes_counter > 10)
	{
		state_yes_counter = 0;
		state_no_counter = 0;
		state = true;
	}
	if (state && state_no_counter > 1000)
	{
		state_yes_counter = 0;
		state_no_counter = 0;
		state = false;
	}

	// move min averages
	if (!state && state_no_counter > 500)
	{
		Min_E1 = 0.9f * Min_E1 + 0.1f * power1;
		Min_E2 = 0.9f * Min_E2 + 0.1f * power2;
		Min_MD1 = 0.9f * Min_MD1 + 0.1f * MD1;
		Min_MD2 = 0.9f * Min_MD2 + 0.1f * MD2;
		state_no_counter = 0;
	}
	if (state && state_yes_counter > 500)
	{
		Min_E1 = 0.999f * Min_E1 + 0.01f * power1;
		Min_E2 = 0.999f * Min_E2 + 0.01f * power2;
		Min_MD1 = 0.999f * Min_MD1 + 0.01f * MD1;
		Min_MD2 = 0.999f * Min_MD2 + 0.01f * MD2;
		state_yes_counter = 0;
	}

	// set mute state
	VAD_Muting = !state;
}
