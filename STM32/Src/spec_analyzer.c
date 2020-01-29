#include "spec_analyzer.h"
#include "main.h"
#include "lcd_driver.h"
#include "trx_manager.h"
#include "functions.h"
#include "fpga.h"
#include "lcd.h"

const uint16_t graph_start_x = 20;
const uint16_t graph_width = LCD_WIDTH - 30;
const uint16_t graph_start_y = 5;
const uint16_t graph_height = LCD_HEIGHT - 25;
uint32_t now_freq;
uint32_t freq_step;
uint16_t graph_print_x;
uint16_t graph_prev_y;

//подготовка спектрального анализатора
void SPEC_Start(void)
{
	//рисуем GUI
	LCDDriver_Fill(COLOR_BLACK);
	LCDDriver_drawFastVLine(graph_start_x, graph_start_y, graph_height, COLOR_WHITE);
	LCDDriver_drawFastHLine(graph_start_x, graph_start_y + graph_height, graph_width, COLOR_WHITE);

	char ctmp[64] = {0};
	sprintf(ctmp, "%d", TRX.SPEC_Begin);
	LCDDriver_printText(ctmp, graph_start_x - 8, graph_start_y + graph_height + 3, COLOR_GREEN, COLOR_BLACK, 1);
	sprintf(ctmp, "%d", TRX.SPEC_End);
	LCDDriver_printText(ctmp, graph_start_x + graph_width - 16, graph_start_y + graph_height + 3, COLOR_GREEN, COLOR_BLACK, 1);

	//начинаем сканирование
	TRX.BandMapEnabled = false;
	TRX_setFrequency(TRX.SPEC_Begin * SPEC_Resolution, CurrentVFO());
	TRX_setMode(TRX_MODE_CW_U, CurrentVFO());
	now_freq = TRX.SPEC_Begin * SPEC_Resolution;
	freq_step = (TRX.SPEC_End * SPEC_Resolution - TRX.SPEC_Begin * SPEC_Resolution) / graph_width;
	graph_print_x = graph_start_x + 1;
	HAL_Delay(SPEC_StepDelay);
}

//отрисовка спектрального анализатора
void SPEC_Draw(void)
{
	LCD_busy = true;
	//S-Meter Calculate
	//TRX_DBMCalculate();
	int32_t dbm = TRX_RX_dBm;
	dbm += 130;
	dbm *= 4;
	now_freq += freq_step;
	TRX_setFrequency(now_freq, CurrentVFO());
	FPGA_NeedSendParams = true;
	FPGA_NeedGetParams = true;
	//рисуем
	int16_t y = (graph_start_y + graph_height) - dbm;
	if (y < 1)
		y = 1;
	if (y > (graph_start_y + graph_height))
		y = (graph_start_y + graph_height);
	if (graph_print_x != graph_start_x + 1)
		LCDDriver_drawLine(graph_print_x - 1, graph_prev_y, graph_print_x, y, COLOR_RED);
	LCDDriver_drawFastVLine(graph_print_x + 1, graph_start_y, graph_height, COLOR_BLACK);
	graph_prev_y = y;
	graph_print_x++;
	HAL_Delay(SPEC_StepDelay);
	if (now_freq > (TRX.SPEC_End * SPEC_Resolution))
	{
		graph_print_x = graph_start_x + 1;
		now_freq = TRX.SPEC_Begin * SPEC_Resolution;
	}
	LCD_UpdateQuery.SystemMenu = true;
	HAL_IWDG_Refresh(&hiwdg1);
	LCD_busy = false;
}
