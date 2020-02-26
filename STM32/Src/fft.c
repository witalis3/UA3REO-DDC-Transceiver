#include "fft.h"
#include "main.h"
#include "lcd.h"
#include <stdlib.h>
#include "arm_math.h"
#include "arm_const_structs.h"
#include "functions.h"
#include "audio_processor.h"
#include "wm8731.h"
#include "settings.h"
#include "audio_filters.h"
#include "screen_layout.h"

#if FFT_SIZE == 1024
const static arm_cfft_instance_f32 *FFT_Inst = &arm_cfft_sR_f32_len1024;
#endif
#if FFT_SIZE == 512
const static arm_cfft_instance_f32 *FFT_Inst = &arm_cfft_sR_f32_len512;
#endif
#if FFT_SIZE == 256
const static arm_cfft_instance_f32 *FFT_Inst = &arm_cfft_sR_f32_len256;
#endif

bool NeedFFTInputBuffer = true; //флаг необходимости заполнения буфера с FPGA
bool FFT_need_fft = true;		 //необходимо подготовить данные для отображения на экран
bool FFT_buffer_ready = false;   //буффер наполнен, можно обрабатывать

volatile uint32_t FFT_buff_index = 0;							 //текущий индекс буфера при его наполнении с FPGA
float32_t FFTInput_I[FFT_SIZE] = {0};							 //входящий буфер FFT I
float32_t FFTInput_Q[FFT_SIZE] = {0};							 //входящий буфер FFT Q
static float32_t FFTInput[FFT_DOUBLE_SIZE_BUFFER] = {0};		 //совмещённый буфер FFT I и Q
static float32_t FFTInput_ZOOMFFT[FFT_DOUBLE_SIZE_BUFFER] = {0}; //совмещённый буфер FFT I и Q для обработки ZoomFFT
static float32_t FFTOutput_mean[LAY_FFT_PRINT_SIZE] = {0};		 //усредненный буфер FFT (для вывода)
static uint16_t maxValueErrors = 0;								 //количество превышений сигнала в FFT
static float32_t maxValueFFT_rx = 0;							 //максимальное значение амплитуды в результирующей АЧХ
static float32_t maxValueFFT_tx = 0;							 //максимальное значение амплитуды в результирующей АЧХ
static uint32_t currentFFTFreq = 0;
static uint16_t color_scale[LAY_FFT_MAX_HEIGHT] = {0};							  //цветовой градиент по высоте FFT
static SRAM2 uint16_t wtf_buffer[LAY_FFT_WTF_HEIGHT][LAY_FFT_PRINT_SIZE] = {{0}}; //буфер водопада
static SRAM2 uint16_t wtf_line_tmp[LAY_FFT_PRINT_SIZE] = {0};					  //временный буффер для перемещения водопада

//Дециматор для Zoom FFT
static arm_fir_decimate_instance_f32 DECIMATE_ZOOM_FFT_I;
static arm_fir_decimate_instance_f32 DECIMATE_ZOOM_FFT_Q;
static float32_t decimZoomFFTIState[FFT_SIZE + 4 - 1];
static float32_t decimZoomFFTQState[FFT_SIZE + 4 - 1];
static uint16_t zoomed_width = 0;

static uint16_t print_wtf_xindex = 0;
static uint16_t print_wtf_yindex = 0;
static uint16_t getFFTColor(uint8_t height);
static void fft_fill_color_scale(void);

//Коэффициенты для ZoomFFT lowpass filtering / дециматора
static arm_biquad_casd_df1_inst_f32 IIR_biquad_Zoom_FFT_I =
	{
		.numStages = 4,
		.pCoeffs = (float32_t *)(float32_t[]){
			1, 0, 0, 0, 0, 1, 0, 0, 0, 0
		},
		.pState = (float32_t *)(float32_t[]){
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		}
	};
