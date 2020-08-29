#include "stm32h7xx_hal.h"
#include "main.h"
#include "rf_unit.h"
#include "lcd.h"
#include "trx_manager.h"
#include "settings.h"
#include "system_menu.h"
#include "functions.h"
#include "audio_filters.h"

static uint8_t getBPFByFreq(uint32_t freq)
{
	if (freq >= CALIBRATE.BPF_0_START && freq < CALIBRATE.BPF_0_END)
		return 0;
	if (freq >= CALIBRATE.BPF_1_START && freq < CALIBRATE.BPF_1_END)
		return 1;
	if (freq >= CALIBRATE.BPF_2_START && freq < CALIBRATE.BPF_2_END)
		return 2;
	if (freq >= CALIBRATE.BPF_3_START && freq < CALIBRATE.BPF_3_END)
		return 3;
	if (freq >= CALIBRATE.BPF_4_START && freq < CALIBRATE.BPF_4_END)
		return 4;
	if (freq >= CALIBRATE.BPF_5_START && freq < CALIBRATE.BPF_5_END)
		return 5;
	if (freq >= CALIBRATE.BPF_6_START && freq < CALIBRATE.BPF_6_END)
		return 6;
	if (freq >= CALIBRATE.BPF_7_HPF)
		return 7;
	return 255;
}

void PERIPH_RF_UNIT_UpdateState(bool clean) //передаём значения в RF-UNIT
{
	bool hpf_lock = false;

	bool dualrx_lpf_disabled = false;
	bool dualrx_bpf_disabled = false;
	if ((TRX.Dual_RX_Type != VFO_SEPARATE) && SecondaryVFO()->Freq > CALIBRATE.LPF_END)
		dualrx_lpf_disabled = true;
	if ((TRX.Dual_RX_Type != VFO_SEPARATE) && getBPFByFreq(CurrentVFO()->Freq) != getBPFByFreq(SecondaryVFO()->Freq))
		dualrx_bpf_disabled = true;

	HAL_GPIO_WritePin(RFUNIT_RCLK_GPIO_Port, RFUNIT_RCLK_Pin, GPIO_PIN_RESET); //защёлка
	MINI_DELAY
	for (uint8_t registerNumber = 0; registerNumber < 24; registerNumber++)
	{
		HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_RESET); //клок данных
		MINI_DELAY
		HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_RESET); //данные
		MINI_DELAY
		if (!clean)
		{
			//REGISTER 1
			//if(registerNumber==0) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // unused
			if (registerNumber == 1 && !TRX_on_TX() && TRX.LNA)
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // LNA
			//if(registerNumber==2) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // unused
			//if(registerNumber==3) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // unused
			//if(registerNumber==4) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // unused
			//if(registerNumber==5) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // unused
			//if(registerNumber==6) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // unused
			if (registerNumber == 7 && TRX_on_TX() && CurrentVFO()->Mode != TRX_MODE_LOOPBACK)
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); //TX_RX

			//REGISTER 2
			if (registerNumber == 8 && TRX_on_TX() && CurrentVFO()->Mode != TRX_MODE_LOOPBACK) //TX_AMP
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			if (registerNumber == 9 && TRX.ATT) //ATT_ON
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			if (registerNumber == 10 && (!TRX.LPF || CurrentVFO()->Freq > CALIBRATE.LPF_END || dualrx_lpf_disabled)) //LPF_OFF
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			if (registerNumber == 11 && (!TRX.BPF || CurrentVFO()->Freq < CALIBRATE.BPF_1_START || dualrx_bpf_disabled)) //BPF_OFF
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			if (registerNumber == 12 && TRX.BPF && getBPFByFreq(CurrentVFO()->Freq) == 0) //BPF_0
			{
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				hpf_lock = true; //блокируем HPF для выделенного BPF фильтра УКВ
			}
			if (registerNumber == 13 && TRX.BPF && getBPFByFreq(CurrentVFO()->Freq) == 7 && !hpf_lock && !dualrx_bpf_disabled)
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); //BPF_7_HPF
			if (registerNumber == 14 && TRX.BPF && getBPFByFreq(CurrentVFO()->Freq) == 6 && !dualrx_bpf_disabled)
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); //BPF_6
			//if(registerNumber==15) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // unused

			//REGISTER 3
			//if(registerNumber==16) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // unused
			//if(registerNumber==17) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // unused
			if (registerNumber == 18 && ((TRX_on_TX() && CurrentVFO()->Mode != TRX_MODE_LOOPBACK) || TRX_Fan_Timeout > 0)) //FAN
			{
				if(TRX_Fan_Timeout < (30 * 100)) //PWM
				{
					const uint8_t on_ticks = 1;
					const uint8_t off_ticks = 1;
					static bool pwm_status = false; //true - on false - off
					static uint8_t pwm_ticks = 0;
					pwm_ticks++;
					if(pwm_status)
						HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
					if((pwm_status && pwm_ticks==on_ticks) || (!pwm_status && pwm_ticks==off_ticks))
					{
						pwm_status = !pwm_status;
						pwm_ticks = 0;
					}
				}
				else
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				if (TRX_Fan_Timeout > 0)
					TRX_Fan_Timeout--;
			}
			if (registerNumber == 19 && TRX.BPF && getBPFByFreq(CurrentVFO()->Freq) == 1 && !dualrx_bpf_disabled) //BPF_1
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			if (registerNumber == 20 && TRX.BPF && getBPFByFreq(CurrentVFO()->Freq) == 2 && !dualrx_bpf_disabled) //BPF_2
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			if (registerNumber == 21 && TRX.BPF && getBPFByFreq(CurrentVFO()->Freq) == 3 && !dualrx_bpf_disabled) //BPF_3
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			if (registerNumber == 22 && TRX.BPF && getBPFByFreq(CurrentVFO()->Freq) == 4 && !dualrx_bpf_disabled) //BPF_4
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			if (registerNumber == 23 && TRX.BPF && getBPFByFreq(CurrentVFO()->Freq) == 5 && !dualrx_bpf_disabled) //BPF_5
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
		}
		MINI_DELAY
		HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_SET);
	}
	MINI_DELAY
	HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_RESET);
	MINI_DELAY
	HAL_GPIO_WritePin(RFUNIT_RCLK_GPIO_Port, RFUNIT_RCLK_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RFUNIT_OE_GPIO_Port, RFUNIT_OE_Pin, GPIO_PIN_RESET);
}

