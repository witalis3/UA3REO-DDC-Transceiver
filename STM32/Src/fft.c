#include "fft.h"
#include "main.h"
#include "arm_const_structs.h"
#include "audio_filters.h"
#include "screen_layout.h"
#include "vad.h"
#include "lcd.h"
#include "cw_decoder.h"
#include "wifi.h"
#include "trx_manager.h"

// Public variables
bool FFT_need_fft = true;						   // need to prepare data for display on the screen
bool FFT_new_buffer_ready = false;				   // buffer is full, can be processed
uint32_t FFT_buff_index = 0;					   // current buffer index when it is filled with FPGA
bool FFT_buff_current = 0;						   // current FFT Input buffer A - false, B - true
IRAM2 float32_t FFTInput_I_A[FFT_HALF_SIZE] = {0}; // incoming buffer FFT I
IRAM2 float32_t FFTInput_Q_A[FFT_HALF_SIZE] = {0}; // incoming buffer FFT Q
IRAM2 float32_t FFTInput_I_B[FFT_HALF_SIZE] = {0}; // incoming buffer FFT I
IRAM2 float32_t FFTInput_Q_B[FFT_HALF_SIZE] = {0}; // incoming buffer FFT Q
uint16_t FFT_FPS = 0;
uint16_t FFT_FPS_Last = 0;
bool NeedWTFRedraw = false;
bool NeedFFTReinit = false;
uint32_t FFT_current_spectrum_width_hz = 96000; // Current sectrum width

// Private variables
#if FFT_SIZE == 2048
const static arm_cfft_instance_f32 *FFT_Inst = &arm_cfft_sR_f32_len2048;
#endif
#if FFT_SIZE == 1024
const static arm_cfft_instance_f32 *FFT_Inst = &arm_cfft_sR_f32_len1024;
#endif
#if FFT_SIZE == 512
const static arm_cfft_instance_f32 *FFT_Inst = &arm_cfft_sR_f32_len512;
#endif
#if FFT_SIZE == 256
const static arm_cfft_instance_f32 *FFT_Inst = &arm_cfft_sR_f32_len256;
#endif

static float32_t FFTInputCharge[FFT_DOUBLE_SIZE_BUFFER] = {0};				   // charge FFT I and Q buffer
IRAM2 static float32_t FFTInput[FFT_DOUBLE_SIZE_BUFFER] = {0};				   // combined FFT I and Q buffer
IRAM2 static float32_t FFTInput_tmp[MAX_FFT_PRINT_SIZE] = {0};				   // temporary buffer for sorting, moving and fft compressing
SRAM static float32_t FFT_meanBuffer[FFT_MAX_MEANS][MAX_FFT_PRINT_SIZE] = {0}; // averaged FFT buffer (for output)
IRAM2 static float32_t FFTOutput_mean[MAX_FFT_PRINT_SIZE] = {0};			   // averaged FFT buffer (for output)
IRAM2 static float32_t FFTOutput_average[MAX_FFT_PRINT_SIZE] = {0};			   // averaged FFT buffer (for output)
static float32_t maxValueFFT_rx = 0;										   // maximum value of the amplitude in the resulting frequency response
static float32_t maxValueFFT_tx = 0;										   // maximum value of the amplitude in the resulting frequency response
static uint64_t currentFFTFreq = 0;
static uint64_t lastWTFFreq = 0;													// last WTF printed freq
static uint16_t palette_fft[MAX_FFT_HEIGHT + 1] = {0};								// color palette with FFT colors
static uint16_t palette_wtf[MAX_FFT_HEIGHT + 1] = {0};								// color palette with FFT colors
static uint16_t palette_bg_gradient[MAX_FFT_HEIGHT + 1] = {0};						// color palette with gradient background of FFT
static uint16_t palette_bw_fft_colors[MAX_FFT_HEIGHT + 1] = {0};					// color palette with bw highlighted FFT colors
static uint16_t palette_bw_wtf_colors[MAX_FFT_HEIGHT + 1] = {0};					// color palette with bw highlighted FFT colors
static uint16_t palette_bw_bg_colors[MAX_FFT_HEIGHT + 1] = {0};						// color palette with bw highlighted background colors
IRAM2 uint16_t print_output_buffer[FFT_AND_WTF_HEIGHT][MAX_FFT_PRINT_SIZE] = {{0}}; // buffer with fft/3d fft/wtf print data
SRAM static uint8_t indexed_wtf_buffer[MAX_WTF_HEIGHT][MAX_FFT_PRINT_SIZE] = {{0}}; // indexed color buffer with wtf
static uint64_t wtf_buffer_freqs[MAX_WTF_HEIGHT] = {0};								// frequencies for each row of the waterfall
static uint64_t fft_meanbuffer_freqs[FFT_MAX_MEANS] = {0};							// frequencies for each row of the fft mean buffer
IRAM2 static uint16_t fft_header[MAX_FFT_PRINT_SIZE] = {0};							// buffer with fft colors output
IRAM2 static uint16_t fft_peaks[MAX_FFT_PRINT_SIZE] = {0};							// buffer with fft peaks
static int32_t grid_lines_pos[20] = {-1};											// grid lines positions
static uint64_t grid_lines_freq[20] = {-1};											// grid lines frequencies
static int32_t rx2_line_pos = -1;													// secondary receiver position on fft
static int32_t bw_rx1_line_start = 0;												// BW bar params RX1
static int32_t bw_rx1_line_end = 0;													// BW bar params RX1
static int32_t bw_rx2_line_start = 0;												// BW bar params RX2
static int32_t bw_rx2_line_end = 0;													// BW bar params RX2
static float32_t window_multipliers[FFT_SIZE] = {0};								// coefficients of the selected window function
static float32_t hz_in_pixel = 1.0f;												// current FFT density value
static uint16_t bandmap_line_tmp[MAX_FFT_PRINT_SIZE] = {0};							// temporary buffer to move the waterfall
static uint32_t print_fft_dma_estimated_size = 0;									// block size for dma
static uint32_t print_fft_dma_position = 0;											// positior for dma fft print
static uint8_t needredraw_wtf_counter = 3;											// redraw cycles after event
static bool fft_charge_ready = false;
static bool fft_charge_copying = false;
static uint8_t FFT_meanBuffer_index = 0;
static uint32_t FFT_ChargeBuffer_collected = 0;
static uint64_t FFT_lastFFTChargeBufferFreq = 0;
//static uint8_t FFTOutput_mean_count[FFT_SIZE] = {0};
static float32_t minAmplValue_averaged = 0;
static float32_t FFT_minDBM = 0;
static float32_t FFT_maxDBM = 0;
// Decimator for Zoom FFT
static arm_fir_decimate_instance_f32 DECIMATE_ZOOM_FFT_I;
static arm_fir_decimate_instance_f32 DECIMATE_ZOOM_FFT_Q;
static float32_t decimZoomFFTIState[FFT_HALF_SIZE + ZOOMFFT_DECIM_STAGES_FIR - 1] = {0};
static float32_t decimZoomFFTQState[FFT_HALF_SIZE + ZOOMFFT_DECIM_STAGES_FIR - 1] = {0};
static uint8_t fft_zoom = 1;
static uint_fast16_t zoomed_width = 0;
//Коэффициенты для ZoomFFT lowpass filtering / дециматора
static arm_biquad_cascade_df2T_instance_f32 IIR_biquad_Zoom_FFT_I =
	{
		.numStages = ZOOMFFT_DECIM_STAGES_IIR,
		.pCoeffs = (float32_t *)(float32_t[ZOOMFFT_DECIM_STAGES_IIR * 5]){0},
		.pState = (float32_t *)(float32_t[ZOOMFFT_DECIM_STAGES_IIR * 2]){0}};
static arm_biquad_cascade_df2T_instance_f32 IIR_biquad_Zoom_FFT_Q =
	{
		.numStages = ZOOMFFT_DECIM_STAGES_IIR,
		.pCoeffs = (float32_t *)(float32_t[ZOOMFFT_DECIM_STAGES_IIR * 5]){0},
		.pState = (float32_t *)(float32_t[ZOOMFFT_DECIM_STAGES_IIR * 2]){0}};

static const float32_t *mag_coeffs[17] =
	{
		NULL, // 0
		NULL, // 1
		// 2x magnify lpf iir
		(const float32_t *)(const float32_t[ZOOMFFT_DECIM_STAGES_IIR * 5]){0.8384843639921, 0, 0, 0, 0, 1, 0.5130084793341, 1, 0.1784114407685, -0.6967733943344, 0.8744089756375, 0, 0, 0, 0, 1, 1.046379755684, 1, 0.3420998857106, -0.3982809814397, 1.83222755502, 0, 0, 0, 0, 1, 1.831496024383, 1, 0.5072844084012, -0.1179052535088, 0.01953722920982, 0, 0, 0, 0, 1, 0.3029841730578, 1, 0.09694668293684, -0.9095549467394, 1, 0, 0, 0, 0},
		NULL, // 3
		// 4x magnify lpf iir
		(const float32_t *)(const float32_t[ZOOMFFT_DECIM_STAGES_IIR * 5]){0.6737499659657, 0, 0, 0, 0, 1, -1.102065194995, 1, 1.353694541279, -0.7896377861467, 0.53324811147, 0, 0, 0, 0, 1, -0.5853766477218, 1, 1.289175897987, -0.5882714065646, 0.6143152247695, 0, 0, 0, 0, 1, 1.182778527244, 1, 1.236309127239, -0.4063767082903, 0.01708381580242, 0, 0, 0, 0, 1, -1.245590418009, 1, 1.418191929315, -0.9374008035325, 1, 0, 0, 0, 0},
		NULL, // 5
		NULL, // 6
		NULL, // 7
		// 8x magnify lpf iir
		(const float32_t *)(const float32_t[ZOOMFFT_DECIM_STAGES_IIR * 5]){0.6469981129046, 0, 0, 0, 0, 1, -1.750671284068, 1, 1.766710155669, -0.8829517893283, 0.4645312725883, 0, 0, 0, 0, 1, -1.553480572725, 1, 1.681513354365, -0.7637556184482, 0.2925692260954, 0, 0, 0, 0, 1, -0.1114766808264, 1, 1.601891439147, -0.6499504503566, 0.01652325734055, 0, 0, 0, 0, 1, -1.797298202754, 1, 1.831125104215, -0.9660534813317, 1, 0, 0, 0, 0},
		NULL, // 9
		NULL, // 10
		NULL, // 11
		NULL, // 12
		NULL, // 13
		NULL, // 14
		NULL, // 15
		// 16x magnify lpf iir
		(const float32_t *)(const float32_t[ZOOMFFT_DECIM_STAGES_IIR * 5]){0.6500044972642, 0, 0, 0, 0, 1, -1.935616780918, 1, 1.908632776595, -0.9387888949475, 0.4599444315799, 0, 0, 0, 0, 1, -1.880017827578, 1, 1.851418291083, -0.8732990221737, 0.2087317940803, 0, 0, 0, 0, 1, -1.278402634611, 1, 1.794539349192, -0.80764043772, 0.01645106748385, 0, 0, 0, 0, 1, -1.948135342532, 1, 1.948194658987, -0.9825675157696, 1, 0, 0, 0, 0},
};

static const arm_fir_decimate_instance_f32 FirZoomFFTDecimate[17] =
	{
		{0}, // 0
		{0}, // 1
		// 2x magnify decimate fir
		{
			.numTaps = ZOOMFFT_DECIM_STAGES_FIR,
			.pCoeffs = (const float32_t *)(const float32_t[ZOOMFFT_DECIM_STAGES_FIR]){-0.05698952454792, 0.5574889164132, 0.5574889164132, -0.05698952454792},
			.pState = NULL},
		{0}, // 3
		// 4x magnify decimate fir
		{
			.numTaps = ZOOMFFT_DECIM_STAGES_FIR,
			.pCoeffs = (const float32_t *)(const float32_t[ZOOMFFT_DECIM_STAGES_FIR]){-0.05698952454792, 0.5574889164132, 0.5574889164132, -0.05698952454792},
			.pState = NULL},
		{0}, // 5
		{0}, // 6
		{0}, // 7
		// 8x magnify decimate fir
		{
			.numTaps = ZOOMFFT_DECIM_STAGES_FIR,
			.pCoeffs = (const float32_t *)(const float32_t[ZOOMFFT_DECIM_STAGES_FIR]){-0.05698952454792, 0.5574889164132, 0.5574889164132, -0.05698952454792},
			.pState = NULL},
		{0}, // 9
		{0}, // 10
		{0}, // 11
		{0}, // 12
		{0}, // 13
		{0}, // 14
		{0}, // 15
		// 16x magnify decimate fir
		{
			.numTaps = ZOOMFFT_DECIM_STAGES_FIR,
			.pCoeffs = (const float32_t *)(const float32_t[ZOOMFFT_DECIM_STAGES_FIR]){-0.05698952454792, 0.5574889164132, 0.5574889164132, -0.05698952454792},
			.pState = NULL},
};

