#include "fft.h"
#include "main.h"
#include "arm_const_structs.h"
#include "audio_filters.h"
#include "screen_layout.h"
#include "vad.h"
#include "lcd.h"
#include "cw_decoder.h"

//Public variables
bool FFT_need_fft = true;					// need to prepare data for display on the screen
bool FFT_new_buffer_ready = false;				// buffer is full, can be processed
uint32_t FFT_buff_index = 0;		// current buffer index when it is filled with FPGA
bool FFT_buff_current = 0;		// current FFT Input buffer A - false, B - true
IRAM2 float32_t FFTInput_I_A[FFT_SIZE] = {0}; // incoming buffer FFT I
IRAM2 float32_t FFTInput_Q_A[FFT_SIZE] = {0}; // incoming buffer FFT Q
IRAM2 float32_t FFTInput_I_B[FFT_SIZE] = {0}; // incoming buffer FFT I
IRAM2 float32_t FFTInput_Q_B[FFT_SIZE] = {0}; // incoming buffer FFT Q
uint16_t FFT_FPS = 0;
uint16_t FFT_FPS_Last = 0;
bool NeedWTFRedraw = false;

//Private variables
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

IRAM2 static float32_t FFTInputCharge[FFT_DOUBLE_SIZE_BUFFER] = {0}; // charge FFT I and Q buffer
IRAM2 static float32_t FFTInput[FFT_DOUBLE_SIZE_BUFFER] = {0}; // combined FFT I and Q buffer
IRAM2 static float32_t FFTInput_tmp[MAX_FFT_PRINT_SIZE] = {0};	   // temporary buffer for sorting, moving and fft compressing
static float32_t FFTOutput_mean[MAX_FFT_PRINT_SIZE] = {0};	   // averaged FFT buffer (for output)
static float32_t maxValueFFT_rx = 0;						   // maximum value of the amplitude in the resulting frequency response
static float32_t maxValueFFT_tx = 0;						   // maximum value of the amplitude in the resulting frequency response
static uint32_t currentFFTFreq = 0;
static uint32_t lastWTFFreq = 0;													 //last WTF printed freq
static uint16_t palette_fft[MAX_FFT_HEIGHT + 1] = {0};									 // color palette with FFT colors
static uint16_t palette_bg_gradient[MAX_FFT_HEIGHT + 1] = {0};							 // color palette with gradient background of FFT
static uint16_t palette_bw_fft_colors[MAX_FFT_HEIGHT + 1] = {0};						 // color palette with bw highlighted FFT colors
static uint16_t palette_bw_bg_colors[MAX_FFT_HEIGHT + 1] = {0};							 // color palette with bw highlighted background colors
SRAM static uint16_t fft_output_buffer[MAX_FFT_HEIGHT][MAX_FFT_PRINT_SIZE] = {{0}};	 //buffer with fft print data
IRAM2 static uint8_t indexed_wtf_buffer[MAX_WTF_HEIGHT][MAX_FFT_PRINT_SIZE] = {{0}}; //indexed color buffer with wtf
IRAM2 static uint32_t wtf_buffer_freqs[MAX_WTF_HEIGHT] = {0};						 // frequencies for each row of the waterfall
SRAM static uint16_t wtf_output_line[MAX_FFT_PRINT_SIZE] = {0};						 // temporary buffer to draw the waterfall
IRAM2 static uint8_t indexed_3d_fft_buffer[FFT_AND_WTF_HEIGHT][MAX_FFT_PRINT_SIZE] = {{0}}; //indexed color buffer with 3d WTF output
static uint16_t fft_header[MAX_FFT_PRINT_SIZE] = {0};								 //buffer with fft colors output
static int32_t grid_lines_pos[20] = {-1};											 //grid lines positions
static int16_t bw_line_start = 0;													 //BW bar params
static int16_t bw_line_width = 0;													 //BW bar params
static int16_t bw_line_end = 0;														 //BW bar params
static int16_t bw_line_center = 0;														 //BW bar params
static uint16_t print_wtf_yindex = 0;												 // the current coordinate of the waterfall output via DMA
static float32_t window_multipliers[FFT_SIZE] = {0};								 // coefficients of the selected window function
static float32_t hz_in_pixel = 1.0f;												 // current FFT density value
static uint16_t bandmap_line_tmp[MAX_FFT_PRINT_SIZE] = {0};							 // temporary buffer to move the waterfall
static arm_sort_instance_f32 FFT_sortInstance = {0};								 // sorting instance (to find the median)
static uint32_t print_fft_dma_estimated_size = 0;			//block size for dma
static uint32_t print_fft_dma_position = 0;			//positior for dma fft print
static uint8_t needredraw_wtf_counter = 3;		//redraw cycles after event
static bool fft_charge_ready = false;
// Decimator for Zoom FFT
static arm_fir_decimate_instance_f32 DECIMATE_ZOOM_FFT_I;
static arm_fir_decimate_instance_f32 DECIMATE_ZOOM_FFT_Q;
static float32_t decimZoomFFTIState[FFT_SIZE + 4 - 1] = {0};
static float32_t decimZoomFFTQState[FFT_SIZE + 4 - 1] = {0};
static uint8_t fft_zoom = 1;
static uint_fast16_t zoomed_width = 0;
//Коэффициенты для ZoomFFT lowpass filtering / дециматора
static arm_biquad_cascade_df2T_instance_f32 IIR_biquad_Zoom_FFT_I =
	{
		.numStages = ZOOMFFT_DECIM_STAGES,
		.pCoeffs = (float32_t *)(float32_t[ZOOMFFT_DECIM_STAGES * 5]){0},
		.pState = (float32_t *)(float32_t[ZOOMFFT_DECIM_STAGES * 2]){0}};
static arm_biquad_cascade_df2T_instance_f32 IIR_biquad_Zoom_FFT_Q =
	{
		.numStages = ZOOMFFT_DECIM_STAGES,
		.pCoeffs = (float32_t *)(float32_t[ZOOMFFT_DECIM_STAGES * 5]){0},
		.pState = (float32_t *)(float32_t[ZOOMFFT_DECIM_STAGES * 2]){0}};

static const float32_t *mag_coeffs[17] =
	{
		NULL, // 0
		NULL, // 1
		// 2x magnify, 24kHz, sample rate 96k, 60dB stopband
		(float32_t *)(const float32_t[ZOOMFFT_DECIM_STAGES * 5]){2.484242790213,0,0,0,0,1,1.898288195851,1,0.6974136319001,-0.3065583381015,0.02825209609289,0,0,0,0,1,1.500253906972,1,0.008112054577326,-0.7721086004364,1,0,0,0,0},
		NULL, // 3
		// 4x magnify, 12kHz, sample rate 96k, 60dB stopband
		(float32_t *)(const float32_t[ZOOMFFT_DECIM_STAGES * 5]){0.7134827863049,0,0,0,0,1,1.472005720002,1,1.415948015621,-0.5717655848516,0.01368406787431,0,0,0,0,1,0.1832282425444,1,1.300866126796,-0.8337859400983,1,0,0,0,0},
		NULL, // 5
		NULL, // 6
		NULL, // 7
		// 8x magnify, 6kHz, sample rate 96k, 60dB stopband
		(float32_t *)(const float32_t[ZOOMFFT_DECIM_STAGES * 5]){0.2759821831997,0,0,0,0,1,0.4104549042334,1,1.718681040914,-0.7605249789395,0.009291375667003,0,0,0,0,1,-1.132037961389,1,1.762711686353,-0.9065677781814,1,0,0,0,0},
		NULL, // 9
		NULL, // 10
		NULL, // 11
		NULL, // 12
		NULL, // 13
		NULL, // 14
		NULL, // 15
		// 16x magnify, 3kHz, sample rate 96k, 60dB stopband
		(float32_t *)(const float32_t[ZOOMFFT_DECIM_STAGES * 5]){0.1614831396677,0,0,0,0,1,-0.9158954187733,1,1.861713716539,-0.8727253274572,0.008184762202728,0,0,0,0,1,-1.745517115598,1,1.914110094101,-0.9512646565581,1,0,0,0,0},
};

