#include "functions.h"
#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "arm_math.h"
#include "fpga.h"
#include "trx_manager.h"
#include "usbd_debug_if.h"
#include "usbd_cat_if.h"

CPULOAD_t CPU_LOAD = {0};

void dma_memcpy32(uint32_t dest, uint32_t src, uint32_t len)
{
	HAL_DMA_Start(&hdma_memtomem_dma2_stream3, src, dest, len);
	HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream3, HAL_DMA_FULL_TRANSFER, 10);
}

void readHalfFromCircleBuffer32(uint32_t *source, uint32_t *dest, uint32_t index, uint32_t length)
{
	uint16_t halflen = length / 2;
	if (index >= halflen)
	{
		dma_memcpy32((uint32_t)&dest[0], (uint32_t)&source[index - halflen], halflen);
	}
	else
	{
		uint16_t prev_part = halflen - index;
		dma_memcpy32((uint32_t)&dest[0], (uint32_t)&source[length - prev_part], prev_part);
		dma_memcpy32((uint32_t)&dest[prev_part], (uint32_t)&source[0], (halflen - prev_part));
	}
}

void readHalfFromCircleUSBBuffer(int32_t *source, int32_t *dest, uint32_t index, uint32_t length)
{
	uint16_t halflen = length / 2;
	uint16_t readed_index = 0;
	if (index >= halflen)
	{
		for (uint16_t i = (index - halflen); i < index; i++)
		{
			dest[readed_index] = source[i];
			readed_index++;
		}
	}
	else
	{
		uint16_t prev_part = halflen - index;
		for (uint16_t i = (length - prev_part); i < length; i++)
		{
			dest[readed_index] = source[i];
			readed_index++;
		}
		for (uint16_t i = 0; i < (halflen - prev_part); i++)
		{
			dest[readed_index] = source[i];
			readed_index++;
		}
	}
}

void readHalfFromCircleUSBBuffer24Bit(uint8_t *source, int32_t *dest, uint32_t index, uint32_t length)
{
	uint16_t halflen = length / 2;
	uint16_t readed_index = 0;
	if (index >= halflen)
	{
		for (uint16_t i = (index - halflen); i < index; i++)
		{
			dest[readed_index] = (source[i*3+0] << 8) | (source[i*3+1] << 16) | (source[i*3+2] << 24);
			readed_index++;
		}
	}
	else
	{
		uint16_t prev_part = halflen - index;
		for (uint16_t i = (length - prev_part); i < length; i++)
		{
			dest[readed_index] = (source[i*3+0] << 8) | (source[i*3+1] << 16) | (source[i*3+2] << 24);
			readed_index++;
		}
		for (uint16_t i = 0; i < (halflen - prev_part); i++)
		{
			dest[readed_index] = (source[i*3+0] << 8) | (source[i*3+1] << 16) | (source[i*3+2] << 24);
			readed_index++;
		}
	}
}

void sendToDebug_str(char *data)
{
	printf("%s", data);
	DEBUG_Transmit_FIFO((uint8_t *)data, strlen(data));
	HAL_UART_Transmit(&huart1, (uint8_t *)data, strlen(data), 1000);
}

void sendToDebug_strln(char *data)
{
	sendToDebug_str(data);
	sendToDebug_newline();
}

void sendToDebug_str2(char *data1, char *data2)
{
	sendToDebug_str(data1);
	sendToDebug_str(data2);
}

void sendToDebug_str3(char *data1, char *data2, char *data3)
{
	sendToDebug_str(data1);
	sendToDebug_str(data2);
	sendToDebug_str(data3);
}

void sendToDebug_newline(void)
{
	sendToDebug_str("\r\n");
}

void sendToDebug_flush(void)
{
	uint16_t tryes = 0;
	while (!DEBUG_Transmit_FIFO_Events() && tryes < 100)
	{
		HAL_IWDG_Refresh(&hiwdg1);
		HAL_Delay(1);
		tryes++;
	}
}

void sendToDebug_uint8(uint8_t data, bool _inline)
{
	char tmp[50] = "";
	if (_inline)
		sprintf(tmp, "%d", data);
	else
		sprintf(tmp, "%d\r\n", data);
	sendToDebug_str(tmp);
}

void sendToDebug_hex(uint8_t data, bool _inline)
{
	char tmp[50] = "";
	if (_inline)
		sprintf(tmp, "%02X", data);
	else
		sprintf(tmp, "%02X\r\n", data);
	sendToDebug_str(tmp);
}

