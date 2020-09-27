#include "fft.h"
#include "main.h"
#include "arm_const_structs.h"
#include "audio_filters.h"
#include "screen_layout.h"

//Public variables
bool NeedFFTInputBuffer = true;				// flag of the need to fill the buffer with FPGA
bool FFT_need_fft = true;					// need to prepare data for display on the screen
bool FFT_buffer_ready = false;				// buffer is full, can be processed
volatile uint32_t FFT_buff_index = 0;		// current buffer index when it is filled with FPGA
IRAM2 float32_t FFTInput_I[FFT_SIZE] = {0}; // incoming buffer FFT I
IRAM2 float32_t FFTInput_Q[FFT_SIZE] = {0}; // incoming buffer FFT Q

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
static float32_t FFTInput[FFT_DOUBLE_SIZE_BUFFER] = {0};   // combined FFT I and Q buffer
static float32_t FFTInput_sorted[FFT_SIZE] = {0};		   // buffer for sorted values ​​(when looking for a median)
static float32_t FFTOutput_mean[LAY_FFT_PRINT_SIZE] = {0}; // averaged FFT buffer (for output)
static float32_t maxValueFFT_rx = 0;					   // maximum value of the amplitude in the resulting frequency response
static float32_t maxValueFFT_tx = 0;					   // maximum value of the amplitude in the resulting frequency response
static uint32_t currentFFTFreq = 0;
static uint16_t color_scale[LAY_FFT_WTF_MAX_HEIGHT] = {0};							  // color gradient in height FFT
static SRAM1 uint16_t wtf_buffer[LAY_FFT_WTF_MAX_HEIGHT][LAY_FFT_PRINT_SIZE] = {{0}}; // waterfall buffer
static SRAM1 uint32_t wtf_buffer_freqs[LAY_FFT_WTF_MAX_HEIGHT] = {0};				  // frequencies for each row of the waterfall
static SRAM1 uint16_t wtf_line_tmp[LAY_FFT_PRINT_SIZE] = {0};						  // temporary buffer to move the waterfall
static uint16_t print_wtf_xindex = 0;												  // the current coordinate of the waterfall output via DMA
static uint16_t print_wtf_yindex = 0;												  // the current coordinate of the waterfall output via DMA
static float32_t window_multipliers[FFT_SIZE] = {0};								  // coefficients of the selected window function
static float32_t hz_in_pixel = 1.0f;												  // current FFT density value
static SRAM1 uint16_t bandmap_line_tmp[LAY_FFT_PRINT_SIZE] = {0};					  // temporary buffer to move the waterfall
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
		{NULL}, // 0
		{NULL}, // 1
		// 48ksps, 12kHz lowpass
		{
			.numTaps = 4,
			.pCoeffs = (float32_t *)(const float32_t[]){475.1179397144384210E-6f, 0.503905202786044337f, 0.503905202786044337f, 475.1179397144384210E-6f},
			.pState = NULL},
		{NULL}, // 3
		// 48ksps, 6kHz lowpass
		{
			.numTaps = 4,
			.pCoeffs = (float32_t *)(const float32_t[]){0.198273254218889416f, 0.298085149879260325f, 0.298085149879260325f, 0.198273254218889416f},
			.pState = NULL},
		{NULL}, // 5
		{NULL}, // 6
		{NULL}, // 7
		// 48ksps, 3kHz lowpass
		{
			.numTaps = 4,
			.pCoeffs = (float32_t *)(const float32_t[]){0.199820836596682871f, 0.272777397353925699f, 0.272777397353925699f, 0.199820836596682871f},
			.pState = NULL},
		{NULL}, // 9
		{NULL}, // 10
		{NULL}, // 11
		{NULL}, // 12
		{NULL}, // 13
		{NULL}, // 14
		{NULL}, // 15
		// 48ksps, 1.5kHz lowpass
		{
			.numTaps = 4,
			.pCoeffs = (float32_t *)(const float32_t[]){0.199820836596682871f, 0.272777397353925699f, 0.272777397353925699f, 0.199820836596682871f},
			.pState = NULL},
};