static const arm_fir_decimate_instance_f32 FirZoomFFTDecimate[17] =
	{
		{0}, // 0
		{0}, // 1
		// 48ksps, 12kHz lowpass
		{
			.numTaps = 4,
			.pCoeffs = (float32_t *)(const float32_t[]){475.1179397144384210E-6f, 0.503905202786044337f, 0.503905202786044337f, 475.1179397144384210E-6f},
			.pState = NULL},
		{0}, // 3
		// 48ksps, 6kHz lowpass
		{
			.numTaps = 4,
			.pCoeffs = (float32_t *)(const float32_t[]){0.198273254218889416f, 0.298085149879260325f, 0.298085149879260325f, 0.198273254218889416f},
			.pState = NULL},
		{0}, // 5
		{0}, // 6
		{0}, // 7
		// 48ksps, 3kHz lowpass
		{
			.numTaps = 4,
			.pCoeffs = (float32_t *)(const float32_t[]){0.199820836596682871f, 0.272777397353925699f, 0.272777397353925699f, 0.199820836596682871f},
			.pState = NULL},
		{0}, // 9
		{0}, // 10
		{0}, // 11
		{0}, // 12
		{0}, // 13
		{0}, // 14
		{0}, // 15
		// 48ksps, 1.5kHz lowpass
		{
			.numTaps = 4,
			.pCoeffs = (float32_t *)(const float32_t[]){0.199820836596682871f, 0.272777397353925699f, 0.272777397353925699f, 0.199820836596682871f},
			.pState = NULL},
};

//Prototypes
static uint16_t getFFTColor(uint_fast8_t height);											  // get color from signal strength
static void FFT_fill_color_palette(void);													  // prepare the color palette
static void FFT_move(int32_t _freq_diff);													  // shift the waterfall
static int32_t getFreqPositionOnFFT(uint32_t freq);											  // get the position on the FFT for a given frequency
static uint32_t FFT_getLensCorrection(uint32_t normal_distance_from_center);
static void FFT_3DPrintFFT(void);

// FFT initialization
void FFT_PreInit(void)
{
	//Windowing
	//Dolph–Chebyshev
	if (TRX.FFT_Window == 1)
	{
		const float64_t atten = 100.0;
		float64_t max = 0.0;
		float64_t tg = pow(10.0, atten / 20.0);
		float64_t x0 = cosh((1.0 / ((float64_t)FFT_SIZE - 1.0)) * acosh(tg));
		float64_t M = (FFT_SIZE - 1) / 2;
		if((FFT_SIZE % 2) == 0) 
			M = M + 0.5; /* handle even length windows */
		for(uint32_t nn=0; nn < ((FFT_SIZE / 2) + 1); nn++)
		{
			float64_t n = nn - M;
			float64_t sum = 0.0;
			for(uint32_t i = 1; i <= M; i++)
			{
				float64_t cheby_poly = 0.0;
				float64_t cp_x = x0 * cos(F_PI * i / (float64_t)FFT_SIZE);
				float64_t cp_n = FFT_SIZE - 1;
				if (fabs(cp_x) <= 1) 
					cheby_poly = cos(cp_n * acos(cp_x));
				else 
					cheby_poly = cosh(cp_n * acosh(cp_x));
				
				sum += cheby_poly * cos(2.0 * n * F_PI * (float64_t)i / (float64_t)FFT_SIZE);
			}
			window_multipliers[nn] = tg + 2 * sum;
			window_multipliers[FFT_SIZE - nn - 1] = window_multipliers[nn];
			if(window_multipliers[nn] > max)
				max = window_multipliers[nn];
		}
		for(uint32_t nn=0; nn < FFT_SIZE; nn++) 
			window_multipliers[nn] /= max; /* normalise everything */
	}
	for (uint_fast16_t i = 0; i < FFT_SIZE; i++)
	{
		//Blackman-Harris
		if (TRX.FFT_Window == 2)
			window_multipliers[i] = 0.35875f - 0.48829f * arm_cos_f32(2.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) + 0.14128f * arm_cos_f32(4.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) - 0.01168f * arm_cos_f32(6.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f));
		//Nutall
		else if (TRX.FFT_Window == 3)
			window_multipliers[i] = 0.355768f - 0.487396f * arm_cos_f32(2.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) + 0.144232f * arm_cos_f32(4.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) - 0.012604 * arm_cos_f32(6.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f));
		//Blackman-Nutall
		else if (TRX.FFT_Window == 4)
			window_multipliers[i] = 0.3635819f - 0.4891775f * arm_cos_f32(2.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) + 0.1365995f * arm_cos_f32(4.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) - 0.0106411f * arm_cos_f32(6.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f));
		//Hann
		else if (TRX.FFT_Window == 5)
			window_multipliers[i] = 0.5f * (1.0f - arm_cos_f32(2.0f * F_PI * (float32_t)i / (float32_t)FFT_SIZE));
		//Hamming
		else if (TRX.FFT_Window == 6)
			window_multipliers[i] = 0.54f - 0.46f * arm_cos_f32((2.0f * F_PI * (float32_t)i) / ((float32_t)FFT_SIZE - 1.0f));
		//No window
		else if (TRX.FFT_Window == 7)
			window_multipliers[i] = 1.0f;
	}
	
	// initialize sort
	arm_sort_init_f32(&FFT_sortInstance, ARM_SORT_QUICK, ARM_SORT_ASCENDING);
}

void FFT_Init(void)
{
	FFT_fill_color_palette();
	//ZoomFFT
	fft_zoom = TRX.FFT_Zoom;
	if (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U)
		fft_zoom = TRX.FFT_ZoomCW;
	if (fft_zoom > 1)
	{
		IIR_biquad_Zoom_FFT_I.pCoeffs = mag_coeffs[fft_zoom];
		IIR_biquad_Zoom_FFT_Q.pCoeffs = mag_coeffs[fft_zoom];
		memset(IIR_biquad_Zoom_FFT_I.pState, 0x00, sizeof(float32_t) * ZOOMFFT_DECIM_STAGES * 2);
		memset(IIR_biquad_Zoom_FFT_Q.pState, 0x00, sizeof(float32_t) * ZOOMFFT_DECIM_STAGES * 2);
		arm_fir_decimate_init_f32(&DECIMATE_ZOOM_FFT_I,
								  FirZoomFFTDecimate[fft_zoom].numTaps,
								  fft_zoom, // Decimation factor
								  FirZoomFFTDecimate[fft_zoom].pCoeffs,
								  decimZoomFFTIState, // Filter state variables
								  FFT_SIZE);

		arm_fir_decimate_init_f32(&DECIMATE_ZOOM_FFT_Q,
								  FirZoomFFTDecimate[fft_zoom].numTaps,
								  fft_zoom, // Decimation factor
								  FirZoomFFTDecimate[fft_zoom].pCoeffs,
								  decimZoomFFTQState, // Filter state variables
								  FFT_SIZE);
		zoomed_width = FFT_SIZE / fft_zoom;
	}
	else
		zoomed_width = FFT_SIZE;
	
	// clear the buffers
	memset(&fft_output_buffer, 0x00, sizeof(fft_output_buffer));
	memset(&indexed_wtf_buffer, GET_FFTHeight, sizeof(indexed_wtf_buffer));
	memset(&FFTInputCharge, 0x00, sizeof(FFTInputCharge));
	NeedWTFRedraw = true;
}