// Prototypes
static uint16_t getFFTColor(uint_fast8_t height, bool type);	   // Get FFT color warmth (blue to red) , type 0 - fft, type 1 - wtf
static void FFT_fill_color_palette(void);						   // prepare the color palette
static int32_t getFreqPositionOnFFT(uint64_t freq, bool full_pos); // get the position on the FFT for a given frequency
static uint32_t FFT_getLensCorrection(uint32_t normal_distance_from_center);
static void FFT_3DPrintFFT(void);
static float32_t getDBFromFFTAmpl(float32_t ampl);
static float32_t getFFTAmplFromDB(float32_t ampl);

// FFT initialization
void FFT_PreInit(void)
{
	// Windowing
	// Dolph–Chebyshev
	if (TRX.FFT_Window == 1)
	{
		const float64_t atten = 100.0;
		float64_t max = 0.0;
		float64_t tg = pow(10.0, atten / 20.0);
		float64_t x0 = cosh((1.0 / ((float64_t)FFT_SIZE - 1.0)) * acosh(tg));
		float64_t M = (float32_t)(FFT_SIZE - 1) / 2.0f;
		//if ((FFT_SIZE % 2) == 0) M = M + 0.5; /* handle even length windows */
		for (uint32_t nn = 0; nn < ((FFT_SIZE / 2) + 1); nn++)
		{
			float64_t n = nn - M;
			float64_t sum = 0.0;
			for (uint32_t i = 1; i <= M; i++)
			{
				float64_t cheby_poly = 0.0;
				float64_t cp_x = x0 * arm_cos_f32(F_PI * i / (float64_t)FFT_SIZE);
				float64_t cp_n = FFT_SIZE - 1;
				if (fabs(cp_x) <= 1)
					cheby_poly = cos(cp_n * acos(cp_x));
				else
					cheby_poly = cosh(cp_n * acosh(cp_x));

				sum += cheby_poly * cos(2.0 * n * F_PI * (float64_t)i / (float64_t)FFT_SIZE);
			}
			window_multipliers[nn] = tg + 2 * sum;
			window_multipliers[FFT_SIZE - nn - 1] = window_multipliers[nn];
			if (window_multipliers[nn] > max)
				max = window_multipliers[nn];
		}
		for (uint32_t nn = 0; nn < FFT_SIZE; nn++)
			window_multipliers[nn] /= max; /* normalise everything */
	}
	for (uint_fast16_t i = 0; i < FFT_SIZE; i++)
	{
		// Blackman-Harris
		if (TRX.FFT_Window == 2)
			window_multipliers[i] = 0.35875f - 0.48829f * arm_cos_f32(2.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) + 0.14128f * arm_cos_f32(4.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) - 0.01168f * arm_cos_f32(6.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f));
		// Nutall
		else if (TRX.FFT_Window == 3)
			window_multipliers[i] = 0.355768f - 0.487396f * arm_cos_f32(2.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) + 0.144232f * arm_cos_f32(4.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) - 0.012604 * arm_cos_f32(6.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f));
		// Blackman-Nutall
		else if (TRX.FFT_Window == 4)
			window_multipliers[i] = 0.3635819f - 0.4891775f * arm_cos_f32(2.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) + 0.1365995f * arm_cos_f32(4.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) - 0.0106411f * arm_cos_f32(6.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f));
		// Hann
		else if (TRX.FFT_Window == 5)
			window_multipliers[i] = 0.5f * (1.0f - arm_cos_f32(2.0f * F_PI * (float32_t)i / (float32_t)FFT_SIZE));
		// Hamming
		else if (TRX.FFT_Window == 6)
			window_multipliers[i] = 0.54f - 0.46f * arm_cos_f32((2.0f * F_PI * (float32_t)i) / ((float32_t)FFT_SIZE - 1.0f));
		// No window
		else if (TRX.FFT_Window == 7)
			window_multipliers[i] = 1.0f;
	}
}

void FFT_Init(void)
{
	FFT_fill_color_palette();
	// ZoomFFT
	fft_zoom = TRX.FFT_Zoom;
	if (CurrentVFO->Mode == TRX_MODE_CW)
		fft_zoom = TRX.FFT_ZoomCW;
	if (fft_zoom > 1)
	{
		arm_biquad_cascade_df2T_init_f32(&IIR_biquad_Zoom_FFT_I, ZOOMFFT_DECIM_STAGES_IIR, mag_coeffs[fft_zoom], IIR_biquad_Zoom_FFT_I.pState);
		arm_biquad_cascade_df2T_init_f32(&IIR_biquad_Zoom_FFT_Q, ZOOMFFT_DECIM_STAGES_IIR, mag_coeffs[fft_zoom], IIR_biquad_Zoom_FFT_Q.pState);
		arm_fir_decimate_init_f32(&DECIMATE_ZOOM_FFT_I,
								  FirZoomFFTDecimate[fft_zoom].numTaps,
								  fft_zoom, // Decimation factor
								  FirZoomFFTDecimate[fft_zoom].pCoeffs,
								  decimZoomFFTIState, // Filter state variables
								  FFT_HALF_SIZE);

		arm_fir_decimate_init_f32(&DECIMATE_ZOOM_FFT_Q,
								  FirZoomFFTDecimate[fft_zoom].numTaps,
								  fft_zoom, // Decimation factor
								  FirZoomFFTDecimate[fft_zoom].pCoeffs,
								  decimZoomFFTQState, // Filter state variables
								  FFT_HALF_SIZE);
		zoomed_width = FFT_SIZE / fft_zoom;
	}
	else
		zoomed_width = FFT_SIZE;

	if (TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK)
		FFT_current_spectrum_width_hz = TRX_SAMPLERATE / fft_zoom;
	else
		FFT_current_spectrum_width_hz = TRX_GetRXSampleRate / fft_zoom;

	// clear the buffers
	uint16_t color = palette_wtf[GET_FFTHeight];
	if (TRX.FFT_Automatic)
		color = palette_wtf[(uint32_t)(GET_FFTHeight * 0.9f)];
	memset16(print_output_buffer, color, sizeof(print_output_buffer) / 2);
	dma_memset(indexed_wtf_buffer, GET_FFTHeight, sizeof(indexed_wtf_buffer));
	dma_memset(wtf_buffer_freqs, 0x00, sizeof(wtf_buffer_freqs));
	dma_memset(fft_meanbuffer_freqs, 0x00, sizeof(fft_meanbuffer_freqs));
	dma_memset(FFT_meanBuffer, 0x00, sizeof(FFT_meanBuffer));
	dma_memset(FFTInputCharge, 0x00, sizeof(FFTInputCharge));
	dma_memset(FFTInput, 0x00, sizeof(FFTInput));
	dma_memset(FFTInput_I_A, 0x00, sizeof(FFTInput_I_A));
	dma_memset(FFTInput_Q_A, 0x00, sizeof(FFTInput_Q_A));
	dma_memset(FFTInput_I_B, 0x00, sizeof(FFTInput_I_B));
	dma_memset(FFTInput_Q_B, 0x00, sizeof(FFTInput_Q_B));
	dma_memset(FFTOutput_mean, 0x00, sizeof(FFTOutput_mean));
	dma_memset(FFTInput_tmp, 0x00, sizeof(FFTInput_tmp));
	NeedWTFRedraw = true;
	FFT_new_buffer_ready = false;
	
	FFT_buff_index = 0;
	FFTInput_I_current = (float32_t *)&FFTInput_I_A[0];
	FFTInput_Q_current = (float32_t *)&FFTInput_Q_A[0];
	
	NeedFFTReinit = false;
}

// FFT calculation
void FFT_bufferPrepare(void)
{
	if (!TRX.FFT_Enabled)
		return;
	if (!FFT_new_buffer_ready)
		return;
	if (fft_charge_copying)
		return;

	/*if (CPU_LOAD.Load > 90)
		return;*/
	fft_charge_ready = false;

	float32_t *FFTInput_I_current = FFT_buff_current ? (float32_t *)FFTInput_I_B : (float32_t *)FFTInput_I_A; // inverted
	float32_t *FFTInput_Q_current = FFT_buff_current ? (float32_t *)FFTInput_Q_B : (float32_t *)FFTInput_Q_A;

	// Process DC corrector filter
	if (!TRX_on_TX)
	{
		// dc_filter(FFTInput_I_current, FFT_HALF_SIZE, DC_FILTER_FFT_I);
		// dc_filter(FFTInput_Q_current, FFT_HALF_SIZE, DC_FILTER_FFT_Q);
	}

	// Reset old samples if frequency changed
	uint64_t nowFFTChargeBufferFreq = CurrentVFO->Freq;
	if (TRX.WTF_Moving && fabsl((float64_t)FFT_lastFFTChargeBufferFreq - (float64_t)nowFFTChargeBufferFreq) > (500 / fft_zoom)) // zeroing threshold
	{
		dma_memset(FFTInputCharge, 0x00, sizeof(FFTInputCharge));
		FFT_ChargeBuffer_collected = 0;
	}
	FFT_lastFFTChargeBufferFreq = nowFFTChargeBufferFreq;

	// ZoomFFT
	if (fft_zoom > 1)
	{
		uint32_t zoomed_width_half = zoomed_width / 2;
		// Biquad LPF filter
		arm_biquad_cascade_df2T_f32_IQ(&IIR_biquad_Zoom_FFT_I, &IIR_biquad_Zoom_FFT_Q, FFTInput_I_current, FFTInput_Q_current, FFTInput_I_current, FFTInput_Q_current, FFT_HALF_SIZE);
		// Decimator
		arm_fir_decimate_f32(&DECIMATE_ZOOM_FFT_I, FFTInput_I_current, FFTInput_I_current, FFT_HALF_SIZE);
		arm_fir_decimate_f32(&DECIMATE_ZOOM_FFT_Q, FFTInput_Q_current, FFTInput_Q_current, FFT_HALF_SIZE);
		// Shift old data
		dma_memcpy(&FFTInputCharge[0], &FFTInputCharge[zoomed_width_half * 2], sizeof(float32_t) * (FFT_SIZE - zoomed_width_half) * 2); //*2 - >i+q
		// Add new data with 50% overlap
		for (uint_fast16_t i = 0; i < zoomed_width_half; i++)
		{
			FFTInputCharge[(FFT_SIZE - zoomed_width_half + i) * 2] = FFTInput_I_current[i];
			FFTInputCharge[(FFT_SIZE - zoomed_width_half + i) * 2 + 1] = FFTInput_Q_current[i];
		}
		FFT_ChargeBuffer_collected += zoomed_width_half;
	}
	else
	{
		// Make a combined buffer for calculation with 50% overlap
		dma_memcpy(&FFTInputCharge[0], &FFTInputCharge[FFT_HALF_SIZE * 2], sizeof(float32_t) * FFT_HALF_SIZE * 2); //*2 - >i+q
		for (uint_fast16_t i = 0; i < FFT_HALF_SIZE; i++)
		{
			FFTInputCharge[(FFT_HALF_SIZE + i) * 2] = FFTInput_I_current[i];
			FFTInputCharge[(FFT_HALF_SIZE + i) * 2 + 1] = FFTInput_Q_current[i];
		}
		FFT_ChargeBuffer_collected += FFT_HALF_SIZE;
	}

	FFT_new_buffer_ready = false;
	fft_charge_ready = true;
}