//Prototypes
static uint16_t getFFTColor(uint_fast8_t height);	// get color from signal strength
static void fft_fill_color_scale(void);				// prepare the color palette
static uint16_t getFFTHeight(void);					// get FFT height
static uint16_t getWTFHeight(void);					// get the height of the waterfall
static void FFT_move(int32_t _freq_diff);			// shift the waterfall
static int32_t getFreqPositionOnFFT(uint32_t freq); // get the position on the FFT for a given frequency

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
	memset(&wtf_buffer, 0x00, sizeof wtf_buffer);
	// initialize sort
	arm_sort_init_f32(&FFT_sortInstance, ARM_SORT_QUICK, ARM_SORT_ASCENDING);
}

// FFT calculation
void FFT_doFFT(void)
{
	if (!TRX.FFT_Enabled)
		return;
	if (!FFT_need_fft)
		return;
	if (NeedFFTInputBuffer)
		return;
	if (!FFT_buffer_ready)
		return;
	if (CPU_LOAD.Load > 90)
		return;

	float32_t medianValue = 0; // Median value in the resulting frequency response
	float32_t diffValue = 0;   // Difference Between Maximum FFT And Threshold In Waterfall

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

	// Reduce the calculated FFT to visible
	if (FFT_SIZE > LAY_FFT_PRINT_SIZE)
	{
		float32_t fft_compress_rate = (float32_t)FFT_SIZE / (float32_t)LAY_FFT_PRINT_SIZE;
		for (uint_fast16_t i = 0; i < LAY_FFT_PRINT_SIZE; i++)
		{
			float32_t fft_compress_tmp = 0;
			for (uint_fast8_t c = 0; c < fft_compress_rate; c++)
				fft_compress_tmp += FFTInput[(uint_fast16_t)(i * fft_compress_rate + c)];
			FFTInput[i] = fft_compress_tmp / fft_compress_rate;
		}
	}

	// Looking for the median and maximum in frequency response
	arm_sort_f32(&FFT_sortInstance, FFTInput, FFTInput_sorted, FFT_SIZE);
	medianValue = FFTInput_sorted[FFT_SIZE / 2];
	float32_t maxAmplValue = 0;
	arm_max_no_idx_f32(FFTInput, FFT_SIZE, &maxAmplValue);

	// Maximum amplitude
	float32_t maxValueFFT = maxValueFFT_rx;
	if (TRX_on_TX())
		maxValueFFT = maxValueFFT_tx;
	float32_t maxValue = (medianValue * FFT_MAX);
	float32_t targetValue = (medianValue * FFT_TARGET);
	float32_t minValue = (medianValue * FFT_MIN);

	// Auto-calibrate FFT levels
	diffValue = (targetValue - maxValueFFT) / FFT_STEP_COEFF;
	maxValueFFT += diffValue;

	// minimum-maximum threshold
	if (maxValueFFT < minValue)
		maxValueFFT = minValue;
	if (maxValueFFT > maxValue)
		maxValueFFT = maxValue;
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
	arm_scale_f32(FFTInput, 1.0f / maxValueFFT, FFTInput, LAY_FFT_PRINT_SIZE);

	// Averaging values ​​for subsequent output
	float32_t averaging = (float32_t)TRX.FFT_Averaging;
	if (averaging < 1.0f)
		averaging = 1.0f;
	for (uint_fast16_t i = 0; i < LAY_FFT_PRINT_SIZE; i++)
		if (FFTOutput_mean[i] < FFTInput[i])
			FFTOutput_mean[i] += (FFTInput[i] - FFTOutput_mean[i]) / averaging;
		else
			FFTOutput_mean[i] -= (FFTOutput_mean[i] - FFTInput[i]) / averaging;

	FFT_need_fft = false;
}

