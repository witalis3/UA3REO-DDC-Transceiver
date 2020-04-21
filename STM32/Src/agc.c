#include "stm32h7xx_hal.h"
#include "agc.h"
#include "settings.h"

//Private variables
static float32_t AGC_RX1_need_gain_db = 0.0f;
static float32_t AGC_RX2_need_gain_db = 0.0f;
static float32_t AGC_RX1_need_gain_db_old = 1.0f;
static float32_t AGC_RX2_need_gain_db_old = 1.0f;

//запуск AGC на блок данных
void DoAGC(float32_t *agcBuffer, uint_fast16_t blockSize, AUDIO_PROC_RX_NUM rx_id)
{
	//определение RX1 или RX2 приёмника
	float32_t *AGC_need_gain_db = &AGC_RX1_need_gain_db;
	float32_t *AGC_need_gain_db_old = &AGC_RX1_need_gain_db_old;
	if (rx_id == AUDIO_RX2)
	{
		AGC_need_gain_db = &AGC_RX2_need_gain_db;
		AGC_need_gain_db_old = &AGC_RX2_need_gain_db_old;
	}

	//выше скорость в настройках - выше скорость отработки AGC
	float32_t RX_AGC_STEPSIZE_UP = 100.0f / (float32_t)TRX.RX_AGC_speed;
	float32_t RX_AGC_STEPSIZE_DOWN = 10.0f / (float32_t)TRX.RX_AGC_speed;

	//считаем магнитуду в dBFS
	float32_t AGC_RX_magnitude = 0;
	arm_power_f32(agcBuffer, blockSize, &AGC_RX_magnitude);
	AGC_RX_magnitude = AGC_RX_magnitude / (float32_t)blockSize;
	if (AGC_RX_magnitude == 0.0f)
		AGC_RX_magnitude = 0.001f;
	float32_t full_scale_rate = AGC_RX_magnitude / FLOAT_FULL_SCALE_POW;
	float32_t AGC_RX_dbFS = rate2dbP(full_scale_rate);

	//двигаем усиление на шаг
	float32_t diff = (AGC_OPTIMAL_THRESHOLD - (AGC_RX_dbFS + *AGC_need_gain_db));
	if (diff > 0)
		*AGC_need_gain_db += diff / RX_AGC_STEPSIZE_UP;
	else
		*AGC_need_gain_db += diff / RX_AGC_STEPSIZE_DOWN;

	//перегрузка (клиппинг), резко снижаем усиление
	if ((AGC_RX_dbFS + *AGC_need_gain_db) > AGC_CLIP_THRESHOLD)
		*AGC_need_gain_db = AGC_OPTIMAL_THRESHOLD - AGC_RX_dbFS;

	//шумовой порог, ниже него - не усиливаем
	if (AGC_RX_dbFS < AGC_NOISE_GATE)
		*AGC_need_gain_db = 1.0f;

	//AGC выключен, ничего не регулируем
	if ((rx_id == AUDIO_RX1 && !CurrentVFO()->AGC) || (rx_id == AUDIO_RX2 && !SecondaryVFO()->AGC))
		*AGC_need_gain_db = 1.0f;

	//ограничение по усилению
	if (*AGC_need_gain_db > AGC_MAX_GAIN)
		*AGC_need_gain_db = AGC_MAX_GAIN;

	//применяем усиление
	if (fabsf(*AGC_need_gain_db_old - *AGC_need_gain_db) > 0.00001f) //усиление изменилось
	{
		float32_t gainApplyStep = 0;
		if (*AGC_need_gain_db_old > *AGC_need_gain_db)
			gainApplyStep = -(*AGC_need_gain_db_old - *AGC_need_gain_db) / (float32_t)blockSize;
		if (*AGC_need_gain_db_old < *AGC_need_gain_db)
			gainApplyStep = (*AGC_need_gain_db - *AGC_need_gain_db_old) / (float32_t)blockSize;
		for (uint_fast16_t i = 0; i < blockSize; i++)
		{
			*AGC_need_gain_db_old += gainApplyStep;
			agcBuffer[i] = agcBuffer[i] * db2rateV(*AGC_need_gain_db_old);
		}
	}
	else //усиление не менялось, применяем усиление по всем выборкам
	{
		arm_scale_f32(agcBuffer, db2rateP(*AGC_need_gain_db), agcBuffer, blockSize);
	}
}
