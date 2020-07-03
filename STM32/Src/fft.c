#include "fft.h"
#include "main.h"
#include "arm_const_structs.h"
#include "audio_filters.h"
#include "screen_layout.h"

//Public variables
bool NeedFFTInputBuffer = true;		  //флаг необходимости заполнения буфера с FPGA
bool FFT_need_fft = true;			  //необходимо подготовить данные для отображения на экран
bool FFT_buffer_ready = false;		  //буффер наполнен, можно обрабатывать
volatile uint32_t FFT_buff_index = 0; //текущий индекс буфера при его наполнении с FPGA
IRAM2 float32_t FFTInput_I[FFT_SIZE] = {0}; //входящий буфер FFT I
IRAM2 float32_t FFTInput_Q[FFT_SIZE] = {0}; //входящий буфер FFT Q

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
static float32_t FFTInput[FFT_DOUBLE_SIZE_BUFFER] = {0};		 //совмещённый буфер FFT I и Q
static float32_t FFTInput_sorted[FFT_SIZE] = {0};				 //буфер для отсортированных значений (при поиске медианы)
static float32_t FFTOutput_mean[LAY_FFT_PRINT_SIZE] = {0};		 //усредненный буфер FFT (для вывода)
static float32_t maxValueFFT_rx = 0;							 //максимальное значение амплитуды в результирующей АЧХ
static float32_t maxValueFFT_tx = 0;							 //максимальное значение амплитуды в результирующей АЧХ
static uint32_t currentFFTFreq = 0;
static uint16_t color_scale[LAY_FFT_WTF_MAX_HEIGHT] = {0};							  //цветовой градиент по высоте FFT
static SRAM1 uint16_t wtf_buffer[LAY_FFT_WTF_MAX_HEIGHT][LAY_FFT_PRINT_SIZE] = {{0}}; //буфер водопада
static SRAM1 uint32_t wtf_buffer_freqs[LAY_FFT_WTF_MAX_HEIGHT] = {0}; //частоты для каждой строки водопада
static SRAM1 uint16_t wtf_line_tmp[LAY_FFT_PRINT_SIZE] = {0};						  //временный буффер для перемещения водопада
static uint16_t print_wtf_xindex = 0;												  //текущая координата вывода водопада через DMA
static uint16_t print_wtf_yindex = 0;												  //текущая координата вывода водопада через DMA
static float32_t window_multipliers[FFT_SIZE] = {0};								  //коэффициенты выбранной оконной функции
static arm_sort_instance_f32 FFT_sortInstance = {0};			//инстанс сортировки (для поиска медианы)
//Дециматор для Zoom FFT
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
static uint16_t getFFTColor(uint_fast8_t height); //получить цвет из силы сигнала
static void fft_fill_color_scale(void);			  //подготовка цветовой палитры
static uint16_t getFFTHeight(void);				  //получение высоты FFT
static uint16_t getWTFHeight(void);				  //получение высоты водопада
static void FFT_move(int32_t _freq_diff); //сдвиг водопада

//инициализация FFT
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
		//Окно Hamming
		if (TRX.FFT_Window == 1)
			window_multipliers[i] = 0.54f - 0.46f * arm_cos_f32((2.0f * PI * i) / ((float32_t)FFT_SIZE - 1.0f));
		//Окно Blackman-Harris
		else if (TRX.FFT_Window == 2)
			window_multipliers[i] = 0.35875f - 0.48829f * arm_cos_f32(2.0f * PI * i / ((float32_t)FFT_SIZE - 1.0f)) + 0.14128f * arm_cos_f32(4.0f * PI * i / ((float32_t)FFT_SIZE - 1.0f)) - 0.01168f * arm_cos_f32(6.0f * PI * i / ((float32_t)FFT_SIZE - 1.0f));
		//Окно Hanning
		else if (TRX.FFT_Window == 3)
			window_multipliers[i] = 0.5f * (1.0f - arm_cos_f32(2.0f * PI * i / (float32_t)FFT_SIZE));
	}
	//очищаем буффер
	memset(&wtf_buffer, 0x00, sizeof wtf_buffer);
	//инициализация сортировки
	arm_sort_init_f32(&FFT_sortInstance, ARM_SORT_QUICK, ARM_SORT_ASCENDING);
}