// FFT calculation
void FFT_bufferPrepare(void)
{
	if (!TRX.FFT_Enabled)
		return;
	if (!FFT_new_buffer_ready)
		return;
	/*if (CPU_LOAD.Load > 90)
		return;*/
	fft_charge_ready = false;
	
	float32_t* FFTInput_I_current = !FFT_buff_current ? (float32_t*)&FFTInput_I_A : (float32_t*)&FFTInput_I_B; //inverted
	float32_t* FFTInput_Q_current = !FFT_buff_current ? (float32_t*)&FFTInput_Q_A : (float32_t*)&FFTInput_Q_B;
	
	//Process DC corrector filter
	if (!TRX_on_TX())
	{
		dc_filter(FFTInput_I_current, FFT_SIZE, DC_FILTER_FFT_I);
		dc_filter(FFTInput_Q_current, FFT_SIZE, DC_FILTER_FFT_Q);
	}

	//Process Notch filter
	if (CurrentVFO()->ManualNotchFilter && !TRX_on_TX())
	{
		arm_biquad_cascade_df2T_f32(&NOTCH_FFT_I_FILTER, FFTInput_I_current, FFTInput_I_current, FFT_SIZE);
		arm_biquad_cascade_df2T_f32(&NOTCH_FFT_Q_FILTER, FFTInput_Q_current, FFTInput_Q_current, FFT_SIZE);
	}

	//ZoomFFT
	if (fft_zoom > 1)
	{
		//Biquad LPF filter
		arm_biquad_cascade_df2T_f32(&IIR_biquad_Zoom_FFT_I, FFTInput_I_current, FFTInput_I_current, FFT_SIZE);
		arm_biquad_cascade_df2T_f32(&IIR_biquad_Zoom_FFT_Q, FFTInput_Q_current, FFTInput_Q_current, FFT_SIZE);
		// Decimator
		arm_fir_decimate_f32(&DECIMATE_ZOOM_FFT_I, FFTInput_I_current, FFTInput_I_current, FFT_SIZE);
		arm_fir_decimate_f32(&DECIMATE_ZOOM_FFT_Q, FFTInput_Q_current, FFTInput_Q_current, FFT_SIZE);
		// Shift old data
		memcpy(&FFTInputCharge[0], &FFTInputCharge[zoomed_width], sizeof(float32_t) * (FFT_SIZE - zoomed_width));
		// Add new data
		for (uint_fast16_t i = 0; i < zoomed_width; i++)
		{
			uint16_t wind_pos = i * (FFT_SIZE / zoomed_width);
			FFTInputCharge[(FFT_SIZE - zoomed_width + i) * 2] = FFTInput_I_current[i] * window_multipliers[wind_pos];
			FFTInputCharge[(FFT_SIZE - zoomed_width + i) * 2 + 1] = FFTInput_Q_current[i] * window_multipliers[wind_pos];
		}
	}
	else
	{
		// make a combined buffer for calculation
		for (uint_fast16_t i = 0; i < FFT_SIZE; i++)
		{
			FFTInputCharge[i * 2] = FFTInput_I_current[i] * window_multipliers[i];
			FFTInputCharge[i * 2 + 1] = FFTInput_Q_current[i] * window_multipliers[i];
		}
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
	/*if (CPU_LOAD.Load > 90)
		return;*/

	// Get charge buffer
	memcpy(&FFTInput, &FFTInputCharge, sizeof(FFTInput));
	memset(&FFTInputCharge, 0x00, sizeof(FFTInputCharge));

	arm_cfft_f32(FFT_Inst, FFTInput, 0, 1);
	arm_cmplx_mag_f32(FFTInput, FFTInput, FFT_SIZE);

	//Debug VAD
	/*memset(FFTInput, 0x00, sizeof(FFTInput));
	for (uint_fast16_t i = 0; i < FFT_SIZE; i++)
	{
		FFTInput[i] = VAD_FFTBuffer_Export[i];
	}
	NeedFFTInputBuffer = true;*/
	
	//Debug CW Decoder
	/*memset(FFTInput, 0x00, sizeof(FFTInput));
	for (uint_fast16_t i = 0; i < CWDECODER_FFTSIZE; i++)
	{
		FFTInput[i] = CWDEC_FFTBuffer_Export[i];
		FFTInput[i + CWDECODER_FFTSIZE] = CWDEC_FFTBuffer_Export[i];
		FFTInput[i + CWDECODER_FFTSIZE * 2] = CWDEC_FFTBuffer_Export[i];
		FFTInput[i + CWDECODER_FFTSIZE * 3] = CWDEC_FFTBuffer_Export[i];
	}*/

	// Swap fft parts
	memcpy(&FFTInput[FFT_SIZE], &FFTInput[0], sizeof(float32_t) * (FFT_SIZE / 2)); //left - > tmp
	memcpy(&FFTInput[0], &FFTInput[FFT_SIZE / 2], sizeof(float32_t) * (FFT_SIZE / 2)); //right - > left
	memcpy(&FFTInput[FFT_SIZE / 2], &FFTInput[FFT_SIZE], sizeof(float32_t) * (FFT_SIZE / 2)); //tmp - > right
	
	// Compress the calculated FFT to visible
	memcpy(&FFTInput[0], &FFTInput[FFT_SIZE / 2 - FFT_USEFUL_SIZE / 2], sizeof(float32_t) * FFT_USEFUL_SIZE); //useful fft part
	float32_t fft_compress_rate = (float32_t)FFT_USEFUL_SIZE / (float32_t)LAYOUT->FFT_PRINT_SIZE;
	float32_t fft_compress_rate_half = floorf(fft_compress_rate / 2.0f); //full points
	float32_t fft_compress_rate_parts = fmodf(fft_compress_rate / 2.0f, 1.0f); //partial points
	
	if(!TRX.FFT_Lens) //normal compress
	{
		for (uint32_t i = 0; i < LAYOUT->FFT_PRINT_SIZE; i++)
		{
			int32_t left_index = (uint32_t)((float32_t)i * fft_compress_rate - fft_compress_rate_half);
			if(left_index < 0)
				left_index = 0;
			int32_t right_index = (uint32_t)((float32_t)i * fft_compress_rate + fft_compress_rate_half);
			if(right_index >= FFT_USEFUL_SIZE)
				right_index = FFT_USEFUL_SIZE - 1;
			
			float32_t points = 0;
			float32_t accum = 0.0f;
			//full points
			for(uint32_t index = left_index; index <= right_index ; index++)
			{
				accum += FFTInput[index];
				points += 1.0f;
			}
			//partial points
			if(fft_compress_rate_parts > 0.0f)
			{
				if(left_index > 0)
				{
					accum += FFTInput[left_index - 1] * fft_compress_rate_parts;
					points += fft_compress_rate_parts;
				}
				if(right_index < (FFT_USEFUL_SIZE - 1))
				{
					accum += FFTInput[right_index + 1] * fft_compress_rate_parts;
					points += fft_compress_rate_parts;
				}
			}
			FFTInput_tmp[i] = accum / points;
		}
	}
	else //lens compress
	{
		float32_t step_now = FFT_LENS_STEP_START;
		float32_t index1 = (float32_t)FFT_USEFUL_SIZE / 2.0f;
		float32_t index2 = index1;
		for (uint32_t i = 0; i <= (LAYOUT->FFT_PRINT_SIZE / 2); i++)
		{
			FFTInput_tmp[(LAYOUT->FFT_PRINT_SIZE / 2) - i] = FFTInput[(uint32_t)roundf(index1)];
			if(i != (LAYOUT->FFT_PRINT_SIZE / 2))
				FFTInput_tmp[(LAYOUT->FFT_PRINT_SIZE / 2) + i] = FFTInput[(uint32_t)roundf(index2)];
			
			step_now += FFT_LENS_STEP;
			index1 -= step_now;
			index2 += step_now;

			if(index1 >= FFT_USEFUL_SIZE)
				index1 = FFT_USEFUL_SIZE - 1;
			if(index1 < 0)
				index1 = 0;
			if(index2 >= FFT_USEFUL_SIZE)
				index2 = FFT_USEFUL_SIZE - 1;
			if(index2 < 0)
				index2 = 0;
		}
	}
	memcpy(&FFTInput, FFTInput_tmp, sizeof(FFTInput_tmp));
	
	//Delete noise
	float32_t minAmplValue = 0;
	uint32_t minAmplIndex = 0;
	arm_min_f32(FFTInput, LAYOUT->FFT_PRINT_SIZE, &minAmplValue, &minAmplIndex);
	if (!TRX_on_TX())
		arm_offset_f32(FFTInput, -minAmplValue * 0.8f, FFTInput, LAYOUT->FFT_PRINT_SIZE);

	// Looking for the median in frequency response
	arm_sort_f32(&FFT_sortInstance, FFTInput, FFTInput_tmp, LAYOUT->FFT_PRINT_SIZE);
	float32_t medianValue = FFTInput_tmp[LAYOUT->FFT_PRINT_SIZE / 2];

	// Maximum amplitude
	float32_t maxValueFFT = maxValueFFT_rx;
	if (TRX_on_TX())
		maxValueFFT = maxValueFFT_tx;
	float32_t maxValue = (medianValue * FFT_MAX);
	float32_t targetValue = (medianValue * FFT_TARGET);
	float32_t minValue = (medianValue * FFT_MIN);

	// Looking for the maximum in frequency response
	float32_t maxAmplValue = 0;
	arm_max_no_idx_f32(FFTInput, LAYOUT->FFT_PRINT_SIZE, &maxAmplValue);

	// Auto-calibrate FFT levels
	maxValueFFT += (targetValue - maxValueFFT) / FFT_STEP_COEFF;
	
	//DEBUG CW DECODER
	/*maxValueFFT = maxValueFFT * 0.999f + maxAmplValue * 0.001f;
	if(maxValueFFT < maxAmplValue)
		maxValueFFT = maxAmplValue;*/

	// minimum-maximum threshold for median
	if (maxValueFFT < minValue)
		maxValueFFT = minValue;
	if (maxValueFFT > maxValue)
		maxValueFFT = maxValue;

	// Compress peaks
	float32_t compressTargetValue = (maxValueFFT * FFT_COMPRESS_INTERVAL);
	float32_t compressSourceInterval = maxAmplValue - compressTargetValue;
	float32_t compressTargetInterval = maxValueFFT - compressTargetValue;
	float32_t compressRate = compressTargetInterval / compressSourceInterval;
	if (!TRX_on_TX() && TRX.FFT_Compressor)
	{
		for (uint_fast16_t i = 0; i < LAYOUT->FFT_PRINT_SIZE; i++)
			if (FFTInput[i] > compressTargetValue)
				FFTInput[i] = compressTargetValue + ((FFTInput[i] - compressTargetValue) * compressRate);
	}

	//limits
	if (TRX_on_TX())
		maxValueFFT = maxAmplValue;
	if (maxValueFFT < 0.0000001f)
		maxValueFFT = 0.0000001f;

	// save values ​​for switching RX / TX
	if (TRX_on_TX())
		maxValueFFT_tx = maxValueFFT;
	else
		maxValueFFT_rx = maxValueFFT;

	// Normalize the frequency response to one
	if (maxValueFFT > 0)
		arm_scale_f32(FFTInput, 1.0f / maxValueFFT, FFTInput, LAYOUT->FFT_PRINT_SIZE);

	// Averaging values ​​for subsequent output
	float32_t averaging = (float32_t)TRX.FFT_Averaging;
	if (averaging < 1.0f)
		averaging = 1.0f;
	for (uint_fast16_t i = 0; i < LAYOUT->FFT_PRINT_SIZE; i++)
		if (FFTOutput_mean[i] < FFTInput[i])
			FFTOutput_mean[i] += (FFTInput[i] - FFTOutput_mean[i]) / averaging;
		else
			FFTOutput_mean[i] -= (FFTOutput_mean[i] - FFTInput[i]) / averaging;

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
	hz_in_pixel = TRX_on_TX() ? FFT_TX_HZ_IN_PIXEL : FFT_HZ_IN_PIXEL;

	if (CurrentVFO()->Freq != currentFFTFreq)
	{
		//calculate scale lines
		memset(grid_lines_pos, 0x00, sizeof(grid_lines_pos));
		uint8_t index = 0;
		for (int8_t i = 0; i < FFT_MAX_GRID_NUMBER; i++)
		{
			int32_t pos = -1;
			if (TRX.FFT_Grid > 0)
			{
				if (fft_zoom == 1)
					pos = getFreqPositionOnFFT((CurrentVFO()->Freq / 10000 * 10000) + ((i - 6) * 10000));
				else
					pos = getFreqPositionOnFFT((CurrentVFO()->Freq / 5000 * 5000) + ((i - 6) * 5000));
			}
			if (pos >= 0)
			{
				grid_lines_pos[index] = pos;
				index++;
			}
		}

		// offset the fft if needed
		FFT_move((int32_t)CurrentVFO()->Freq - (int32_t)currentFFTFreq);
		currentFFTFreq = CurrentVFO()->Freq;
	}

	// move the waterfall down using DMA
	for (tmp = wtfHeight - 1; tmp > 0; tmp--)
	{
		HAL_DMA_Start(&hdma_memtomem_dma2_stream7, (uint32_t)&indexed_wtf_buffer[tmp - 1], (uint32_t)&indexed_wtf_buffer[tmp], LAYOUT->FFT_PRINT_SIZE / 4); //32bit dma, 8bit index data
		HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream7, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
		wtf_buffer_freqs[tmp] = wtf_buffer_freqs[tmp - 1];
	}

	// calculate the colors for the waterfall
	for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++)
	{
		height = (uint16_t)((float32_t)FFTOutput_mean[(uint_fast16_t)fft_x] * fftHeight);
		if (height > fftHeight)
			height = fftHeight;

		wtf_buffer_freqs[0] = currentFFTFreq;
		fft_header[fft_x] = height;
		indexed_wtf_buffer[0][fft_x] = fftHeight - height;
		if (fft_x == (LAYOUT->FFT_PRINT_SIZE / 2))
			continue;
	}

	// calculate bw bar size
	switch (CurrentVFO()->Mode)
	{
	case TRX_MODE_LSB:
	case TRX_MODE_CW_L:
	case TRX_MODE_DIGI_L:
		bw_line_width = (int16_t)(CurrentVFO()->LPF_Filter_Width / hz_in_pixel * fft_zoom);
		if (bw_line_width > (LAYOUT->FFT_PRINT_SIZE / 2))
			bw_line_width = LAYOUT->FFT_PRINT_SIZE / 2;
		bw_line_start = LAYOUT->FFT_PRINT_SIZE / 2 - bw_line_width;
		break;
	case TRX_MODE_USB:
	case TRX_MODE_CW_U:
	case TRX_MODE_DIGI_U:
		bw_line_width = (int16_t)(CurrentVFO()->LPF_Filter_Width / hz_in_pixel * fft_zoom);
		if (bw_line_width > (LAYOUT->FFT_PRINT_SIZE / 2))
			bw_line_width = LAYOUT->FFT_PRINT_SIZE / 2;
		bw_line_start = LAYOUT->FFT_PRINT_SIZE / 2;
		break;
	case TRX_MODE_NFM:
	case TRX_MODE_AM:
		bw_line_width = (int16_t)(CurrentVFO()->LPF_Filter_Width / hz_in_pixel * fft_zoom * 2);
		if (bw_line_width > LAYOUT->FFT_PRINT_SIZE)
			bw_line_width = LAYOUT->FFT_PRINT_SIZE;
		bw_line_start = LAYOUT->FFT_PRINT_SIZE / 2 - (bw_line_width / 2);
		break;
	case TRX_MODE_WFM:
		bw_line_width = LAYOUT->FFT_PRINT_SIZE;
		bw_line_start = LAYOUT->FFT_PRINT_SIZE / 2 - (bw_line_width / 2);
		break;
	default:
		break;
	}
	bw_line_center = bw_line_start + bw_line_width / 2;
	bw_line_end = bw_line_start + bw_line_width;
	if(TRX.FFT_Lens) //lens correction
	{
		bw_line_start = FFT_getLensCorrection(bw_line_start);
		bw_line_center = FFT_getLensCorrection(bw_line_center);
		bw_line_end = FFT_getLensCorrection(bw_line_end);
	}

	if(TRX.FFT_3D > 0)
	{
		FFT_3DPrintFFT();
		return true;
	}
	
	// prepare FFT print over the waterfall
	uint16_t background = BG_COLOR;
	for (uint32_t fft_y = 0; fft_y < fftHeight; fft_y++)
	{
		if (TRX.FFT_Background)
			background = palette_bg_gradient[fft_y];

		if(TRX.FFT_Style == 1) //gradient
		{
			for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++)
			{
				if (fft_x >= bw_line_start && fft_x <= bw_line_end) //bw bar
				{
					if (fft_y >= (fftHeight - fft_header[fft_x]))
						fft_output_buffer[fft_y][fft_x] = palette_bw_fft_colors[fft_y];
					else
						fft_output_buffer[fft_y][fft_x] = palette_bw_bg_colors[fft_y];
				}
				else //other fft data
				{
					if (fft_y >= (fftHeight - fft_header[fft_x]))
						fft_output_buffer[fft_y][fft_x] = palette_fft[fft_y];
					else
						fft_output_buffer[fft_y][fft_x] = background;
				}
			}
		}
		if(TRX.FFT_Style == 2) //fill
		{
			for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++)
			{
				if (fft_x >= bw_line_start && fft_x <= bw_line_end) //bw bar
				{
					if (fft_y >= (fftHeight - fft_header[fft_x]))
						fft_output_buffer[fft_y][fft_x] = palette_bw_fft_colors[fftHeight / 2];
					else
						fft_output_buffer[fft_y][fft_x] = palette_bw_bg_colors[fft_y];
				}
				else //other fft data
				{
					if (fft_y >= (fftHeight - fft_header[fft_x]))
						fft_output_buffer[fft_y][fft_x] = palette_fft[fftHeight / 2];
					else
						fft_output_buffer[fft_y][fft_x] = background;
				}
			}
		}
		if(TRX.FFT_Style == 3) //dots
		{
			for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++)
			{
				if (fft_x >= bw_line_start && fft_x <= bw_line_end) //bw bar
				{
					if (fft_y == (fftHeight - fft_header[fft_x]))
						fft_output_buffer[fft_y][fft_x] = palette_bw_fft_colors[fftHeight / 2];
					else
						fft_output_buffer[fft_y][fft_x] = palette_bw_bg_colors[fft_y];
				}
				else //other fft data
				{
					if (fft_y == (fftHeight - fft_header[fft_x]))
						fft_output_buffer[fft_y][fft_x] = palette_fft[fftHeight / 2];
					else
						fft_output_buffer[fft_y][fft_x] = background;
				}
			}
		}
		if(TRX.FFT_Style == 4) //contour (prepare background)
		{
			for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++)
			{
				if (fft_x >= bw_line_start && fft_x <= bw_line_end) //bw bar
					fft_output_buffer[fft_y][fft_x] = palette_bw_bg_colors[fft_y];
				else //other fft data
					fft_output_buffer[fft_y][fft_x] = background;
			}
		}
	}
	if(TRX.FFT_Style == 4) //contour
	{
		uint32_t fft_y_prev = 0;
		for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++)
		{
			uint32_t fft_y = fftHeight - fft_header[fft_x];
			int32_t y_diff = (int32_t)fft_y - (int32_t)fft_y_prev;
			if(fft_x == 0 || (y_diff <= 1 && y_diff >= -1))
			{
				fft_output_buffer[fft_y][fft_x] = palette_fft[fftHeight / 2];
			}
			else
			{
				for(uint32_t l = 1; l < (abs(y_diff / 2) + 1); l++) //draw line
				{
					fft_output_buffer[fft_y_prev + ((y_diff > 0) ? l : -l)][fft_x - 1] = palette_fft[fftHeight / 2];
					fft_output_buffer[fft_y + ((y_diff > 0) ? -l : l)][fft_x] = palette_fft[fftHeight / 2];
				}
			}
			fft_y_prev = fft_y;
		}
	}

	//draw grids
	if (TRX.FFT_Grid == 1 || TRX.FFT_Grid == 2)
	{
		for (int32_t grid_line_index = 0; grid_line_index < FFT_MAX_GRID_NUMBER; grid_line_index++)
			if (grid_lines_pos[grid_line_index] > 0 && grid_lines_pos[grid_line_index] < LAYOUT->FFT_PRINT_SIZE && grid_lines_pos[grid_line_index] != (LAYOUT->FFT_PRINT_SIZE / 2))
				for (uint32_t fft_y = 0; fft_y < fftHeight; fft_y++)
					fft_output_buffer[fft_y][grid_lines_pos[grid_line_index]] = palette_fft[fftHeight * 3 / 4]; //mixColors(fft_output_buffer[fft_y][grid_lines_pos[grid_line_index]], palette_fft[fftHeight / 2], FFT_SCALE_LINES_BRIGHTNESS);
	}

	//Gauss filter center
	if (TRX.CW_GaussFilter && (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U))
	{
		for (uint32_t fft_y = 0; fft_y < fftHeight; fft_y++)
			fft_output_buffer[fft_y][bw_line_center] = palette_fft[fftHeight / 2]; //mixColors(fft_output_buffer[fft_y][bw_line_center], palette_fft[fftHeight / 2], FFT_SCALE_LINES_BRIGHTNESS);
	}

	//draw center line
	for (uint32_t fft_y = 0; fft_y < fftHeight; fft_y++)
		fft_output_buffer[fft_y][(LAYOUT->FFT_PRINT_SIZE / 2)] = palette_fft[fftHeight / 2]; //mixColors(fft_output_buffer[fft_y][(LAYOUT->FFT_PRINT_SIZE / 2)], palette_fft[fftHeight / 2], FFT_SCALE_LINES_BRIGHTNESS);

	//Print FFT
	LCDDriver_SetCursorAreaPosition(0, LAYOUT->FFT_FFTWTF_POS_Y, LAYOUT->FFT_PRINT_SIZE - 1, (LAYOUT->FFT_FFTWTF_POS_Y + fftHeight));
	print_fft_dma_estimated_size = LAYOUT->FFT_PRINT_SIZE * fftHeight;
	print_fft_dma_position = 0;
	FFT_afterPrintFFT();
	return true;
}

