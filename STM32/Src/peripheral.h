#ifndef Peripheral_h
#define Peripheral_h

#include "stm32f4xx_hal.h"
#include <stdbool.h>

#define MCP3008_THRESHOLD 10

typedef struct {
	//MCP3008 - 1
	bool key_ab;
	bool key_ab_prev;
	bool key_tune;
	bool key_tune_prev;
	bool key_preatt;
	bool key_preatt_prev;
	bool key_fast;
	bool key_fast_prev;
	bool key_modep;
	bool key_modep_prev;
	bool key_moden;
	bool key_moden_prev;
	bool key_bandp;
	bool key_bandp_prev;
	bool key_bandn;
	bool key_bandn_prev;
	//MCP3008 - 2
	bool key_f1;
	bool key_f1_prev;
	bool key_f2;
	bool key_f2_prev;
	bool key_f3;
	bool key_f3_prev;
	bool key_f4;
	bool key_f4_prev;
	bool key_f5;
	bool key_f5_prev;
	bool key_f6;
	bool key_f6_prev;
	uint16_t key_afgain; //10-bit
	uint16_t key_shift; //10-bit
	//
} PERIPH_FrontPanel_Type;

extern void PERIPH_ENCODER_checkRotate(void);
extern void PERIPH_ENCODER2_checkRotate(void);
extern void PERIPH_ENCODER2_checkSwitch(void);
extern void PERIPH_RF_UNIT_UpdateState(bool clean);
extern void PERIPH_ProcessFrontPanel(void);
extern bool PERIPH_SPI_Transmit(uint8_t* out_data, uint8_t* in_data, uint8_t count, GPIO_TypeDef* CS_PORT, uint16_t CS_PIN);

volatile extern PERIPH_FrontPanel_Type PERIPH_FrontPanel;

extern RTC_HandleTypeDef hrtc;

#endif