static arm_biquad_casd_df1_inst_f32 IIR_biquad_Zoom_FFT_Q =
	{
		.numStages = 4,
		.pCoeffs = (float32_t *)(float32_t[]){
			1, 0, 0, 0, 0, 1, 0, 0, 0, 0
		},
		.pState = (float32_t *)(float32_t[]){
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		}
	};

static const float32_t *mag_coeffs[17] =
	{
		NULL, // 0
		NULL, // 1
		(float32_t *)(const float32_t[]){
			// 2x magnify
			// 12kHz, sample rate 48k, 60dB stopband, elliptic
			// a1 and coeffs[A2] negated! order: coeffs[B0], coeffs[B1], coeffs[B2], a1, coeffs[A2]
			// Iowa Hills IIR Filter Designer
			0.228454526413293696f, 0.077639329099949764f, 0.228454526413293696f, 0.635534925142242080f, -0.170083307068779194f, 0.436788292542003964f, 0.232307972937606161f, 0.436788292542003964f, 0.365885230717786780f, -0.471769788739400842f, 0.535974654742658707f, 0.557035600464780845f, 0.535974654742658707f, 0.125740787233286133f, -0.754725697183384336f, 0.501116342273565607f, 0.914877831284765408f, 0.501116342273565607f, 0.013862536615004284f, -0.930973052446900984f
		},
		NULL, // 3
		(float32_t *)(const float32_t[]){
			// 4x magnify
			// 6kHz, sample rate 48k, 60dB stopband, elliptic
			// a1 and coeffs[A2] negated! order: coeffs[B0], coeffs[B1], coeffs[B2], a1, coeffs[A2]
			// Iowa Hills IIR Filter Designer
			0.182208761527446556f, -0.222492493114674145f, 0.182208761527446556f, 1.326111070880959810f, -0.468036100821178802f, 0.337123762652097259f, -0.366352718812586853f, 0.337123762652097259f, 1.337053579516321200f, -0.644948386007929031f, 0.336163175380826074f, -0.199246162162897811f, 0.336163175380826074f, 1.354952684569386670f, -0.828032873168141115f, 0.178588201750411041f, 0.207271695028067304f, 0.178588201750411041f, 1.386486967455699220f, -0.950935065984588657f
		},
		NULL, // 5
		NULL, // 6
		NULL, // 7
		(float32_t *)(const float32_t[]){
			// 8x magnify
			// 3kHz, sample rate 48k, 60dB stopband, elliptic
			// a1 and coeffs[A2] negated! order: coeffs[B0], coeffs[B1], coeffs[B2], a1, coeffs[A2]
			// Iowa Hills IIR Filter Designer
			0.185643392652478922f, -0.332064345389014803f, 0.185643392652478922f, 1.654637402827731090f, -0.693859842743674182f, 0.327519300813245984f, -0.571358085216950418f, 0.327519300813245984f, 1.715375037176782860f, -0.799055553586324407f, 0.283656142708241688f, -0.441088976843048652f, 0.283656142708241688f, 1.778230635987093860f, -0.904453944560528522f, 0.079685368654848945f, -0.011231810140649204f, 0.079685368654848945f, 1.825046003243238070f, -0.973184930412286708f
		},
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
			0.194769868656866380f, -0.379098413160710079f, 0.194769868656866380f, 1.824436402073870810f, -0.834877726226893380f, 0.333973874901496770f, -0.646106479315673776f, 0.333973874901496770f, 1.871892825636887640f, -0.893734096124207178f, 0.272903880596429671f, -0.513507745397738469f, 0.272903880596429671f, 1.918161772571113750f, -0.950461788366234739f, 0.053535383722369843f, -0.069683422367188122f, 0.053535383722369843f, 1.948900719896301760f, -0.986288064973853129f
		},
};