//actions after FFT_printFFT
void FFT_afterPrintFFT(void)
{
	//continue DMA draw?
	if(print_fft_dma_estimated_size > 0)
	{
		if(print_fft_dma_estimated_size <= FFT_DMA_MAX_BLOCK)
		{
			HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream5, (uint32_t)&fft_output_buffer[0] + print_fft_dma_position * 2, LCD_FSMC_DATA_ADDR, print_fft_dma_estimated_size);
			print_fft_dma_estimated_size = 0;
			print_fft_dma_position = 0;
		}
		else
		{
			print_fft_dma_estimated_size -= FFT_DMA_MAX_BLOCK;
			HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream5, (uint32_t)&fft_output_buffer[0] + print_fft_dma_position * 2, LCD_FSMC_DATA_ADDR, FFT_DMA_MAX_BLOCK);
			print_fft_dma_position += FFT_DMA_MAX_BLOCK;
		}
		return;
	}
	
	// clear and display part of the vertical bar
	memset(bandmap_line_tmp, 0x00, sizeof(bandmap_line_tmp));

	// output bandmaps
	int8_t band_curr = getBandFromFreq(CurrentVFO()->Freq, true);
	int8_t band_left = band_curr;
	if (band_curr > 0)
		band_left = band_curr - 1;
	int8_t band_right = band_curr;
	if (band_curr < (BANDS_COUNT - 1))
		band_right = band_curr + 1;
	int32_t fft_freq_position_start = 0;
	int32_t fft_freq_position_stop = 0;
	for (uint16_t band = band_left; band <= band_right; band++)
	{
		//regions
		for (uint16_t region = 0; region < BANDS[band].regionsCount; region++)
		{
			uint16_t region_color = COLOR->BANDMAP_SSB;
			if (BANDS[band].regions[region].mode == TRX_MODE_CW_L || BANDS[band].regions[region].mode == TRX_MODE_CW_U)
				region_color = COLOR->BANDMAP_CW;
			else if (BANDS[band].regions[region].mode == TRX_MODE_DIGI_L || BANDS[band].regions[region].mode == TRX_MODE_DIGI_U)
				region_color = COLOR->BANDMAP_DIGI;
			else if (BANDS[band].regions[region].mode == TRX_MODE_NFM || BANDS[band].regions[region].mode == TRX_MODE_WFM)
				region_color = COLOR->BANDMAP_FM;
			else if (BANDS[band].regions[region].mode == TRX_MODE_AM)
				region_color = COLOR->BANDMAP_AM;

			fft_freq_position_start = getFreqPositionOnFFT(BANDS[band].regions[region].startFreq);
			fft_freq_position_stop = getFreqPositionOnFFT(BANDS[band].regions[region].endFreq);
			if (fft_freq_position_start != -1 && fft_freq_position_stop == -1)
				fft_freq_position_stop = LAYOUT->FFT_PRINT_SIZE;
			if (fft_freq_position_start == -1 && fft_freq_position_stop != -1)
				fft_freq_position_start = 0;
			if (fft_freq_position_start == -1 && fft_freq_position_stop == -1 && BANDS[band].regions[region].startFreq < CurrentVFO()->Freq && BANDS[band].regions[region].endFreq > CurrentVFO()->Freq)
			{
				fft_freq_position_start = 0;
				fft_freq_position_stop = LAYOUT->FFT_PRINT_SIZE;
			}

			if (fft_freq_position_start != -1 && fft_freq_position_stop != -1)
				for (int32_t pixel_counter = fft_freq_position_start; pixel_counter < fft_freq_position_stop; pixel_counter++)
					bandmap_line_tmp[(uint16_t)pixel_counter] = region_color;
		}
	}

	LCDDriver_SetCursorAreaPosition(0, LAYOUT->FFT_FFTWTF_POS_Y - 4, LAYOUT->FFT_PRINT_SIZE - 1, LAYOUT->FFT_FFTWTF_POS_Y - 3);
	for (uint8_t r = 0; r < 2; r++)
		for (uint32_t pixel_counter = 0; pixel_counter < LAYOUT->FFT_PRINT_SIZE; pixel_counter++)
			LCDDriver_SendData(bandmap_line_tmp[pixel_counter]);

	// display the waterfall using DMA
	print_wtf_yindex = 0;
	FFT_printWaterfallDMA();
}