// FFT output
void FFT_printFFT(void)
{
	if (LCD_busy)
		return;
	if (!TRX.FFT_Enabled)
		return;
	if (FFT_need_fft)
		return;
	if (LCD_systemMenuOpened)
		return;
	if (CPU_LOAD.Load > 90)
		return;
	LCD_busy = true;

	uint16_t height = 0; // column height in FFT output
	uint16_t tmp = 0;
	uint16_t fftHeight = getFFTHeight();
	uint16_t wtfHeight = getWTFHeight();

	// offset the waterfall if needed
	if (((int32_t)CurrentVFO()->Freq - (int32_t)currentFFTFreq) != 0)
	{
		FFT_move((int32_t)CurrentVFO()->Freq - (int32_t)currentFFTFreq);
		currentFFTFreq = CurrentVFO()->Freq;
	}

	// move the waterfall down using DMA
	for (tmp = wtfHeight - 1; tmp > 0; tmp--)
	{
		HAL_DMA_Start(&hdma_memtomem_dma2_stream7, (uint32_t)&wtf_buffer[tmp - 1], (uint32_t)&wtf_buffer[tmp], LAY_FFT_PRINT_SIZE / 2);
		HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream7, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
		wtf_buffer_freqs[tmp] = wtf_buffer_freqs[tmp - 1];
	}

	// calculate the colors for the waterfall
	uint_fast16_t new_x = 0;
	uint16_t fft_header[LAY_FFT_PRINT_SIZE] = {0};
	for (uint32_t fft_x = 0; fft_x < LAY_FFT_PRINT_SIZE; fft_x++)
	{
		if (fft_x < (LAY_FFT_PRINT_SIZE / 2))
			new_x = fft_x + (LAY_FFT_PRINT_SIZE / 2);
		if (fft_x >= (LAY_FFT_PRINT_SIZE / 2))
			new_x = fft_x - (LAY_FFT_PRINT_SIZE / 2);
		height = (uint16_t)((float32_t)FFTOutput_mean[(uint_fast16_t)fft_x] * fftHeight);
		if (height > fftHeight - 1)
		{
			height = fftHeight;
			tmp = COLOR_RED;
		}
		else
			tmp = color_scale[fftHeight - height];
		wtf_buffer[0][new_x] = tmp;
		wtf_buffer_freqs[0] = currentFFTFreq;
		fft_header[new_x] = height;
		if (new_x == (LAY_FFT_PRINT_SIZE / 2))
			continue;
	}

	// calculate bw bar size
	int16_t bw_line_start = 0;
	int16_t bw_line_width = 0;
	switch (CurrentVFO()->Mode)
	{
	case TRX_MODE_LSB:
	case TRX_MODE_CW_L:
	case TRX_MODE_DIGI_L:
		bw_line_width = (int16_t)(CurrentVFO()->LPF_Filter_Width / hz_in_pixel * TRX.FFT_Zoom);
		if (bw_line_width > (LAY_FFT_PRINT_SIZE / 2))
			bw_line_width = LAY_FFT_PRINT_SIZE / 2;
		bw_line_start = LAY_FFT_PRINT_SIZE / 2 - bw_line_width;
		break;
	case TRX_MODE_USB:
	case TRX_MODE_CW_U:
	case TRX_MODE_DIGI_U:
		bw_line_width = (int16_t)(CurrentVFO()->LPF_Filter_Width / hz_in_pixel * TRX.FFT_Zoom);
		if (bw_line_width > (LAY_FFT_PRINT_SIZE / 2))
			bw_line_width = LAY_FFT_PRINT_SIZE / 2;
		bw_line_start = LAY_FFT_PRINT_SIZE / 2;
		break;
	case TRX_MODE_NFM:
	case TRX_MODE_AM:
		bw_line_width = (int16_t)(CurrentVFO()->LPF_Filter_Width / hz_in_pixel * TRX.FFT_Zoom * 2);
		if (bw_line_width > LAY_FFT_PRINT_SIZE)
			bw_line_width = LAY_FFT_PRINT_SIZE;
		bw_line_start = LAY_FFT_PRINT_SIZE / 2 - (bw_line_width / 2);
		break;
	default:
		break;
	}
	
	// display FFT over the waterfall
	LCDDriver_SetCursorAreaPosition(0, LAY_FFT_WTF_POS_Y, LAY_FFT_PRINT_SIZE - 1, (LAY_FFT_WTF_POS_Y + fftHeight));
	uint16_t color = 0;
	for (uint32_t fft_y = 0; fft_y < fftHeight; fft_y++)
	{
		for (uint32_t fft_x = 0; fft_x < LAY_FFT_PRINT_SIZE; fft_x++)
		{
			if (fft_x == (LAY_FFT_PRINT_SIZE / 2))
				color = COLOR_GREEN;
			else if (fft_y <= (fftHeight - fft_header[fft_x]))
				color = COLOR_BLACK;
			else
			{
				if (TRX.FFT_Style == 3 || TRX.FFT_Style == 4)
					color = LAY_FFT_STYLE_3_4_COLOR;
				else
					color = color_scale[fft_y];
			}
			
			if(fft_x >= (uint32_t)bw_line_start && fft_x <= (uint32_t)(bw_line_start + bw_line_width)) // add opacity to bandw bar
			{
				uint8_t r = ((color >> 11) & 0x1F) + FFT_BW_BRIGHTNESS;
				uint8_t g = ((color >> 5) & 0x3F) + FFT_BW_BRIGHTNESS;
				uint8_t b = ((color >> 0) & 0x1F) + FFT_BW_BRIGHTNESS;
				if(r > 31) r = 31;
				if(g > 63) g = 63;
				if(b > 31) b = 31;
				color = (uint16_t)(r << 11) | (uint16_t)(g << 5) | (uint16_t)b;
			}
			
			LCDDriver_SendData(color);
		}
	}

	// clear and display part of the vertical bar
	LCDDriver_drawFastHLine(0, LAY_FFT_WTF_POS_Y - 1, LAY_FFT_PRINT_SIZE, COLOR_BLACK);
	LCDDriver_drawPixel(LAY_FFT_PRINT_SIZE / 2, LAY_FFT_WTF_POS_Y, COLOR_GREEN);
	memset(bandmap_line_tmp, 0x00, sizeof(bandmap_line_tmp));
	hz_in_pixel = TRX_on_TX() ? FFT_TX_HZ_IN_PIXEL : FFT_HZ_IN_PIXEL;

	// output bendmaps
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
			uint16_t region_color = LAY_BANDMAP_SSB_COLOR;
			if (BANDS[band].regions[region].mode == TRX_MODE_CW_L || BANDS[band].regions[region].mode == TRX_MODE_CW_U)
				region_color = LAY_BANDMAP_CW_COLOR;
			else if (BANDS[band].regions[region].mode == TRX_MODE_DIGI_L || BANDS[band].regions[region].mode == TRX_MODE_DIGI_U)
				region_color = LAY_BANDMAP_DIGI_COLOR;
			else if (BANDS[band].regions[region].mode == TRX_MODE_NFM || BANDS[band].regions[region].mode == TRX_MODE_WFM)
				region_color = LAY_BANDMAP_FM_COLOR;
			else if (BANDS[band].regions[region].mode == TRX_MODE_AM)
				region_color = LAY_BANDMAP_AM_COLOR;

			fft_freq_position_start = getFreqPositionOnFFT(BANDS[band].regions[region].startFreq);
			fft_freq_position_stop = getFreqPositionOnFFT(BANDS[band].regions[region].endFreq);
			if (fft_freq_position_start != -1 && fft_freq_position_stop == -1)
				fft_freq_position_stop = LAY_FFT_PRINT_SIZE;
			if (fft_freq_position_start == -1 && fft_freq_position_stop != -1)
				fft_freq_position_start = 0;
			if (fft_freq_position_start == -1 && fft_freq_position_stop == -1 && BANDS[band].regions[region].startFreq < CurrentVFO()->Freq && BANDS[band].regions[region].endFreq > CurrentVFO()->Freq)
			{
				fft_freq_position_start = 0;
				fft_freq_position_stop = LAY_FFT_PRINT_SIZE;
			}

			if (fft_freq_position_start != -1 && fft_freq_position_stop != -1)
				for (int32_t pixel_counter = fft_freq_position_start; pixel_counter < fft_freq_position_stop; pixel_counter++)
					bandmap_line_tmp[(uint16_t)pixel_counter] = region_color;
		}
	}
	LCDDriver_SetCursorAreaPosition(0, LAY_FFT_WTF_POS_Y - 4, LAY_FFT_PRINT_SIZE - 1, LAY_FFT_WTF_POS_Y - 4);
	for (uint32_t pixel_counter = 0; pixel_counter < LAY_FFT_PRINT_SIZE; pixel_counter++)
		LCDDriver_SendData(bandmap_line_tmp[pixel_counter]);

	// separator and receive band
	LCDDriver_drawFastHLine(bw_line_start, LAY_FFT_WTF_POS_Y - 1, bw_line_width, COLOR_GREEN);

	// display the waterfall using DMA
	print_wtf_xindex = 0;
	print_wtf_yindex = 0;
	FFT_printWaterfallDMA();
}