static const arm_fir_decimate_instance_f32 FirZoomFFTDecimate[17] =
	{
		{NULL}, // 0
		{NULL}, // 1
		// 48ksps, 12kHz lowpass
		{
			.numTaps = 4,
			.pCoeffs = (float32_t *)(const float32_t[]){475.1179397144384210E-6f, 0.503905202786044337f, 0.503905202786044337f, 475.1179397144384210E-6f},
			.pState = NULL
		},
		{NULL}, // 3
		// 48ksps, 6kHz lowpass
		{
			.numTaps = 4, 
			.pCoeffs = (float32_t *)(const float32_t[]){0.198273254218889416f, 0.298085149879260325f, 0.298085149879260325f, 0.198273254218889416f}, 
			.pState = NULL
		},
		{NULL}, // 5
		{NULL}, // 6
		{NULL}, // 7
		// 48ksps, 3kHz lowpass
		{
			.numTaps = 4, 
			.pCoeffs = (float32_t *)(const float32_t[]){0.199820836596682871f, 0.272777397353925699f, 0.272777397353925699f, 0.199820836596682871f}, 
			.pState = NULL
		},
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
			.pState = NULL
		},
};

void FFT_Init(void)
{
	fft_fill_color_scale();
	//ZoomFFT
	if (TRX.FFT_Zoom > 1)
	{
		IIR_biquad_Zoom_FFT_I.pCoeffs = mag_coeffs[TRX.FFT_Zoom];
		memset(IIR_biquad_Zoom_FFT_I.pState, 0x00, 4 * 4 * 32);
		IIR_biquad_Zoom_FFT_Q.pCoeffs = mag_coeffs[TRX.FFT_Zoom];
		memset(IIR_biquad_Zoom_FFT_Q.pState, 0x00, 4 * 4 * 32);
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
	sendToDebug_strln("[OK] FFT/Waterfall Inited");
}

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

	uint32_t maxIndex = 0;			 // Индекс элемента массива с максимальной амплитудой в результирующей АЧХ
	float32_t maxValue = 0;			 // Максимальное значение амплитуды в результирующей АЧХ
	float32_t meanValue = 0;		 // Среднее значение амплитуды в результирующей АЧХ
	float32_t diffValue = 0;		 // Разница между максимальным значением в FFT и пороге в водопаде
	float32_t window_multiplier = 0; //Множитель для вычисления окна к FFT

	//Process DC corrector filter
	if (!TRX_on_TX())
	{
		dc_filter(FFTInput_I, FFT_SIZE, 4);
		dc_filter(FFTInput_Q, FFT_SIZE, 5);
	}

	//Process Notch filter
	if (TRX.NotchFilter && !TRX_on_TX())
	{
		arm_biquad_cascade_df2T_f32(&NOTCH_FILTER_FFT_I, FFTInput_I, FFTInput_I, FFT_SIZE);
		arm_biquad_cascade_df2T_f32(&NOTCH_FILTER_FFT_Q, FFTInput_Q, FFTInput_Q, FFT_SIZE);
	}

	//ZoomFFT
	if (TRX.FFT_Zoom > 1)
	{
		//Biquad LPF фильтр
		arm_biquad_cascade_df1_f32(&IIR_biquad_Zoom_FFT_I, FFTInput_I, FFTInput_I, FFT_SIZE);
		arm_biquad_cascade_df1_f32(&IIR_biquad_Zoom_FFT_Q, FFTInput_Q, FFTInput_Q, FFT_SIZE);
		//Дециматор
		arm_fir_decimate_f32(&DECIMATE_ZOOM_FFT_I, FFTInput_I, FFTInput_I, FFT_SIZE);
		arm_fir_decimate_f32(&DECIMATE_ZOOM_FFT_Q, FFTInput_Q, FFTInput_Q, FFT_SIZE);
		//Смещаем старые данные в  буфере, т.к. будем их использовать (иначе скорость FFT упадёт, ведь для получения большего разрешения необходимо накапливать больше данных)
		for (uint16_t i = 0; i < FFT_SIZE; i++)
		{
			if (i < (FFT_SIZE - zoomed_width))
			{
				FFTInput_ZOOMFFT[i * 2] = FFTInput_ZOOMFFT[(i + zoomed_width) * 2];
				FFTInput_ZOOMFFT[i * 2 + 1] = FFTInput_ZOOMFFT[(i + zoomed_width) * 2 + 1];
			}
			else //Добавляем новые данные в буфер FFT для расчёта
			{
				FFTInput_ZOOMFFT[i * 2] = FFTInput_I[i - (FFT_SIZE - zoomed_width)];
				FFTInput_ZOOMFFT[i * 2 + 1] = FFTInput_Q[i - (FFT_SIZE - zoomed_width)];
			}

			FFTInput[i * 2] = FFTInput_ZOOMFFT[i * 2];
			FFTInput[i * 2 + 1] = FFTInput_ZOOMFFT[i * 2 + 1];
		}
	}
	else
	{
		//делаем совмещённый буфер для расчёта
		for (uint16_t i = 0; i < FFT_SIZE; i++)
		{
			FFTInput[i * 2] = FFTInput_I[i];
			FFTInput[i * 2 + 1] = FFTInput_Q[i];
		}
	}

	NeedFFTInputBuffer = true;

	//Окно для FFT
	for (uint16_t i = 0; i < FFT_SIZE; i++)
	{
		//Окно Hamming
		if (TRX.FFT_Window == 1)
			window_multiplier = 0.54f - 0.46f * arm_cos_f32((2.0f * PI * i) / ((float32_t)FFT_SIZE - 1.0f));
		//Окно Blackman-Harris
		else if (TRX.FFT_Window == 2)
			window_multiplier = 0.35875f - 0.48829f * arm_cos_f32(2.0f * PI * i / ((float32_t)FFT_SIZE - 1.0f)) + 0.14128f * arm_cos_f32(4.0f * PI * i / ((float32_t)FFT_SIZE - 1.0f)) - 0.01168f * arm_cos_f32(6.0f * PI * i / ((float32_t)FFT_SIZE - 1.0f));
		//Окно Hanning
		else if (TRX.FFT_Window == 3)
			window_multiplier = 0.5f * (1.0f - arm_cos_f32(2.0f * PI * i / (float32_t)FFT_SIZE * 2));

		FFTInput[i * 2] = window_multiplier * FFTInput[i * 2];
		FFTInput[i * 2 + 1] = window_multiplier * FFTInput[i * 2 + 1];
	}

	arm_cfft_f32(FFT_Inst, FFTInput, 0, 1);
	arm_cmplx_mag_f32(FFTInput, FFTInput, FFT_SIZE);

	//Уменьшаем расчитанный FFT до видимого
	if (FFT_SIZE > LAY_FFT_PRINT_SIZE)
	{
		float32_t fft_compress_rate = (float32_t)FFT_SIZE / (float32_t)LAY_FFT_PRINT_SIZE;
		for (uint16_t i = 0; i < LAY_FFT_PRINT_SIZE; i++)
		{
			float32_t fft_compress_tmp = 0;
			for (uint8_t c = 0; c < fft_compress_rate; c++)
				fft_compress_tmp += FFTInput[(uint16_t)(i * fft_compress_rate + c)];
			FFTInput[i] = fft_compress_tmp / fft_compress_rate;
		}
	}

	//Ищем Максимум амплитуды
	float32_t maxValueFFT = maxValueFFT_rx;
	if (TRX_on_TX())
		maxValueFFT = maxValueFFT_tx;
	arm_max_f32(FFTInput, LAY_FFT_PRINT_SIZE, &maxValue, &maxIndex); //ищем максимум в АЧХ

	//Ищем медиану в АЧХ
	float32_t median_max = maxValue;
	float32_t median_min = 0;
	for (uint16_t f = 0; ((f < LAY_FFT_PRINT_SIZE) && (median_max > median_min)); f++)
	{
		float32_t median_max_find = 0;
		float32_t median_min_find = median_max;
		for (uint16_t i = 0; i < LAY_FFT_PRINT_SIZE; i++)
		{
			if (FFTInput[i] < median_max && FFTInput[i] > median_max_find)
				median_max_find = FFTInput[i];
			if (FFTInput[i] > median_min && FFTInput[i] < median_min_find)
				median_min_find = FFTInput[i];
		}
		median_max = median_max_find;
		median_min = median_min_find;
	}
	meanValue = median_max;

	//Автокалибровка уровней FFT
	diffValue = (maxValue - maxValueFFT) / FFT_STEP_COEFF;
	if (maxValueErrors >= FFT_MAX_IN_RED_ZONE && diffValue > 0)
		maxValueFFT += diffValue;
	else if (maxValueErrors <= FFT_MIN_IN_RED_ZONE && diffValue < 0 && diffValue < -FFT_STEP_FIX)
		maxValueFFT += diffValue;
	else if (maxValueErrors <= FFT_MIN_IN_RED_ZONE && maxValueFFT > FFT_STEP_FIX)
		maxValueFFT -= FFT_STEP_FIX;
	else if (maxValueErrors <= FFT_MIN_IN_RED_ZONE && diffValue < 0 && diffValue < -FFT_STEP_PRECISION)
		maxValueFFT += diffValue;
	else if (maxValueErrors <= FFT_MIN_IN_RED_ZONE && maxValueFFT > FFT_STEP_PRECISION)
		maxValueFFT -= FFT_STEP_PRECISION;
	//Автокалибровка по средней амплитуде
	if (!TRX_on_TX())
	{
		if ((meanValue * 4.0f) > maxValueFFT)
			maxValueFFT = (meanValue * 4.0f);
		if ((meanValue * 8.0f) < maxValueFFT)
			maxValueFFT = (meanValue * 8.0f);
	}
	
	maxValueErrors = 0;
	if (maxValueFFT < FFT_MIN)
		maxValueFFT = FFT_MIN;
	if (TRX_on_TX())
		maxValueFFT_tx = maxValueFFT;
	else
		maxValueFFT_rx = maxValueFFT;

	//Нормируем АЧХ к единице
	arm_scale_f32(FFTInput, 1.0f / maxValueFFT, FFTInput, LAY_FFT_PRINT_SIZE);
	
	//Усреднение значений для последующего вывода (от резких всплесков)
	for (uint16_t i = 0; i < LAY_FFT_PRINT_SIZE; i++)
		if (FFTOutput_mean[i] < FFTInput[i])
			FFTOutput_mean[i] += (FFTInput[i] - FFTOutput_mean[i]) / TRX.FFT_Averaging;
		else
			FFTOutput_mean[i] -= (FFTOutput_mean[i] - FFTInput[i]) / TRX.FFT_Averaging;
	
	FFT_need_fft = false;
}

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
	LCD_busy = true;

	uint16_t height = 0; //высота столбца в выводе FFT
	uint16_t tmp = 0;

	//смещаем водопад, если нужно
	if ((CurrentVFO()->Freq - currentFFTFreq) != 0)
	{
		FFT_moveWaterfall(CurrentVFO()->Freq - currentFFTFreq);
		currentFFTFreq = CurrentVFO()->Freq;
	}

	//смещаем водопад вниз c помощью DMA
	for (tmp = LAY_FFT_WTF_HEIGHT - 1; tmp > 0; tmp--)
	{
		HAL_DMA_Start(&hdma_memtomem_dma2_stream7, (uint32_t)&wtf_buffer[tmp - 1], (uint32_t)&wtf_buffer[tmp], LAY_FFT_PRINT_SIZE / 2);
		HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream7, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
	}

	//расчитываем цвета для водопада
	uint16_t new_x = 0;
	uint8_t fft_header[LAY_FFT_PRINT_SIZE] = {0};
	for (uint32_t fft_x = 0; fft_x < LAY_FFT_PRINT_SIZE; fft_x++)
	{
		if (fft_x < (LAY_FFT_PRINT_SIZE / 2))
			new_x = fft_x + (LAY_FFT_PRINT_SIZE / 2);
		if (fft_x >= (LAY_FFT_PRINT_SIZE / 2))
			new_x = fft_x - (LAY_FFT_PRINT_SIZE / 2);
		height = FFTOutput_mean[(uint16_t)fft_x] * LAY_FFT_MAX_HEIGHT;
		if (height > LAY_FFT_MAX_HEIGHT - 1)
		{
			height = LAY_FFT_MAX_HEIGHT;
			tmp = COLOR_RED;
			maxValueErrors++;
		}
		else
			tmp = getFFTColor(height);
		wtf_buffer[0][new_x] = tmp;
		fft_header[new_x] = height;
		if (new_x == (LAY_FFT_PRINT_SIZE / 2))
			continue;
	}

	//выводим FFT над водопадом
	LCDDriver_SetCursorAreaPosition(0, LAY_FFT_BOTTOM_OFFSET - LAY_FFT_MAX_HEIGHT, LAY_FFT_PRINT_SIZE - 1, LAY_FFT_BOTTOM_OFFSET);
	for (uint32_t fft_y = 0; fft_y < LAY_FFT_MAX_HEIGHT; fft_y++)
		for (uint32_t fft_x = 0; fft_x < LAY_FFT_PRINT_SIZE; fft_x++)
		{
			if (fft_x == (LAY_FFT_PRINT_SIZE / 2))
				LCDDriver_SendData(COLOR_GREEN);
			else if (fft_y <= (LAY_FFT_MAX_HEIGHT - fft_header[fft_x]))
				LCDDriver_SendData(COLOR_BLACK);
			else
				LCDDriver_SendData(color_scale[fft_y]);
		}

	//разделитель и полоса приёма
	//LCDDriver_drawFastVLine(LAY_FFT_PRINT_SIZE / 2, LAY_FFT_BOTTOM_OFFSET, -LAY_FFT_MAX_HEIGHT, COLOR_GREEN);
	LCDDriver_drawFastHLine(0, LAY_FFT_BOTTOM_OFFSET - LAY_FFT_MAX_HEIGHT - 2, LAY_FFT_PRINT_SIZE, COLOR_BLACK);
	uint16_t line_width = 0;
	line_width = CurrentVFO()->Filter_Width / FFT_HZ_IN_PIXEL * TRX.FFT_Zoom;
	switch (CurrentVFO()->Mode)
	{
	case TRX_MODE_LSB:
	case TRX_MODE_CW_L:
	case TRX_MODE_DIGI_L:
		if (line_width > (LAY_FFT_PRINT_SIZE / 2))
			line_width = LAY_FFT_PRINT_SIZE / 2;
		LCDDriver_drawFastHLine(LAY_FFT_PRINT_SIZE / 2, LAY_FFT_BOTTOM_OFFSET - LAY_FFT_MAX_HEIGHT - 2, -line_width, COLOR_GREEN);
		break;
	case TRX_MODE_USB:
	case TRX_MODE_CW_U:
	case TRX_MODE_DIGI_U:
		if (line_width > (LAY_FFT_PRINT_SIZE / 2))
			line_width = LAY_FFT_PRINT_SIZE / 2;
		LCDDriver_drawFastHLine(LAY_FFT_PRINT_SIZE / 2, LAY_FFT_BOTTOM_OFFSET - LAY_FFT_MAX_HEIGHT - 2, line_width, COLOR_GREEN);
		break;
	case TRX_MODE_NFM:
	case TRX_MODE_AM:
		if (line_width > LAY_FFT_PRINT_SIZE)
			line_width = LAY_FFT_PRINT_SIZE;
		LCDDriver_drawFastHLine((LAY_FFT_PRINT_SIZE / 2) - (line_width / 2), LAY_FFT_BOTTOM_OFFSET - LAY_FFT_MAX_HEIGHT - 2, line_width, COLOR_GREEN);
		break;
	default:
		break;
	}

	//выводим на экран водопада с помощью DMA
	print_wtf_xindex = 0;
	print_wtf_yindex = 0;
	FFT_printWaterfallDMA();
}

