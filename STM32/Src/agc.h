#ifndef AGC_H
#define AGC_H

#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include "audio_processor.h"

#define AGC_RINGBUFFER_TAPS_SIZE 3
#define AGC_HOLDTIME_STEP 5
#define AGC_HOLDTIME_LIMITER_DB 10.0f

typedef struct
{
	float32_t need_gain_db;
	float32_t need_gain_db_old;
	float32_t AGC_RX_dbFS_old;
	uint32_t last_agc_peak_time;
	float32_t agcBuffer_kw[AUDIO_BUFFER_HALF_SIZE];
	float32_t ringbuffer[AGC_RINGBUFFER_TAPS_SIZE * AUDIO_BUFFER_HALF_SIZE];
	uint16_t hold_time;
} AGC_RX_Instance;

typedef struct
{
	float32_t need_gain_db;
	float32_t need_gain_db_old;
} AGC_TX_Instance;

// Public methods
extern void DoRxAGC(float32_t *agcbuffer_i, float32_t *agcbuffer_q, uint_fast16_t blockSize, AUDIO_PROC_RX_NUM rx_id, uint_fast8_t mode, bool stereo); // start RX AGC on a data block
extern void DoTxAGC(float32_t *agcbuffer_i, uint_fast16_t blockSize, float32_t target, uint_fast8_t mode);											   // start TX AGC on a data block
extern void ResetAGC(void);

#endif