// waterfall output
void FFT_printWaterfallDMA(void)
{
	uint16_t fftHeight = getFFTHeight();
	uint16_t wtfHeight = getWTFHeight();
	uint_fast8_t cwdecoder_offset = 0;
	if (TRX.CWDecoder && (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U || CurrentVFO()->Mode == TRX_MODE_LOOPBACK))
		cwdecoder_offset = LAY_FFT_CWDECODER_OFFSET;

	if (print_wtf_yindex < (wtfHeight - cwdecoder_offset))
	{
		// calculate offset
		int32_t freq_diff = (int32_t)(((float32_t)((int32_t)currentFFTFreq - (int32_t)wtf_buffer_freqs[print_wtf_yindex]) / FFT_HZ_IN_PIXEL) * (float32_t)TRX.FFT_Zoom);
		uint16_t *wtf_draw_buffer = wtf_buffer[print_wtf_yindex];

		if (freq_diff != 0) // there is a frequency offset
		{
			wtf_draw_buffer = wtf_line_tmp; // use the modified line

			int32_t margin_left = 0;
			if (freq_diff < 0)
				margin_left = -freq_diff;
			if (margin_left > LAY_FFT_PRINT_SIZE)
				margin_left = LAY_FFT_PRINT_SIZE;
			int32_t margin_right = 0;
			if (freq_diff > 0)
				margin_right = freq_diff;
			if (margin_right > LAY_FFT_PRINT_SIZE)
				margin_right = LAY_FFT_PRINT_SIZE;
			if ((margin_left + margin_right) > LAY_FFT_PRINT_SIZE)
				margin_right = 0;
			int32_t body_width = LAY_FFT_PRINT_SIZE - margin_left - margin_right;

			if (print_wtf_xindex == 0) // only for a new line
			{
				if (body_width <= 0)
					memset(&wtf_line_tmp, 0x00, sizeof(wtf_line_tmp));
				else
				{
					if (margin_right > 0)
					{
						HAL_DMA_Start(&hdma_memtomem_dma2_stream4, (uint32_t)&wtf_buffer[print_wtf_yindex][margin_right], (uint32_t)&wtf_line_tmp[0], LAY_FFT_PRINT_SIZE); // copy the line with the offset
						HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream4, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
					}
					if (margin_left > 0)
					{
						HAL_DMA_Start(&hdma_memtomem_dma2_stream4, (uint32_t)&wtf_buffer[print_wtf_yindex], (uint32_t)&wtf_line_tmp[margin_left], LAY_FFT_PRINT_SIZE - margin_left); // copy the line with the offset
						HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream4, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
						memset(&wtf_line_tmp, 0x00, (uint32_t)(margin_left * 2)); // fill the space to the left
					}
					if (margin_right > 0)
						memset(&wtf_line_tmp[(LAY_FFT_PRINT_SIZE - margin_right)], 0x00, (uint32_t)(margin_right * 2)); // fill the space to the right
				}
			}
		}
		if (print_wtf_xindex == 0)
		{
			// display the left half
			LCDDriver_SetCursorAreaPosition(0, (LAY_FFT_WTF_POS_Y + fftHeight) + print_wtf_yindex, LAY_FFT_PRINT_SIZE / 2, (LAY_FFT_WTF_POS_Y + getFFTHeight()) + print_wtf_yindex + 1);
			HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream6, (uint32_t)&wtf_draw_buffer[0], LCD_FSMC_DATA_ADDR, LAY_FFT_PRINT_SIZE / 2);
			print_wtf_xindex = 1;
		}
		else
		{
			// display the right half
			LCDDriver_SetCursorAreaPosition(LAY_FFT_PRINT_SIZE / 2 + 1, (LAY_FFT_WTF_POS_Y + fftHeight) + print_wtf_yindex, LAY_FFT_PRINT_SIZE - 1, (LAY_FFT_WTF_POS_Y + getFFTHeight()) + print_wtf_yindex + 1);
			HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream6, (uint32_t)&wtf_draw_buffer[LAY_FFT_PRINT_SIZE / 2 + 1], LCD_FSMC_DATA_ADDR, LAY_FFT_PRINT_SIZE / 2 - 1);
			print_wtf_yindex ++;
			print_wtf_xindex = 0;
		}
	}
	else
	{
		LCDDriver_drawFastVLine(LAY_FFT_PRINT_SIZE / 2, LAY_FFT_WTF_POS_Y + fftHeight, (int16_t)(wtfHeight - cwdecoder_offset), COLOR_GREEN);
		FFT_need_fft = true;
		LCD_busy = false;
	}
}

