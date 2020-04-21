#include "spec_analyzer.h"
#include "main.h"
#include "lcd_driver.h"
#include "trx_manager.h"
#include "functions.h"
#include "fpga.h"
#include "lcd.h"

//Private variables
static const uint16_t graph_start_x = 25;
static const uint16_t graph_width = LCD_WIDTH - 30;
static const uint16_t graph_start_y = 5;
static const uint16_t graph_height = LCD_HEIGHT - 25;
static uint32_t now_freq;
static uint32_t freq_step;
static uint16_t graph_sweep_x = 0;
static uint32_t tick_start_time = 0;
static int16_t graph_selected_x = graph_width / 2;
static int_fast16_t data[LCD_WIDTH - 30] = {0};

//Prototypes
static void SPEC_DrawBottomGUI(void);				   //вывод статуса внизу экрана
static void SPEC_DrawGraphCol(uint16_t x, bool clear); //вывод колонки данных
static uint16_t SPEC_getYfromX(uint16_t x);			   //получить высоту из id данных

//подготовка спектрального анализатора
void SPEC_Start(void)
{
	LCD_busy = true;

	//рисуем GUI
	LCDDriver_Fill(COLOR_BLACK);
	LCDDriver_drawFastVLine(graph_start_x, graph_start_y, graph_height, COLOR_WHITE);
	LCDDriver_drawFastHLine(graph_start_x, graph_start_y + graph_height, graph_width, COLOR_WHITE);

	//горизонтальные подписи
	char ctmp[64] = {0};
	sprintf(ctmp, "%u", TRX.SPEC_Begin);
	LCDDriver_printText(ctmp, graph_start_x + 2, graph_start_y + graph_height + 3, COLOR_GREEN, COLOR_BLACK, 1);
	sprintf(ctmp, "%u", TRX.SPEC_End);
	LCDDriver_printText(ctmp, graph_start_x + graph_width - 36, graph_start_y + graph_height + 3, COLOR_GREEN, COLOR_BLACK, 1);
	SPEC_DrawBottomGUI();

	//вертикальные подписи
	int16_t vres = (TRX.SPEC_BottomDBM - TRX.SPEC_TopDBM);
	int16_t partsize = vres / (SPEC_VParts - 1);
	for (uint8_t n = 0; n < SPEC_VParts; n++)
	{
		int32_t y = graph_start_y + (partsize * n * graph_height / vres);
		sprintf(ctmp, "%d", TRX.SPEC_TopDBM + partsize * n);
		LCDDriver_printText(ctmp, 0, (uint16_t)y, COLOR_GREEN, COLOR_BLACK, 1);
		LCDDriver_drawFastHLine(graph_start_x, (uint16_t)y, graph_width, COLOR_DGRAY);
	}

	//начинаем сканирование
	TRX.BandMapEnabled = false;
	TRX_setFrequency(TRX.SPEC_Begin * SPEC_Resolution, CurrentVFO());
	TRX_setMode(TRX_MODE_CW_U, CurrentVFO());
	CurrentVFO()->ManualNotchFilter = false;
	CurrentVFO()->AutoNotchFilter = false;
	CurrentVFO()->DNR = false;
	FPGA_NeedSendParams = true;
	now_freq = TRX.SPEC_Begin * SPEC_Resolution;
	freq_step = (TRX.SPEC_End * SPEC_Resolution - TRX.SPEC_Begin * SPEC_Resolution) / graph_width;
	graph_sweep_x = 0;
	tick_start_time = HAL_GetTick();

	LCD_busy = false;

	LCD_UpdateQuery.SystemMenu = true;
}

