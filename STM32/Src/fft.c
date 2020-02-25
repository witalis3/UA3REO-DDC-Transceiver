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
const static arm_cfft_instance_q31 *FFT_Inst = &arm_cfft_sR_q31_len1024;
#endif
#if FFT_SIZE == 512
const static arm_cfft_instance_q31 *FFT_Inst = &arm_cfft_sR_q31_len512;
#endif
#if FFT_SIZE == 256
const static arm_cfft_instance_q31 *FFT_Inst = &arm_cfft_sR_q31_len256;
#endif

bool NeedFFTInputBuffer = true; //флаг необходимости заполнения буфера с FPGA
bool FFT_need_fft = true;		 //необходимо подготовить данные для отображения на экран
bool FFT_buffer_ready = false;   //буффер наполнен, можно обрабатывать

volatile uint32_t FFT_buff_index = 0;							 //текущий индекс буфера при его наполнении с FPGA
q31_t FFTInput_I[FFT_SIZE] = {0};							 //входящий буфер FFT I
q31_t FFTInput_Q[FFT_SIZE] = {0};							 //входящий буфер FFT Q
static q31_t FFTInput[FFT_DOUBLE_SIZE_BUFFER] = {0};		 //совмещённый буфер FFT I и Q
static q31_t FFTInput_ZOOMFFT[FFT_DOUBLE_SIZE_BUFFER] = {0}; //совмещённый буфер FFT I и Q для обработки ZoomFFT
static float32_t FFTOutput_mean[LAY_FFT_PRINT_SIZE] = {0};		 //усредненный буфер FFT (для вывода)
static uint16_t maxValueErrors = 0;								 //количество превышений сигнала в FFT
static q31_t maxValueFFT_rx = 0;							 //максимальное значение амплитуды в результирующей АЧХ
static q31_t maxValueFFT_tx = 0;							 //максимальное значение амплитуды в результирующей АЧХ
static uint32_t currentFFTFreq = 0;
static uint16_t color_scale[LAY_FFT_MAX_HEIGHT] = {0};							  //цветовой градиент по высоте FFT
static SRAM2 uint16_t wtf_buffer[LAY_FFT_WTF_HEIGHT][LAY_FFT_PRINT_SIZE] = {{0}}; //буфер водопада
static SRAM2 uint16_t wtf_line_tmp[LAY_FFT_PRINT_SIZE] = {0};					  //временный буффер для перемещения водопада

//Дециматор для Zoom FFT
static arm_fir_decimate_instance_q31 DECIMATE_ZOOM_FFT_I;
static arm_fir_decimate_instance_q31 DECIMATE_ZOOM_FFT_Q;
static q31_t decimZoomFFTIState[FFT_SIZE + 4 - 1];
static q31_t decimZoomFFTQState[FFT_SIZE + 4 - 1];
static uint16_t zoomed_width = 0;

static uint16_t print_wtf_xindex = 0;
static uint16_t print_wtf_yindex = 0;
static uint16_t getFFTColor(uint8_t height);
static void fft_fill_color_scale(void);

//Коэффициенты для ZoomFFT lowpass filtering / дециматора
static arm_biquad_casd_df1_inst_q31 IIR_biquad_Zoom_FFT_I =
	{
		.numStages = 4,
		.pCoeffs = (q31_t *)(q31_t[]){
			1073741800,0,0,0,0,1073741800,0,0,0,0
		},
		.pState = (q31_t *)(q31_t[]){
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		}
	};
static arm_biquad_casd_df1_inst_q31 IIR_biquad_Zoom_FFT_Q =
	{
		.numStages = 4,
		.pCoeffs = (q31_t *)(q31_t[]){
			1073741800,0,0,0,0,1073741800,0,0,0,0
		},
		.pState = (q31_t *)(q31_t[]){
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		}
	};

static const q31_t *mag_coeffs[17] =
	{
		NULL, // 0
		NULL, // 1
		(q31_t *)(const q31_t[]){
			// 2x magnify
			// 12kHz, sample rate 48k, 60dB stopband, elliptic
			// a1 and coeffs[A2] negated! order: coeffs[B0], coeffs[B1], coeffs[B2], a1, coeffs[A2]
			// Iowa Hills IIR Filter Designer
			268124515,91121011,268124515,745892393,-199617424,512634399,272647093,512634399,429419375,-553690258,629043978,653762052,629043978,147575047,-885780047,588132694,1073741800,588132694,16269697,-1092631876
		},
		NULL, // 3
		(q31_t *)(const q31_t[]){
			// 4x magnify
			// 6kHz, sample rate 48k, 60dB stopband, elliptic
			// a1 and coeffs[A2] negated! order: coeffs[B0], coeffs[B1], coeffs[B2], a1, coeffs[A2]
			// Iowa Hills IIR Filter Designer
			141108549,-172305615,141108549,1026984690,-362462783,261079898,-283715777,261079898,1035458934,-499469564,260335987,-154302880,260335987,1049320598,-641256304,138304666,160518121,138304666,1073741800,-736435865
		},
		NULL, // 5
		NULL, // 6
		NULL, // 7
		(q31_t *)(const q31_t[]){
			// 8x magnify
			// 3kHz, sample rate 48k, 60dB stopband, elliptic
			// a1 and coeffs[A2] negated! order: coeffs[B0], coeffs[B1], coeffs[B2], a1, coeffs[A2]
			// Iowa Hills IIR Filter Designer
			109220847,-195365688,109220847,973484142,-408223308,192691671,-336151011,192691671,1009218330,-470113820,166885359,-259508895,166885359,1046198595,-532123576,46881838,-6608088,46881838,1073741800,-572560548
		},
		NULL, // 9
		NULL, // 10
		NULL, // 11
		NULL, // 12
		NULL, // 13
		NULL, // 14
		NULL, // 15
		(q31_t *)(const q31_t[]){
			// 16x magnify
			// 1k5, sample rate 48k, 60dB stopband, elliptic
			// a1 and coeffs[A2] negated! order: coeffs[B0], coeffs[B1], coeffs[B2], a1, coeffs[A2]
			// Iowa Hills IIR Filter Designer
			107307954,-208863288,107307954,1005168507,-459973719,184002041,-355970690,184002041,1031314500,-492400484,150355686,-282915761,150355686,1056806257,-523654459,29495181,-38391901,29495181,1073741800,-543392853
		},
};