// shift the waterfall
static void FFT_move(int32_t _freq_diff)
{
	if (_freq_diff == 0)
		return;
	int32_t new_x = 0;
	int32_t freq_diff = (_freq_diff / FFT_HZ_IN_PIXEL) * TRX.FFT_Zoom;
	//Move mean Buffer
	if (freq_diff > 0) //freq up
	{
		for (int32_t x = 0; x < LAY_FFT_PRINT_SIZE; x++)
		{
			new_x = x + freq_diff;
			if (new_x >= LAY_FFT_PRINT_SIZE)
				new_x -= LAY_FFT_PRINT_SIZE;
			if (new_x >= LAY_FFT_PRINT_SIZE)
			{
				FFTOutput_mean[x] = 0;
				continue;
			}
			FFTOutput_mean[x] = FFTOutput_mean[new_x];
		}
	}
	else //freq down
	{
		for (int32_t x = LAY_FFT_PRINT_SIZE - 1; x >= 0; x--)
		{
			new_x = x + freq_diff;
			if (new_x < 0)
				new_x += LAY_FFT_PRINT_SIZE;
			if (new_x < 0)
			{
				FFTOutput_mean[x] = 0;
				continue;
			}
			FFTOutput_mean[x] = FFTOutput_mean[new_x];
		}
	}
}

