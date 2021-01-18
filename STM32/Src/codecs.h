#ifndef CODECS_H
#define CODECS_H

#include "stm32h7xx_hal.h"

typedef struct {
// RIFF header
uint32_t riffsig;	  // Chunk ID - "RIFF" (0x52494646)
uint32_t filesize;    // file size
uint32_t wavesig;     // RIFF Type - "WAVE" (0x57415645)

// format chunk
uint32_t fmtsig;	  // Chunk ID - "fmt " (0x666D7420)
uint32_t fmtsize;     // Chunk Data Size - 16
uint16_t type;        // Compression code - WAVE_FORMAT_PCM = 1
uint16_t nch;         // Number of channels - 1
uint32_t freq;        // Sample rate
uint32_t rate;        // Data rate - Average Bytes Per Second
uint16_t block;       // BlockAlign = SignificantBitsPerSample / 8 * NumChannels; bytes in sample
uint16_t bits;        // Significant bits per sample - 8, 16, 24, 32.

// data chunk
uint32_t datasig;     // chunk ID - "data" (0x64617461)
uint32_t datasize;    // data length
uint8_t data[];       // data body

} WAV_header;

#endif