// FFT calculation
void FFT_doFFT(void)
{
	if (!TRX.FFT_Enabled)
		return;
	if (!FFT_need_fft)
		return;
	if (!TRX_Inited)
		return;
	if (!fft_charge_ready)
		return;
	if (FFT_ChargeBuffer_collected == 0)
		return;
	/*if (CPU_LOAD.Load > 90)
		return;*/

	if (NeedFFTReinit)
	{
		FFT_Init();
		return;
	}

	// Get charge buffer
	fft_charge_copying = true;
	dma_memcpy(FFTInput, FFTInputCharge, sizeof(FFTInput));
	fft_charge_copying = false;

	// Do full windowing
	if (FFT_ChargeBuffer_collected >= FFT_SIZE)
	{
		FFT_ChargeBuffer_collected = FFT_SIZE;
		arm_cmplx_mult_real_f32(FFTInput, window_multipliers, FFTInput, FFT_SIZE);
	}
	else // partial windowing
	{
		float32_t coeff_rate = (float32_t)FFT_SIZE / (float32_t)FFT_ChargeBuffer_collected;
		for (uint16_t i = (FFT_SIZE - FFT_ChargeBuffer_collected); i < FFT_SIZE; i++)
		{
			uint16_t coeff_idx = coeff_rate * (float32_t)(i - (FFT_SIZE - FFT_ChargeBuffer_collected));
			if (coeff_idx > (FFT_SIZE - 1))
				coeff_idx = (FFT_SIZE - 1);
			FFTInput[i * 2] = FFTInput[i * 2] * window_multipliers[coeff_idx];
			FFTInput[i * 2 + 1] = FFTInput[i * 2 + 1] * window_multipliers[coeff_idx];
		}

		// Gain signal if partial buffer (for normalize)
		arm_scale_f32(FFTInput, ((float32_t)FFT_SIZE / (float32_t)FFT_ChargeBuffer_collected / 2.0f), FFTInput, FFT_DOUBLE_SIZE_BUFFER);
	}

	arm_cfft_f32(FFT_Inst, FFTInput, 0, 1);
	arm_cmplx_mag_f32(FFTInput, FFTInput, FFT_SIZE);

	// Debug VAD
	/*dma_memset(FFTInput, 0x00, sizeof(FFTInput));
	for (uint_fast16_t i = 0; i < FFT_SIZE; i++)
	{
		FFTInput[i] = VAD_FFTBuffer_Export[i];
	}
	NeedFFTInputBuffer = true;*/

	// Debug CW Decoder
	/*dma_memset(FFTInput, 0x00, sizeof(FFTInput));
	for (uint_fast16_t i = 0; i < CWDECODER_FFTSIZE; i++)
	{
		FFTInput[i] = CWDEC_FFTBuffer_Export[i];
		FFTInput[i + CWDECODER_FFTSIZE] = CWDEC_FFTBuffer_Export[i];
		FFTInput[i + CWDECODER_FFTSIZE * 2] = CWDEC_FFTBuffer_Export[i];
		FFTInput[i + CWDECODER_FFTSIZE * 3] = CWDEC_FFTBuffer_Export[i];
	}*/

	// dBm scale
	if (TRX.FFT_Scale_Type == 1)
	{
		for (uint_fast16_t i = 0; i < FFT_SIZE; i++)
			FFTInput[i] = getDBFromFFTAmpl(FFTInput[i]);
	}

	// Swap fft parts
	dma_memcpy(&FFTInput[FFT_SIZE], &FFTInput[0], sizeof(float32_t) * (FFT_SIZE / 2));			  // left - > tmp
	dma_memcpy(&FFTInput[0], &FFTInput[FFT_SIZE / 2], sizeof(float32_t) * (FFT_SIZE / 2));		  // right - > left
	dma_memcpy(&FFTInput[FFT_SIZE / 2], &FFTInput[FFT_SIZE], sizeof(float32_t) * (FFT_SIZE / 2)); // tmp - > right

	// Compress the calculated FFT to visible
	dma_memcpy(&FFTInput[0], &FFTInput[FFT_SIZE / 2 - FFT_USEFUL_SIZE / 2], sizeof(float32_t) * FFT_USEFUL_SIZE); // useful fft part
	float32_t fft_compress_rate = (float32_t)FFT_USEFUL_SIZE / (float32_t)LAYOUT->FFT_PRINT_SIZE;
	float32_t fft_compress_rate_half = floorf(fft_compress_rate / 2.0f);	   // full points
	float32_t fft_compress_rate_parts = fmodf(fft_compress_rate / 2.0f, 1.0f); // partial points

	if (!TRX.FFT_Lens) // normal compress
	{
		for (uint32_t i = 0; i < LAYOUT->FFT_PRINT_SIZE; i++)
		{
			int32_t left_index = (uint32_t)((float32_t)i * fft_compress_rate - fft_compress_rate_half);
			if (left_index < 0)
				left_index = 0;
			int32_t right_index = (uint32_t)((float32_t)i * fft_compress_rate + fft_compress_rate_half);
			if (right_index >= FFT_USEFUL_SIZE)
				right_index = FFT_USEFUL_SIZE - 1;

			float32_t points = 0;
			float32_t accum = 0.0f;
			// full points
			for (uint32_t index = left_index; index <= right_index; index++)
			{
				accum += FFTInput[index];
				points += 1.0f;
			}
			// partial points
			if (fft_compress_rate_parts > 0.0f)
			{
				if (left_index > 0)
				{
					accum += FFTInput[left_index - 1] * fft_compress_rate_parts;
					points += fft_compress_rate_parts;
				}
				if (right_index < (FFT_USEFUL_SIZE - 1))
				{
					accum += FFTInput[right_index + 1] * fft_compress_rate_parts;
					points += fft_compress_rate_parts;
				}
			}
			FFTInput_tmp[i] = accum / points;
		}
	}
	else // lens compress
	{
		float32_t step_now = FFT_LENS_STEP_START;
		float32_t index1 = (float32_t)FFT_USEFUL_SIZE / 2.0f;
		float32_t index2 = index1;
		for (uint32_t i = 0; i <= (LAYOUT->FFT_PRINT_SIZE / 2); i++)
		{
			FFTInput_tmp[(LAYOUT->FFT_PRINT_SIZE / 2) - i] = FFTInput[(uint32_t)roundf(index1)];
			if (i != (LAYOUT->FFT_PRINT_SIZE / 2))
				FFTInput_tmp[(LAYOUT->FFT_PRINT_SIZE / 2) + i] = FFTInput[(uint32_t)roundf(index2)];

			step_now += FFT_LENS_STEP;
			index1 -= step_now;
			index2 += step_now;

			if (index1 >= FFT_USEFUL_SIZE)
				index1 = FFT_USEFUL_SIZE - 1;
			if (index1 < 0)
				index1 = 0;
			if (index2 >= FFT_USEFUL_SIZE)
				index2 = FFT_USEFUL_SIZE - 1;
			if (index2 < 0)
				index2 = 0;
		}
	}
	dma_memcpy(&FFTInput, FFTInput_tmp, sizeof(FFTInput_tmp));

	// Averaging
	if (TRX.FFT_Averaging > (FFT_MAX_MEANS + FFT_MAX_AVER))
		TRX.FFT_Averaging = (FFT_MAX_MEANS + FFT_MAX_AVER);
	uint8_t max_mean = TRX.FFT_Averaging;
	if (max_mean > FFT_MAX_MEANS)
		max_mean = FFT_MAX_MEANS;
	uint8_t averaging = 0;
	if (TRX.FFT_Averaging > FFT_MAX_MEANS)
		averaging = TRX.FFT_Averaging - FFT_MAX_MEANS + 1;

	// Store old FFT for averaging
	dma_memcpy(&FFT_meanBuffer[FFT_meanBuffer_index][0], FFTInput, sizeof(float32_t) * LAYOUT->FFT_PRINT_SIZE);
	fft_meanbuffer_freqs[FFT_meanBuffer_index] = FFT_lastFFTChargeBufferFreq;

	FFT_meanBuffer_index++;
	if (FFT_meanBuffer_index >= max_mean)
		FFT_meanBuffer_index = 0;

	// Averaging values
	dma_memset(FFTOutput_mean, 0x00, sizeof(FFTOutput_mean));
	//dma_memset(FFTOutput_mean_count, 0x00, sizeof(FFTOutput_mean_count));

	for (uint_fast16_t avg_idx = 0; avg_idx < max_mean; avg_idx++)
	{
		int64_t freq_diff = roundl(((float64_t)((float64_t)fft_meanbuffer_freqs[avg_idx] - (float64_t)CurrentVFO->Freq) / hz_in_pixel) * (float64_t)fft_zoom);

		if (!TRX.WTF_Moving)
			freq_diff = 0;
		for (uint_fast16_t i = 0; i < LAYOUT->FFT_PRINT_SIZE; i++)
		{
			int64_t new_x = (int64_t)i - (int64_t)freq_diff;

			if (new_x > -1 && new_x < LAYOUT->FFT_PRINT_SIZE)
			{
				FFTOutput_mean[i] += FFT_meanBuffer[avg_idx][new_x];
				//FFTOutput_mean_count[i]++;
			}
		}
	}

	for (uint_fast16_t i = 0; i < LAYOUT->FFT_PRINT_SIZE; i++)
	{
		//if (FFTOutput_mean_count[i] > 1)
			//FFTOutput_mean[i] /= (float32_t)FFTOutput_mean_count[i];
		FFTOutput_mean[i] /= max_mean;
	}

	if (averaging > 0)
	{
		for (uint_fast16_t i = 0; i < LAYOUT->FFT_PRINT_SIZE; i++)
		{
			float32_t alpha = 1.0f / averaging;
			FFTOutput_average[i] = FFTOutput_average[i] * (1.0f - alpha) + FFTOutput_mean[i] * (alpha);
			FFTOutput_mean[i] = FFTOutput_average[i];
		}
	}

	FFT_need_fft = false;
}