void PERIPH_ProcessSWRMeter(void)
{
	float32_t forward = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2) * TRX_STM32_VREF / 65535.0f;
	float32_t backward = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1) * TRX_STM32_VREF / 65535.0f;
	static float32_t TRX_VLT_forward = 0.0f;
	static float32_t TRX_VLT_backward = 0.0f;
	
	forward = forward / (510.0f / (0.1f + 510.0f)); //корректируем напряжение исходя из делителя напряжения (0.1ом и 510ом)
	if (forward < 0.01f)							//меньше 10mV не измеряем
	{
		TRX_VLT_forward = 0.0f;
		TRX_VLT_backward = 0.0f;
		TRX_SWR = 1.0f;
		return;
	}

	forward += 0.62f;							  // падение на диоде
	forward = forward * CALIBRATE.swr_trans_rate; // Коэффициент трансформации КСВ метра

	backward = backward / (510.0f / (0.1f + 510.0f)); //корректируем напряжение исходя из делителя напряжения (0.1ом и 510ом)
	if (backward >= 0.01f)							  //меньше 10mV не измеряем
	{
		backward += 0.62f;								// падение на диоде
		backward = backward * CALIBRATE.swr_trans_rate; //Коэффициент трансформации КСВ метра
	}
	else
		backward = 0.001f;

	TRX_VLT_forward = TRX_VLT_forward + (forward - TRX_VLT_forward) / 2;
	TRX_VLT_backward = TRX_VLT_backward + (backward - TRX_VLT_backward) / 2;
	TRX_SWR = (TRX_VLT_forward + TRX_VLT_backward) / (TRX_VLT_forward - TRX_VLT_backward);

	if (TRX_VLT_backward > TRX_VLT_forward)
		TRX_SWR = 10.0f;
	if (TRX_SWR > 10.0f)
		TRX_SWR = 10.0f;
	
	TRX_PWR_Forward = (TRX_VLT_forward * TRX_VLT_forward) / 50.0f;
	if (TRX_PWR_Forward < 0.0f)
			TRX_PWR_Forward = 0.0f;
	TRX_PWR_Backward = (TRX_VLT_backward * TRX_VLT_backward) / 50.0f;
	if (TRX_PWR_Backward < 0.0f)
			TRX_PWR_Backward = 0.0f;

	LCD_UpdateQuery.StatusInfoBar = true;
}
