#ifndef CW_H
#define CW_H

#include "hardware.h"
#include <stdbool.h>
#include "settings.h"

#define CW_EDGES_SMOOTH (1.0f/((float32_t)TRX_SAMPLERATE / 1000.0f * 5.0f)) //5 msec

extern void CW_key_change(void);
extern float32_t CW_GenerateSignal(float32_t power);

volatile extern bool CW_key_serial;
volatile extern bool CW_old_key_serial;
volatile extern bool CW_key_dot_hard;
volatile extern bool CW_key_dash_hard;
volatile extern uint_fast8_t KEYER_symbol_status;

#endif