//3D mode print
static void FFT_3DPrintFFT(void)
{
	uint16_t wtfHeight = GET_WTFHeight;
	uint16_t fftHeight = GET_FFTHeight;
	uint_fast8_t cwdecoder_offset = 0;
	if (TRX.CWDecoder && (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U || CurrentVFO()->Mode == TRX_MODE_LOOPBACK))
		cwdecoder_offset = LAYOUT->FFT_CWDECODER_OFFSET;
	
	//clear old data
	memset(indexed_3d_fft_buffer, fftHeight, sizeof(indexed_3d_fft_buffer));
	
	//draw 3D WTF
	for(int32_t wtf_yindex = FFT_3D_SLIDES; wtf_yindex >= 0; wtf_yindex--) //each slides
	{
		//calc perspective parameters
		uint32_t print_y = fftHeight + wtfHeight - cwdecoder_offset - wtf_yindex * FFT_Y_OFFSET;
		float32_t x_compress = (float32_t)(LAYOUT->FFT_PRINT_SIZE - FFT_X_OFFSET * wtf_yindex) / (float32_t)LAYOUT->FFT_PRINT_SIZE;
		uint32_t x_left_offset = (uint32_t)roundf(((float32_t)LAYOUT->FFT_PRINT_SIZE - (float32_t)LAYOUT->FFT_PRINT_SIZE * x_compress) / 2.0f);
		int16_t prev_x = -1;
		
		//each bin
		for (uint32_t wtf_x = 0; wtf_x < LAYOUT->FFT_PRINT_SIZE; wtf_x++)
		{
			//calc bin perspective
			uint32_t print_bin_height = print_y - (fftHeight - indexed_wtf_buffer[wtf_yindex][wtf_x]);
			if(print_bin_height > wtfHeight + fftHeight - cwdecoder_offset)
				continue;
			if(print_bin_height >= FFT_AND_WTF_HEIGHT)
				continue;
			uint32_t print_x = x_left_offset + (uint32_t)roundf((float32_t)wtf_x * x_compress);
			if(prev_x == print_x)
				continue;
			prev_x = print_x;
			
			if(TRX.FFT_3D == 1) //line mode
			{
				for(uint16_t h = 0; h < (fftHeight - indexed_wtf_buffer[wtf_yindex][wtf_x]); h++)
					if((print_bin_height + h) < FFT_AND_WTF_HEIGHT)
						indexed_3d_fft_buffer[print_bin_height + h][print_x] = indexed_wtf_buffer[wtf_yindex][wtf_x] + h;
			}
			if(TRX.FFT_3D == 2) //pixel mode
				indexed_3d_fft_buffer[print_bin_height][print_x] = indexed_wtf_buffer[wtf_yindex][wtf_x];
		}
	}
	
	//draw front fft
	for (uint32_t fft_y = 0; fft_y < fftHeight; fft_y++)
	{
		for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++)
		{
			if (fft_y > (fftHeight - fft_header[fft_x]))
				indexed_3d_fft_buffer[wtfHeight - cwdecoder_offset + fft_y][fft_x] = fft_y;
		}
	}
	
	//do after events
	FFT_afterPrintFFT();
}

