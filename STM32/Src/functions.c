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
	uint_fast16_t halflen = length / 2;
	if (index >= halflen)
	{
		dma_memcpy32((uint32_t)&dest[0], (uint32_t)&source[index - halflen], halflen);
	}
	else
	{
		uint_fast16_t prev_part = halflen - index;
		dma_memcpy32((uint32_t)&dest[0], (uint32_t)&source[length - prev_part], prev_part);
		dma_memcpy32((uint32_t)&dest[prev_part], (uint32_t)&source[0], (halflen - prev_part));
	}
}

void readHalfFromCircleUSBBuffer24Bit(uint8_t *source, int32_t *dest, uint32_t index, uint32_t length)
{
	uint_fast16_t halflen = length / 2;
	uint_fast16_t readed_index = 0;
	if (index >= halflen)
	{
		for (uint_fast16_t i = (index - halflen); i < index; i++)
		{
			dest[readed_index] = (source[i*3+0] << 8) | (source[i*3+1] << 16) | (source[i*3+2] << 24);
			readed_index++;
		}
	}
	else
	{
		uint_fast16_t prev_part = halflen - index;
		for (uint_fast16_t i = (length - prev_part); i < length; i++)
		{
			dest[readed_index] = (source[i*3+0] << 8) | (source[i*3+1] << 16) | (source[i*3+2] << 24);
			readed_index++;
		}
		for (uint_fast16_t i = 0; i < (halflen - prev_part); i++)
		{
			dest[readed_index] = (source[i*3+0] << 8) | (source[i*3+1] << 16) | (source[i*3+2] << 24);
			readed_index++;
		}
	}
}

void sendToDebug_str(char *data)
{
	if(SWD_DEBUG_ENABLED) printf("%s", data);
	if(USB_DEBUG_ENABLED) DEBUG_Transmit_FIFO((uint8_t *)data, (uint16_t)strlen(data));
	if(UART_DEBUG_ENABLED) HAL_UART_Transmit(&huart1, (uint8_t *)data, (uint16_t)strlen(data), 1000);
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
	sendToDebug_str("\n");
}

void sendToDebug_flush(void)
{
	uint_fast16_t tryes = 0;
	while (DEBUG_Transmit_FIFO_Events()==USBD_BUSY && tryes < 512)
		tryes++;
}

void sendToDebug_uint8(uint8_t data, bool _inline)
{
	char tmp[50] = "";
	if (_inline)
		sprintf(tmp, "%d", data);
	else
		sprintf(tmp, "%d\n", data);
	sendToDebug_str(tmp);
}

void sendToDebug_hex(uint8_t data, bool _inline)
{
	char tmp[50] = "";
	if (_inline)
		sprintf(tmp, "%02X", data);
	else
		sprintf(tmp, "%02X\n", data);
	sendToDebug_str(tmp);
}

void sendToDebug_uint16(uint16_t data, bool _inline)
{
	char tmp[50] = "";
	if (_inline)
		sprintf(tmp, "%d", data);
	else
		sprintf(tmp, "%d\n", data);
	sendToDebug_str(tmp);
}
void sendToDebug_uint32(uint32_t data, bool _inline)
{
	char tmp[50] = "";
	if (_inline)
		sprintf(tmp, "%d", data);
	else
		sprintf(tmp, "%d\n", data);
	sendToDebug_str(tmp);
}
void sendToDebug_int8(int8_t data, bool _inline)
{
	char tmp[50] = "";
	if (_inline)
		sprintf(tmp, "%d", data);
	else
		sprintf(tmp, "%d\n", data);
	sendToDebug_str(tmp);
}
void sendToDebug_int16(int16_t data, bool _inline)
{
	char tmp[50] = "";
	if (_inline)
		sprintf(tmp, "%d", data);
	else
		sprintf(tmp, "%d\n", data);
	sendToDebug_str(tmp);
}
void sendToDebug_int32(int32_t data, bool _inline)
{
	char tmp[50] = "";
	if (_inline)
		sprintf(tmp, "%d", data);
	else
		sprintf(tmp, "%d\n", data);
	sendToDebug_str(tmp);
}