static const arm_fir_decimate_instance_q31 FirZoomFFTDecimate[17] =
	{
		{NULL}, // 0
		{NULL}, // 1
		// 48ksps, 12kHz lowpass
		{
			.numTaps = 4,
			.pCoeffs = (q31_t *)(const q31_t[]){1012401,1073741800,1073741800,1012401},
			.pState = NULL
		},
		{NULL}, // 3
		// 48ksps, 6kHz lowpass
		{
			.numTaps = 4, 
			.pCoeffs = (q31_t *)(const q31_t[]){714206263,1073741800,1073741800,714206263}, 
			.pState = NULL
		},
		{NULL}, // 5
		{NULL}, // 6
		{NULL}, // 7
		// 48ksps, 3kHz lowpass
		{
			.numTaps = 4, 
			.pCoeffs = (q31_t *)(const q31_t[]){786560715,1073741800,1073741800,786560715}, 
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
			.pCoeffs = (q31_t *)(const q31_t[]){786560715,1073741800,1073741800,786560715}, 
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
		arm_fir_decimate_init_q31(&DECIMATE_ZOOM_FFT_I,
								  FirZoomFFTDecimate[TRX.FFT_Zoom].numTaps,
								  TRX.FFT_Zoom, // Decimation factor
								  FirZoomFFTDecimate[TRX.FFT_Zoom].pCoeffs,
								  decimZoomFFTIState, // Filter state variables
								  FFT_SIZE);

		arm_fir_decimate_init_q31(&DECIMATE_ZOOM_FFT_Q,
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
	q31_t maxValue = 0;			 // Максимальное значение амплитуды в результирующей АЧХ
	q31_t meanValue = 0;		 // Среднее значение амплитуды в результирующей АЧХ
	q31_t diffValue = 0;		 // Разница между максимальным значением в FFT и пороге в водопаде
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
		//arm_biquad_cascade_df2T_f32(&NOTCH_FILTER_FFT_I, FFTInput_I, FFTInput_I, FFT_SIZE);
		//arm_biquad_cascade_df2T_f32(&NOTCH_FILTER_FFT_Q, FFTInput_Q, FFTInput_Q, FFT_SIZE);
	}

	//ZoomFFT
	if (TRX.FFT_Zoom > 1)
	{
		//Biquad LPF фильтр
		arm_biquad_cascade_df1_q31(&IIR_biquad_Zoom_FFT_I, FFTInput_I, FFTInput_I, FFT_SIZE);
		arm_biquad_cascade_df1_q31(&IIR_biquad_Zoom_FFT_Q, FFTInput_Q, FFTInput_Q, FFT_SIZE);
		//Дециматор
		arm_fir_decimate_q31(&DECIMATE_ZOOM_FFT_I, FFTInput_I, FFTInput_I, FFT_SIZE);
		arm_fir_decimate_q31(&DECIMATE_ZOOM_FFT_Q, FFTInput_Q, FFTInput_Q, FFT_SIZE);
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

	arm_cfft_q31(FFT_Inst, FFTInput, 0, 1);
	arm_cmplx_mag_q31(FFTInput, FFTInput, FFT_SIZE);

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
	q31_t maxValueFFT = maxValueFFT_rx;
	if (TRX_on_TX())
		maxValueFFT = maxValueFFT_tx;
	arm_max_q31(FFTInput, LAY_FFT_PRINT_SIZE, &maxValue, &maxIndex); //ищем максимум в АЧХ

	//Ищем медиану в АЧХ
	q31_t median_max = maxValue;
	q31_t median_min = 0;
	for (uint16_t f = 0; ((f < LAY_FFT_PRINT_SIZE) && (median_max > median_min)); f++)
	{
		q31_t median_max_find = 0;
		q31_t median_min_find = median_max;
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
		if ((meanValue * 4) > maxValueFFT)
			maxValueFFT = (meanValue * 4);
		if ((meanValue * 8) < maxValueFFT)
			maxValueFFT = (meanValue * 8);
	}
	
	maxValueErrors = 0;
	if (maxValueFFT < FFT_MIN)
		maxValueFFT = FFT_MIN;
	if (TRX_on_TX())
		maxValueFFT_tx = maxValueFFT;
	else
		maxValueFFT_rx = maxValueFFT;

	//Усреднение значений для последующего вывода (от резких всплесков)
	for (uint16_t i = 0; i < LAY_FFT_PRINT_SIZE; i++)
	{
		float32_t val = FFTInput[i]*(1.0f / (float32_t)maxValueFFT); //Нормируем АЧХ к единице
		if (FFTOutput_mean[i] < val)
			FFTOutput_mean[i] += (val - FFTOutput_mean[i]) / TRX.FFT_Averaging;
		else
			FFTOutput_mean[i] -= (FFTOutput_mean[i] - val) / TRX.FFT_Averaging;
	}
	
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