void FFT_printWaterfallDMA(void)
{
	uint8_t cwdecoder_offset = 0;
	if (TRX.CWDecoder && (TRX_getMode(CurrentVFO()) == TRX_MODE_CW_L || TRX_getMode(CurrentVFO()) == TRX_MODE_CW_U))
		cwdecoder_offset = LAY_FFT_CWDECODER_OFFSET;

	if (print_wtf_yindex < (LAY_FFT_WTF_HEIGHT - cwdecoder_offset))
	{
		SCB_CleanDCache();
		if (print_wtf_xindex == 0)
		{
			LCDDriver_SetCursorAreaPosition(0, LAY_FFT_BOTTOM_OFFSET + print_wtf_yindex, LAY_FFT_PRINT_SIZE / 2, LAY_FFT_BOTTOM_OFFSET + print_wtf_yindex + 1);
			HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream6, (uint32_t)&wtf_buffer[print_wtf_yindex][0], LCD_FSMC_DATA_ADDR, LAY_FFT_PRINT_SIZE / 2);
			print_wtf_xindex = 1;
		}
		else
		{
			LCDDriver_SetCursorAreaPosition(LAY_FFT_PRINT_SIZE / 2 + 1, LAY_FFT_BOTTOM_OFFSET + print_wtf_yindex, LAY_FFT_PRINT_SIZE - 1, LAY_FFT_BOTTOM_OFFSET + print_wtf_yindex + 1);
			HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream6, (uint32_t)&wtf_buffer[print_wtf_yindex][LAY_FFT_PRINT_SIZE / 2 + 1], LCD_FSMC_DATA_ADDR, LAY_FFT_PRINT_SIZE / 2 - 1);
			print_wtf_yindex++;
			print_wtf_xindex = 0;
		}
	}
	else
	{
		LCDDriver_drawFastVLine(LAY_FFT_PRINT_SIZE / 2, LAY_FFT_BOTTOM_OFFSET, LAY_FFT_WTF_HEIGHT - cwdecoder_offset, COLOR_GREEN);
		FFT_need_fft = true;
		LCD_busy = false;
	}
}

