#include "decoder.h"
#include "cw_decoder.h"

//Private variables
static SRAM float32_t DECODER_Buffer[DECODER_BUFF_SIZE] = {0};
static uint32_t DECODER_head = 0; // index of adding new data to the buffer
static uint32_t DECODER_tail = 0; // index of reading data from the buffer

void DECODER_Init(void)
{
	CWDecoder_Init();
}

void DECODER_PutSamples(float32_t *bufferIn, uint32_t size)
{
	//SCB_CleanDCache_by_Addr((uint32_t *)bufferIn, size * 4);
	if ((DECODER_head + size) <= DECODER_BUFF_SIZE)
	{
		dma_memcpy32((uint32_t *)&DECODER_Buffer[DECODER_head], (uint32_t *)bufferIn, size);
		DECODER_head += size;
		if (DECODER_head >= DECODER_BUFF_SIZE)
			DECODER_head = 0;
	}
	else
	{
		uint32_t firstpart = DECODER_BUFF_SIZE - DECODER_head;
		dma_memcpy32((uint32_t *)&DECODER_Buffer[DECODER_head], (uint32_t *)bufferIn, firstpart);
		DECODER_head = 0;
		dma_memcpy32((uint32_t *)&DECODER_Buffer[DECODER_head], (uint32_t *)bufferIn, (size - firstpart));
		DECODER_head += (size - firstpart);
	}
}

void DECODER_Process(void)
{
	if (DECODER_tail == DECODER_head) //overrun
	{
		//sendToDebug_str("o");
		return;
	}
	// get data from the buffer
	float32_t *bufferOut = &DECODER_Buffer[DECODER_tail];
	DECODER_tail += DECODER_PACKET_SIZE;
	if (DECODER_tail >= DECODER_BUFF_SIZE)
		DECODER_tail = 0;

	//CW Decoder
	if (TRX.CWDecoder && (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U || CurrentVFO()->Mode == TRX_MODE_LOOPBACK))
		CWDecoder_Process(bufferOut);
}