// FFT output
bool FFT_printFFT(void)
{
	if (LCD_busy)
		return false;
	if (!TRX.FFT_Enabled)
		return false;
	if (!TRX_Inited)
		return false;
	if (FFT_need_fft)
		return false;
	if (LCD_systemMenuOpened || LCD_window.opened)
		return false;
	/*if (CPU_LOAD.Load > 90)
		return;*/
	LCD_busy = true;

	uint16_t height = 0; // column height in FFT output
	uint16_t tmp = 0;
	uint16_t fftHeight = GET_FFTHeight;
	uint16_t wtfHeight = GET_WTFHeight;
	uint_fast8_t decoder_offset = 0;
	if (NeedProcessDecoder)
		decoder_offset = LAYOUT->FFT_CWDECODER_OFFSET;
	hz_in_pixel = TRX_on_TX ? FFT_TX_HZ_IN_PIXEL : FFT_HZ_IN_PIXEL;

	if (CurrentVFO->Freq != currentFFTFreq || NeedWTFRedraw)
	{
		// calculate scale lines
		dma_memset(grid_lines_pos, 0x00, sizeof(grid_lines_pos));
		uint8_t index = 0;
		uint64_t grid_step = FFT_current_spectrum_width_hz / 9.6;
		if (grid_step < 1000)
			grid_step = 1000;
		grid_step = (grid_step / 1000) * 1000;

		for (int8_t i = 0; i < FFT_MAX_GRID_NUMBER; i++)
		{
			int64_t pos = -1;
			uint64_t grid_freq = (CurrentVFO->Freq / grid_step * grid_step) + ((i - 6) * grid_step);
			pos = getFreqPositionOnFFT(grid_freq, false);
			if (pos >= 0)
			{
				grid_lines_pos[index] = pos;
				grid_lines_freq[index] = grid_freq;
				index++;
			}
		}

		// offset the fft if needed
		currentFFTFreq = CurrentVFO->Freq;

		// save RX2 position
		rx2_line_pos = getFreqPositionOnFFT(SecondaryVFO->Freq, true);
	}

	// move the waterfall down using DMA
	uint8_t *srcAddr = &indexed_wtf_buffer[wtfHeight - 2][LAYOUT->FFT_PRINT_SIZE - 1];
	uint8_t *destAddr = &indexed_wtf_buffer[wtfHeight - 1][LAYOUT->FFT_PRINT_SIZE - 1];
	uint8_t *endAddr = &indexed_wtf_buffer[0][0];
	uint32_t estimated = (uint32_t)srcAddr - (uint32_t)endAddr + 1;
	Aligned_CleanDCache_by_Addr(indexed_wtf_buffer, sizeof(indexed_wtf_buffer));
	while (estimated > 0)
	{
		uint32_t length = estimated;
		if (length > DMA_MAX_BLOCK)
			length = DMA_MAX_BLOCK;
		HAL_MDMA_Start(&hmdma_mdma_channel43_sw_0, (uint32_t)srcAddr, (uint32_t)destAddr, length, 1);
		SLEEPING_MDMA_PollForTransfer(&hmdma_mdma_channel43_sw_0);
		srcAddr -= length;
		destAddr -= length;
		estimated -= length;
	}
	Aligned_CleanInvalidateDCache_by_Addr(indexed_wtf_buffer, sizeof(indexed_wtf_buffer));
	for (tmp = wtfHeight - 1; tmp > 0; tmp--)
		wtf_buffer_freqs[tmp] = wtf_buffer_freqs[tmp - 1];

	// Looking for the maximum/minimum in frequency response
	float32_t maxAmplValue = 0;
	float32_t minAmplValue = 0;
	uint32_t minAmplValueIndex = 0;
	arm_min_f32(FFTOutput_mean, LAYOUT->FFT_PRINT_SIZE, &minAmplValue, &minAmplValueIndex);
	if (TRX.FFT_Scale_Type == 1)
	{
		for (uint_fast16_t i = 0; i < LAYOUT->FFT_PRINT_SIZE; i++)
			if (FFTOutput_mean[i] == 0.0f)
				FFTOutput_mean[i] = minAmplValue;
	}
	arm_max_no_idx_f32(FFTOutput_mean, LAYOUT->FFT_PRINT_SIZE, &maxAmplValue);
	// println(minAmplValue, " ", maxAmplValue, " ", maxValueFFT);

	// Looking for the median in frequency response
	dma_memcpy(FFTInput_tmp, FFTOutput_mean, LAYOUT->FFT_PRINT_SIZE * 4);
	float32_t medianValue = quick_median_select(FFTInput_tmp, LAYOUT->FFT_PRINT_SIZE);

	// FFT Targets
	float32_t maxValueFFT = maxValueFFT_rx;
	float32_t minValueFFT = maxValueFFT / (float32_t)fftHeight;
	if (TRX_on_TX)
		maxValueFFT = maxValueFFT_tx;
	float32_t maxValue = (medianValue * FFT_MAX);
	float32_t targetValue = (medianValue * FFT_TARGET);
	float32_t minValue = (medianValue * FFT_MIN);

	// dbm scaling
	if (TRX.FFT_Scale_Type == 1)
	{
		if (TRX.FFT_Automatic)
		{
			if (minAmplValue_averaged > minAmplValue)
				minAmplValue_averaged = minAmplValue;
			else
				minAmplValue_averaged = minAmplValue_averaged * 0.99f + minAmplValue * 0.01f;
		}
		else
			minAmplValue_averaged = (float32_t)TRX.FFT_ManualBottom;

		arm_offset_f32(FFTOutput_mean, -minAmplValue_averaged, FFTOutput_mean, LAYOUT->FFT_PRINT_SIZE);
		for (uint_fast16_t i = 0; i < LAYOUT->FFT_PRINT_SIZE; i++)
			if (FFTOutput_mean[i] < 0)
				FFTOutput_mean[i] = 0;
	}

	// Auto-calibrate FFT levels
	if (TRX_on_TX || (TRX.FFT_Automatic && TRX.FFT_Sensitivity == FFT_MAX_TOP_SCALE)) // Fit FFT to MAX
	{
		if (TRX.FFT_Scale_Type == 1)
		{
			float32_t newMaxAmplValue = maxAmplValue - minAmplValue_averaged;
			maxValueFFT = maxValueFFT * 0.95f + newMaxAmplValue * 0.05f;
			if (maxValueFFT < newMaxAmplValue)
				maxValueFFT = newMaxAmplValue;
		}
		else
		{
			maxValueFFT = maxValueFFT * 0.95f + maxAmplValue * 0.05f;
			if (maxValueFFT < maxAmplValue)
				maxValueFFT = maxAmplValue;

			minValue = (medianValue * 6.0f);
			if (maxValueFFT < minValue)
				maxValueFFT = minValue;
		}

		FFT_minDBM = minAmplValue_averaged;
		FFT_maxDBM = maxAmplValue;
	}
	else if (TRX.FFT_Automatic) // Fit by median (automatic)
	{
		if (TRX.FFT_Scale_Type == 1)
		{
			medianValue -= minAmplValue_averaged;
			if (medianValue < 1.0f)
				medianValue = 1.0f;
			maxValue = medianValue * FFT_MAX / 2.0f;
			targetValue = medianValue * FFT_TARGET / 2.0f;
			minValue = medianValue * FFT_MIN / 2.0f;
		}

		maxValueFFT += (targetValue - maxValueFFT) / FFT_STEP_COEFF;
		// println(maxValueFFT, " ", targetValue, " ", medianValue, " ", (medianValue / FFT_TARGET));

		// minimum-maximum threshold for median
		if (maxValueFFT < minValue)
			maxValueFFT = minValue;
		if (maxValueFFT > maxValue)
			maxValueFFT = maxValue;

		FFT_minDBM = minAmplValue_averaged;
		FFT_maxDBM = maxValueFFT + minAmplValue_averaged;

		// Compress peaks
		float32_t compressTargetValue = (maxValueFFT * FFT_COMPRESS_INTERVAL);
		float32_t compressSourceInterval = maxAmplValue - compressTargetValue;
		float32_t compressTargetInterval = maxValueFFT - compressTargetValue;
		float32_t compressRate = compressTargetInterval / compressSourceInterval;
		if (TRX.FFT_Compressor)
		{
			for (uint_fast16_t i = 0; i < LAYOUT->FFT_PRINT_SIZE; i++)
				if (FFTOutput_mean[i] > compressTargetValue)
					FFTOutput_mean[i] = compressTargetValue + ((FFTOutput_mean[i] - compressTargetValue) * compressRate);
		}
	}
	else // Manual Scale
	{
		if (TRX.FFT_Scale_Type == 1)
		{
			float32_t minManualAmplitude = (float32_t)TRX.FFT_ManualBottom - minAmplValue_averaged;
			float32_t maxManualAmplitude = (float32_t)TRX.FFT_ManualTop - minAmplValue_averaged;
			maxValueFFT = maxManualAmplitude;
			minValueFFT = minManualAmplitude;

			FFT_minDBM = minValueFFT + minAmplValue_averaged;
			FFT_maxDBM = maxValueFFT + minAmplValue_averaged;
		}
		else
		{
			float32_t minManualAmplitude = getFFTAmplFromDB((float32_t)TRX.FFT_ManualBottom);
			float32_t maxManualAmplitude = getFFTAmplFromDB((float32_t)TRX.FFT_ManualTop);
			arm_offset_f32(FFTOutput_mean, -minManualAmplitude, FFTOutput_mean, LAYOUT->FFT_PRINT_SIZE);
			for (uint_fast16_t i = 0; i < LAYOUT->FFT_PRINT_SIZE; i++)
				if (FFTOutput_mean[i] < 0)
					FFTOutput_mean[i] = 0;
			maxValueFFT = maxManualAmplitude - minManualAmplitude;
			minValueFFT = minManualAmplitude;
		}
	}

	// limits
	if (maxValueFFT < 0.0000001f && TRX.FFT_Scale_Type == 0)
		maxValueFFT = 0.0000001f;

	// tx noise scale limit
	if (TRX_on_TX && maxValueFFT < FFT_TX_MIN_LEVEL)
		maxValueFFT = FFT_TX_MIN_LEVEL;

    // save values for TX/RX
	if (TRX_on_TX)
		maxValueFFT_tx = maxValueFFT;
	else
		maxValueFFT_rx = maxValueFFT;

	// scale fft mean buffer
	arm_scale_f32(FFTOutput_mean, (1.0f / maxValueFFT) * fftHeight, FFTOutput_mean, LAYOUT->FFT_PRINT_SIZE);

	// calculate the colors for the waterfall
	for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++)
	{
		if (FFTOutput_mean[fft_x] > fftHeight)
			FFTOutput_mean[fft_x] = fftHeight;

		fft_header[fft_x] = FFTOutput_mean[fft_x];
		indexed_wtf_buffer[0][fft_x] = roundf((float32_t)fftHeight - FFTOutput_mean[fft_x]);
	}
	wtf_buffer_freqs[0] = currentFFTFreq;

	// FFT Peaks
	if (TRX.FFT_HoldPeaks)
	{
		// peaks moving
		if (lastWTFFreq != currentFFTFreq)
		{
			float64_t diff = (float64_t)currentFFTFreq - (float64_t)lastWTFFreq;
			diff = diff / (float64_t)(hz_in_pixel * fft_zoom);
			diff = roundl(diff);

			if (diff > 0)
			{
				for (int32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++)
				{
					int32_t new_x = fft_x + (int32_t)diff;
					if (new_x >= 0 && new_x < LAYOUT->FFT_PRINT_SIZE)
						fft_peaks[fft_x] = fft_peaks[new_x];
					else
						fft_peaks[fft_x] = 0;
				}
			}
			else if (diff < 0)
			{
				for (int32_t fft_x = LAYOUT->FFT_PRINT_SIZE - 1; fft_x >= 0; fft_x--)
				{
					int32_t new_x = fft_x + (int32_t)diff;
					if (new_x >= 0 && new_x < LAYOUT->FFT_PRINT_SIZE)
						fft_peaks[fft_x] = fft_peaks[new_x];
					else
						fft_peaks[fft_x] = 0;
				}
			}
		}
		// peaks falling
		for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++)
		{
			if (fft_peaks[fft_x] <= fft_header[fft_x])
				fft_peaks[fft_x] = fft_header[fft_x];
			else
				fft_peaks[fft_x]--;
		}
	}

	// calculate bw bar size
	uint16_t curwidth = CurrentVFO->LPF_RX_Filter_Width;
	if (TRX_on_TX)
		curwidth = CurrentVFO->LPF_TX_Filter_Width;
	int32_t bw_rx1_line_width = 0;
	int32_t bw_rx2_line_width = 0;
	int32_t rx1_notch_line_pos = 100;
	int32_t rx2_notch_line_pos = 100;

	switch (CurrentVFO->Mode)
	{
	case TRX_MODE_LSB:
	case TRX_MODE_DIGI_L:
		bw_rx1_line_width = (int32_t)(curwidth / hz_in_pixel * fft_zoom);
		if (bw_rx1_line_width > (LAYOUT->FFT_PRINT_SIZE / 2))
			bw_rx1_line_width = LAYOUT->FFT_PRINT_SIZE / 2;
		bw_rx1_line_start = LAYOUT->FFT_PRINT_SIZE / 2 - bw_rx1_line_width;
		rx1_notch_line_pos = bw_rx1_line_start + bw_rx1_line_width - CurrentVFO->NotchFC / hz_in_pixel * fft_zoom;
		break;
	case TRX_MODE_USB:
	case TRX_MODE_RTTY:
	case TRX_MODE_DIGI_U:
		bw_rx1_line_width = (int32_t)(curwidth / hz_in_pixel * fft_zoom);
		if (bw_rx1_line_width > (LAYOUT->FFT_PRINT_SIZE / 2))
			bw_rx1_line_width = LAYOUT->FFT_PRINT_SIZE / 2;
		bw_rx1_line_start = LAYOUT->FFT_PRINT_SIZE / 2;
		rx1_notch_line_pos = bw_rx1_line_start + CurrentVFO->NotchFC / hz_in_pixel * fft_zoom;
		break;
	case TRX_MODE_NFM:
	case TRX_MODE_AM:
	case TRX_MODE_SAM:
	case TRX_MODE_CW:
		bw_rx1_line_width = (int32_t)(curwidth / hz_in_pixel * fft_zoom);
		if (bw_rx1_line_width > LAYOUT->FFT_PRINT_SIZE)
			bw_rx1_line_width = LAYOUT->FFT_PRINT_SIZE;
		bw_rx1_line_start = LAYOUT->FFT_PRINT_SIZE / 2 - (bw_rx1_line_width / 2);
		if (CurrentVFO->Mode == TRX_MODE_CW)
			rx1_notch_line_pos = bw_rx1_line_start + bw_rx1_line_width - CurrentVFO->NotchFC / hz_in_pixel * fft_zoom;
		else
			rx1_notch_line_pos = bw_rx1_line_start + CurrentVFO->NotchFC / hz_in_pixel * fft_zoom;
		break;
	case TRX_MODE_WFM:
		bw_rx1_line_width = 0;
		bw_rx1_line_start = LAYOUT->FFT_PRINT_SIZE / 2 - (bw_rx1_line_width / 2);
		rx1_notch_line_pos = bw_rx1_line_start + CurrentVFO->NotchFC / hz_in_pixel * fft_zoom;
		break;
	default:
		break;
	}
	switch (SecondaryVFO->Mode)
	{
	case TRX_MODE_LSB:
	case TRX_MODE_DIGI_L:
		bw_rx2_line_width = (int32_t)(SecondaryVFO->LPF_RX_Filter_Width / hz_in_pixel * fft_zoom);
		if (bw_rx2_line_width > (LAYOUT->FFT_PRINT_SIZE / 2))
			bw_rx2_line_width = LAYOUT->FFT_PRINT_SIZE / 2;
		bw_rx2_line_start = rx2_line_pos - bw_rx2_line_width;
		rx2_notch_line_pos = bw_rx2_line_start + bw_rx2_line_width - SecondaryVFO->NotchFC / hz_in_pixel * fft_zoom;
		break;
	case TRX_MODE_USB:
	case TRX_MODE_RTTY:
	case TRX_MODE_DIGI_U:
		bw_rx2_line_width = (int32_t)(SecondaryVFO->LPF_RX_Filter_Width / hz_in_pixel * fft_zoom);
		if (bw_rx2_line_width > (LAYOUT->FFT_PRINT_SIZE / 2))
			bw_rx2_line_width = LAYOUT->FFT_PRINT_SIZE / 2;
		bw_rx2_line_start = rx2_line_pos;
		rx2_notch_line_pos = bw_rx2_line_start + SecondaryVFO->NotchFC / hz_in_pixel * fft_zoom;
		break;
	case TRX_MODE_NFM:
	case TRX_MODE_AM:
	case TRX_MODE_SAM:
	case TRX_MODE_CW:
		bw_rx2_line_width = (int32_t)(SecondaryVFO->LPF_RX_Filter_Width / hz_in_pixel * fft_zoom);
		if (bw_rx2_line_width > LAYOUT->FFT_PRINT_SIZE)
			bw_rx2_line_width = LAYOUT->FFT_PRINT_SIZE;
		bw_rx2_line_start = rx2_line_pos - (bw_rx2_line_width / 2);
		if (SecondaryVFO->Mode == TRX_MODE_CW)
			rx2_notch_line_pos = bw_rx2_line_start + bw_rx2_line_width - SecondaryVFO->NotchFC / hz_in_pixel * fft_zoom;
		else
			rx2_notch_line_pos = bw_rx2_line_start + SecondaryVFO->NotchFC / hz_in_pixel * fft_zoom;
		break;
	case TRX_MODE_WFM:
		bw_rx2_line_width = LAYOUT->FFT_PRINT_SIZE;
		bw_rx2_line_start = rx2_line_pos - (bw_rx2_line_width / 2);
		rx2_notch_line_pos = bw_rx2_line_start + SecondaryVFO->NotchFC / hz_in_pixel * fft_zoom;
		break;
	default:
		break;
	}
	int32_t bw_rx1_line_center = bw_rx1_line_start + bw_rx1_line_width / 2;
	int32_t bw_rx2_line_center = bw_rx2_line_start + bw_rx2_line_width / 2;
	bw_rx1_line_end = bw_rx1_line_start + bw_rx1_line_width;
	bw_rx2_line_end = bw_rx2_line_start + bw_rx2_line_width;
	if (TRX.FFT_Lens) // lens correction
	{
		bw_rx1_line_start = FFT_getLensCorrection(bw_rx1_line_start);
		bw_rx1_line_center = FFT_getLensCorrection(bw_rx1_line_center);
		bw_rx1_line_end = FFT_getLensCorrection(bw_rx1_line_end);

		bw_rx2_line_start = FFT_getLensCorrection(bw_rx2_line_start);
		bw_rx2_line_center = FFT_getLensCorrection(bw_rx2_line_center);
		bw_rx2_line_end = FFT_getLensCorrection(bw_rx2_line_end);
	}
	if (!TRX.Show_Sec_VFO) // disable RX2 bw show
	{
		bw_rx2_line_start = LCD_WIDTH + 10;
		bw_rx2_line_center = LCD_WIDTH + 10;
		bw_rx2_line_end = LCD_WIDTH + 10;
	}

	if (TRX.FFT_3D > 0)
	{
		FFT_3DPrintFFT();
		return true;
	}

	// prepare FFT print over the waterfall
	uint16_t background = BG_COLOR;
	uint16_t grid_color = palette_fft[fftHeight * 3 / 4];
	for (uint32_t fft_y = 0; fft_y < fftHeight; fft_y++) // Background
	{
		bool dbm_grid = false;
		if (TRX.FFT_Background)
			background = palette_bg_gradient[fft_y];
		else
			background = BG_COLOR;

		if (TRX.FFT_dBmGrid)
			for (uint16_t y = FFT_DBM_GRID_TOP_MARGIN; y <= fftHeight - 4; y += FFT_DBM_GRID_INTERVAL)
				if (y == fft_y)
				{
					background = grid_color;
					dbm_grid = true;
				}

		for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++)
		{
			if ((fft_x >= bw_rx1_line_start && fft_x <= bw_rx1_line_end) || ((int32_t)fft_x >= bw_rx2_line_start && (int32_t)fft_x <= bw_rx2_line_end)) // bw bar
			{
				print_output_buffer[fft_y][fft_x] = dbm_grid ? background : palette_bw_bg_colors[fft_y];
			}
			else
				print_output_buffer[fft_y][fft_x] = background;
		}
	}

	if (TRX.FFT_Style == 1) // gradient
	{
		for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++)
		{
			if ((fft_x >= bw_rx1_line_start && fft_x <= bw_rx1_line_end) || ((int32_t)fft_x >= bw_rx2_line_start && (int32_t)fft_x <= bw_rx2_line_end)) // bw bar
			{
				for (uint32_t fft_y = (fftHeight - fft_header[fft_x]); fft_y < fftHeight; fft_y++)
					print_output_buffer[fft_y][fft_x] = palette_bw_fft_colors[fft_y];
			}
			else
			{
				for (uint32_t fft_y = (fftHeight - fft_header[fft_x]); fft_y < fftHeight; fft_y++)
					print_output_buffer[fft_y][fft_x] = palette_fft[fft_y];
			}
		}
	}

	if (TRX.FFT_Style == 2) // fill
	{
		for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++)
		{
			if ((fft_x >= bw_rx1_line_start && fft_x <= bw_rx1_line_end) || ((int32_t)fft_x >= bw_rx2_line_start && (int32_t)fft_x <= bw_rx2_line_end)) // bw bar
			{
				for (uint32_t fft_y = (fftHeight - fft_header[fft_x]); fft_y < fftHeight; fft_y++)
					print_output_buffer[fft_y][fft_x] = palette_bw_fft_colors[fftHeight / 2];
			}
			else
			{
				for (uint32_t fft_y = (fftHeight - fft_header[fft_x]); fft_y < fftHeight; fft_y++)
					print_output_buffer[fft_y][fft_x] = palette_fft[fftHeight / 2];
			}
		}
	}

	if (TRX.FFT_Style == 3) // dots
	{
		for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++)
		{
			uint32_t fft_y = fftHeight - fft_header[fft_x];
			if ((fft_x >= bw_rx1_line_start && fft_x <= bw_rx1_line_end) || ((int32_t)fft_x >= bw_rx2_line_start && (int32_t)fft_x <= bw_rx2_line_end)) // bw bar
				print_output_buffer[fft_y][fft_x] = palette_bw_fft_colors[fftHeight / 2];
			else
				print_output_buffer[fft_y][fft_x] = palette_fft[fftHeight / 2];
		}
	}

	if (TRX.FFT_Style == 4) // contour
	{
		uint32_t fft_y_prev = 0;
		for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++)
		{
			uint32_t fft_y = fftHeight - fft_header[fft_x];
			int32_t y_diff = (int32_t)fft_y - (int32_t)fft_y_prev;
			if (fft_x == 0 || (y_diff <= 1 && y_diff >= -1))
			{
				print_output_buffer[fft_y][fft_x] = palette_fft[fftHeight / 2];
			}
			else
			{
				for (uint32_t l = 0; l < (abs(y_diff / 2) + 1); l++) // draw line
				{
					print_output_buffer[fft_y_prev + ((y_diff > 0) ? l : -l)][fft_x - 1] = palette_fft[fftHeight / 2];
					print_output_buffer[fft_y + ((y_diff > 0) ? -l : l)][fft_x] = palette_fft[fftHeight / 2];
				}
			}
			fft_y_prev = fft_y;
		}
	}

	// FFT Peaks
	if (TRX.FFT_HoldPeaks)
	{
		uint32_t fft_y_prev = 0;
		for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++)
		{
			uint32_t fft_y = fftHeight - fft_peaks[fft_x];
			int32_t y_diff = (int32_t)fft_y - (int32_t)fft_y_prev;
			if (fft_x == 0 || (y_diff <= 1 && y_diff >= -1))
			{
				print_output_buffer[fft_y][fft_x] = palette_fft[fftHeight / 2];
			}
			else
			{
				for (uint32_t l = 0; l < (abs(y_diff / 2) + 1); l++) // draw line
				{
					print_output_buffer[fft_y_prev + ((y_diff > 0) ? l : -l)][fft_x - 1] = palette_fft[fftHeight / 2];
					print_output_buffer[fft_y + ((y_diff > 0) ? -l : l)][fft_x] = palette_fft[fftHeight / 2];
				}
			}
			fft_y_prev = fft_y;
		}
	}

	////Waterfall

	// clear old data
	if (lastWTFFreq != currentFFTFreq || NeedWTFRedraw)
	{
		uint16_t color = palette_wtf[fftHeight];
		if (TRX.FFT_Automatic)
			color = palette_wtf[(uint32_t)(fftHeight * 0.9f)];
		memset16(print_output_buffer[fftHeight], color, LAYOUT->FFT_PRINT_SIZE * (wtfHeight - decoder_offset));
	}

	// BTE
	static uint8_t line_repeats_need = 1;