void FFT_moveWaterfall(int32_t _freq_diff)
{
	if (_freq_diff == 0)
		return;
	int32_t new_x = 0;
	int32_t freq_diff = (_freq_diff / FFT_HZ_IN_PIXEL) * TRX.FFT_Zoom;
	freq_diff = freq_diff * 1.4f;
	//Move mean Buffer
	if (freq_diff > 0) //freq up
	{
		for (int16_t x = 0; x < LAY_FFT_PRINT_SIZE; x++)
		{
			new_x = x + freq_diff;
			if (new_x >= LAY_FFT_PRINT_SIZE)
				new_x -= LAY_FFT_PRINT_SIZE;
			if (new_x < 0 || new_x >= LAY_FFT_PRINT_SIZE)
			{
				FFTOutput_mean[x] = 0;
				continue;
			}
			FFTOutput_mean[x] = FFTOutput_mean[new_x];
		}
	}
	else //freq down
	{
		for (int16_t x = LAY_FFT_PRINT_SIZE - 1; x >= 0; x--)
		{
			new_x = x + freq_diff;
			if (new_x < 0)
				new_x += LAY_FFT_PRINT_SIZE;
			if (new_x < 0 || new_x >= LAY_FFT_PRINT_SIZE)
			{
				FFTOutput_mean[x] = 0;
				continue;
			}
			FFTOutput_mean[x] = FFTOutput_mean[new_x];
		}
	}
	//Move Waterfall
	uint16_t margin_left = 0;
	if (freq_diff < 0)
		margin_left = -freq_diff;
	if (margin_left > LAY_FFT_PRINT_SIZE)
		margin_left = LAY_FFT_PRINT_SIZE;
	uint16_t margin_right = 0;
	if (freq_diff > 0)
		margin_right = freq_diff;
	if (margin_right > LAY_FFT_PRINT_SIZE)
		margin_right = LAY_FFT_PRINT_SIZE;
	if ((margin_left + margin_right) > LAY_FFT_PRINT_SIZE)
		margin_right = 0;
	uint16_t body_width = LAY_FFT_PRINT_SIZE - margin_left - margin_right;

	for (uint8_t y = 0; y < LAY_FFT_WTF_HEIGHT; y++)
	{
		//memcpy(wtf_line_tmp, wtf_buffer[y], sizeof(wtf_line_tmp));
		HAL_DMA_Start(&hdma_memtomem_dma2_stream4, (uint32_t)&wtf_buffer[y], (uint32_t)&wtf_line_tmp, LAY_FFT_PRINT_SIZE); //копируем строку до смещения
		HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream4, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
		memset(&wtf_buffer[y][0], 0x00, margin_left * 2);																   //заполняем пространство слева
		memset(&wtf_buffer[y][(LAY_FFT_PRINT_SIZE - margin_right)], 0x00, margin_right * 2);							   //заполняем пространство справа

		//memcpy(&wtf_buffer[y][margin_left], &wtf_line_tmp[margin_left + freq_diff], body_width*2);
		if (body_width > 0) //рисуем смещенный водопад
		{
			HAL_DMA_Start(&hdma_memtomem_dma2_stream4, (uint32_t)&wtf_line_tmp[margin_left + freq_diff], (uint32_t)&wtf_buffer[y][margin_left], body_width);
			HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream4, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
		}
	}
}