//расчёт FFT
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

	float32_t medianValue = 0; // Значение медианы в результирующей АЧХ
	float32_t diffValue = 0;   // Разница между максимальным значением в FFT и пороге в водопаде

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
		//Дециматор
		arm_fir_decimate_f32(&DECIMATE_ZOOM_FFT_I, FFTInput_I, FFTInput_I, FFT_SIZE);
		arm_fir_decimate_f32(&DECIMATE_ZOOM_FFT_Q, FFTInput_Q, FFTInput_Q, FFT_SIZE);
		//Заполняем ненужную часть буффера нулями
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
		//делаем совмещённый буфер для расчёта
		for (uint_fast16_t i = 0; i < FFT_SIZE; i++)
		{
			FFTInput[i * 2] = FFTInput_I[i];
			FFTInput[i * 2 + 1] = FFTInput_Q[i];
		}
	}
	NeedFFTInputBuffer = true;

	//Окно для FFT
	for (uint_fast16_t i = 0; i < FFT_SIZE; i++)
	{
		FFTInput[i * 2] = window_multipliers[i] * FFTInput[i * 2];
		FFTInput[i * 2 + 1] = window_multipliers[i] * FFTInput[i * 2 + 1];
	}

	arm_cfft_f32(FFT_Inst, FFTInput, 0, 1);
	arm_cmplx_mag_f32(FFTInput, FFTInput, FFT_SIZE);

	//Уменьшаем расчитанный FFT до видимого
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

	//Ищем медиану и максимум в АЧХ
	arm_sort_f32(&FFT_sortInstance, FFTInput, FFTInput_sorted, FFT_SIZE);
	medianValue = FFTInput_sorted[FFT_SIZE / 2];
	float32_t maxAmplValue = 0;
	arm_max_no_idx_f32(FFTInput, FFT_SIZE, &maxAmplValue);
	
	//Максимум амплитуды
	float32_t maxValueFFT = maxValueFFT_rx;
	if (TRX_on_TX())
		maxValueFFT = maxValueFFT_tx;
	float32_t maxValue = (medianValue * FFT_MAX);
	float32_t targetValue = (medianValue * FFT_TARGET);
	float32_t minValue = (medianValue * FFT_MIN);

	//Автокалибровка уровней FFT
	diffValue = (targetValue - maxValueFFT) / FFT_STEP_COEFF;
	maxValueFFT += diffValue;

	//минимальный-максимальный порог
	if (maxValueFFT < minValue)
		maxValueFFT = minValue;
	if (maxValueFFT > maxValue)
		maxValueFFT = maxValue;
	if (TRX_on_TX())
		maxValueFFT = maxAmplValue;
	if (maxValueFFT < 0.0000001f)
		maxValueFFT = 0.0000001f;

	//сохраняем значения для переключения RX/TX
	if (TRX_on_TX())
		maxValueFFT_tx = maxValueFFT;
	else
		maxValueFFT_rx = maxValueFFT;

	//Нормируем АЧХ к единице
	arm_scale_f32(FFTInput, 1.0f / maxValueFFT, FFTInput, LAY_FFT_PRINT_SIZE);
	
	//Усреднение значений для последующего вывода
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