#ifdef HAS_BTE
	// move exist lines down with BTE
	if (lastWTFFreq == currentFFTFreq && !NeedWTFRedraw)
	{
		if (TRX.FFT_Speed <= 3)
		{
			// 1 line
			LCDDriver_BTE_copyArea(0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight, 0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + 1, LAYOUT->FFT_PRINT_SIZE, (uint16_t)(wtfHeight - decoder_offset - 1), true);
			line_repeats_need = 1;
		}
		if (TRX.FFT_Speed == 4 && FFT_FPS_Last < 44)
		{
			// 2 line
			LCDDriver_BTE_copyArea(0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight, 0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + 1, LAYOUT->FFT_PRINT_SIZE, (uint16_t)(wtfHeight - decoder_offset - 1), true);
			LCDDriver_BTE_copyArea(0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight, 0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + 1, LAYOUT->FFT_PRINT_SIZE, (uint16_t)(wtfHeight - decoder_offset - 1), true);
			line_repeats_need = 2;
		}
		if (TRX.FFT_Speed == 5 && FFT_FPS_Last < 55)
		{
			// 3 line
			LCDDriver_BTE_copyArea(0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight, 0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + 1, LAYOUT->FFT_PRINT_SIZE, (uint16_t)(wtfHeight - decoder_offset - 1), true);
			LCDDriver_BTE_copyArea(0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight, 0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + 1, LAYOUT->FFT_PRINT_SIZE, (uint16_t)(wtfHeight - decoder_offset - 1), true);
			LCDDriver_BTE_copyArea(0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight, 0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + 1, LAYOUT->FFT_PRINT_SIZE, (uint16_t)(wtfHeight - decoder_offset - 1), true);
			line_repeats_need = 3;
		}
	}
#endif

	uint16_t wtf_printed_lines = 0;
	uint16_t print_wtf_yindex = fftHeight;
#ifdef HAS_BTE
	while ((print_wtf_yindex < (fftHeight + wtfHeight - decoder_offset) && (lastWTFFreq != currentFFTFreq || NeedWTFRedraw)) || (print_wtf_yindex == fftHeight && lastWTFFreq == currentFFTFreq && !NeedWTFRedraw))
#else
	while (print_wtf_yindex < (fftHeight + wtfHeight - decoder_offset))