// waterfall output
void FFT_printWaterfallDMA(void)
{
	uint16_t fftHeight = GET_FFTHeight;
	uint16_t wtfHeight = GET_WTFHeight;
	uint_fast8_t cwdecoder_offset = 0;
	if (TRX.CWDecoder && (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U || CurrentVFO()->Mode == TRX_MODE_LOOPBACK))
		cwdecoder_offset = LAYOUT->FFT_CWDECODER_OFFSET;
	
	//3D version printout
	if(TRX.FFT_3D > 0)
	{
		if (print_wtf_yindex == 0)
			LCDDriver_SetCursorAreaPosition(0, LAYOUT->FFT_FFTWTF_POS_Y, LAYOUT->FFT_PRINT_SIZE - 1, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + (uint16_t)(wtfHeight - cwdecoder_offset) - 1);
		
		if (print_wtf_yindex < (fftHeight + wtfHeight - cwdecoder_offset))
		{
			for (uint32_t wtf_x = 0; wtf_x < LAYOUT->FFT_PRINT_SIZE; wtf_x++)
				wtf_output_line[wtf_x] = palette_fft[indexed_3d_fft_buffer[print_wtf_yindex][wtf_x]];
			
			//bw bar highlight
			if (print_wtf_yindex > wtfHeight)
			{
				uint16_t fft_y = print_wtf_yindex - wtfHeight + cwdecoder_offset;
				for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++)
				{
					if (fft_x >= bw_line_start && fft_x <= bw_line_end)
					{
						if (fft_y > (fftHeight - fft_header[fft_x]))
							wtf_output_line[fft_x] = palette_bw_fft_colors[indexed_3d_fft_buffer[print_wtf_yindex][fft_x]];
					}
				}
			}
			
			//Gauss filter center
			if (TRX.CW_GaussFilter && (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U))
				wtf_output_line[bw_line_center] = palette_fft[fftHeight * 3 / 4]; //mixColors(wtf_output_line[bw_line_center], palette_fft[fftHeight / 2], FFT_SCALE_LINES_BRIGHTNESS);
			
			//Сenter line
			wtf_output_line[LAYOUT->FFT_PRINT_SIZE / 2] = palette_fft[fftHeight / 2];
				
			//Send To DMA
			HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream6, (uint32_t)&wtf_output_line[0], LCD_FSMC_DATA_ADDR, LAYOUT->FFT_PRINT_SIZE);
			print_wtf_yindex++;
		}
		else
		{
			FFT_FPS++;
			lastWTFFreq = currentFFTFreq;
			FFT_need_fft = true;
			LCD_busy = false;
		}
		return;
	}
	//
	