void sendToDebug_uint16(uint16_t data, bool _inline)
{
	char tmp[50] = "";
	if (_inline)
		sprintf(tmp, "%d", data);
	else
		sprintf(tmp, "%d\r\n", data);
	sendToDebug_str(tmp);
}
void sendToDebug_uint32(uint32_t data, bool _inline)
{
	char tmp[50] = "";
	if (_inline)
		sprintf(tmp, "%d", data);
	else
		sprintf(tmp, "%d\r\n", data);
	sendToDebug_str(tmp);
}
void sendToDebug_int8(int8_t data, bool _inline)
{
	char tmp[50] = "";
	if (_inline)
		sprintf(tmp, "%d", data);
	else
		sprintf(tmp, "%d\r\n", data);
	sendToDebug_str(tmp);
}
void sendToDebug_int16(int16_t data, bool _inline)
{
	char tmp[50] = "";
	if (_inline)
		sprintf(tmp, "%d", data);
	else
		sprintf(tmp, "%d\r\n", data);
	sendToDebug_str(tmp);
}
void sendToDebug_int32(int32_t data, bool _inline)
{
	char tmp[50] = "";
	if (_inline)
		sprintf(tmp, "%d", data);
	else
		sprintf(tmp, "%d\r\n", data);
	sendToDebug_str(tmp);
}

void sendToDebug_float32(float32_t data, bool _inline)
{
	char tmp[50] = "";
	if (_inline)
		sprintf(tmp, "%f", (double)data);
	else
		sprintf(tmp, "%f\r\n", (double)data);
	sendToDebug_str(tmp);
}

/*
void delay_us(uint32_t us)
{
	if (bitRead(DWT->CTRL, DWT_CTRL_CYCCNTENA_Pos))
	{
		HAL_Delay(1);
		return;
	}
	unsigned long us_count_tick = us * (SystemCoreClock / 1000000);
	//разрешаем использовать счётчик
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	//обнуляем значение счётного регистра
	DWT->CYCCNT = 0;
	//запускаем счётчик
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	while (DWT->CYCCNT < us_count_tick)
		;
	//останавливаем счётчик
	DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
}
*/

bool beetween(float32_t a, float32_t b, float32_t val)
{
	if (a <= val && val <= b)
		return true;
	if (b <= val && val <= a)
		return true;
	return false;
}

uint32_t getFrequencyFromPhrase(uint32_t phrase) //высчитываем фазу частоты для FPGA
{
	uint32_t res = 0;
	res = ceil(((double)phrase / 4194304) * ADCDAC_CLOCK / 100) * 100; //freq in hz/oscil in hz*2^bits = (freq/48000000)*4194304;
	return res;
}

uint32_t getPhraseFromFrequency(uint32_t freq) //высчитываем частоту из фразы ля FPGA
{
	bool inverted = false;
	uint32_t res = 0;
	uint32_t _freq = freq;
	if (_freq > ADCDAC_CLOCK / 2) //Go Nyquist
	{
		while (_freq > ADCDAC_CLOCK / 2)
		{
			_freq -= ADCDAC_CLOCK / 2;
			inverted = !inverted;
		}
		if (inverted)
		{
			_freq = ADCDAC_CLOCK / 2 - _freq;
		}
	}
	TRX_IQ_swap = inverted;
	res = round(((double)_freq / ADCDAC_CLOCK) * 4194304); //freq in hz/oscil in hz*2^bits = (freq/48000000)*4194304;
	return res;
}

void addSymbols(char *dest, char *str, uint8_t length, char *symbol, bool toEnd) //добавляем нули
{
	char res[50] = "";
	strcpy(res, str);
	while (strlen(res) < length)
	{
		if (toEnd)
			strcat(res, symbol);
		else
		{
			char tmp[50] = "";
			strcat(tmp, symbol);
			strcat(tmp, res);
			strcpy(res, tmp);
		}
	}
	strcpy(dest, res);
}

float32_t log10f_fast(float32_t X)
{
	float32_t Y, F;
	int E;
	F = frexpf(fabsf(X), &E);
	Y = 1.23149591368684f;
	Y *= F;
	Y += -4.11852516267426f;
	Y *= F;
	Y += 6.02197014179219f;
	Y *= F;
	Y += -3.13396450166353f;
	Y += E;
	return (Y * 0.3010299956639812f);
}

float32_t db2rateV(float32_t i) //из децибелл в разы (для напряжения)
{
	return powf(10.0f, (i / 20.0f));
}

float32_t db2rateP(float32_t i) //из децибелл в разы (для мощности)
{
	return powf(10.0f, (i / 10.0f));
}