#endif
	{
		uint16_t wtf_y_index = (print_wtf_yindex - fftHeight) / line_repeats_need;
		// calculate offset
		float64_t freq_diff = (((float64_t)currentFFTFreq - (float64_t)wtf_buffer_freqs[wtf_y_index]) / hz_in_pixel) * (float64_t)fft_zoom;
		float64_t freq_diff_part = fmodl(freq_diff, 1.0f);
		int64_t margin_left = 0;
		if (freq_diff < 0)
			margin_left = -floorf(freq_diff);
		if (margin_left > LAYOUT->FFT_PRINT_SIZE)
			margin_left = LAYOUT->FFT_PRINT_SIZE;
		int32_t margin_right = 0;
		if (freq_diff > 0)
			margin_right = ceilf(freq_diff);
		if (margin_right > LAYOUT->FFT_PRINT_SIZE)
			margin_right = LAYOUT->FFT_PRINT_SIZE;
		if ((margin_left + margin_right) > LAYOUT->FFT_PRINT_SIZE)
			margin_right = 0;
		// rounding
		int32_t body_width = LAYOUT->FFT_PRINT_SIZE - margin_left - margin_right;

		// skip WTF moving
		if (!TRX.WTF_Moving)
		{
			body_width = LAYOUT->FFT_PRINT_SIZE;
			margin_left = 0;
			margin_right = 0;
		}

		// printing
		if (body_width > 0)
		{
			if (margin_left == 0 && margin_right == 0) // print full line
			{
				for (uint32_t wtf_x = 0; wtf_x < LAYOUT->FFT_PRINT_SIZE; wtf_x++)
					if ((wtf_x >= bw_rx1_line_start && wtf_x <= bw_rx1_line_end) || ((int32_t)wtf_x >= bw_rx2_line_start && (int32_t)wtf_x <= bw_rx2_line_end)) // print bw bar
						print_output_buffer[print_wtf_yindex][wtf_x] = palette_bw_wtf_colors[indexed_wtf_buffer[wtf_y_index][wtf_x]];
					else
						print_output_buffer[print_wtf_yindex][wtf_x] = palette_wtf[indexed_wtf_buffer[wtf_y_index][wtf_x]];
			}
			else if (margin_left > 0)
			{
				for (uint32_t wtf_x = 0; wtf_x < (LAYOUT->FFT_PRINT_SIZE - margin_left); wtf_x++)
					if (((margin_left + wtf_x) >= bw_rx1_line_start && (margin_left + wtf_x) <= bw_rx1_line_end) || ((int32_t)(margin_left + wtf_x) >= bw_rx2_line_start && (int32_t)(margin_left + wtf_x) <= bw_rx2_line_end)) // print bw bar
						print_output_buffer[print_wtf_yindex][margin_left + wtf_x] = palette_bw_wtf_colors[indexed_wtf_buffer[wtf_y_index][wtf_x]];
					else
						print_output_buffer[print_wtf_yindex][margin_left + wtf_x] = palette_wtf[indexed_wtf_buffer[wtf_y_index][wtf_x]];
			}
			if (margin_right > 0)
			{
				for (uint32_t wtf_x = 0; wtf_x < (LAYOUT->FFT_PRINT_SIZE - margin_right); wtf_x++)
					if ((wtf_x >= bw_rx1_line_start && wtf_x <= bw_rx1_line_end) || ((int32_t)wtf_x >= bw_rx2_line_start && (int32_t)wtf_x <= bw_rx2_line_end)) // print bw bar
						print_output_buffer[print_wtf_yindex][wtf_x] = palette_bw_wtf_colors[indexed_wtf_buffer[wtf_y_index][wtf_x + margin_right]];
					else
						print_output_buffer[print_wtf_yindex][wtf_x] = palette_wtf[indexed_wtf_buffer[wtf_y_index][wtf_x + margin_right]];
			}
		}

		print_wtf_yindex++;
		wtf_printed_lines++;
	}

	// Draw grids
	if (TRX.FFT_FreqGrid == 1 || TRX.FFT_FreqGrid == 2)
	{
		for (int32_t grid_line_index = 0; grid_line_index < FFT_MAX_GRID_NUMBER; grid_line_index++)
			if (grid_lines_pos[grid_line_index] > 0 && grid_lines_pos[grid_line_index] < LAYOUT->FFT_PRINT_SIZE && grid_lines_pos[grid_line_index] != (LAYOUT->FFT_PRINT_SIZE / 2))
				for (uint32_t fft_y = 0; fft_y < fftHeight; fft_y++)
					print_output_buffer[fft_y][grid_lines_pos[grid_line_index]] = grid_color;
	}
	if (TRX.FFT_FreqGrid >= 2)
	{
		for (int8_t grid_line_index = 0; grid_line_index < FFT_MAX_GRID_NUMBER; grid_line_index++)
			if (grid_lines_pos[grid_line_index] > 0 && grid_lines_pos[grid_line_index] < LAYOUT->FFT_PRINT_SIZE && grid_lines_pos[grid_line_index] != (LAYOUT->FFT_PRINT_SIZE / 2))
				for (uint32_t fft_y = fftHeight; fft_y < (fftHeight + wtfHeight); fft_y++)
					print_output_buffer[fft_y][grid_lines_pos[grid_line_index]] = grid_color;
	}

	// Gauss filter center
	if (TRX.CW_GaussFilter && CurrentVFO->Mode == TRX_MODE_CW)
	{
		uint16_t color = palette_fft[fftHeight / 2];
		for (uint32_t fft_y = 0; fft_y < FFT_AND_WTF_HEIGHT; fft_y++)
			print_output_buffer[fft_y][bw_rx1_line_center] = color;
	}
	if (TRX.CW_GaussFilter && SecondaryVFO->Mode == TRX_MODE_CW)
	{
		uint16_t color = palette_fft[fftHeight / 2];
		for (uint32_t fft_y = 0; fft_y < FFT_AND_WTF_HEIGHT; fft_y++)
			print_output_buffer[fft_y][bw_rx2_line_center] = color;
	}

	// RTTY center frequency
	if (CurrentVFO->Mode == TRX_MODE_RTTY)
	{
		uint16_t color = palette_fft[fftHeight / 2];
		uint16_t x1 = (LAYOUT->FFT_PRINT_SIZE / 2) + (TRX.RTTY_Freq - TRX.RTTY_Shift / 2) / hz_in_pixel * fft_zoom;
		uint16_t x2 = (LAYOUT->FFT_PRINT_SIZE / 2) + (TRX.RTTY_Freq + TRX.RTTY_Shift / 2) / hz_in_pixel * fft_zoom;
		for (uint32_t fft_y = 0; fft_y < FFT_AND_WTF_HEIGHT; fft_y++)
		{
			print_output_buffer[fft_y][x1] = color;
			print_output_buffer[fft_y][x2] = color;
		}
	}

	// Show manual Notch filter line
	if (CurrentVFO->ManualNotchFilter && !TRX_on_TX && rx1_notch_line_pos >= 0 && rx1_notch_line_pos < LAYOUT->FFT_PRINT_SIZE)
	{
		uint16_t color = palette_fft[fftHeight * 1 / 4];
		for (uint32_t fft_y = 0; fft_y < FFT_AND_WTF_HEIGHT; fft_y++)
			print_output_buffer[fft_y][rx1_notch_line_pos] = color;
	}
	if (SecondaryVFO->ManualNotchFilter && !TRX_on_TX && rx2_notch_line_pos >= 0 && rx2_notch_line_pos < LAYOUT->FFT_PRINT_SIZE)
	{
		uint16_t color = palette_fft[fftHeight * 1 / 4];
		for (uint32_t fft_y = 0; fft_y < FFT_AND_WTF_HEIGHT; fft_y++)
			print_output_buffer[fft_y][rx2_notch_line_pos] = color;
	}

	// Draw RX2 center line
	if (rx2_line_pos >= 0 && rx2_line_pos < LAYOUT->FFT_PRINT_SIZE && TRX.Show_Sec_VFO)
	{
		uint16_t color = palette_fft[fftHeight / 2];
		for (uint32_t fft_y = 0; fft_y < FFT_AND_WTF_HEIGHT; fft_y++)
			print_output_buffer[fft_y][rx2_line_pos] = color;
	}

	// Draw RX1 center line
	uint16_t color = palette_fft[fftHeight / 2];
	for (uint32_t fft_y = 0; fft_y < FFT_AND_WTF_HEIGHT; fft_y++)
		print_output_buffer[fft_y][(LAYOUT->FFT_PRINT_SIZE / 2)] = color;

	// Draw BW lines
	if (TRX.FFT_BW_Style == 3)
	{
		uint16_t color_bw = palette_fft[fftHeight / 2];
		uint16_t color_center = palette_fft[0];
		for (uint32_t fft_y = 0; fft_y < FFT_AND_WTF_HEIGHT; fft_y++)
		{
			print_output_buffer[fft_y][bw_rx1_line_start] = color_bw;
			print_output_buffer[fft_y][bw_rx1_line_end] = color_bw;
			print_output_buffer[fft_y][(LAYOUT->FFT_PRINT_SIZE / 2)] = color_center;
		}
	}

	// DXCluster labels
	if (TRX.FFT_DXCluster)
	{
		int32_t prev_pos = -999;
		int32_t prev_w = 0;
		uint16_t prev_y = 5;
		for (uint16_t i = 0; i < WIFI_DXCLUSTER_list_count; i++)
		{
			int32_t pos = getFreqPositionOnFFT(WIFI_DXCLUSTER_list[i].Freq, true);
			if (pos >= -50 && pos < LAYOUT->FFT_PRINT_SIZE)
			{
				uint16_t y = 5;
				if ((pos - prev_pos) < prev_w)
					y = prev_y + 10;
				if (y < (fftHeight - 10))
				{
					prev_y = y;
					prev_w = strlen(WIFI_DXCLUSTER_list[i].Callsign) * 6 + 4;

					char str[64] = {0};
					strcat(str, WIFI_DXCLUSTER_list[i].Callsign);
					if (TRX.FFT_DXCluster_Azimuth)
					{
						sprintf(str, "%s %u^o", WIFI_DXCLUSTER_list[i].Callsign, WIFI_DXCLUSTER_list[i].Azimuth);
						prev_w += 5 * 6;
					}

					LCDDriver_printTextInMemory(str, pos + 2, y, FG_COLOR, BG_COLOR, 1, (uint16_t *)print_output_buffer, LAYOUT->FFT_PRINT_SIZE, FFT_AND_WTF_HEIGHT);
					// vertical line
					if (pos >= 0)
						for (uint8_t y_line = 0; y_line < 8; y_line++)
							print_output_buffer[y + y_line][pos] = COLOR_RED;
				}
			}
			prev_pos = pos;
		}
	}

	// Print DBM grid (LOG Scale)
	if (TRX.FFT_dBmGrid && TRX.FFT_Scale_Type == 0)
	{
		char tmp[64] = {0};
		float32_t ampl_on_bin = maxValueFFT / (float32_t)fftHeight;
		if (!TRX.FFT_Automatic)
		{
			ampl_on_bin = (maxValueFFT + minValueFFT) / (float32_t)fftHeight;
		}

		for (uint16_t y = FFT_DBM_GRID_TOP_MARGIN; y <= fftHeight - 4; y += FFT_DBM_GRID_INTERVAL)
		{
			int16_t dbm = 0;
			if (TRX.FFT_Automatic)
			{
				dbm = getDBFromFFTAmpl(maxValueFFT - ampl_on_bin * (float32_t)y);
			}
			else
			{
				dbm = getDBFromFFTAmpl((maxValueFFT + minValueFFT) - ampl_on_bin * (float32_t)y);
			}
			if (dbm > 50)
				continue;
			sprintf(tmp, "%d", dbm);
			LCDDriver_printTextInMemory(tmp, 0, y - 4, FG_COLOR, BG_COLOR, 1, (uint16_t *)print_output_buffer, LAYOUT->FFT_PRINT_SIZE, FFT_AND_WTF_HEIGHT);
		}
	}

	// Print DBM grid (dBm Scale)
	if (TRX.FFT_dBmGrid && TRX.FFT_Scale_Type == 1)
	{
		char tmp[64] = {0};
		float32_t dbm_on_bin = (FFT_maxDBM - FFT_minDBM) / (float32_t)fftHeight;
		// println(FFT_minDBM, " ", FFT_maxDBM);
		for (uint16_t y = FFT_DBM_GRID_TOP_MARGIN; y <= fftHeight - 4; y += FFT_DBM_GRID_INTERVAL)
		{
			int16_t dbm = FFT_maxDBM - dbm_on_bin * (float32_t)y;
			if (dbm > 50)
				continue;
			sprintf(tmp, "%d", dbm);
			LCDDriver_printTextInMemory(tmp, 0, y - 4, FG_COLOR, BG_COLOR, 1, (uint16_t *)print_output_buffer, LAYOUT->FFT_PRINT_SIZE, FFT_AND_WTF_HEIGHT);
		}
	}

	// Init print 2D FFT+WTF
	Aligned_CleanDCache_by_Addr(print_output_buffer, sizeof(print_output_buffer));
	uint32_t fft_2d_print_height = fftHeight + wtf_printed_lines;
	LCDDriver_SetCursorAreaPosition(0, LAYOUT->FFT_FFTWTF_POS_Y, LAYOUT->FFT_PRINT_SIZE - 1, LAYOUT->FFT_FFTWTF_POS_Y + fft_2d_print_height - 1);
	print_fft_dma_estimated_size = LAYOUT->FFT_PRINT_SIZE * fft_2d_print_height;
	print_fft_dma_position = 0;

	FFT_afterPrintFFT();
	return true;
}