#ifdef HAS_BTE
	//move exist lines down with BTE
	if (print_wtf_yindex == 0 && lastWTFFreq == currentFFTFreq && !NeedWTFRedraw)
	{
		LCDDriver_BTE_copyArea(0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight, 0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + 1, LAYOUT->FFT_PRINT_SIZE - 1, (uint16_t)(wtfHeight - cwdecoder_offset - 1), true);
		//1 line
		LCDDriver_SetCursorAreaPosition(0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + print_wtf_yindex, LAYOUT->FFT_PRINT_SIZE - 1, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + print_wtf_yindex);
	}
	else if (print_wtf_yindex == 0 && (lastWTFFreq != currentFFTFreq || NeedWTFRedraw))
	{
		//full draw
		LCDDriver_SetCursorAreaPosition(0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight, LAYOUT->FFT_PRINT_SIZE - 1, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + (uint16_t)(wtfHeight - cwdecoder_offset) - 1);
	}
	
#else
	//wtf area
	if (print_wtf_yindex == 0)
		LCDDriver_SetCursorAreaPosition(0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight, LAYOUT->FFT_PRINT_SIZE - 1, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + (uint16_t)(wtfHeight - cwdecoder_offset) - 1);
#endif

//print waterfall line
#ifdef HAS_BTE
	if ((print_wtf_yindex < (wtfHeight - cwdecoder_offset) && (lastWTFFreq != currentFFTFreq || NeedWTFRedraw)) || (print_wtf_yindex == 0 && lastWTFFreq == currentFFTFreq && !NeedWTFRedraw))
#else
	if (print_wtf_yindex < (wtfHeight - cwdecoder_offset))
