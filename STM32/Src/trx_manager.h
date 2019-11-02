#ifndef TRX_MANAGER_H
#define TRX_MANAGER_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include "settings.h"

extern void TRX_Init(void);
extern void TRX_setFrequency(int32_t _freq, VFO* vfo);
extern uint32_t TRX_getFrequency(VFO* vfo);
extern void TRX_setMode(uint8_t _mode, VFO* vfo);
extern uint8_t TRX_getMode(VFO* vfo);
extern void TRX_ptt_change(void);
extern void TRX_key_change(void);
extern bool TRX_on_TX(void);
extern void TRX_DoAutoGain(void);
extern void TRX_Restart_Mode(void);
extern void TRX_DBMCalculate(void);
extern float32_t TRX_GetALC(void);

volatile extern bool TRX_ptt_hard;
volatile extern bool TRX_ptt_cat;
volatile extern bool TRX_old_ptt_cat;
volatile extern bool TRX_key_serial;
volatile extern bool TRX_old_key_serial;
volatile extern bool TRX_key_hard;
volatile extern uint16_t TRX_Key_Timeout_est;
volatile extern bool TRX_IQ_swap;
volatile extern bool TRX_Squelched;
volatile extern bool TRX_Tune;
volatile extern bool TRX_Inited;
volatile extern int16_t TRX_RX_dBm;
volatile extern bool TRX_ADC_OTR;
volatile extern bool TRX_DAC_OTR;
volatile extern int16_t TRX_ADC_MINAMPLITUDE;
volatile extern int16_t TRX_ADC_MAXAMPLITUDE;
volatile extern uint8_t TRX_Time_InActive;
volatile extern uint8_t TRX_Fan_Timeout;
volatile extern bool TRX_SNMP_Synced;
volatile extern int16_t TRX_SHIFT;
volatile extern float32_t TRX_MAX_TX_Amplitude;

extern const char *MODE_DESCR[];
extern ADC_HandleTypeDef hadc1;

#endif