//отрисовка спектрального анализатора
void SPEC_Draw(void)
{
	if (LCD_busy)
		return;

	//Ждем пока набираются данные
	if ((HAL_GetTick() - tick_start_time) < SPEC_StepDelay)
	{
		LCD_UpdateQuery.SystemMenu = true;
		return;
	}
	if (Processor_RX_Power_value == 0) //-V550
		return;
	tick_start_time = HAL_GetTick();

	LCD_busy = true;
	//Считаем мощность сигнала
	TRX_DBMCalculate();

	//Рисуем
	data[graph_sweep_x] = TRX_RX_dBm;
	SPEC_DrawGraphCol(graph_sweep_x, true);
	//рисуем маркер
	if (graph_sweep_x == graph_selected_x)
		SPEC_DrawBottomGUI();

	//Переходим к расчёту следующего шага
	graph_sweep_x++;
	if (now_freq > (TRX.SPEC_End * SPEC_Resolution))
	{
		graph_sweep_x = 0;
		now_freq = TRX.SPEC_Begin * SPEC_Resolution;
	}
	now_freq += freq_step;
	TRX_setFrequency(now_freq, CurrentVFO());
	FPGA_NeedSendParams = true;
	LCD_busy = false;
}

//получить высоту из id данных
static uint16_t SPEC_getYfromX(uint16_t x)
{
	int32_t y = graph_start_y + ((data[x] - TRX.SPEC_TopDBM) * (graph_height - graph_start_y) / (TRX.SPEC_BottomDBM - TRX.SPEC_TopDBM));
	if (y < graph_start_y)
		y = graph_start_y;
	if (y > ((graph_start_y + graph_height) - 1))
		y = (graph_start_y + graph_height) - 1;
	return (uint16_t)y;
}

//вывод колонки данных
static void SPEC_DrawGraphCol(uint16_t x, bool clear)
{
	if (x >= graph_width)
		return;

	if (clear)
	{
		//очищаем
		LCDDriver_drawFastVLine((graph_start_x + x + 1), graph_start_y, graph_height, COLOR_BLACK);
		//рисуем полосы за графиком
		int16_t vres = (TRX.SPEC_BottomDBM - TRX.SPEC_TopDBM);
		for (uint8_t n = 0; n < (SPEC_VParts - 1); n++)
			LCDDriver_drawPixel((graph_start_x + x + 1), (uint16_t)(graph_start_y + ((vres / (SPEC_VParts - 1)) * n * graph_height / vres)), COLOR_DGRAY);
	}
	//рисуем график
	if (x > 0)
		LCDDriver_drawLine((graph_start_x + x), SPEC_getYfromX(x - 1), (graph_start_x + x + 1), SPEC_getYfromX(x), COLOR_RED);
	else
		LCDDriver_drawPixel((graph_start_x + x + 1), SPEC_getYfromX(x), COLOR_RED);
}

//вывод статуса внизу экрана
static void SPEC_DrawBottomGUI(void)
{
	char ctmp[64] = {0};
	int32_t freq = (int32_t)TRX.SPEC_Begin + (graph_selected_x * (int32_t)(TRX.SPEC_End - TRX.SPEC_Begin) / (graph_width - 1));
	sprintf(ctmp, "Freq=%dkHz DBM=%d", freq, data[graph_selected_x]);
	LCDDriver_Fill_RectWH(170, graph_start_y + graph_height + 3, 200, 6, COLOR_BLACK);
	LCDDriver_printText(ctmp, 170, graph_start_y + graph_height + 3, COLOR_GREEN, COLOR_BLACK, 1);
	LCDDriver_drawFastVLine(graph_start_x + (uint16_t)graph_selected_x + 1, graph_start_y, graph_height, COLOR_GREEN);
}

//события анализатора на энкодер
void SPEC_EncRotate(int8_t direction)
{
	//стираем старый маркер
	SPEC_DrawGraphCol((uint16_t)graph_selected_x, true);
	if (direction < 0)
		SPEC_DrawGraphCol((uint16_t)graph_selected_x + 1, false);
	//рисуем новый
	graph_selected_x += direction;
	if (graph_selected_x < 0)
		graph_selected_x = 0;
	if (graph_selected_x > (graph_width - 1))
		graph_selected_x = graph_width - 1;
	LCD_busy = true;
	SPEC_DrawBottomGUI();
	LCD_busy = false;
}