float32_t rate2dbV(float32_t i) //из разов в децибеллы (для напряжения)
{
	return 20 * log10f_fast(i);
}

float32_t rate2dbP(float32_t i) //из разов в децибеллы (для мощности)
{
	return 10 * log10f_fast(i);
}

#define VOLUME_LOW_DB (-20.0f)
#define VOLUME_EPSILON powf(10.0f, (VOLUME_LOW_DB / 20.0f))
float32_t volume2rate(float32_t i) //из положения ручки громкости в усиление
{
	if (i < 0.01f)
		return 0.0f;
	return powf(VOLUME_EPSILON, (1.0f - i));
}

void shiftTextLeft(char *string, int16_t shiftLength)
{
	int16_t i, size = strlen(string);
	if (shiftLength >= size)
	{
		memset(string, '\0', size);
		return;
	}
	for (i = 0; i < size - shiftLength; i++)
	{
		string[i] = string[i + shiftLength];
		string[i + shiftLength] = '\0';
	}
}

float32_t getMaxTXAmplitudeOnFreq(uint32_t freq)
{
	if (freq > MAX_TX_FREQ_HZ)
		return 0.0f;
	const uint8_t calibration_points = 31;
	uint8_t mhz_left = 0;
	uint8_t mhz_right = calibration_points;
	for (uint8_t i = 0; i <= calibration_points; i++)
		if ((i * 1000000) < freq)
			mhz_left = i;
	for (uint8_t i = calibration_points; i > 0; i--)
		if ((i * 1000000) >= freq)
			mhz_right = i;

	float32_t power_left = (float32_t)CALIBRATE.rf_out_power[mhz_left] / 100.0f * (float32_t)MAX_TX_AMPLITUDE;
	float32_t power_right = (float32_t)CALIBRATE.rf_out_power[mhz_right] / 100.0f * (float32_t)MAX_TX_AMPLITUDE;
	float32_t freq_point = (freq - (mhz_left * 1000000.0f)) / 1000000.0f;
	float32_t ret = (power_left * (1.0f - freq_point)) + (power_right * (freq_point));

	//sendToDebug_float32(power_left, false);
	//sendToDebug_float32(power_right, false);
	//sendToDebug_float32(freq_point, false);
	//sendToDebug_float32(ret, false);
	//sendToDebug_newline();

	return ret;
}

float32_t generateSin(float32_t amplitude, uint32_t index, uint32_t samplerate, uint32_t freq)
{
	float32_t ret = amplitude * arm_sin_f32(((float32_t)index / (float32_t)samplerate) * PI * 2.0f * (float32_t)freq);
	return ret;
}

void CPULOAD_Init(void)
{
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
}

static uint32_t CPULOAD_startWorkTime = 0;
static uint32_t CPULOAD_startSleepTime = 0;
static uint32_t CPULOAD_WorkingTime = 0;
static uint32_t CPULOAD_SleepingTime = 0;
static bool CPULOAD_status = true; // true - wake up ; false - sleep

void CPULOAD_GoToSleepMode(void) {
	//if(!CPULOAD_status) return;
	/* Add to working time */
	CPULOAD_WorkingTime += DWT->CYCCNT - CPULOAD_startWorkTime;
	/* Save count cycle time */
	CPULOAD_startSleepTime = DWT->CYCCNT;
	CPULOAD_status = false;
	/* Go to sleep mode Wait for wake up interrupt */
	__WFI();
}

void CPULOAD_WakeUp(void)
{
	if(CPULOAD_status) return;
	CPULOAD_status = true;
	/* Increase number of sleeping time in CPU cycles */
	CPULOAD_SleepingTime += DWT->CYCCNT - CPULOAD_startSleepTime;
	/* Save current time to get number of working CPU cycles */
	CPULOAD_startWorkTime = DWT->CYCCNT;
}

void CPULOAD_Calc(void)
{
	/* Save values */
	CPU_LOAD.SCNT = CPULOAD_SleepingTime;
	CPU_LOAD.WCNT = CPULOAD_WorkingTime;
	CPU_LOAD.Load = ((float)CPULOAD_WorkingTime / (float)(CPULOAD_SleepingTime + CPULOAD_WorkingTime) * 100);
	
	/* Reset time */
	CPULOAD_SleepingTime = 0;
	CPULOAD_WorkingTime = 0;
}

inline int32_t convertToSPIBigEndian(int32_t in)
{
	return (0xFFFF0000 & in<<16) | (0x0000FFFF & in>>16);
}