void sendToDebug_float32(float32_t data, bool _inline)
{
	char tmp[50] = "";
	if (_inline)
		sprintf(tmp, "%f", (double)data);
	else
		sprintf(tmp, "%f\n", (double)data);
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

uint32_t getPhraseFromFrequency(int32_t freq) //высчитываем частоту из фразы ля FPGA
{
	if(freq < 0) return 0;
	bool inverted = false;
	int32_t _freq = freq;
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
	double res = round(((double)_freq / ADCDAC_CLOCK) * 4194304); //freq in hz/oscil in hz*2^bits = (freq/48000000)*4194304;
	return (uint32_t)res;
}

void addSymbols(char *dest, char *str, uint_fast8_t length, char *symbol, bool toEnd) //добавляем нули
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
	int32_t E;
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

void shiftTextLeft(char *string, uint_fast16_t shiftLength)
{
	uint_fast16_t size = strlen(string);
	if (shiftLength >= size)
	{
		memset(string, '\0', size);
		return;
	}
	for (uint_fast16_t i = 0; i < size - shiftLength; i++)
	{
		string[i] = string[i + shiftLength];
		string[i + shiftLength] = '\0';
	}
}

float32_t getMaxTXAmplitudeOnFreq(uint32_t freq)
{
	if (freq > MAX_TX_FREQ_HZ)
		return 0.0f;
	const uint_fast8_t calibration_points = 31;
	uint_fast8_t mhz_left = 0;
	uint_fast8_t mhz_right = calibration_points;
	for (uint_fast8_t i = 0; i <= calibration_points; i++)
		if ((i * 1000000) < freq)
			mhz_left = i;
	for (uint_fast8_t i = calibration_points; i > 0; i--)
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

static uint32_t CPULOAD_startWorkTime = 0;
static uint32_t CPULOAD_startSleepTime = 0;
static uint32_t CPULOAD_WorkingTime = 0;
static uint32_t CPULOAD_SleepingTime = 0;
static uint32_t CPULOAD_SleepCounter = 0;
static bool CPULOAD_status = true; // true - wake up ; false - sleep

void CPULOAD_Init(void)
{
	DBGMCU->CR |= (DBGMCU_CR_DBG_SLEEPD1_Msk | DBGMCU_CR_DBG_STOPD1_Msk | DBGMCU_CR_DBG_STANDBYD1_Msk);
	//разрешаем использовать счётчик
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	//запускаем счётчик
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	//обнуляем значение счётного регистра
	DWT->CYCCNT = 0;
	CPULOAD_status = true;
}

void CPULOAD_GoToSleepMode(void) {
	//if(!CPULOAD_status) return;
	/* Add to working time */
	CPULOAD_WorkingTime += DWT->CYCCNT - CPULOAD_startWorkTime;
	/* Save count cycle time */
	CPULOAD_SleepCounter++;
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
	CPU_LOAD.SINC = CPULOAD_SleepCounter;
	CPU_LOAD.Load = ((float)CPULOAD_WorkingTime / (float)(CPULOAD_SleepingTime + CPULOAD_WorkingTime) * 100);
	if(CPU_LOAD.SCNT==0)
	{
		CPU_LOAD.Load = 100;
	}
	if(CPU_LOAD.SCNT==0 && CPU_LOAD.WCNT==0)
	{
		CPU_LOAD.Load = 255;
		CPULOAD_Init();
	}
	/* Reset time */
	CPULOAD_SleepingTime = 0;
	CPULOAD_WorkingTime = 0;
	CPULOAD_SleepCounter = 0;
}

inline int32_t convertToSPIBigEndian(int32_t in)
{
	return (int32_t)(0xFFFF0000 & (uint32_t)(in << 16)) | (int32_t)(0x0000FFFF & (uint32_t)(in >> 16));
}

//Сортировка QuickSort из develop-ветки CMSIS
static int32_t arm_quick_sort_partition_f32(float32_t *pSrc, int32_t first, int32_t last, uint8_t dir)
{
    int32_t i, j, pivot_index;
    float32_t pivot;
    float32_t temp;
    pivot_index = first; 
    pivot = pSrc[pivot_index];
    i = first - 1;
    j = last + 1; 

    while(i < j) 
    {
        if(dir)
        {    
            do
            {
                i++; 
            } while (pSrc[i] < pivot && i<last);
            do
            {
                j--; 
            } while (pSrc[j] > pivot);
        }
        else
        {
            do
            {
                i++; 
            } while (pSrc[i] > pivot && i<last);
            do
            {
                j--; 
            } while (pSrc[j] < pivot);
        }
        if (i < j) 
        { 
            temp=pSrc[i];
            pSrc[i]=pSrc[j];
            pSrc[j]=temp;
        }
    }
    return j; 
}

static void arm_quick_sort_core_f32(float32_t *pSrc, int32_t first, int32_t last, uint8_t dir)
{
    if(first<last)
    {
        int32_t pivot;
        pivot = arm_quick_sort_partition_f32(pSrc, first, last, dir);
        arm_quick_sort_core_f32(pSrc, first,   pivot, dir);
        arm_quick_sort_core_f32(pSrc, pivot+1, last,  dir);
    }
}

void arm_quick_sort_f32(float32_t * pSrc, float32_t * pDst, uint32_t blockSize, uint8_t dir)
{
    float32_t * pA;
    if(pSrc != pDst) 
    {   
        memcpy(pDst, pSrc, blockSize*sizeof(float32_t) );
        pA = pDst;
    }
    else
        pA = pSrc;
    arm_quick_sort_core_f32(pA, 0, (int32_t)blockSize-1, dir);
}
