#ifndef PRE_DISTORTION_h
#define PRE_DISTORTION_h

#include "hardware.h"

#define DPD_POINTS 2
#define DPD_CORRECTION_GAIN_STEP 0.005f
#define DPD_CORRECTION_PHASE_STEP 0.0001f

void DPD_Init();
void DPD_ProcessPredistortion(float32_t *buffer_i, float32_t *buffer_q, uint32_t size);
void DPD_StartCalibration();
void DPD_ProcessCalibration();

#endif