// 3D mode print
static void FFT_3DPrintFFT(void)
{
	uint16_t wtfHeight = GET_WTFHeight;
	uint16_t fftHeight = GET_FFTHeight;
	uint_fast8_t decoder_offset = 0;
	if (NeedProcessDecoder)
		decoder_offset = LAYOUT->FFT_CWDECODER_OFFSET;

	// clear old data
	dma_memset(print_output_buffer, 0, sizeof(print_output_buffer));

	// draw 3D WTF
	for (int32_t wtf_yindex = 0; wtf_yindex <= FFT_3D_SLIDES; wtf_yindex++) // each slides
	{
		// calc perspective parameters
		uint32_t print_y = fftHeight + wtfHeight - decoder_offset - wtf_yindex * FFT_3D_Y_OFFSET;
		float32_t x_compress = (float32_t)(LAYOUT->FFT_PRINT_SIZE - FFT_3D_X_OFFSET * wtf_yindex) / (float32_t)LAYOUT->FFT_PRINT_SIZE;
		uint32_t x_left_offset = (uint32_t)roundf(((float32_t)LAYOUT->FFT_PRINT_SIZE - (float32_t)LAYOUT->FFT_PRINT_SIZE * x_compress) / 2.0f);
		int16_t prev_x = -1;

		// each bin
		for (uint32_t wtf_x = 0; wtf_x < LAYOUT->FFT_PRINT_SIZE; wtf_x++)
		{
			// calc bin perspective
			uint32_t print_bin_height = print_y - (fftHeight - indexed_wtf_buffer[wtf_yindex][wtf_x]);
			if (print_bin_height > wtfHeight + fftHeight - decoder_offset)
				continue;
			if (print_bin_height >= FFT_AND_WTF_HEIGHT)
				continue;
			uint32_t print_x = x_left_offset + (uint32_t)roundf((float32_t)wtf_x * x_compress);
			if (prev_x == print_x)
				continue;
			prev_x = print_x;

			if (TRX.FFT_3D == 1) // line mode
			{
				int32_t line_max = (fftHeight - indexed_wtf_buffer[wtf_yindex][wtf_x] - 1);
				if ((print_bin_height + line_max) >= FFT_AND_WTF_HEIGHT)
					line_max = FFT_AND_WTF_HEIGHT - print_bin_height - 1;

				for (uint16_t h = 0; h < line_max; h++)
				{
					if (print_output_buffer[print_bin_height + h][print_x] != palette_wtf[fftHeight])
						break;
					print_output_buffer[print_bin_height + h][print_x] = palette_wtf[indexed_wtf_buffer[wtf_yindex][wtf_x] + h];
				}
			}
			if (TRX.FFT_3D == 2) // pixel mode
				if (print_output_buffer[print_bin_height][print_x] == palette_wtf[fftHeight])
					print_output_buffer[print_bin_height][print_x] = palette_wtf[indexed_wtf_buffer[wtf_yindex][wtf_x]];
		}
	}

	// draw front fft
	for (uint32_t fft_y = 0; fft_y < fftHeight; fft_y++)
	{
		for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++)
		{
			// bg
			if (fft_y > (fftHeight - fft_header[fft_x]))
			{
				if ((fft_x >= bw_rx1_line_start && fft_x <= bw_rx1_line_end) || ((int32_t)fft_x >= bw_rx2_line_start && (int32_t)fft_x <= bw_rx2_line_end))
					print_output_buffer[wtfHeight - decoder_offset + fft_y][fft_x] = palette_bw_fft_colors[fft_y];
				else
					print_output_buffer[wtfHeight - decoder_offset + fft_y][fft_x] = palette_fft[fft_y];
			}
		}
	}

	// draw contour
	uint32_t fft_y_prev = 0;
	for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++)
	{
		uint32_t fft_y = fftHeight - fft_header[fft_x];
		int32_t y_diff = (int32_t)fft_y - (int32_t)fft_y_prev;
		if (fft_x == 0 || (y_diff <= 1 && y_diff >= -1))
		{
			print_output_buffer[wtfHeight - decoder_offset - 1 + fft_y][fft_x] = palette_fft[fftHeight];
		}
		else
		{
			for (uint32_t l = 0; l < (abs(y_diff / 2) + 1); l++) // draw line
			{
				print_output_buffer[wtfHeight - decoder_offset - 1 + fft_y_prev + ((y_diff > 0) ? l : -l)][fft_x - 1] = palette_fft[fftHeight];
				print_output_buffer[wtfHeight - decoder_offset - 1 + fft_y + ((y_diff > 0) ? -l : l)][fft_x] = palette_fft[fftHeight];
			}
		}
		fft_y_prev = fft_y;
	}

	//Сenter line
	for (uint32_t fft_y = 0; fft_y < FFT_AND_WTF_HEIGHT; fft_y++)
	{
		print_output_buffer[fft_y][LAYOUT->FFT_PRINT_SIZE / 2] = palette_fft[fftHeight / 2];
	}

	// Init print 3D FFT
	Aligned_CleanDCache_by_Addr(print_output_buffer, sizeof(print_output_buffer));
	uint32_t fft_3d_print_height = fftHeight + (uint16_t)(wtfHeight - decoder_offset) - 1;
	LCDDriver_SetCursorAreaPosition(0, LAYOUT->FFT_FFTWTF_POS_Y, LAYOUT->FFT_PRINT_SIZE - 1, LAYOUT->FFT_FFTWTF_POS_Y + fft_3d_print_height);
	print_fft_dma_estimated_size = LAYOUT->FFT_PRINT_SIZE * fft_3d_print_height;
	print_fft_dma_position = 0;

	// do after events
	FFT_afterPrintFFT();
}

// actions after FFT_printFFT
void FFT_afterPrintFFT(void)
{
	// continue DMA draw?
	if (print_fft_dma_estimated_size > 0)
	{
#ifdef LCD_TYPE_FSMC
		if (print_fft_dma_estimated_size <= DMA_MAX_BLOCK)
		{
			HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream5, (uint32_t)&print_output_buffer[0] + print_fft_dma_position * 2, LCD_FSMC_DATA_ADDR, print_fft_dma_estimated_size);
			print_fft_dma_estimated_size = 0;
			print_fft_dma_position = 0;
		}
		else
		{
			print_fft_dma_estimated_size -= DMA_MAX_BLOCK;
			HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream5, (uint32_t)&print_output_buffer[0] + print_fft_dma_position * 2, LCD_FSMC_DATA_ADDR, DMA_MAX_BLOCK);
			print_fft_dma_position += DMA_MAX_BLOCK;
		}
		return;
#endif
#ifdef LCD_TYPE_SPI
		if (hspi4.Init.DataSize != SPI_DATASIZE_16BIT)
		{
			hspi4.Init.DataSize = SPI_DATASIZE_16BIT;
			HAL_SPI_Init(&hspi4);
		}
		LCD_DC_GPIO_Port->BSRR = LCD_DC_Pin;

		HAL_SPI_Transmit_DMA(&hspi4, (uint8_t *)(&print_output_buffer[0] + print_fft_dma_position * 2), print_fft_dma_estimated_size);
		while (HAL_SPI_GetState(&hspi4) != HAL_SPI_STATE_READY)
		{
			CPULOAD_GoToSleepMode();
		}

		print_fft_dma_estimated_size = 0;
		print_fft_dma_position = 0;
#endif
	}

	// calc bandmap
	if (lastWTFFreq != currentFFTFreq || NeedWTFRedraw)
	{
		// clear and display part of the vertical bar
		dma_memset(bandmap_line_tmp, 0x00, sizeof(bandmap_line_tmp));

		// output bandmaps
		int8_t band_curr = getBandFromFreq(CurrentVFO->Freq, true);
		int8_t band_left = band_curr;
		if (band_curr > 0)
			band_left = band_curr - 1;
		int8_t band_right = band_curr;
		if (band_curr < (BANDS_COUNT - 1))
			band_right = band_curr + 1;
		int64_t fft_freq_position_start = 0;
		int64_t fft_freq_position_stop = 0;
		for (uint16_t band = band_left; band <= band_right; band++)
		{
			// regions
			for (uint16_t region = 0; region < BANDS[band].regionsCount; region++)
			{
				uint16_t region_color = COLOR->BANDMAP_SSB;
				if (BANDS[band].regions[region].mode == TRX_MODE_CW)
					region_color = COLOR->BANDMAP_CW;
				else if (BANDS[band].regions[region].mode == TRX_MODE_DIGI_L || BANDS[band].regions[region].mode == TRX_MODE_DIGI_U || BANDS[band].regions[region].mode == TRX_MODE_RTTY)
					region_color = COLOR->BANDMAP_DIGI;
				else if (BANDS[band].regions[region].mode == TRX_MODE_NFM || BANDS[band].regions[region].mode == TRX_MODE_WFM)
					region_color = COLOR->BANDMAP_FM;
				else if (BANDS[band].regions[region].mode == TRX_MODE_AM || BANDS[band].regions[region].mode == TRX_MODE_SAM)
					region_color = COLOR->BANDMAP_AM;

				fft_freq_position_start = getFreqPositionOnFFT(BANDS[band].regions[region].startFreq, false);
				fft_freq_position_stop = getFreqPositionOnFFT(BANDS[band].regions[region].endFreq, false);
				if (fft_freq_position_start != -1 && fft_freq_position_stop == -1)
					fft_freq_position_stop = LAYOUT->FFT_PRINT_SIZE;
				if (fft_freq_position_start == -1 && fft_freq_position_stop != -1)
					fft_freq_position_start = 0;
				if (fft_freq_position_start == -1 && fft_freq_position_stop == -1 && BANDS[band].regions[region].startFreq < CurrentVFO->Freq && BANDS[band].regions[region].endFreq > CurrentVFO->Freq)
				{
					fft_freq_position_start = 0;
					fft_freq_position_stop = LAYOUT->FFT_PRINT_SIZE;
				}

				if (fft_freq_position_start != -1 && fft_freq_position_stop != -1)
					for (int64_t pixel_counter = fft_freq_position_start; pixel_counter < fft_freq_position_stop; pixel_counter++)
						bandmap_line_tmp[(uint16_t)pixel_counter] = region_color;
			}
		}
	}

	LCDDriver_SetCursorAreaPosition(0, LAYOUT->FFT_FFTWTF_POS_Y - LAYOUT->FFT_FREQLABELS_HEIGHT - 4, LAYOUT->FFT_PRINT_SIZE - 1, LAYOUT->FFT_FFTWTF_POS_Y - 3);
	for (uint8_t r = 0; r < 2; r++)
		for (uint32_t pixel_counter = 0; pixel_counter < LAYOUT->FFT_PRINT_SIZE; pixel_counter++)
			LCDDriver_SendData16(bandmap_line_tmp[pixel_counter]);

	// Print FFT frequency labels
	if (LAYOUT->FFT_FREQLABELS_HEIGHT > 0 && (lastWTFFreq != currentFFTFreq || NeedWTFRedraw))
	{
		bool first = true;
		char str[32] = {0};
		for (int32_t grid_line_index = 0; grid_line_index < FFT_MAX_GRID_NUMBER; grid_line_index++)
			if (grid_lines_pos[grid_line_index] > 0 && grid_lines_pos[grid_line_index] < LAYOUT->FFT_PRINT_SIZE)
			{
				if (first)
				{
					LCDDriver_Fill_RectWH(0, LAYOUT->FFT_FFTWTF_POS_Y - LAYOUT->FFT_FREQLABELS_HEIGHT, grid_lines_pos[grid_line_index], LAYOUT->FFT_FREQLABELS_HEIGHT - 1, BG_COLOR);
					first = false;
				}
				if (grid_lines_pos[grid_line_index + 1] > 0)
					LCDDriver_Fill_RectWH(grid_lines_pos[grid_line_index], LAYOUT->FFT_FFTWTF_POS_Y - LAYOUT->FFT_FREQLABELS_HEIGHT, grid_lines_pos[grid_line_index + 1] - grid_lines_pos[grid_line_index], LAYOUT->FFT_FREQLABELS_HEIGHT - 1, BG_COLOR);
				else
					LCDDriver_Fill_RectWH(grid_lines_pos[grid_line_index], LAYOUT->FFT_FFTWTF_POS_Y - LAYOUT->FFT_FREQLABELS_HEIGHT, LAYOUT->FFT_PRINT_SIZE - grid_lines_pos[grid_line_index], LAYOUT->FFT_FREQLABELS_HEIGHT - 1, BG_COLOR);
				uint64_t freq = grid_lines_freq[grid_line_index] / 1000;
				float64_t freq2 = (float32_t)freq / 1000.f;
				sprintf(str, "%.3f", freq2);
				int32_t x = grid_lines_pos[grid_line_index] - (strlen(str) * 6 / 2);
				LCDDriver_printText(str, x, LAYOUT->FFT_FFTWTF_POS_Y - LAYOUT->FFT_FREQLABELS_HEIGHT, FG_COLOR, BG_COLOR, 1);
			}
	}

	// finish
	FFT_FPS++;
	lastWTFFreq = currentFFTFreq;
	if (NeedWTFRedraw) // redraw cycles counter
	{
		if (needredraw_wtf_counter == 0)
		{
			needredraw_wtf_counter = 3;
			NeedWTFRedraw = false;
		}
		else
		{
			needredraw_wtf_counter--;
		}
	}
	FFT_need_fft = true;
	LCD_busy = false;
}

