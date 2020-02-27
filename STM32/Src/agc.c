#include "agc.h"
#include "stm32h7xx_hal.h"
#include "math.h"
#include "arm_math.h"
#include "functions.h"
#include "settings.h"
#include "profiler.h"

static float32_t AGC_need_gain_db = 0.0f;
static float32_t AGC_need_gain_db_old = 1.0f;
static float32_t RX_AGC_STEPSIZE_UP = 1.0f;
static float32_t RX_AGC_STEPSIZE_DOWN = 1.0f;

void InitAGC(void)
{
	//выше скорость в настройках - выше скорость отработки AGC
	RX_AGC_STEPSIZE_UP = 100.0f / (float32_t)TRX.RX_AGC_speed;
	RX_AGC_STEPSIZE_DOWN = 10.0f / (float32_t)TRX.RX_AGC_speed;
}

void DoAGC(float32_t *agcBuffer, uint_fast16_t blockSize)
{
	float32_t AGC_RX_magnitude = 0;
	float32_t full_scale_rate = 0;
	float32_t AGC_RX_dbFS = 0;

	//считаем магнитуду
	arm_power_f32(agcBuffer, blockSize, &AGC_RX_magnitude);
	AGC_RX_magnitude = AGC_RX_magnitude / blockSize;
	if (AGC_RX_magnitude == 0.0f)
		AGC_RX_magnitude = 0.001f;
	full_scale_rate = AGC_RX_magnitude / FLOAT_FULL_SCALE_POW;
	AGC_RX_dbFS = rate2dbP(full_scale_rate);

	//двигаем усиление на шаг
	float32_t diff = (AGC_OPTIMAL_THRESHOLD - (AGC_RX_dbFS + AGC_need_gain_db));
	if (diff > 0)
		AGC_need_gain_db += diff / RX_AGC_STEPSIZE_UP;
	else
		AGC_need_gain_db += diff / RX_AGC_STEPSIZE_DOWN;

	//перегрузка (клиппинг), резко снижаем усиление
	if ((AGC_RX_dbFS + AGC_need_gain_db) > AGC_CLIP_THRESHOLD)
	{
		AGC_need_gain_db = AGC_OPTIMAL_THRESHOLD - AGC_RX_dbFS;
		//sendToDebug_str("C");
	}

	//AGC выключен, ничего не регулируем
	if (!TRX.AGC)
		AGC_need_gain_db = 1.0f;

	//применяем усиление
	if (AGC_need_gain_db_old != AGC_need_gain_db) //усиление изменилось
	{
		float32_t gainApplyStep = 0;
		if (AGC_need_gain_db_old > AGC_need_gain_db)
			gainApplyStep = -(AGC_need_gain_db_old - AGC_need_gain_db) / blockSize;
		if (AGC_need_gain_db_old < AGC_need_gain_db)
			gainApplyStep = (AGC_need_gain_db - AGC_need_gain_db_old) / blockSize;
		for (uint_fast16_t i = 0; i < blockSize; i++)
		{
			AGC_need_gain_db_old += gainApplyStep;
			agcBuffer[i] = agcBuffer[i] * db2rateV(AGC_need_gain_db_old);
		}
	}
	else //усиление не менялось, применяем усиление по всем выборкам
	{
		arm_scale_f32(agcBuffer, db2rateP(AGC_need_gain_db), agcBuffer, blockSize);
	}
	//sendToDebug_float32(AGC_need_gain_db,false);
}
