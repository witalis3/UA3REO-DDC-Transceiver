#ifndef PRE_DISTORTION_h
#define PRE_DISTORTION_h

#include "hardware.h"

#define DPD_POINTS 3
#define DPD_CORRECTION_GAIN_STEP 0.001f

void DPD_Init();
void DPD_ProcessPredistortion(float32_t *buffer_i, float32_t *buffer_q, uint32_t size);
void DPD_StartCalibration();
void DPD_ProcessCalibration();

#endif