#endif
	{
		// calculate offset
		float32_t freq_diff = (((float32_t)currentFFTFreq - (float32_t)wtf_buffer_freqs[print_wtf_yindex]) / FFT_HZ_IN_PIXEL) * (float32_t)fft_zoom;
		float32_t freq_diff_part = fmodf(freq_diff, 1.0f);
		int32_t margin_left = 0;
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
		//rounding
		int32_t body_width = LAYOUT->FFT_PRINT_SIZE - margin_left - margin_right;
		
		if (body_width <= 0)
		{
			memset(&wtf_output_line, BG_COLOR, sizeof(wtf_output_line));
		}
		else
		{
			if (margin_left == 0 && margin_right == 0)
			{
				for (uint32_t wtf_x = 0; wtf_x < LAYOUT->FFT_PRINT_SIZE; wtf_x++)
					if (wtf_x >= bw_line_start && wtf_x <= bw_line_end) //print bw bar
						wtf_output_line[wtf_x] = palette_bw_fft_colors[indexed_wtf_buffer[print_wtf_yindex][wtf_x]];
					else
						wtf_output_line[wtf_x] = palette_fft[indexed_wtf_buffer[print_wtf_yindex][wtf_x]];
			}
			else if (margin_left > 0)
			{
				memset(&wtf_output_line, BG_COLOR, (uint32_t)(margin_left * 2)); // fill the space to the left
				for (uint32_t wtf_x = 0; wtf_x < (LAYOUT->FFT_PRINT_SIZE - margin_left); wtf_x++)
					if ((margin_left + wtf_x) >= bw_line_start && (margin_left + wtf_x) <= bw_line_end) //print bw bar
						wtf_output_line[margin_left + wtf_x] = palette_bw_fft_colors[indexed_wtf_buffer[print_wtf_yindex][wtf_x]];
					else
						wtf_output_line[margin_left + wtf_x] = palette_fft[indexed_wtf_buffer[print_wtf_yindex][wtf_x]];
			}
			if (margin_right > 0)
			{
				memset(&wtf_output_line[(LAYOUT->FFT_PRINT_SIZE - margin_right)], BG_COLOR, (uint32_t)(margin_right * 2)); // fill the space to the right
				for (uint32_t wtf_x = 0; wtf_x < (LAYOUT->FFT_PRINT_SIZE - margin_right); wtf_x++)
					if (wtf_x >= bw_line_start && wtf_x <= bw_line_end) //print bw bar
						wtf_output_line[wtf_x] = palette_bw_fft_colors[indexed_wtf_buffer[print_wtf_yindex][wtf_x + margin_right]];
					else
						wtf_output_line[wtf_x] = palette_fft[indexed_wtf_buffer[print_wtf_yindex][wtf_x + margin_right]];
			}
		}

		//print scale lines
		if (TRX.FFT_Grid >= 2)
			for (int8_t i = 0; i < FFT_MAX_GRID_NUMBER; i++)
				if (grid_lines_pos[i] > 0)
					wtf_output_line[grid_lines_pos[i]] = palette_fft[fftHeight * 3 / 4]; //mixColors(wtf_output_line[grid_lines_pos[i]], palette_fft[fftHeight / 2], FFT_SCALE_LINES_BRIGHTNESS);

		//Gauss filter center
		if (TRX.CW_GaussFilter && (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U))
			wtf_output_line[bw_line_center] = palette_fft[fftHeight / 2]; //mixColors(fft_output_buffer[fft_y][bw_line_center], palette_fft[fftHeight / 2], FFT_SCALE_LINES_BRIGHTNESS);
				
		//center line
		wtf_output_line[LAYOUT->FFT_PRINT_SIZE / 2] = palette_fft[fftHeight / 2]; //mixColors(wtf_output_line[LAYOUT->FFT_PRINT_SIZE / 2], palette_fft[fftHeight / 2], FFT_SCALE_LINES_BRIGHTNESS);

		//draw the line
		HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream6, (uint32_t)&wtf_output_line[0], LCD_FSMC_DATA_ADDR, LAYOUT->FFT_PRINT_SIZE);
		print_wtf_yindex++;
	}
	else
	{
		FFT_FPS++;
		lastWTFFreq = currentFFTFreq;
		if(NeedWTFRedraw) //redraw cycles counter
		{
			if(needredraw_wtf_counter == 0)
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
}

// shift the waterfall
static void FFT_move(int32_t _freq_diff)
{
	if (_freq_diff == 0)
		return;
	float32_t old_x_true = 0.0f;
	int32_t old_x_l = 0;
	int32_t old_x_r = 0;
	float32_t freq_diff = ((float32_t)_freq_diff / FFT_HZ_IN_PIXEL) * (float32_t)fft_zoom;
	float32_t old_x_part_r = fmodf(freq_diff, 1.0f);
	float32_t old_x_part_l = 1.0f - old_x_part_r;
	if (freq_diff < 0.0f)
	{
		old_x_part_l = fmodf(-freq_diff, 1.0f);
		old_x_part_r = 1.0f - old_x_part_l;
	}

	//Move mean Buffer
	for (int32_t x = 0; x < LAYOUT->FFT_PRINT_SIZE; x++)
	{
		old_x_true = (float32_t)x + freq_diff;
		old_x_l = (int32_t)(floorf(old_x_true));
		old_x_r = (int32_t)(ceilf(old_x_true));

		FFTInput_tmp[x] = 0;

		if ((old_x_true >= LAYOUT->FFT_PRINT_SIZE) || (old_x_true < 0.0f))
			continue;
		if ((old_x_l < LAYOUT->FFT_PRINT_SIZE) && (old_x_l >= 0))
			FFTInput_tmp[x] += (FFTOutput_mean[old_x_l] * old_x_part_l);
		if ((old_x_r < LAYOUT->FFT_PRINT_SIZE) && (old_x_r >= 0))
			FFTInput_tmp[x] += (FFTOutput_mean[old_x_r] * old_x_part_r);

		//sides
		if (old_x_r >= LAYOUT->FFT_PRINT_SIZE)
			FFTInput_tmp[x] = FFTOutput_mean[old_x_l];
	}
	//save results
	memcpy(&FFTOutput_mean, &FFTInput_tmp, sizeof FFTOutput_mean);
	
	//clean charge buffer
	if(fft_zoom > 1)
		memset(&FFTInputCharge[0], 0x00, sizeof(float32_t) * (FFT_SIZE - zoomed_width) * 2);
}

// get color from signal strength
static uint16_t getFFTColor(uint_fast8_t height) // Get FFT color warmth (blue to red)
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
	//blue -> yellow -> red
	if (TRX.FFT_Color == 1)
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
	//black -> yellow -> red
	if (TRX.FFT_Color == 2)
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
	//black -> yellow -> green
	if (TRX.FFT_Color == 3)
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
	//black -> red
	if (TRX.FFT_Color == 4)
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
	//black -> green
	if (TRX.FFT_Color == 5)
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
	//black -> blue
	if (TRX.FFT_Color == 6)
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
	//black -> white
	if (TRX.FFT_Color == 7)
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
	//unknown
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
	for (uint_fast8_t i = 0; i <= GET_FFTHeight; i++)
	{
		palette_fft[i] = getFFTColor(GET_FFTHeight - i);
		palette_bg_gradient[i] = getBGColor(GET_FFTHeight - i);
		palette_bw_fft_colors[i] = addColor(palette_fft[i], FFT_BW_BRIGHTNESS, FFT_BW_BRIGHTNESS, FFT_BW_BRIGHTNESS);
		palette_bw_bg_colors[i] = addColor(palette_bg_gradient[i], FFT_BW_BRIGHTNESS, FFT_BW_BRIGHTNESS, FFT_BW_BRIGHTNESS);
	}
}

// reset FFT
void FFT_Reset(void) // clear the FFT
{
	FFT_new_buffer_ready = false;
	memset(FFTInput_I_A, 0x00, sizeof FFTInput_I_A);
	memset(FFTInput_Q_A, 0x00, sizeof FFTInput_Q_A);
	memset(FFTInput_I_B, 0x00, sizeof FFTInput_I_B);
	memset(FFTInput_Q_B, 0x00, sizeof FFTInput_Q_B);
	memset(FFTInputCharge, 0x00, sizeof FFTInputCharge);
	memset(FFTInput, 0x00, sizeof FFTInput);
	memset(FFTOutput_mean, 0x00, sizeof FFTOutput_mean);
	FFT_buff_index = 0;
}

static inline int32_t getFreqPositionOnFFT(uint32_t freq)
{
	int32_t pos = (int32_t)((float32_t)LAYOUT->FFT_PRINT_SIZE / 2 + (float32_t)((float32_t)freq - (float32_t)CurrentVFO()->Freq) / hz_in_pixel * (float32_t)fft_zoom);
	if (pos < 0 || pos >= LAYOUT->FFT_PRINT_SIZE)
		return -1;
	if(TRX.FFT_Lens) //lens correction
		pos = FFT_getLensCorrection(pos);
	return pos;
}

uint32_t getFreqOnFFTPosition(uint16_t position)
{
	return (uint32_t)((int32_t)CurrentVFO()->Freq + (int32_t)(-((float32_t)LAYOUT->FFT_PRINT_SIZE * (hz_in_pixel  / (float32_t)fft_zoom) / 2.0f) + (float32_t)position * (hz_in_pixel / (float32_t)fft_zoom)));
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
		if(normal_distance_from_center < (LAYOUT->FFT_PRINT_SIZE / 2))
		{
			if(normal_distance_from_center_converted > index1)
			{
				return (LAYOUT->FFT_PRINT_SIZE / 2 - i);
			}
		}
		else
		{
			if(normal_distance_from_center_converted < index2)
			{
				if(i != (LAYOUT->FFT_PRINT_SIZE / 2))
					return (LAYOUT->FFT_PRINT_SIZE / 2 + i);
				else
					return (LAYOUT->FFT_PRINT_SIZE - 1);
			}
		}
		
		step_now += FFT_LENS_STEP;
		index1 -= step_now;
		index2 += step_now;

		if(index1 >= FFT_USEFUL_SIZE)
			index1 = FFT_USEFUL_SIZE - 1;
		if(index1 < 0)
			index1 = 0;
		if(index2 >= FFT_USEFUL_SIZE)
			index2 = FFT_USEFUL_SIZE - 1;
		if(index2 < 0)
			index2 = 0;
	}
	return normal_distance_from_center;
}
