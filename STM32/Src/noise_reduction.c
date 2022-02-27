#include "noise_reduction.h"
#include "arm_const_structs.h"
#include "settings.h"
#include "speex/speex_preprocess.h"
#include "lcd.h"

SpeexPreprocessState *SPEEX_state_handle_RX1;
SpeexPreprocessState *SPEEX_state_handle_RX2;
static spx_int32_t SPEEX_supress = -20;
static int SPEEX_allocated = 0;
static SRAM uint8_t SPEEX_heapbuff [SPEEXALLOCSIZE];

// initialize DNR
void InitNoiseReduction(void)
{
	//Speex
	static bool speex_inited = false;
	if(!speex_inited) 
	{
		speex_inited = true;
		SPEEX_state_handle_RX1 = speex_preprocess_state_init(NOISE_REDUCTION_BLOCK_SIZE, TRX_SAMPLERATE);
		SPEEX_state_handle_RX2 = speex_preprocess_state_init(NOISE_REDUCTION_BLOCK_SIZE, TRX_SAMPLERATE);
		spx_int32_t denoise = 1;
		speex_preprocess_ctl(SPEEX_state_handle_RX1, SPEEX_PREPROCESS_SET_DENOISE, &denoise);
		speex_preprocess_ctl(SPEEX_state_handle_RX2, SPEEX_PREPROCESS_SET_DENOISE, &denoise);
	}
}

// run DNR for the data block
void processNoiseReduction(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id)
{
	SpeexPreprocessState *SPEEX_state_handle = SPEEX_state_handle_RX1;
	if (rx_id == AUDIO_RX2)
		SPEEX_state_handle = SPEEX_state_handle_RX1;
	
	SPEEX_supress = TRX.DNR_LEVEL;
	speex_preprocess_ctl(SPEEX_state_handle, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &SPEEX_supress);
	
	arm_scale_f32(buffer, 32768.0f, buffer, NOISE_REDUCTION_BLOCK_SIZE);
	speex_preprocess_run(SPEEX_state_handle, buffer);
	arm_scale_f32(buffer, 1.0f / 32768.0f, buffer, NOISE_REDUCTION_BLOCK_SIZE);
}

void *speex_alloc (int size)
{
	size = (size + 0x03) & ~ 0x03;
	if (! ((SPEEX_allocated + size) <= sizeof SPEEX_heapbuff / sizeof SPEEX_heapbuff [0]))
	{
		LCD_showError("SPEEX mem error", true);
	}
	void * p = (void *) (SPEEX_heapbuff + SPEEX_allocated);
	SPEEX_allocated += size;
	return p;
}

void speex_free (void *ptr)
{
}
