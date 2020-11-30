#include "fft.h"
#include "main.h"
#include "arm_const_structs.h"
#include "audio_filters.h"
#include "screen_layout.h"
#include "vad.h"
#include "lcd.h"

//Public variables
bool NeedFFTInputBuffer = true;				// flag of the need to fill the buffer with FPGA
bool FFT_need_fft = true;					// need to prepare data for display on the screen
bool FFT_buffer_ready = false;				// buffer is full, can be processed
volatile uint32_t FFT_buff_index = 0;		// current buffer index when it is filled with FPGA
IRAM2 float32_t FFTInput_I[FFT_SIZE] = {0}; // incoming buffer FFT I
IRAM2 float32_t FFTInput_Q[FFT_SIZE] = {0}; // incoming buffer FFT Q
uint16_t FFT_FPS = 0;

//Private variables
#if FFT_SIZE == 1024
const static arm_cfft_instance_f32 *FFT_Inst = &arm_cfft_sR_f32_len1024;
#endif
#if FFT_SIZE == 512
const static arm_cfft_instance_f32 *FFT_Inst = &arm_cfft_sR_f32_len512;
#endif
#if FFT_SIZE == 256
const static arm_cfft_instance_f32 *FFT_Inst = &arm_cfft_sR_f32_len256;
#endif
#if FFT_SIZE == 128
const static arm_cfft_instance_f32 *FFT_Inst = &arm_cfft_sR_f32_len128;
#endif