static uint16_t getFFTColor(uint8_t height) //получение теплоты цвета FFT (от синего к красному)
{
	//r g b
	//0 0 0
	//0 0 255
	//255 255 0
	//255 0 0

	uint8_t red = 0;
	uint8_t green = 0;
	uint8_t blue = 0;
	//контраст каждой из 3-х зон, в сумме должна быть единица
	const float32_t contrast1 = 0.1f;
	const float32_t contrast2 = 0.4f;
	const float32_t contrast3 = 0.5f;

	if (height < LAY_FFT_MAX_HEIGHT * contrast1)
	{
		blue = (height * 255 / (LAY_FFT_MAX_HEIGHT * contrast1));
	}
	else if (height < LAY_FFT_MAX_HEIGHT * (contrast1 + contrast2))
	{
		green = (height - LAY_FFT_MAX_HEIGHT * contrast1) * 255 / ((LAY_FFT_MAX_HEIGHT - LAY_FFT_MAX_HEIGHT * contrast1) * (contrast1 + contrast2));
		red = green;
		blue = 255 - green;
	}
	else
	{
		red = 255;
		blue = 0;
		green = 255 - (height - (LAY_FFT_MAX_HEIGHT * (contrast1 + contrast2))) * 255 / ((LAY_FFT_MAX_HEIGHT - (LAY_FFT_MAX_HEIGHT * (contrast1 + contrast2))) * (contrast1 + contrast2 + contrast3));
	}
	return rgb888torgb565(red, green, blue);
}

static void fft_fill_color_scale(void) //заполняем градиент цветов FFT при инициализации
{
	for (uint8_t i = 0; i < LAY_FFT_MAX_HEIGHT; i++)
	{
		color_scale[i] = getFFTColor(LAY_FFT_MAX_HEIGHT - i);
	}
}

void FFT_Reset(void) //очищаем FFT
{
	NeedFFTInputBuffer = false;
	FFT_buffer_ready = false;
	memset(FFTInput_I, 0x00, sizeof FFTInput_I);
	memset(FFTInput_Q, 0x00, sizeof FFTInput_Q);
	memset(FFTInput, 0x00, sizeof FFTInput);
	memset(FFTInput_ZOOMFFT, 0x00, sizeof FFTInput_ZOOMFFT);
	memset(FFTOutput_mean, 0x00, sizeof FFTOutput_mean);
	FFT_buff_index = 0;
	NeedFFTInputBuffer = true;
}