// get color from signal strength
static uint16_t getFFTColor(uint_fast8_t height, bool type) // Get FFT color warmth (blue to red) , type 0 - fft, type 1 - wtf
{
	uint_fast8_t red = 0;
	uint_fast8_t green = 0;
	uint_fast8_t blue = 0;
	if (COLOR->WTF_BG_WHITE)
	{
		red = 255;
		green = 255;
		blue = 255;
	}

	// blue -> yellow -> red
	if ((!type && TRX.FFT_Color == 1) || (type && TRX.WTF_Color == 1))
	{
		// r g b
		// 0 0 0
		// 0 0 255
		// 255 255 0
		// 255 0 0
		// contrast of each of the 3 zones, the total should be 1.0f
		const float32_t contrast1 = 0.02f;
		const float32_t contrast2 = 0.45f;
		const float32_t contrast3 = 0.53f;

		if (height < GET_FFTHeight * contrast1)
		{
			blue = (uint_fast8_t)(height * 255 / (GET_FFTHeight * contrast1));
			if (COLOR->WTF_BG_WHITE)
			{
				red -= blue;
				green -= blue;
			}
		}
		else if (height < GET_FFTHeight * (contrast1 + contrast2))
		{
			green = (uint_fast8_t)((height - GET_FFTHeight * contrast1) * 255 / ((GET_FFTHeight - GET_FFTHeight * contrast1) * (contrast1 + contrast2)));
			red = green;
			blue = 255 - green;
		}
		else
		{
			red = 255;
			blue = 0;
			green = (uint_fast8_t)(255 - (height - (GET_FFTHeight * (contrast1 + contrast2))) * 255 / ((GET_FFTHeight - (GET_FFTHeight * (contrast1 + contrast2))) * (contrast1 + contrast2 + contrast3)));
		}
		return rgb888torgb565(red, green, blue);
	}

	// blue -> yellow -> red // version 2
	if ((!type && TRX.FFT_Color == 2) || (type && TRX.WTF_Color == 2))
	{
		// r g b
		// 0 0 0
		// 0 0 255
		// 255 255 0
		// 255 0 0
		// contrast of each of the 3 zones, the total should be 1.0f
		const float32_t contrast1 = 0.40f; // blue
		const float32_t contrast2 = 0.30f; // yellow
		const float32_t contrast3 = 0.30f; // red

		if (height < GET_FFTHeight * contrast1)
		{
			blue = (uint_fast8_t)(height * 255 / (GET_FFTHeight * contrast1));
			if (COLOR->WTF_BG_WHITE)
			{
				red -= blue;
				green -= blue;
			}
		}
		else if (height < GET_FFTHeight * (contrast1 + contrast2))
		{
			green = (uint_fast8_t)((height - GET_FFTHeight * contrast1) * 255 / ((GET_FFTHeight - GET_FFTHeight * contrast1) * (contrast1 + contrast2)));
			red = green;
			blue = 255 - green;
		}
		else
		{
			red = 255;
			blue = 0;
			green = (uint_fast8_t)(255 - (height - (GET_FFTHeight * (contrast1 + contrast2))) * 255 / ((GET_FFTHeight - (GET_FFTHeight * (contrast1 + contrast2))) * (contrast1 + contrast2 + contrast3)));
		}
		return rgb888torgb565(red, green, blue);
	}

	// black -> yellow -> red
	if ((!type && TRX.FFT_Color == 3) || (type && TRX.WTF_Color == 3))
	{
		// r g b
		// 0 0 0
		// 255 255 0
		// 255 0 0
		// contrast of each of the 2 zones, the total should be 1.0f
		float32_t contrast1 = 0.5f;
		float32_t contrast2 = 0.5f;
		if (COLOR->WTF_BG_WHITE)
		{
			contrast1 = 0.2f;
			contrast2 = 0.8f;
		}

		if (height < GET_FFTHeight * contrast1)
		{
			if (!COLOR->WTF_BG_WHITE)
			{
				red = (uint_fast8_t)(height * 255 / (GET_FFTHeight * contrast1));
				green = (uint_fast8_t)(height * 255 / (GET_FFTHeight * contrast1));
				blue = 0;
			}
			else
			{
				red = 255;
				green = 255;
				blue = 255 - (uint_fast8_t)(height * 255 / (GET_FFTHeight * contrast1));
			}
		}
		else
		{
			red = 255;
			blue = 0;
			green = (uint_fast8_t)(255 - (height - (GET_FFTHeight * (contrast1))) * 255 / ((GET_FFTHeight - (GET_FFTHeight * (contrast1))) * (contrast1 + contrast2)));
			if (COLOR->WTF_BG_WHITE)
			{
				blue = green;
			}
		}
		return rgb888torgb565(red, green, blue);
	}

	// black -> yellow -> green
	if ((!type && TRX.FFT_Color == 4) || (type && TRX.WTF_Color == 4))
	{
		// r g b
		// 0 0 0
		// 255 255 0
		// 0 255 0
		// contrast of each of the 2 zones, the total should be 1.0f
		float32_t contrast1 = 0.5f;
		float32_t contrast2 = 0.5f;
		if (COLOR->WTF_BG_WHITE)
		{
			contrast1 = 0.2f;
			contrast2 = 0.8f;
		}

		if (height < GET_FFTHeight * contrast1)
		{
			if (!COLOR->WTF_BG_WHITE)
			{
				red = (uint_fast8_t)(height * 255 / (GET_FFTHeight * contrast1));
				green = (uint_fast8_t)(height * 255 / (GET_FFTHeight * contrast1));
				blue = 0;
			}
			else
			{
				red = 255;
				green = 255;
				blue = 255 - (uint_fast8_t)(height * 255 / (GET_FFTHeight * contrast1));
			}
		}
		else
		{
			green = 255;
			blue = 0;
			red = (uint_fast8_t)(255 - (height - (GET_FFTHeight * (contrast1))) * 255 / ((GET_FFTHeight - (GET_FFTHeight * (contrast1))) * (contrast1 + contrast2)));
			if (COLOR->WTF_BG_WHITE)
			{
				green = red;
			}
		}
		return rgb888torgb565(red, green, blue);
	}

	// black -> red
	if ((!type && TRX.FFT_Color == 5) || (type && TRX.WTF_Color == 5))
	{
		// r g b
		// 0 0 0
		// 255 0 0

		if (height <= GET_FFTHeight)
		{
			red = (uint_fast8_t)(height * 255 / (GET_FFTHeight));
			if (COLOR->WTF_BG_WHITE)
			{
				green -= (uint_fast8_t)(height * 255 / (GET_FFTHeight));
				blue -= (uint_fast8_t)(height * 255 / (GET_FFTHeight));
				red = 255;
			}
		}
		return rgb888torgb565(red, green, blue);
	}

	// black -> green
	if ((!type && TRX.FFT_Color == 6) || (type && TRX.WTF_Color == 6))
	{
		// r g b
		// 0 0 0
		// 0 255 0

		if (height <= GET_FFTHeight)
		{
			green = (uint_fast8_t)(height * 255 / (GET_FFTHeight));
			if (COLOR->WTF_BG_WHITE)
			{
				green = 255;
				blue -= (uint_fast8_t)(height * 255 / (GET_FFTHeight));
				red -= (uint_fast8_t)(height * 255 / (GET_FFTHeight));
			}
		}
		return rgb888torgb565(red, green, blue);
	}

	// black -> blue
	if ((!type && TRX.FFT_Color == 7) || (type && TRX.WTF_Color == 7))
	{
		// r g b
		// 0 0 0
		// 0 0 255

		if (height <= GET_FFTHeight)
		{
			blue = (uint_fast8_t)(height * 255 / (GET_FFTHeight));
			if (COLOR->WTF_BG_WHITE)
			{
				green -= (uint_fast8_t)(height * 255 / (GET_FFTHeight));
				blue = 255;
				red -= (uint_fast8_t)(height * 255 / (GET_FFTHeight));
			}
		}
		return rgb888torgb565(red, green, blue);
	}

	// black -> white
	if ((!type && TRX.FFT_Color == 8) || (type && TRX.WTF_Color == 8))
	{
		// r g b
		// 0 0 0
		// 255 255 255

		if (height <= GET_FFTHeight)
		{
			red = (uint_fast8_t)(height * 255 / (GET_FFTHeight));
			green = red;
			blue = red;
			if (COLOR->WTF_BG_WHITE)
			{
				red = 255 - red;
				green = 255 - green;
				blue = 255 - blue;
			}
		}
		return rgb888torgb565(red, green, blue);
	}

	// unknown
	return COLOR_WHITE;
}

static uint16_t getBGColor(uint_fast8_t height) // Get FFT background gradient
{
	float32_t fftheight = GET_FFTHeight;
	float32_t step_red = (float32_t)(COLOR->FFT_GRADIENT_END_R - COLOR->FFT_GRADIENT_START_R) / fftheight;
	float32_t step_green = (float32_t)(COLOR->FFT_GRADIENT_END_G - COLOR->FFT_GRADIENT_START_G) / fftheight;
	float32_t step_blue = (float32_t)(COLOR->FFT_GRADIENT_END_B - COLOR->FFT_GRADIENT_START_B) / fftheight;

	uint_fast8_t red = (uint_fast8_t)(COLOR->FFT_GRADIENT_START_R + (float32_t)height * step_red);
	uint_fast8_t green = (uint_fast8_t)(COLOR->FFT_GRADIENT_START_G + (float32_t)height * step_green);
	uint_fast8_t blue = (uint_fast8_t)(COLOR->FFT_GRADIENT_START_B + (float32_t)height * step_blue);

	return rgb888torgb565(red, green, blue);
}

// prepare the color palette
static void FFT_fill_color_palette(void) // Fill FFT Color Gradient On Initialization
{
	uint8_t FFT_BW_BRIGHTNESS = 0;
	if (TRX.FFT_BW_Style == 1)
		FFT_BW_BRIGHTNESS = FFT_BW_BRIGHTNESS_1;
	if (TRX.FFT_BW_Style == 2)
		FFT_BW_BRIGHTNESS = FFT_BW_BRIGHTNESS_2;

	for (uint_fast8_t i = 0; i <= GET_FFTHeight; i++)
	{
		palette_fft[i] = getFFTColor(GET_FFTHeight - i, false);
		palette_wtf[i] = getFFTColor(GET_FFTHeight - i, true);
		if (TRX.FFT_BW_Style == 3 && !TRX.FFT_Background) // lines BW without background
			palette_bg_gradient[i] = getFFTColor(0, false);
		else
			palette_bg_gradient[i] = getBGColor(GET_FFTHeight - i);
		
		palette_bw_fft_colors[i] = addColor(palette_fft[i], FFT_BW_BRIGHTNESS, FFT_BW_BRIGHTNESS, FFT_BW_BRIGHTNESS);
		palette_bw_wtf_colors[i] = addColor(palette_wtf[i], FFT_BW_BRIGHTNESS, FFT_BW_BRIGHTNESS, FFT_BW_BRIGHTNESS);
		palette_bw_bg_colors[i] = addColor(palette_bg_gradient[i], FFT_BW_BRIGHTNESS, FFT_BW_BRIGHTNESS, FFT_BW_BRIGHTNESS);
	}
}

static inline int32_t getFreqPositionOnFFT(uint64_t freq, bool full_pos)
{
	int32_t pos = (int32_t)((float32_t)LAYOUT->FFT_PRINT_SIZE / 2 + (float32_t)((float32_t)freq - (float32_t)CurrentVFO->Freq) / hz_in_pixel * (float32_t)fft_zoom);
	if (!full_pos && (pos < 0 || pos >= LAYOUT->FFT_PRINT_SIZE))
		return -1;
	if (TRX.FFT_Lens) // lens correction
		pos = FFT_getLensCorrection(pos);
	return pos;
}

uint32_t getFreqOnFFTPosition(uint16_t position)
{
	return (uint32_t)((int32_t)CurrentVFO->Freq + (int32_t)(-((float32_t)LAYOUT->FFT_PRINT_SIZE * (hz_in_pixel / (float32_t)fft_zoom) / 2.0f) + (float32_t)position * (hz_in_pixel / (float32_t)fft_zoom)));
}

static uint32_t FFT_getLensCorrection(uint32_t normal_distance_from_center)
{
	float32_t fft_normal_compress_rate = (float32_t)FFT_USEFUL_SIZE / (float32_t)LAYOUT->FFT_PRINT_SIZE;
	float32_t normal_distance_from_center_converted = (float32_t)normal_distance_from_center * fft_normal_compress_rate;
	float32_t step_now = FFT_LENS_STEP_START;
	float32_t index1 = (float32_t)FFT_USEFUL_SIZE / 2.0f;
	float32_t index2 = index1;
	for (uint32_t i = 0; i <= (LAYOUT->FFT_PRINT_SIZE / 2); i++)
	{
		if (normal_distance_from_center < (LAYOUT->FFT_PRINT_SIZE / 2))
		{
			if (normal_distance_from_center_converted > index1)
			{
				return (LAYOUT->FFT_PRINT_SIZE / 2 - i);
			}
		}
		else
		{
			if (normal_distance_from_center_converted < index2)
			{
				if (i != (LAYOUT->FFT_PRINT_SIZE / 2))
					return (LAYOUT->FFT_PRINT_SIZE / 2 + i);
				else
					return (LAYOUT->FFT_PRINT_SIZE - 1);
			}
		}

		step_now += FFT_LENS_STEP;
		index1 -= step_now;
		index2 += step_now;

		if (index1 >= FFT_USEFUL_SIZE)
			index1 = FFT_USEFUL_SIZE - 1;
		if (index1 < 0)
			index1 = 0;
		if (index2 >= FFT_USEFUL_SIZE)
			index2 = FFT_USEFUL_SIZE - 1;
		if (index2 < 0)
			index2 = 0;
	}
	return normal_distance_from_center;
}

static float32_t getDBFromFFTAmpl(float32_t ampl)
{
	float32_t db = rate2dbP(powf(ampl / (float32_t)FFT_SIZE, 2) / 50.0f / 0.001f) + FFT_DBM_COMPENSATION; // roughly... because window and other...
	if (TRX.ADC_Driver)
		db -= ADC_DRIVER_GAIN_DB;

	if (CurrentVFO->Freq < 70000000)
		db += CALIBRATE.smeter_calibration_hf;
	else
		db += CALIBRATE.smeter_calibration_vhf;

	return db;
}

static float32_t getFFTAmplFromDB(float32_t ampl)
{
	float32_t result;
	arm_sqrt_f32(db2rateP(ampl - FFT_DBM_COMPENSATION + (TRX.ADC_Driver ? ADC_DRIVER_GAIN_DB : 0.0f) - ((CurrentVFO->Freq < 70000000) ? CALIBRATE.smeter_calibration_hf : CALIBRATE.smeter_calibration_vhf)) * 0.001f * 50.0f, &result);
	return result * (float32_t)FFT_SIZE;
}
