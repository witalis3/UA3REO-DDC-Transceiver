#include "vocoder.h"
#include "functions.h"
#include "sd.h"

IRAM2 int16_t VOCODER_InBuffer[SIZE_ADPCM_BLOCK] = {0};
uint16_t VOCODER_InBuffer_Index = 0;
void *ADPCM_cnxt = NULL;

void ADPCM_Init(void)
{
	int32_t average_deltas[2] = {0, 0};
	ADPCM_cnxt = adpcm_create_context(1, 2, NOISE_SHAPING_DYNAMIC, average_deltas); //num_channels, lookahead, noise_shaping, average_deltas
}

void VOCODER_Process(void)
{
	//encode audio
	uint32_t outbuff_size = 0;
	if (!SD_workbuffer_current)
		adpcm_encode_block(ADPCM_cnxt, (uint8_t *)&SD_workbuffer_A[SD_RecordBufferIndex], &outbuff_size, VOCODER_InBuffer, SIZE_ADPCM_BLOCK);
	else
		adpcm_encode_block(ADPCM_cnxt, (uint8_t *)&SD_workbuffer_B[SD_RecordBufferIndex], &outbuff_size, VOCODER_InBuffer, SIZE_ADPCM_BLOCK);
	SD_RecordBufferIndex += outbuff_size;
	if (SD_RecordBufferIndex == _MAX_SS)
	{
		SD_RecordBufferIndex = 0;
		SD_workbuffer_current = !SD_workbuffer_current;
		SD_doCommand(SDCOMM_PROCESS_RECORD, false);
	}
}