IRAM2 static float32_t FFTInput[FFT_DOUBLE_SIZE_BUFFER] = {0};   // combined FFT I and Q buffer
static float32_t FFTInput_sorted[FFT_SIZE] = {0};		   // buffer for sorted values ​​(when looking for a median)
static float32_t FFTOutput_mean[MAX_FFT_PRINT_SIZE] = {0}; // averaged FFT buffer (for output)
static float32_t FFTOutput_mean_new[MAX_FFT_PRINT_SIZE] = {0}; // averaged FFT buffer (for moving)
static float32_t maxValueFFT_rx = 0;					   // maximum value of the amplitude in the resulting frequency response
static float32_t maxValueFFT_tx = 0;					   // maximum value of the amplitude in the resulting frequency response
static uint32_t currentFFTFreq = 0;
static uint32_t lastWTFFreq = 0;								//last WTF printed freq
static uint16_t color_scale[MAX_FFT_HEIGHT] = {0};							  // color gradient in height FFT
static uint16_t bg_gradient_color[MAX_FFT_HEIGHT] = {0};							  // color gradient on background of FFT
SRAM static uint16_t fft_and_wtf_buffer[MAX_FFT_PLUS_WTF_HEIGHT][MAX_FFT_PRINT_SIZE] = {{0}};	//union buffer with fft and wtf
IRAM2 static uint32_t wtf_buffer_freqs[MAX_WTF_HEIGHT] = {0};				  // frequencies for each row of the waterfall
SRAM static uint16_t wtf_line_tmp[MAX_FFT_PRINT_SIZE] = {0};						  // temporary buffer to move the waterfall
static uint16_t fft_header[MAX_FFT_PRINT_SIZE] = {0};				//buffer with fft colors output
static int32_t grid_lines_pos[20] = {-1};										//grid lines positions
static int16_t bw_line_start = 0;															//BW bar params
static int16_t bw_line_width = 0;															//BW bar params
static uint16_t print_wtf_yindex = 0;												  // the current coordinate of the waterfall output via DMA
static float32_t window_multipliers[FFT_SIZE] = {0};								  // coefficients of the selected window function
static float32_t hz_in_pixel = 1.0f;												  // current FFT density value
static uint16_t bandmap_line_tmp[MAX_FFT_PRINT_SIZE] = {0};					  // temporary buffer to move the waterfall
static arm_sort_instance_f32 FFT_sortInstance = {0};								  // sorting instance (to find the median)
// Decimator for Zoom FFT
static arm_fir_decimate_instance_f32 DECIMATE_ZOOM_FFT_I;
static arm_fir_decimate_instance_f32 DECIMATE_ZOOM_FFT_Q;
static float32_t decimZoomFFTIState[FFT_SIZE + 4 - 1];
static float32_t decimZoomFFTQState[FFT_SIZE + 4 - 1];
static uint_fast16_t zoomed_width = 0;
//Коэффициенты для ZoomFFT lowpass filtering / дециматора
static arm_biquad_casd_df1_inst_f32 IIR_biquad_Zoom_FFT_I =
	{
		.numStages = 4,
		.pCoeffs = (float32_t *)(float32_t[]){
			1, 0, 0, 0, 0, 1, 0, 0, 0, 0},
		.pState = (float32_t *)(float32_t[]){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
static arm_biquad_casd_df1_inst_f32 IIR_biquad_Zoom_FFT_Q =
	{
		.numStages = 4,
		.pCoeffs = (float32_t *)(float32_t[]){
			1, 0, 0, 0, 0, 1, 0, 0, 0, 0},
		.pState = (float32_t *)(float32_t[]){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

static const float32_t *mag_coeffs[17] =
	{
		NULL, // 0
		NULL, // 1
		(float32_t *)(const float32_t[]){
			// 2x magnify
			// 12kHz, sample rate 48k, 60dB stopband, elliptic
			// a1 and coeffs[A2] negated! order: coeffs[B0], coeffs[B1], coeffs[B2], a1, coeffs[A2]
			// Iowa Hills IIR Filter Designer
			0.228454526413293696f, 0.077639329099949764f, 0.228454526413293696f, 0.635534925142242080f, -0.170083307068779194f, 0.436788292542003964f, 0.232307972937606161f, 0.436788292542003964f, 0.365885230717786780f, -0.471769788739400842f, 0.535974654742658707f, 0.557035600464780845f, 0.535974654742658707f, 0.125740787233286133f, -0.754725697183384336f, 0.501116342273565607f, 0.914877831284765408f, 0.501116342273565607f, 0.013862536615004284f, -0.930973052446900984f},
		NULL, // 3
		(float32_t *)(const float32_t[]){
			// 4x magnify
			// 6kHz, sample rate 48k, 60dB stopband, elliptic
			// a1 and coeffs[A2] negated! order: coeffs[B0], coeffs[B1], coeffs[B2], a1, coeffs[A2]
			// Iowa Hills IIR Filter Designer
			0.182208761527446556f, -0.222492493114674145f, 0.182208761527446556f, 1.326111070880959810f, -0.468036100821178802f, 0.337123762652097259f, -0.366352718812586853f, 0.337123762652097259f, 1.337053579516321200f, -0.644948386007929031f, 0.336163175380826074f, -0.199246162162897811f, 0.336163175380826074f, 1.354952684569386670f, -0.828032873168141115f, 0.178588201750411041f, 0.207271695028067304f, 0.178588201750411041f, 1.386486967455699220f, -0.950935065984588657f},
		NULL, // 5
		NULL, // 6
		NULL, // 7
		(float32_t *)(const float32_t[]){
			// 8x magnify
			// 3kHz, sample rate 48k, 60dB stopband, elliptic
			// a1 and coeffs[A2] negated! order: coeffs[B0], coeffs[B1], coeffs[B2], a1, coeffs[A2]
			// Iowa Hills IIR Filter Designer
			0.185643392652478922f, -0.332064345389014803f, 0.185643392652478922f, 1.654637402827731090f, -0.693859842743674182f, 0.327519300813245984f, -0.571358085216950418f, 0.327519300813245984f, 1.715375037176782860f, -0.799055553586324407f, 0.283656142708241688f, -0.441088976843048652f, 0.283656142708241688f, 1.778230635987093860f, -0.904453944560528522f, 0.079685368654848945f, -0.011231810140649204f, 0.079685368654848945f, 1.825046003243238070f, -0.973184930412286708f},
		NULL, // 9
		NULL, // 10
		NULL, // 11
		NULL, // 12
		NULL, // 13
		NULL, // 14
		NULL, // 15
		(float32_t *)(const float32_t[]){
			// 16x magnify
			// 1k5, sample rate 48k, 60dB stopband, elliptic
			// a1 and coeffs[A2] negated! order: coeffs[B0], coeffs[B1], coeffs[B2], a1, coeffs[A2]
			// Iowa Hills IIR Filter Designer
			0.194769868656866380f, -0.379098413160710079f, 0.194769868656866380f, 1.824436402073870810f, -0.834877726226893380f, 0.333973874901496770f, -0.646106479315673776f, 0.333973874901496770f, 1.871892825636887640f, -0.893734096124207178f, 0.272903880596429671f, -0.513507745397738469f, 0.272903880596429671f, 1.918161772571113750f, -0.950461788366234739f, 0.053535383722369843f, -0.069683422367188122f, 0.053535383722369843f, 1.948900719896301760f, -0.986288064973853129f},
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
static uint16_t getFFTColor(uint_fast8_t height);	// get color from signal strength
static void fft_fill_color_scale(void);				// prepare the color palette
static void FFT_move(int32_t _freq_diff);			// shift the waterfall
static int32_t getFreqPositionOnFFT(uint32_t freq); // get the position on the FFT for a given frequency
static inline uint16_t addColor(uint16_t color, uint8_t add_r, uint8_t add_g, uint8_t add_b); //add opacity or mix colors
static inline uint16_t mixColors(uint16_t color1, uint16_t color2, float32_t opacity); //mix two colors with opacity
	
// FFT initialization
void FFT_Init(void)
{
	fft_fill_color_scale();
	//ZoomFFT
	if (TRX.FFT_Zoom > 1)
	{
		IIR_biquad_Zoom_FFT_I.pCoeffs = mag_coeffs[TRX.FFT_Zoom];
		IIR_biquad_Zoom_FFT_Q.pCoeffs = mag_coeffs[TRX.FFT_Zoom];
		memset(IIR_biquad_Zoom_FFT_I.pState, 0x00, 16 * 4);
		memset(IIR_biquad_Zoom_FFT_Q.pState, 0x00, 16 * 4);
		arm_fir_decimate_init_f32(&DECIMATE_ZOOM_FFT_I,
								  FirZoomFFTDecimate[TRX.FFT_Zoom].numTaps,
								  TRX.FFT_Zoom, // Decimation factor
								  FirZoomFFTDecimate[TRX.FFT_Zoom].pCoeffs,
								  decimZoomFFTIState, // Filter state variables
								  FFT_SIZE);

		arm_fir_decimate_init_f32(&DECIMATE_ZOOM_FFT_Q,
								  FirZoomFFTDecimate[TRX.FFT_Zoom].numTaps,
								  TRX.FFT_Zoom, // Decimation factor
								  FirZoomFFTDecimate[TRX.FFT_Zoom].pCoeffs,
								  decimZoomFFTQState, // Filter state variables
								  FFT_SIZE);
		zoomed_width = FFT_SIZE / TRX.FFT_Zoom;
	}
	//windowing
	for (uint_fast16_t i = 0; i < FFT_SIZE; i++)
	{
		//Hamming
		if (TRX.FFT_Window == 1)
			window_multipliers[i] = 0.54f - 0.46f * arm_cos_f32((2.0f * PI * i) / ((float32_t)FFT_SIZE - 1.0f));
		//Blackman-Harris
		else if (TRX.FFT_Window == 2)
			window_multipliers[i] = 0.35875f - 0.48829f * arm_cos_f32(2.0f * PI * i / ((float32_t)FFT_SIZE - 1.0f)) + 0.14128f * arm_cos_f32(4.0f * PI * i / ((float32_t)FFT_SIZE - 1.0f)) - 0.01168f * arm_cos_f32(6.0f * PI * i / ((float32_t)FFT_SIZE - 1.0f));
		//Hanning
		else if (TRX.FFT_Window == 3)
			window_multipliers[i] = 0.5f * (1.0f - arm_cos_f32(2.0f * PI * i / (float32_t)FFT_SIZE));
	}
	// clear the buffer
	memset(&fft_and_wtf_buffer, BG_COLOR, sizeof(fft_and_wtf_buffer));
	// initialize sort
	arm_sort_init_f32(&FFT_sortInstance, ARM_SORT_QUICK, ARM_SORT_ASCENDING);
}

// FFT calculation
ITCM void FFT_doFFT(void)
{
	if (!TRX.FFT_Enabled)
		return;
	if (!FFT_need_fft)
		return;
	if (NeedFFTInputBuffer)
		return;
	if (!FFT_buffer_ready)
		return;
	/*if (CPU_LOAD.Load > 90)
		return;*/

	float32_t medianValue = 0; // Median value in the resulting frequency response

	//Process DC corrector filter
	if (!TRX_on_TX())
	{
		dc_filter(FFTInput_I, FFT_SIZE, DC_FILTER_FFT_I);
		dc_filter(FFTInput_Q, FFT_SIZE, DC_FILTER_FFT_Q);
	}

	//Process Notch filter
	if (CurrentVFO()->ManualNotchFilter && !TRX_on_TX())
	{
		arm_biquad_cascade_df2T_f32(&NOTCH_FFT_I_FILTER, FFTInput_I, FFTInput_I, FFT_SIZE);
		arm_biquad_cascade_df2T_f32(&NOTCH_FFT_Q_FILTER, FFTInput_Q, FFTInput_Q, FFT_SIZE);
	}

	//ZoomFFT
	if (TRX.FFT_Zoom > 1)
	{
		//Biquad LPF фильтр
		arm_biquad_cascade_df1_f32(&IIR_biquad_Zoom_FFT_I, FFTInput_I, FFTInput_I, FFT_SIZE);
		arm_biquad_cascade_df1_f32(&IIR_biquad_Zoom_FFT_Q, FFTInput_Q, FFTInput_Q, FFT_SIZE);
		// Decimator
		arm_fir_decimate_f32(&DECIMATE_ZOOM_FFT_I, FFTInput_I, FFTInput_I, FFT_SIZE);
		arm_fir_decimate_f32(&DECIMATE_ZOOM_FFT_Q, FFTInput_Q, FFTInput_Q, FFT_SIZE);
		// Fill the unnecessary part of the buffer with zeros
		for (uint_fast16_t i = 0; i < FFT_SIZE; i++)
		{
			if (i < zoomed_width)
			{
				FFTInput[i * 2] = FFTInput_I[i];
				FFTInput[i * 2 + 1] = FFTInput_Q[i];
			}
			else
			{
				FFTInput[i * 2] = 0.0f;
				FFTInput[i * 2 + 1] = 0.0f;
			}
		}
	}
	else
	{
		// make a combined buffer for calculation
		for (uint_fast16_t i = 0; i < FFT_SIZE; i++)
		{
			FFTInput[i * 2] = FFTInput_I[i];
			FFTInput[i * 2 + 1] = FFTInput_Q[i];
		}
	}
	NeedFFTInputBuffer = true;
	
	// Window for FFT
	for (uint_fast16_t i = 0; i < FFT_SIZE; i++)
	{
		FFTInput[i * 2] = window_multipliers[i] * FFTInput[i * 2];
		FFTInput[i * 2 + 1] = window_multipliers[i] * FFTInput[i * 2 + 1];
	}

	arm_cfft_f32(FFT_Inst, FFTInput, 0, 1);
	arm_cmplx_mag_f32(FFTInput, FFTInput, FFT_SIZE);

	//Debug VAD
	/*memset(FFTInput, 0x00, sizeof(FFTInput));
	for (uint_fast16_t i = 0; i < FFT_SIZE; i++)
	{
		FFTInput[i] = VAD_FFTBuffer_Export[i];
	}
	NeedFFTInputBuffer = true;*/
	
	// Reduce the calculated FFT to visible
	if (FFT_SIZE > LAYOUT->FFT_PRINT_SIZE)
	{
		float32_t fft_compress_rate = (float32_t)FFT_SIZE / (float32_t)LAYOUT->FFT_PRINT_SIZE;
		for (uint_fast16_t i = 0; i < LAYOUT->FFT_PRINT_SIZE; i++)
		{
			float32_t fft_compress_tmp = 0;
			for (uint_fast8_t c = 0; c < fft_compress_rate; c++)
				fft_compress_tmp += FFTInput[(uint_fast16_t)(i * fft_compress_rate + c)];
			FFTInput[i] = fft_compress_tmp / fft_compress_rate;
		}
	}

	//Delete noise
	float32_t minAmplValue = 0;
	uint32_t minAmplIndex = 0;
	arm_min_f32(FFTInput, LAYOUT->FFT_PRINT_SIZE, &minAmplValue, &minAmplIndex);
	if (!TRX_on_TX())
		arm_offset_f32(FFTInput, -minAmplValue * 0.8f, FFTInput, LAYOUT->FFT_PRINT_SIZE);
	
	// Looking for the median in frequency response
	arm_sort_f32(&FFT_sortInstance, FFTInput, FFTInput_sorted, LAYOUT->FFT_PRINT_SIZE);
	medianValue = FFTInput_sorted[LAYOUT->FFT_PRINT_SIZE / 2];
	
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
			if(FFTInput[i] > compressTargetValue)
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
	if(maxValueFFT > 0)
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
ITCM void FFT_printFFT(void)
{
	if (LCD_busy)
		return;
	if (!TRX.FFT_Enabled)
		return;
	if (FFT_need_fft)
		return;
	if (LCD_systemMenuOpened)
		return;
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
		uint8_t index = 0;
		for(int8_t i = 0; i < FFT_MAX_GRID_NUMBER; i++)
		{
			int32_t pos = -1;
			if(TRX.FFT_Grid > 0)
			{
				if(TRX.FFT_Zoom == 1)
					pos = getFreqPositionOnFFT((CurrentVFO()->Freq / 10000 * 10000) + ((i - 6) * 10000));
				else
					pos = getFreqPositionOnFFT((CurrentVFO()->Freq / 5000 * 5000) + ((i - 6) * 5000));
			}
			if(pos >= 0)
			{
				grid_lines_pos[index] = pos;
				index++;
			}
		}
		for(int8_t i = index; i < 13; i++)
			grid_lines_pos[index] = -1;
		
		// offset the waterfall if needed
		FFT_move((int32_t)CurrentVFO()->Freq - (int32_t)currentFFTFreq);
		currentFFTFreq = CurrentVFO()->Freq;
	}

	// move the waterfall down using DMA
	for (tmp = wtfHeight - 1; tmp > 0; tmp--)
	{
		HAL_DMA_Start(&hdma_memtomem_dma2_stream7, (uint32_t)&fft_and_wtf_buffer[fftHeight + tmp - 1], (uint32_t)&fft_and_wtf_buffer[fftHeight + tmp], LAYOUT->FFT_PRINT_SIZE / 2);
		HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream7, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
		wtf_buffer_freqs[tmp] = wtf_buffer_freqs[tmp - 1];
	}

	// calculate the colors for the waterfall
	uint_fast16_t new_x = 0;
	for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++)
	{
		if (fft_x < (LAYOUT->FFT_PRINT_SIZE / 2))
			new_x = fft_x + (LAYOUT->FFT_PRINT_SIZE / 2);
		if (fft_x >= (LAYOUT->FFT_PRINT_SIZE / 2))
			new_x = fft_x - (LAYOUT->FFT_PRINT_SIZE / 2);
		height = (uint16_t)((float32_t)FFTOutput_mean[(uint_fast16_t)fft_x] * fftHeight);
		if (height > fftHeight - 1)
		{
			height = fftHeight;
			tmp = color_scale[0];
		}
		else
			tmp = color_scale[fftHeight - height];
		fft_and_wtf_buffer[fftHeight][new_x] = tmp;
		wtf_buffer_freqs[0] = currentFFTFreq;
		fft_header[new_x] = height;
		if (new_x == (LAYOUT->FFT_PRINT_SIZE / 2))
			continue;
	}

	// calculate bw bar size
	switch (CurrentVFO()->Mode)
	{
	case TRX_MODE_LSB:
	case TRX_MODE_CW_L:
	case TRX_MODE_DIGI_L:
		bw_line_width = (int16_t)(CurrentVFO()->LPF_Filter_Width / hz_in_pixel * TRX.FFT_Zoom);
		if (bw_line_width > (LAYOUT->FFT_PRINT_SIZE / 2))
			bw_line_width = LAYOUT->FFT_PRINT_SIZE / 2;
		bw_line_start = LAYOUT->FFT_PRINT_SIZE / 2 - bw_line_width;
		break;
	case TRX_MODE_USB:
	case TRX_MODE_CW_U:
	case TRX_MODE_DIGI_U:
		bw_line_width = (int16_t)(CurrentVFO()->LPF_Filter_Width / hz_in_pixel * TRX.FFT_Zoom);
		if (bw_line_width > (LAYOUT->FFT_PRINT_SIZE / 2))
			bw_line_width = LAYOUT->FFT_PRINT_SIZE / 2;
		bw_line_start = LAYOUT->FFT_PRINT_SIZE / 2;
		break;
	case TRX_MODE_NFM:
	case TRX_MODE_AM:
		bw_line_width = (int16_t)(CurrentVFO()->LPF_Filter_Width / hz_in_pixel * TRX.FFT_Zoom * 2);
		if (bw_line_width > LAYOUT->FFT_PRINT_SIZE)
			bw_line_width = LAYOUT->FFT_PRINT_SIZE;
		bw_line_start = LAYOUT->FFT_PRINT_SIZE / 2 - (bw_line_width / 2);
		break;
	default:
		break;
	}
	
	// prepare FFT print over the waterfall
	uint16_t background = BG_COLOR;
	for (uint32_t fft_y = 0; fft_y < fftHeight; fft_y++)
	{
		if(TRX.FFT_Background)
				background = bg_gradient_color[fft_y];
			
		uint8_t grid_line_index = 0;
		for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++)
		{
			//fft data
			if (fft_y > (fftHeight - fft_header[fft_x]))
				fft_and_wtf_buffer[fft_y][fft_x] = color_scale[fft_y];
			else
				fft_and_wtf_buffer[fft_y][fft_x] = background;
		}
	}
	
	//draw grids
	if(TRX.FFT_Grid == 1 || TRX.FFT_Grid == 2)
	{
		for(int32_t grid_line_index = 0; grid_line_index < FFT_MAX_GRID_NUMBER ; grid_line_index++)
			if(grid_lines_pos[grid_line_index] > 0 && grid_lines_pos[grid_line_index] < LAYOUT->FFT_PRINT_SIZE && grid_lines_pos[grid_line_index] != (LAYOUT->FFT_PRINT_SIZE / 2))
				for (uint32_t fft_y = 0; fft_y < fftHeight; fft_y++)
					fft_and_wtf_buffer[fft_y][grid_lines_pos[grid_line_index]] = mixColors(fft_and_wtf_buffer[fft_y][grid_lines_pos[grid_line_index]], color_scale[fftHeight / 2], FFT_SCALE_LINES_BRIGHTNESS);
	}
	
	//draw center line
	for (uint32_t fft_y = 0; fft_y < fftHeight; fft_y++)
		fft_and_wtf_buffer[fft_y][(LAYOUT->FFT_PRINT_SIZE / 2)] = mixColors(fft_and_wtf_buffer[fft_y][(LAYOUT->FFT_PRINT_SIZE / 2)], color_scale[fftHeight / 2], FFT_SCALE_LINES_BRIGHTNESS);
	
	//add opacity to bandw bar
	for (uint32_t fft_y = 0; fft_y < fftHeight; fft_y++)
		for(int32_t fft_x = bw_line_start; fft_x <= (bw_line_start + bw_line_width) ; fft_x++)
			fft_and_wtf_buffer[fft_y][fft_x] = addColor(fft_and_wtf_buffer[fft_y][fft_x], FFT_BW_BRIGHTNESS, FFT_BW_BRIGHTNESS, FFT_BW_BRIGHTNESS);
	
	//Print FFT
	LCDDriver_SetCursorAreaPosition(0, LAYOUT->FFT_FFTWTF_POS_Y, LAYOUT->FFT_PRINT_SIZE - 1, (LAYOUT->FFT_FFTWTF_POS_Y + fftHeight));
	HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream5, (uint32_t)&fft_and_wtf_buffer[0], LCD_FSMC_DATA_ADDR, LAYOUT->FFT_PRINT_SIZE * fftHeight);
}

//actions after FFT_printFFT
ITCM void FFT_afterPrintFFT(void)
{
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
	for(uint8_t r =0; r < 2; r++)
		for (uint32_t pixel_counter = 0; pixel_counter < LAYOUT->FFT_PRINT_SIZE; pixel_counter++)
			LCDDriver_SendData(bandmap_line_tmp[pixel_counter]);

	// display the waterfall using DMA
	print_wtf_yindex = 0;
	FFT_printWaterfallDMA();
}

// waterfall output
ITCM void FFT_printWaterfallDMA(void)
{
	uint16_t fftHeight = GET_FFTHeight;
	uint16_t wtfHeight = GET_WTFHeight;
	uint_fast8_t cwdecoder_offset = 0;
	if (TRX.CWDecoder && (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U || CurrentVFO()->Mode == TRX_MODE_LOOPBACK))
		cwdecoder_offset = LAYOUT->FFT_CWDECODER_OFFSET;

	#ifdef HAS_BTE
		//move exist lines down with BTE
		if (print_wtf_yindex == 0 && lastWTFFreq == currentFFTFreq)
			LCDDriver_BTE_copyArea(0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight, 0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + 1, LAYOUT->FFT_PRINT_SIZE - 1, (uint16_t)(wtfHeight - cwdecoder_offset - 1), true);
	#else
		//wtf area
		if(print_wtf_yindex == 0)
			LCDDriver_SetCursorAreaPosition(0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight, LAYOUT->FFT_PRINT_SIZE - 1, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + (uint16_t)(wtfHeight - cwdecoder_offset) - 1);
	#endif
	
	//print waterfall line
	#ifdef HAS_BTE
		if ((print_wtf_yindex < (wtfHeight - cwdecoder_offset) && lastWTFFreq != currentFFTFreq) ||  (print_wtf_yindex == 0 && lastWTFFreq == currentFFTFreq))
	#else
		if (print_wtf_yindex < (wtfHeight - cwdecoder_offset))
	#endif
	{
		// calculate offset
		int32_t freq_diff = (int32_t)(((float32_t)((int32_t)currentFFTFreq - (int32_t)wtf_buffer_freqs[print_wtf_yindex]) / FFT_HZ_IN_PIXEL) * (float32_t)TRX.FFT_Zoom);
		int32_t margin_left = 0;
		if (freq_diff < 0)
			margin_left = -freq_diff;
		if (margin_left > LAYOUT->FFT_PRINT_SIZE)
			margin_left = LAYOUT->FFT_PRINT_SIZE;
		int32_t margin_right = 0;
		if (freq_diff > 0)
			margin_right = freq_diff;
		if (margin_right > LAYOUT->FFT_PRINT_SIZE)
			margin_right = LAYOUT->FFT_PRINT_SIZE;
		if ((margin_left + margin_right) > LAYOUT->FFT_PRINT_SIZE)
			margin_right = 0;
		//rounding
		int32_t body_width = LAYOUT->FFT_PRINT_SIZE - margin_left - margin_right;

		if (body_width <= 0)
			memset(&wtf_line_tmp, BG_COLOR, sizeof(wtf_line_tmp));
		else
		{
			if (margin_left == 0 && margin_right == 0)
			{
				HAL_DMA_Start(&hdma_memtomem_dma2_stream7, (uint32_t)&fft_and_wtf_buffer[fftHeight + print_wtf_yindex], (uint32_t)&wtf_line_tmp[0], LAYOUT->FFT_PRINT_SIZE / 2); // copy the line with the offset
				HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream7, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
			}
			else if (margin_left >= 0)
			{
				memset(&wtf_line_tmp, BG_COLOR, (uint32_t)(margin_left * 2)); // fill the space to the left
				HAL_DMA_Start(&hdma_memtomem_dma2_stream4, (uint32_t)&fft_and_wtf_buffer[fftHeight + print_wtf_yindex], (uint32_t)&wtf_line_tmp[margin_left], LAYOUT->FFT_PRINT_SIZE - margin_left); // copy the line with the offset
				HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream4, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
			}
			if (margin_right > 0)
			{
				memset(&wtf_line_tmp[(LAYOUT->FFT_PRINT_SIZE - margin_right)], BG_COLOR, (uint32_t)(margin_right * 2)); // fill the space to the right
				HAL_DMA_Start(&hdma_memtomem_dma2_stream4, (uint32_t)&fft_and_wtf_buffer[fftHeight + print_wtf_yindex][margin_right], (uint32_t)&wtf_line_tmp[0], LAYOUT->FFT_PRINT_SIZE - margin_right); // copy the line with the offset
				HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream4, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
			}
		}

		//print scale lines
		if(TRX.FFT_Grid >= 2)
			for(int8_t i = 0; i < FFT_MAX_GRID_NUMBER; i++)
				if(grid_lines_pos[i] > 0)
					wtf_line_tmp[grid_lines_pos[i]] = mixColors(wtf_line_tmp[grid_lines_pos[i]], color_scale[fftHeight / 2], FFT_SCALE_LINES_BRIGHTNESS);
		
		//center line
		wtf_line_tmp[LAYOUT->FFT_PRINT_SIZE / 2] = mixColors(wtf_line_tmp[LAYOUT->FFT_PRINT_SIZE / 2], color_scale[fftHeight / 2], FFT_SCALE_LINES_BRIGHTNESS);
		
		// add opacity to bandw bar
		for(int16_t fft_x = bw_line_start; ((fft_x <= (bw_line_start + bw_line_width)) && (fft_x < LAYOUT->FFT_PRINT_SIZE)); fft_x++)
				wtf_line_tmp[fft_x] = addColor(wtf_line_tmp[fft_x], FFT_BW_BRIGHTNESS, FFT_BW_BRIGHTNESS, FFT_BW_BRIGHTNESS);
		
		// display the line
		#ifdef HAS_BTE
		LCDDriver_SetCursorAreaPosition(0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + print_wtf_yindex, LAYOUT->FFT_PRINT_SIZE - 1, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + print_wtf_yindex);
		#endif
		HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream6, (uint32_t)&wtf_line_tmp[0], LCD_FSMC_DATA_ADDR, LAYOUT->FFT_PRINT_SIZE);
		print_wtf_yindex++;
	}
	else
	{
		FFT_FPS++;
		lastWTFFreq = currentFFTFreq;
		FFT_need_fft = true;
		LCD_busy = false;
	}
}

// shift the waterfall
ITCM static void FFT_move(int32_t _freq_diff)
{
	if (_freq_diff == 0)
		return;
	float32_t old_x_true = 0.0f;
	int32_t old_x_l = 0;
	int32_t old_x_r = 0;
	float32_t freq_diff = (_freq_diff / FFT_HZ_IN_PIXEL) * TRX.FFT_Zoom;
	float32_t old_x_part_r = fmodf(freq_diff, 1.0f);
	float32_t old_x_part_l = 1.0f - old_x_part_r;
	if(freq_diff < 0.0f)
	{
		old_x_part_l = fmodf(-freq_diff, 1.0f);
		old_x_part_r = 1.0f - old_x_part_l;
	}
	
	//Move mean Buffer
	for (int32_t x = 0; x < LAYOUT->FFT_PRINT_SIZE; x++)
	{
		old_x_true = (float32_t)x + freq_diff;
		if (old_x_true >= (float32_t)LAYOUT->FFT_PRINT_SIZE)
			old_x_true -= (float32_t)LAYOUT->FFT_PRINT_SIZE;
		if (old_x_true < 0.0f)
			old_x_true += (float32_t)LAYOUT->FFT_PRINT_SIZE;
		old_x_l = (int32_t)(floorf(old_x_true));
		old_x_r = (int32_t)(ceilf(old_x_true));
		
		FFTOutput_mean_new[x] = 0;
		
		if((old_x_true >= LAYOUT->FFT_PRINT_SIZE) || (old_x_true < 0.0f))
			continue;
		if((old_x_l < LAYOUT->FFT_PRINT_SIZE) && (old_x_l >= 0))
			FFTOutput_mean_new[x] += (FFTOutput_mean[old_x_l] * old_x_part_l);
		if((old_x_r < LAYOUT->FFT_PRINT_SIZE) && (old_x_r >= 0))
			FFTOutput_mean_new[x] += (FFTOutput_mean[old_x_r] * old_x_part_r);
		
		//sides
		if(old_x_r >= LAYOUT->FFT_PRINT_SIZE)
			FFTOutput_mean_new[x] = FFTOutput_mean[old_x_l];
	}
	//save results
	memcpy(&FFTOutput_mean, &FFTOutput_mean_new, sizeof FFTOutput_mean);
}

// get color from signal strength
static uint16_t getFFTColor(uint_fast8_t height) // Get FFT color warmth (blue to red)
{
	uint_fast8_t red = 0;
	uint_fast8_t green = 0;
	uint_fast8_t blue = 0;
	if(COLOR->WTF_BG_WHITE)
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
			if(COLOR->WTF_BG_WHITE)
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
		if(COLOR->WTF_BG_WHITE)
		{
			contrast1 = 0.2f;
			contrast2 = 0.8f;
		}
		
		if (height < GET_FFTHeight * contrast1)
		{
			if(!COLOR->WTF_BG_WHITE)
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
			if(COLOR->WTF_BG_WHITE)
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
		if(COLOR->WTF_BG_WHITE)
		{
			contrast1 = 0.2f;
			contrast2 = 0.8f;
		}
		
		if (height < GET_FFTHeight * contrast1)
		{
			if(!COLOR->WTF_BG_WHITE)
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
			if(COLOR->WTF_BG_WHITE)
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
			if(COLOR->WTF_BG_WHITE)
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
			if(COLOR->WTF_BG_WHITE)
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
			if(COLOR->WTF_BG_WHITE)
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
			if(COLOR->WTF_BG_WHITE)
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

ITCM static uint16_t getBGColor(uint_fast8_t height) // Get FFT background gradient
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
static void fft_fill_color_scale(void) // Fill FFT Color Gradient On Initialization
{
	for (uint_fast8_t i = 0; i < GET_FFTHeight; i++)
	{
		color_scale[i] = getFFTColor(GET_FFTHeight - i);
		bg_gradient_color[i] = getBGColor(GET_FFTHeight - i);
	}
}

// reset FFT
void FFT_Reset(void) // clear the FFT
{
	NeedFFTInputBuffer = false;
	FFT_buffer_ready = false;
	memset(FFTInput_I, 0x00, sizeof FFTInput_I);
	memset(FFTInput_Q, 0x00, sizeof FFTInput_Q);
	memset(FFTInput, 0x00, sizeof FFTInput);
	memset(FFTOutput_mean, 0x00, sizeof FFTOutput_mean);
	FFT_buff_index = 0;
	NeedFFTInputBuffer = true;
}

ITCM static inline int32_t getFreqPositionOnFFT(uint32_t freq)
{
	int32_t pos = (int32_t)((float32_t)LAYOUT->FFT_PRINT_SIZE / 2 + (float32_t)((float32_t)freq - (float32_t)CurrentVFO()->Freq) / hz_in_pixel * (float32_t)TRX.FFT_Zoom);
	if (pos < 0 || pos >= LAYOUT->FFT_PRINT_SIZE)
		return -1;
	return pos;
}

uint32_t getFreqOnFFTPosition(uint16_t position)
{
	return (uint32_t)((int32_t)CurrentVFO()->Freq + (int32_t)(-((float32_t)LAYOUT->FFT_PRINT_SIZE * hz_in_pixel / 2) + (float32_t)position * (hz_in_pixel / (float32_t)TRX.FFT_Zoom)));
}

ITCM static inline uint16_t addColor(uint16_t color, uint8_t add_r, uint8_t add_g, uint8_t add_b)
{
	uint8_t r = ((color >> 11) & 0x1F) + add_r;
	uint8_t g = ((color >> 5) & 0x3F) + (uint8_t)(add_g << 1);
	uint8_t b = ((color >> 0) & 0x1F) + add_b;
	if(r > 31) r = 31;
	if(g > 63) g = 63;
	if(b > 31) b = 31;
	return (uint16_t)(r << 11) | (uint16_t)(g << 5) | (uint16_t)b;
}

ITCM static inline uint16_t mixColors(uint16_t color1, uint16_t color2, float32_t opacity)
{
	uint8_t r = (uint8_t)((float32_t)((color1 >> 11) & 0x1F) + (float32_t)((color2 >> 11) & 0x1F) * opacity);
	uint8_t g = (uint8_t)((float32_t)((color1 >> 5) & 0x3F) + (float32_t)((color2 >> 5) & 0x3F) * opacity);
	uint8_t b = (uint8_t)((float32_t)((color1 >> 0) & 0x1F) + (float32_t)((color2 >> 0) & 0x1F) * opacity);
	if(r > 31) r = 31;
	if(g > 63) g = 63;
	if(b > 31) b = 31;
	return (uint16_t)(r << 11) | (uint16_t)(g << 5) | (uint16_t)b;
}