//вывод FFT
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

	uint16_t height = 0; //высота столбца в выводе FFT
	uint16_t tmp = 0;
	uint16_t fftHeight = getFFTHeight();
	uint16_t wtfHeight = getWTFHeight();
	
	//смещаем водопад, если нужно
	if (((int32_t)CurrentVFO()->Freq - (int32_t)currentFFTFreq) != 0)
	{
		FFT_move((int32_t)CurrentVFO()->Freq - (int32_t)currentFFTFreq);
		currentFFTFreq = CurrentVFO()->Freq;
	}

	//смещаем водопад вниз c помощью DMA
	for (tmp = wtfHeight - 1; tmp > 0; tmp--)
	{
		HAL_DMA_Start(&hdma_memtomem_dma2_stream7, (uint32_t)&wtf_buffer[tmp - 1], (uint32_t)&wtf_buffer[tmp], LAY_FFT_PRINT_SIZE / 2);
		HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream7, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
		wtf_buffer_freqs[tmp] = wtf_buffer_freqs[tmp - 1];
	}

	//расчитываем цвета для водопада
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
			tmp = color_scale[fftHeight-height];
		wtf_buffer[0][new_x] = tmp;
		wtf_buffer_freqs[0] = currentFFTFreq;
		fft_header[new_x] = height;
		if (new_x == (LAY_FFT_PRINT_SIZE / 2))
			continue;
	}

	//выводим FFT над водопадом
	LCDDriver_SetCursorAreaPosition(0, LAY_FFT_WTF_POS_Y, LAY_FFT_PRINT_SIZE - 1, (LAY_FFT_WTF_POS_Y + fftHeight));
	for (uint32_t fft_y = 0; fft_y < fftHeight; fft_y++)
		for (uint32_t fft_x = 0; fft_x < LAY_FFT_PRINT_SIZE; fft_x++)
		{
			if (fft_x == (LAY_FFT_PRINT_SIZE / 2))
				LCDDriver_SendData(COLOR_GREEN);
			else if (fft_y <= (fftHeight - fft_header[fft_x]))
				LCDDriver_SendData(COLOR_BLACK);
			else
			{
				if (TRX.FFT_Style == 3 || TRX.FFT_Style == 4)
					LCDDriver_SendData(LAY_FFT_STYLE_3_4_COLOR);
				else
					LCDDriver_SendData(color_scale[fft_y]);
			}
		}

	//разделитель и полоса приёма
	LCDDriver_drawFastHLine(0, LAY_FFT_WTF_POS_Y - 2, LAY_FFT_PRINT_SIZE, COLOR_BLACK);
	int16_t line_width = 0;
	switch (CurrentVFO()->Mode)
	{
	case TRX_MODE_LSB:
	case TRX_MODE_CW_L:
	case TRX_MODE_DIGI_L:
		line_width = (int16_t)(CurrentVFO()->LPF_Filter_Width / FFT_HZ_IN_PIXEL * TRX.FFT_Zoom);
		if (line_width > (LAY_FFT_PRINT_SIZE / 2))
			line_width = LAY_FFT_PRINT_SIZE / 2;
		LCDDriver_drawFastHLine(LAY_FFT_PRINT_SIZE / 2, LAY_FFT_WTF_POS_Y - 2, -line_width, COLOR_GREEN);
		break;
	case TRX_MODE_USB:
	case TRX_MODE_CW_U:
	case TRX_MODE_DIGI_U:
		line_width = (int16_t)(CurrentVFO()->LPF_Filter_Width / FFT_HZ_IN_PIXEL * TRX.FFT_Zoom);
		if (line_width > (LAY_FFT_PRINT_SIZE / 2))
			line_width = LAY_FFT_PRINT_SIZE / 2;
		LCDDriver_drawFastHLine(LAY_FFT_PRINT_SIZE / 2, LAY_FFT_WTF_POS_Y - 2, line_width, COLOR_GREEN);
		break;
	case TRX_MODE_NFM:
	case TRX_MODE_AM:
		line_width = (int16_t)(CurrentVFO()->LPF_Filter_Width / FFT_HZ_IN_PIXEL * TRX.FFT_Zoom * 2);
		if (line_width > LAY_FFT_PRINT_SIZE)
			line_width = LAY_FFT_PRINT_SIZE;
		LCDDriver_drawFastHLine((uint16_t)((LAY_FFT_PRINT_SIZE / 2) - (line_width / 2)), (uint16_t)(LAY_FFT_WTF_POS_Y - 2), line_width, COLOR_GREEN);
		break;
	default:
		break;
	}

	//выводим на экран водопада с помощью DMA
	print_wtf_xindex = 0;
	print_wtf_yindex = 0;
	FFT_printWaterfallDMA();
}

