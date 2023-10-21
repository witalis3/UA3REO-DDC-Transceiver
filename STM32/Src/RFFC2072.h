#ifndef __RFFC2072_PWR_MONITOR_H
#define __RFFC2072_PWR_MONITOR_H

#include "hardware.h"
#if HRDW_HAS_RFFC2072_MIXER

#define RFFC2072_REGS_NUM 31
#define RFFC2072_LO_MAX 5400ULL
#define RFFC2072_REF_FREQ 39ULL
#define RFFC2072_FREQ_ONE_MHZ (1000ULL * 1000ULL)

extern void RFMIXER_Init(void);
extern void RFMIXER_disable(void);
extern void RFMIXER_enable(void);
extern uint64_t RFMIXER_Freq_Set(uint64_t lo_MHz);

#endif
#endif
