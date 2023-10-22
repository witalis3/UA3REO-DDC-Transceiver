#ifndef __RFFC2072_PWR_MONITOR_H
#define __RFFC2072_PWR_MONITOR_H

#include "hardware.h"
#if HRDW_HAS_RFFC2072_MIXER

#define RFMIXER_MIN_FREQ_MHz 113  // 85 lo min + 28 if
#define RFMIXER_MAX_FREQ_MHz 2700 // 4gHz max with loss
#define RFMIXER_IF_FREQ_MHz 28

#define RFFC2072_REGS_NUM 31
#define RFFC2072_LO_MAX 5400000000
#define RFFC2072_REF_FREQ 39000000

extern void RFMIXER_Init(void);
extern void RFMIXER_disable(void);
extern void RFMIXER_enable(void);
extern uint64_t RFMIXER_Freq_Set(uint64_t lo_freq_Hz);

#endif
#endif