// get color from signal strength
static uint16_t getFFTColor(uint_fast8_t height) // Get FFT color warmth (blue to red)
{
	// r g b
	// 0 0 0
	// 0 0 255
	// 255 255 0
	// 255 0 0

	uint_fast8_t red = 0;
	uint_fast8_t green = 0;
	uint_fast8_t blue = 0;
	// contrast of each of the 3 zones, the total should be one
	const float32_t contrast1 = 0.1f;
	const float32_t contrast2 = 0.4f;
	const float32_t contrast3 = 0.5f;

	if (height < getFFTHeight() * contrast1)
	{
		blue = (uint_fast8_t)(height * 255 / (getFFTHeight() * contrast1));
	}
	else if (height < getFFTHeight() * (contrast1 + contrast2))
	{
		green = (uint_fast8_t)((height - getFFTHeight() * contrast1) * 255 / ((getFFTHeight() - getFFTHeight() * contrast1) * (contrast1 + contrast2)));
		red = green;
		blue = 255 - green;
	}
	else
	{
		red = 255;
		blue = 0;
		green = (uint_fast8_t)(255 - (height - (getFFTHeight() * (contrast1 + contrast2))) * 255 / ((getFFTHeight() - (getFFTHeight() * (contrast1 + contrast2))) * (contrast1 + contrast2 + contrast3)));
	}
	return rgb888torgb565(red, green, blue);
}

// prepare the color palette
static void fft_fill_color_scale(void) // Fill FFT Color Gradient On Initialization
{
	for (uint_fast8_t i = 0; i < getFFTHeight(); i++)
	{
		color_scale[i] = getFFTColor(getFFTHeight() - i);
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

// get FFT height
static uint16_t getFFTHeight(void)
{
	uint16_t FFT_HEIGHT = LAY_FFT_HEIGHT_STYLE1;
	if (TRX.FFT_Style == 2 || TRX.FFT_Style == 4)
		FFT_HEIGHT = LAY_FFT_HEIGHT_STYLE2;
	return FFT_HEIGHT;
}

// get the height of the waterfall
static uint16_t getWTFHeight(void)
{
	uint16_t WTF_HEIGHT = LAY_WTF_HEIGHT_STYLE1;
	if (TRX.FFT_Style == 2 || TRX.FFT_Style == 4)
		WTF_HEIGHT = LAY_WTF_HEIGHT_STYLE2;
	return WTF_HEIGHT;
}

static int32_t getFreqPositionOnFFT(uint32_t freq)
{
	int32_t pos = (int32_t)((float32_t)LAY_FFT_PRINT_SIZE / 2 + (float32_t)((float32_t)freq - (float32_t)CurrentVFO()->Freq) / hz_in_pixel * (float32_t)TRX.FFT_Zoom);
	if (pos < 0 || pos >= LAY_FFT_PRINT_SIZE)
		return -1;
	return pos;
}