//вывод водопада
void FFT_printWaterfallDMA(void)
{
	uint16_t fftHeight = getFFTHeight();
	uint16_t wtfHeight = getWTFHeight();
	uint_fast8_t cwdecoder_offset = 0;
	if (TRX.CWDecoder && (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U || CurrentVFO()->Mode == TRX_MODE_LOOPBACK))
		cwdecoder_offset = LAY_FFT_CWDECODER_OFFSET;

	if (print_wtf_yindex < (wtfHeight - cwdecoder_offset))
	{
		//расчёт смещения
		int32_t freq_diff = (int32_t)(((float32_t)((int32_t)currentFFTFreq - (int32_t)wtf_buffer_freqs[print_wtf_yindex]) / FFT_HZ_IN_PIXEL) * (float32_t)TRX.FFT_Zoom);
		uint16_t *wtf_draw_buffer = wtf_buffer[print_wtf_yindex];
		
		if (freq_diff != 0) //есть смещение по частоте
		{
			wtf_draw_buffer = wtf_line_tmp; //используем модифицированную линию
			
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
			
			if (print_wtf_xindex == 0) //только для новой строки
			{
				if(body_width <= 0)
					memset(&wtf_line_tmp, 0x00, sizeof(wtf_line_tmp));
				else
				{
					if (margin_right > 0)
					{
						HAL_DMA_Start(&hdma_memtomem_dma2_stream4, (uint32_t)&wtf_buffer[print_wtf_yindex][margin_right], (uint32_t)&wtf_line_tmp[0], LAY_FFT_PRINT_SIZE); //копируем строку со смещением
						HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream4, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
					}
					if (margin_left > 0)
					{
						HAL_DMA_Start(&hdma_memtomem_dma2_stream4, (uint32_t)&wtf_buffer[print_wtf_yindex], (uint32_t)&wtf_line_tmp[margin_left], LAY_FFT_PRINT_SIZE - margin_left); //копируем строку со смещением
						HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream4, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
						memset(&wtf_line_tmp, 0x00, (uint32_t)(margin_left * 2));									 //заполняем пространство слева
          }
					if (margin_right > 0)
						memset(&wtf_line_tmp[(LAY_FFT_PRINT_SIZE - margin_right)], 0x00, (uint32_t)(margin_right * 2)); //заполняем пространство справа
				}
			}
		}
		if (print_wtf_xindex == 0)
		{
			//выводим левую половину
			LCDDriver_SetCursorAreaPosition(0, (LAY_FFT_WTF_POS_Y + fftHeight) + print_wtf_yindex, LAY_FFT_PRINT_SIZE / 2, (LAY_FFT_WTF_POS_Y + getFFTHeight()) + print_wtf_yindex + 1);
			HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream6, (uint32_t)&wtf_draw_buffer[0], LCD_FSMC_DATA_ADDR, LAY_FFT_PRINT_SIZE / 2);
			print_wtf_xindex = 1;
		}
		else
		{
			//выводим правую половину
			LCDDriver_SetCursorAreaPosition(LAY_FFT_PRINT_SIZE / 2 + 1, (LAY_FFT_WTF_POS_Y + fftHeight) + print_wtf_yindex, LAY_FFT_PRINT_SIZE - 1, (LAY_FFT_WTF_POS_Y + getFFTHeight()) + print_wtf_yindex + 1);
			HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream6, (uint32_t)&wtf_draw_buffer[LAY_FFT_PRINT_SIZE / 2 + 1], LCD_FSMC_DATA_ADDR, LAY_FFT_PRINT_SIZE / 2 - 1);
			print_wtf_yindex++;
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

//сдвиг водопада
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

//получить цвет из силы сигнала
static uint16_t getFFTColor(uint_fast8_t height) //получение теплоты цвета FFT (от синего к красному)
{
	//r g b
	//0 0 0
	//0 0 255
	//255 255 0
	//255 0 0

	uint_fast8_t red = 0;
	uint_fast8_t green = 0;
	uint_fast8_t blue = 0;
	//контраст каждой из 3-х зон, в сумме должна быть единица
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

//подготовка цветовой палитры
static void fft_fill_color_scale(void) //заполняем градиент цветов FFT при инициализации
{
	for (uint_fast8_t i = 0; i < getFFTHeight(); i++)
	{
		color_scale[i] = getFFTColor(getFFTHeight() - i);
	}
}

//сброс FFT
void FFT_Reset(void) //очищаем FFT
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

//получение высоты FFT
static uint16_t getFFTHeight(void)
{
	uint16_t FFT_HEIGHT = LAY_FFT_HEIGHT_STYLE1;
	if (TRX.FFT_Style == 2 || TRX.FFT_Style == 4)
		FFT_HEIGHT = LAY_FFT_HEIGHT_STYLE2;
	return FFT_HEIGHT;
}

//получение высоты водопада
static uint16_t getWTFHeight(void)
{
	uint16_t WTF_HEIGHT = LAY_WTF_HEIGHT_STYLE1;
	if (TRX.FFT_Style == 2 || TRX.FFT_Style == 4)
		WTF_HEIGHT = LAY_WTF_HEIGHT_STYLE2;
	return WTF_HEIGHT;
}
