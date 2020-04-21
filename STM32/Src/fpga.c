#include "fpga.h"
#include "main.h"

#if FPGA_FLASH_IN_HEX
	#include "fpga_flash.h"
#endif

//Public variables
volatile uint32_t FPGA_samples = 0;												//счетчик числа семплов при обмене с FPGA
volatile bool FPGA_NeedSendParams = false;										//флаг необходимости отправить параметры в FPGA
volatile bool FPGA_NeedGetParams = false;										//флаг необходимости получить параметры из FPGA
volatile bool FPGA_NeedRestart = true;											//флаг необходимости рестарта модулей FPGA
volatile bool FPGA_Buffer_underrun = false;										//флаг недостатка данных из FPGA
uint_fast16_t FPGA_Audio_Buffer_Index = 0;										//текущий индекс в буфферах FPGA
bool FPGA_Audio_Buffer_State = true;											//состояние буффера, заполнена половина или целиком true - compleate ; false - half
SRAM1 volatile float32_t FPGA_Audio_Buffer_RX1_Q[FPGA_AUDIO_BUFFER_SIZE] = {0}; //буфферы FPGA
SRAM1 volatile float32_t FPGA_Audio_Buffer_RX1_I[FPGA_AUDIO_BUFFER_SIZE] = {0};
SRAM1 volatile float32_t FPGA_Audio_Buffer_RX2_Q[FPGA_AUDIO_BUFFER_SIZE] = {0};
SRAM1 volatile float32_t FPGA_Audio_Buffer_RX2_I[FPGA_AUDIO_BUFFER_SIZE] = {0};
SRAM1 volatile float32_t FPGA_Audio_SendBuffer_Q[FPGA_AUDIO_BUFFER_SIZE] = {0};
SRAM1 volatile float32_t FPGA_Audio_SendBuffer_I[FPGA_AUDIO_BUFFER_SIZE] = {0};

//Private variables
static GPIO_InitTypeDef FPGA_GPIO_InitStruct; //структура GPIO портов
static bool FPGA_bus_direction = false;		  //текущее направление шины обмена false - OUT; true - in
static bool FPGA_bus_stop = false;					//приостановка работы шины FPGA
	
//Prototypes
static uint_fast8_t FPGA_readPacket(void);		   //чтение пакета
static void FPGA_writePacket(uint_fast8_t packet); //запись пакета
static void FPGA_clockFall(void);				   //снять сигнал CLK
static void FPGA_clockRise(void);				   //поднять сигнал CLK
static void FPGA_syncRise(void);				   //поднять сигнал SYNC
static void FPGA_syncFall(void);				   //снять сигнал SYNC
static void FPGA_fpgadata_sendiq(void);			   //отправить IQ данные
static void FPGA_fpgadata_getiq(void);			   //получить IQ данные
static void FPGA_fpgadata_getparam(void);		   //получить параметры
static void FPGA_fpgadata_sendparam(void);		   //отправить параметры
static void FPGA_setBusInput(void);				   //переключить шину на ввод
static void FPGA_setBusOutput(void);			   //переключить шину на вывод
#if FPGA_FLASH_IN_HEX
static bool FPGA_spi_flash_verify(bool full);				//Прочитать содержимое SPI памяти FPGA
#endif

//инициализация обмена с FPGA
void FPGA_Init(void)
{
	FPGA_GPIO_InitStruct.Pin = FPGA_BUS_D0_Pin | FPGA_BUS_D1_Pin | FPGA_BUS_D2_Pin | FPGA_BUS_D3_Pin | FPGA_BUS_D4_Pin | FPGA_BUS_D5_Pin | FPGA_BUS_D6_Pin | FPGA_BUS_D7_Pin;
	FPGA_GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	FPGA_GPIO_InitStruct.Pull = GPIO_PULLUP;
	FPGA_GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOA, &FPGA_GPIO_InitStruct);

	#if FPGA_FLASH_IN_HEX
	FPGA_spi_flash_verify(false); //проверяем первые 2048 байт прошивки FPGA
	#endif
}

//перезапуск модулей FPGA
void FPGA_restart(void) //перезапуск модулей FPGA
{
	FPGA_writePacket(5); //RESET ON
	FPGA_syncRise();
	FPGA_clockRise();
	FPGA_syncFall();
	FPGA_clockFall();
	HAL_Delay(100);
	FPGA_writePacket(6); //RESET OFF
	FPGA_syncRise();
	FPGA_clockRise();
	FPGA_syncFall();
	FPGA_clockFall();
}

//обмен параметрами с FPGA
void FPGA_fpgadata_stuffclock(void)
{
	if(FPGA_bus_stop) return;
	uint_fast8_t FPGA_fpgadata_out_tmp8 = 0;
	//обмен данными

	//STAGE 1
	//out
	if (FPGA_NeedSendParams)
		FPGA_fpgadata_out_tmp8 = 1;
	else if (FPGA_NeedGetParams)
		FPGA_fpgadata_out_tmp8 = 2;

	if (FPGA_fpgadata_out_tmp8 != 0)
	{
		FPGA_writePacket(FPGA_fpgadata_out_tmp8);
		//clock
		FPGA_syncRise();
		FPGA_clockRise();
		//in
		//clock
		FPGA_syncFall();
		FPGA_clockFall();

		if (FPGA_NeedSendParams)
		{
			FPGA_fpgadata_sendparam();
			FPGA_NeedSendParams = false;
		}
		else if (FPGA_NeedRestart)
		{
			FPGA_restart();
			FPGA_NeedRestart = false;
		}
		else if (FPGA_NeedGetParams)
		{
			FPGA_fpgadata_getparam();
			FPGA_NeedGetParams = false;
		}
	}
}

//обмен IQ данными с FPGA
void FPGA_fpgadata_iqclock(void)
{
	if(FPGA_bus_stop) return;
	uint_fast8_t FPGA_fpgadata_out_tmp8 = 0;
	VFO *current_vfo = CurrentVFO();
	//обмен данными

	//STAGE 1
	//out
	if (TRX_on_TX() && current_vfo->Mode != TRX_MODE_LOOPBACK)
		FPGA_fpgadata_out_tmp8 = 3;
	else
		FPGA_fpgadata_out_tmp8 = 4;

	FPGA_writePacket(FPGA_fpgadata_out_tmp8);
	//clock
	FPGA_syncRise();
	FPGA_clockRise();
	//in
	//clock
	FPGA_syncFall();
	FPGA_clockFall();

	if (TRX_on_TX() && current_vfo->Mode != TRX_MODE_LOOPBACK)
		FPGA_fpgadata_sendiq();
	else
		FPGA_fpgadata_getiq();
}

//отправить параметры
static inline void FPGA_fpgadata_sendparam(void)
{
	uint_fast8_t FPGA_fpgadata_out_tmp8 = 0;
	VFO *current_vfo = CurrentVFO();
	VFO *secondary_vfo = SecondaryVFO();
	uint32_t TRX_freq_phrase = getPhraseFromFrequency((int32_t)current_vfo->Freq + TRX_SHIFT);
	uint32_t TRX_freq_phrase2 = getPhraseFromFrequency((int32_t)secondary_vfo->Freq + TRX_SHIFT);
	if (!TRX_on_TX())
	{
		switch (current_vfo->Mode)
		{
		case TRX_MODE_CW_L:
			TRX_freq_phrase = getPhraseFromFrequency((int32_t)current_vfo->Freq + TRX_SHIFT + TRX.CW_GENERATOR_SHIFT_HZ);
			break;
		case TRX_MODE_CW_U:
			TRX_freq_phrase = getPhraseFromFrequency((int32_t)current_vfo->Freq + TRX_SHIFT - TRX.CW_GENERATOR_SHIFT_HZ);
			break;
		default:
			break;
		}
		switch (secondary_vfo->Mode)
		{
		case TRX_MODE_CW_L:
			TRX_freq_phrase2 = getPhraseFromFrequency((int32_t)secondary_vfo->Freq + TRX_SHIFT + TRX.CW_GENERATOR_SHIFT_HZ);
			break;
		case TRX_MODE_CW_U:
			TRX_freq_phrase2 = getPhraseFromFrequency((int32_t)secondary_vfo->Freq + TRX_SHIFT - TRX.CW_GENERATOR_SHIFT_HZ);
			break;
		default:
			break;
		}
	}
	//STAGE 2
	//out PTT+PREAMP
	bitWrite(FPGA_fpgadata_out_tmp8, 0, (!TRX.ADC_SHDN && !TRX_on_TX() && current_vfo->Mode != TRX_MODE_LOOPBACK));										//RX1
	bitWrite(FPGA_fpgadata_out_tmp8, 1, (!TRX.ADC_SHDN && TRX.Dual_RX_Type != VFO_SEPARATE && !TRX_on_TX() && current_vfo->Mode != TRX_MODE_LOOPBACK)); //RX2
	bitWrite(FPGA_fpgadata_out_tmp8, 2, (TRX_on_TX() && current_vfo->Mode != TRX_MODE_LOOPBACK));														//TX
	bitWrite(FPGA_fpgadata_out_tmp8, 3, TRX.ADC_DITH);
	bitWrite(FPGA_fpgadata_out_tmp8, 4, TRX.ADC_SHDN);
	if (TRX_on_TX())
		bitWrite(FPGA_fpgadata_out_tmp8, 4, true); //shutdown ADC on TX
	bitWrite(FPGA_fpgadata_out_tmp8, 5, TRX.ADC_RAND);
	bitWrite(FPGA_fpgadata_out_tmp8, 6, TRX.ADC_PGA);
	if (!TRX_on_TX())
		bitWrite(FPGA_fpgadata_out_tmp8, 7, TRX.ADC_Driver);
	FPGA_writePacket(FPGA_fpgadata_out_tmp8);
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 3
	//out RX1-FREQ
	FPGA_writePacket(((TRX_freq_phrase & (0XFF << 16)) >> 16));
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 4
	//OUT RX1-FREQ
	FPGA_writePacket(((TRX_freq_phrase & (0XFF << 8)) >> 8));
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 5
	//OUT RX1-FREQ
	FPGA_writePacket(TRX_freq_phrase & 0XFF);
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 6
	//out RX2-FREQ
	FPGA_writePacket(((TRX_freq_phrase2 & (0XFF << 16)) >> 16));
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 7
	//OUT RX2-FREQ
	FPGA_writePacket(((TRX_freq_phrase2 & (0XFF << 8)) >> 8));
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 8
	//OUT RX2-FREQ
	FPGA_writePacket(TRX_freq_phrase2 & 0XFF);
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 9
	//OUT CIC-GAIN
	FPGA_writePacket(CALIBRATE.CIC_GAINER_val);
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 10
	//OUT CICCOMP-GAIN
	FPGA_writePacket(CALIBRATE.CICFIR_GAINER_val);
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 11
	//OUT TX-CICCOMP-GAIN
	FPGA_writePacket(CALIBRATE.TXCICFIR_GAINER_val);
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 12
	//OUT DAC-GAIN
	FPGA_writePacket(CALIBRATE.DAC_GAINER_val);
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 13
	//OUT ADC OFFSET
	FPGA_writePacket(((CALIBRATE.adc_offset & (0XFF << 8)) >> 8));
	FPGA_clockRise();
	FPGA_clockFall();

	//STAGE 14
	//OUT ADC OFFSET
	FPGA_writePacket(CALIBRATE.adc_offset & 0XFF);
	FPGA_clockRise();
	FPGA_clockFall();
}

//получить параметры
static inline void FPGA_fpgadata_getparam(void)
{
	register uint_fast8_t FPGA_fpgadata_in_tmp8 = 0;

	//STAGE 2
	//clock
	FPGA_clockRise();
	//in
	FPGA_fpgadata_in_tmp8 = FPGA_readPacket();
	TRX_ADC_OTR = bitRead(FPGA_fpgadata_in_tmp8, 0);
	TRX_DAC_OTR = bitRead(FPGA_fpgadata_in_tmp8, 1);
	//clock
	FPGA_clockFall();

	//STAGE 3
	//clock
	FPGA_clockRise();
	//in
	FPGA_fpgadata_in_tmp8 = FPGA_readPacket();
	//clock
	FPGA_clockFall();
	//STAGE 4
	//clock
	FPGA_clockRise();
	//in
	TRX_ADC_MINAMPLITUDE = (int16_t)(((FPGA_fpgadata_in_tmp8 << 8) & 0xFF00) | (FPGA_readPacket() & 0xFF));
	//clock
	FPGA_clockFall();

	//STAGE 5
	//clock
	FPGA_clockRise();
	//in
	FPGA_fpgadata_in_tmp8 = FPGA_readPacket();
	//clock
	FPGA_clockFall();
	//STAGE 6
	//clock
	FPGA_clockRise();
	//in
	TRX_ADC_MAXAMPLITUDE = (int16_t)(((FPGA_fpgadata_in_tmp8 << 8) & 0xFF00) | (FPGA_readPacket() & 0xFF));
	//clock
	FPGA_clockFall();
}

//получить IQ данные
static inline void FPGA_fpgadata_getiq(void)
{
	q31_t FPGA_fpgadata_in_tmp32 = 0;
	float32_t FPGA_fpgadata_in_float32 = 0;
	FPGA_samples++;

	//STAGE 2 in Q RX1
	FPGA_clockRise();
	FPGA_fpgadata_in_tmp32 = (q31_t)((FPGA_readPacket() & 0xFF) << 24);
	FPGA_clockFall();

	//STAGE 3
	FPGA_clockRise();
	FPGA_fpgadata_in_tmp32 |= (q31_t)((FPGA_readPacket() & 0xFF) << 16);
	FPGA_clockFall();

	//STAGE 4
	FPGA_clockRise();
	FPGA_fpgadata_in_tmp32 |= (q31_t)((FPGA_readPacket() & 0xFF) << 8);
	FPGA_clockFall();

	//STAGE 5
	FPGA_clockRise();
	FPGA_fpgadata_in_tmp32 |= (q31_t)((FPGA_readPacket() & 0xFF) << 0);

	FPGA_fpgadata_in_float32 = (float32_t)FPGA_fpgadata_in_tmp32 / 2147483648.0f;
	if (TRX_IQ_swap)
	{
		if (NeedFFTInputBuffer)
			FFTInput_I[FFT_buff_index] = FPGA_fpgadata_in_float32;
		FPGA_Audio_Buffer_RX1_I[FPGA_Audio_Buffer_Index] = FPGA_fpgadata_in_float32;
	}
	else
	{
		if (NeedFFTInputBuffer)
			FFTInput_Q[FFT_buff_index] = FPGA_fpgadata_in_float32;
		FPGA_Audio_Buffer_RX1_Q[FPGA_Audio_Buffer_Index] = FPGA_fpgadata_in_float32;
	}
	FPGA_clockFall();

	//STAGE 6 in I RX1
	FPGA_clockRise();
	FPGA_fpgadata_in_tmp32 = (q31_t)((FPGA_readPacket() & 0xFF) << 24);
	FPGA_clockFall();

	//STAGE 7
	FPGA_clockRise();
	FPGA_fpgadata_in_tmp32 |= (q31_t)((FPGA_readPacket() & 0xFF) << 16);
	FPGA_clockFall();

	//STAGE 8
	FPGA_clockRise();
	FPGA_fpgadata_in_tmp32 |= (q31_t)((FPGA_readPacket() & 0xFF) << 8);
	FPGA_clockFall();

	//STAGE 9
	FPGA_clockRise();
	FPGA_fpgadata_in_tmp32 |= (q31_t)((FPGA_readPacket() & 0xFF) << 0);

	FPGA_fpgadata_in_float32 = (float32_t)FPGA_fpgadata_in_tmp32 / 2147483648.0f;
	if (TRX_IQ_swap)
	{
		if (NeedFFTInputBuffer)
			FFTInput_Q[FFT_buff_index] = FPGA_fpgadata_in_float32;
		FPGA_Audio_Buffer_RX1_Q[FPGA_Audio_Buffer_Index] = FPGA_fpgadata_in_float32;
	}
	else
	{
		if (NeedFFTInputBuffer)
			FFTInput_I[FFT_buff_index] = FPGA_fpgadata_in_float32;
		FPGA_Audio_Buffer_RX1_I[FPGA_Audio_Buffer_Index] = FPGA_fpgadata_in_float32;
	}
	FPGA_clockFall();

	if (TRX.Dual_RX_Type != VFO_SEPARATE)
	{
		//STAGE 10 in Q RX2
		FPGA_clockRise();
		FPGA_fpgadata_in_tmp32 = (q31_t)((FPGA_readPacket() & 0xFF) << 24);
		FPGA_clockFall();

		//STAGE 11
		FPGA_clockRise();
		FPGA_fpgadata_in_tmp32 |= (q31_t)((FPGA_readPacket() & 0xFF) << 16);
		FPGA_clockFall();

		//STAGE 12
		FPGA_clockRise();
		FPGA_fpgadata_in_tmp32 |= (q31_t)((FPGA_readPacket() & 0xFF) << 8);
		FPGA_clockFall();

		//STAGE 13
		FPGA_clockRise();
		FPGA_fpgadata_in_tmp32 |= (q31_t)((FPGA_readPacket() & 0xFF) << 0);

		FPGA_fpgadata_in_float32 = (float32_t)FPGA_fpgadata_in_tmp32 / 2147483648.0f;
		if (TRX_IQ_swap)
		{
			FPGA_Audio_Buffer_RX2_I[FPGA_Audio_Buffer_Index] = FPGA_fpgadata_in_float32;
		}
		else
		{
			FPGA_Audio_Buffer_RX2_Q[FPGA_Audio_Buffer_Index] = FPGA_fpgadata_in_float32;
		}
		FPGA_clockFall();

		//STAGE 14 in I RX2
		FPGA_clockRise();
		FPGA_fpgadata_in_tmp32 = (q31_t)((FPGA_readPacket() & 0xFF) << 24);
		FPGA_clockFall();

		//STAGE 15
		FPGA_clockRise();
		FPGA_fpgadata_in_tmp32 |= (q31_t)((FPGA_readPacket() & 0xFF) << 16);
		FPGA_clockFall();

		//STAGE 16
		FPGA_clockRise();
		FPGA_fpgadata_in_tmp32 |= (q31_t)((FPGA_readPacket() & 0xFF) << 8);
		FPGA_clockFall();

		//STAGE 17
		FPGA_clockRise();
		FPGA_fpgadata_in_tmp32 |= (q31_t)((FPGA_readPacket() & 0xFF) << 0);

		FPGA_fpgadata_in_float32 = (float32_t)FPGA_fpgadata_in_tmp32 / 2147483648.0f;
		if (TRX_IQ_swap)
		{
			FPGA_Audio_Buffer_RX2_Q[FPGA_Audio_Buffer_Index] = FPGA_fpgadata_in_float32;
		}
		else
		{
			FPGA_Audio_Buffer_RX2_I[FPGA_Audio_Buffer_Index] = FPGA_fpgadata_in_float32;
		}
	}

	FPGA_Audio_Buffer_Index++;
	if (FPGA_Audio_Buffer_Index == FPGA_AUDIO_BUFFER_SIZE)
		FPGA_Audio_Buffer_Index = 0;

	if (NeedFFTInputBuffer)
	{
		FFT_buff_index++;
		if (FFT_buff_index == FFT_SIZE)
		{
			FFT_buff_index = 0;
			NeedFFTInputBuffer = false;
			FFT_buffer_ready = true;
		}
	}
	FPGA_clockFall();
}

//отправить IQ данные
static inline void FPGA_fpgadata_sendiq(void)
{
	q31_t FPGA_fpgadata_out_q_tmp32 = 0;
	q31_t FPGA_fpgadata_out_i_tmp32 = 0;
	arm_float_to_q31((float32_t *)&FPGA_Audio_SendBuffer_Q[FPGA_Audio_Buffer_Index], &FPGA_fpgadata_out_q_tmp32, 1);
	arm_float_to_q31((float32_t *)&FPGA_Audio_SendBuffer_I[FPGA_Audio_Buffer_Index], &FPGA_fpgadata_out_i_tmp32, 1);
	FPGA_samples++;

	//STAGE 2 out Q
	FPGA_writePacket((FPGA_fpgadata_out_q_tmp32 >> 24) & 0xFF);
	//clock
	FPGA_clockRise();
	//clock
	FPGA_clockFall();

	//STAGE 3
	FPGA_writePacket((FPGA_fpgadata_out_q_tmp32 >> 16) & 0xFF);
	//clock
	FPGA_clockRise();
	//clock
	FPGA_clockFall();

	//STAGE 4
	FPGA_writePacket((FPGA_fpgadata_out_q_tmp32 >> 8) & 0xFF);
	//clock
	FPGA_clockRise();
	//clock
	FPGA_clockFall();

	//STAGE 5
	FPGA_writePacket((FPGA_fpgadata_out_q_tmp32 >> 0) & 0xFF);
	//clock
	FPGA_clockRise();
	//clock
	FPGA_clockFall();

	//STAGE 6 out I
	FPGA_writePacket((FPGA_fpgadata_out_i_tmp32 >> 24) & 0xFF);
	//clock
	FPGA_clockRise();
	//clock
	FPGA_clockFall();

	//STAGE 7
	FPGA_writePacket((FPGA_fpgadata_out_i_tmp32 >> 16) & 0xFF);
	//clock
	FPGA_clockRise();
	//clock
	FPGA_clockFall();

	//STAGE 8
	FPGA_writePacket((FPGA_fpgadata_out_i_tmp32 >> 8) & 0xFF);
	//clock
	FPGA_clockRise();
	//clock
	FPGA_clockFall();

	//STAGE 9
	FPGA_writePacket((FPGA_fpgadata_out_i_tmp32 >> 0) & 0xFF);
	//clock
	FPGA_clockRise();
	//clock
	FPGA_clockFall();

	FPGA_Audio_Buffer_Index++;
	if (FPGA_Audio_Buffer_Index == FPGA_AUDIO_BUFFER_SIZE)
	{
		if (Processor_NeedTXBuffer)
		{
			FPGA_Buffer_underrun = true;
			FPGA_Audio_Buffer_Index--;
		}
		else
		{
			FPGA_Audio_Buffer_Index = 0;
			FPGA_Audio_Buffer_State = true;
			Processor_NeedTXBuffer = true;
		}
	}
	else if (FPGA_Audio_Buffer_Index == FPGA_AUDIO_BUFFER_SIZE / 2)
	{
		if (Processor_NeedTXBuffer)
		{
			FPGA_Buffer_underrun = true;
			FPGA_Audio_Buffer_Index--;
		}
		else
		{
			FPGA_Audio_Buffer_State = false;
			Processor_NeedTXBuffer = true;
		}
	}
}

//переключить шину на ввод
static inline void FPGA_setBusInput(void)
{
	// Configure IO Direction mode (Input)
	register uint32_t temp = GPIOA->MODER;
	temp &= ~(GPIO_MODER_MODE0 << (0 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (0 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (1 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (1 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (2 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (2 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (3 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (3 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (4 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (4 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (5 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (5 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (6 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (6 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (7 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (7 * 2U));
	GPIOA->MODER = temp;
	FPGA_bus_direction = true;
}

//переключить шину на вывод
static inline void FPGA_setBusOutput(void)
{
	// Configure IO Direction mode (Output)
	register uint32_t temp = GPIOA->MODER;
	temp &= ~(GPIO_MODER_MODE0 << (0 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (0 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (1 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (1 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (2 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (2 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (3 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (3 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (4 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (4 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (5 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (5 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (6 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (6 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (7 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (7 * 2U));
	GPIOA->MODER = temp;
	FPGA_bus_direction = false;
}

//поднять сигнал CLK
static inline void FPGA_clockRise(void)
{
	FPGA_CLK_GPIO_Port->BSRR = FPGA_CLK_Pin;
}

//снять сигнал CLK
static inline void FPGA_clockFall(void)
{
	FPGA_CLK_GPIO_Port->BSRR = ((uint32_t)FPGA_CLK_Pin << 16U);
}

//поднять сигнал SYNC
static inline void FPGA_syncRise(void)
{
	FPGA_CLK_GPIO_Port->BSRR = FPGA_SYNC_Pin;
}

//снять сигнал SYNC
static inline void FPGA_syncFall(void)
{
	FPGA_CLK_GPIO_Port->BSRR = ((uint32_t)FPGA_SYNC_Pin << 16U);
}

//чтение пакета
static inline uint_fast8_t FPGA_readPacket(void)
{
	if (!FPGA_bus_direction)
		FPGA_setBusInput();
	return (FPGA_BUS_D0_GPIO_Port->IDR & 0xFF);
}

//запись пакета
static inline void FPGA_writePacket(uint_fast8_t packet)
{
	if (FPGA_bus_direction)
		FPGA_setBusOutput();
	FPGA_BUS_D0_GPIO_Port->BSRR = (packet & 0xFF) | 0xFF0000;
}

#if FPGA_FLASH_IN_HEX

#define FPGA_FLASH_COMMAND_DELAY for(uint32_t wait = 0; wait < 200; wait++) __asm("nop");
#define FPGA_FLASH_WRITE_DELAY for(uint32_t wait = 0; wait < 2000; wait++) __asm("nop");
#define FPGA_FLASH_READ_DELAY for(uint32_t wait = 0; wait < 50; wait++) __asm("nop");
static uint_fast8_t FPGA_spi_start_command(uint_fast8_t command) //выполнение команды к SPI flash
{
	//STAGE 1
	FPGA_writePacket(7); //FPGA FLASH READ command
	FPGA_syncRise();
	FPGA_clockRise();
	FPGA_syncFall();
	FPGA_clockFall();
	FPGA_FLASH_COMMAND_DELAY 	

	//STAGE 2 WRITE (F700)
	FPGA_writePacket(command);
	FPGA_clockRise();
	FPGA_clockFall();
	FPGA_FLASH_WRITE_DELAY

	//STAGE 3 READ ANSWER (F701)
	FPGA_clockRise();
	uint_fast8_t data = FPGA_readPacket();
	FPGA_clockFall();
	FPGA_FLASH_READ_DELAY
	
	return data;
}

static uint_fast8_t FPGA_spi_continue_command(uint_fast8_t writedata) //продолжение чтения и записи SPI flash
{
	//STAGE 2 WRITE (F700)
	FPGA_writePacket(writedata); 
	FPGA_clockRise();
	FPGA_clockFall();
	FPGA_FLASH_WRITE_DELAY

	//STAGE 3 READ ANSWER (F701)
	FPGA_clockRise();
	uint_fast8_t data = FPGA_readPacket();
	FPGA_clockFall();
	FPGA_FLASH_READ_DELAY
	
	return data;
}

/*
Micron M25P80 Serial Flash COMMANDS:
06h - WRITE ENABLE
04h - WRITE DISABLE
9Fh - READ IDENTIFICATION
9Eh - READ IDENTIFICATION
05h - READ STATUS REGISTER
01h - WRITE STATUS REGISTER
03h - READ DATA BYTES
0Bh - READ DATA BYTES at HIGHER SPEED
02h - PAGE PROGRAM
D8h - SECTOR ERASE
C7h - BULK ERASE
B9h - DEEP POWER-DOWN
ABh - RELEASE from DEEP POWER-DOWN
*/

static bool FPGA_spi_flash_verify(bool full) //проверка flash памяти
{
	FPGA_bus_stop = true;
	HAL_Delay(1);
	uint_fast8_t data = 0;
	FPGA_spi_start_command(0xAB);
	FPGA_spi_start_command(0x03); // READ DATA BYTES
	FPGA_spi_continue_command(0x00); //addr 1
	FPGA_spi_continue_command(0x00); //addr 2
	FPGA_spi_continue_command(0x00); //addr 3
	data = FPGA_spi_continue_command(0xFF);
	
	//Uncomress RLE and verify
	const int32_t file_offset = 0xA0 - 1;
	const int32_t flash_size = 0x200000;
	uint32_t errors = 0;
	uint32_t i = 0;
	int32_t decoded = 0;
	while (i < sizeof(FILES_OUTPUT_FILE_JIC))
	{
		if ((int8_t)FILES_OUTPUT_FILE_JIC[i] < 0) //нет повторов
		{
			uint8_t count = (-(int8_t)FILES_OUTPUT_FILE_JIC[i]);
			i++;
			for (uint8_t p = 0; p < count; p++)
			{
				if((decoded - file_offset) >=0 )
				{
					if((uint8_t)(__RBIT(data) >> 24) != FILES_OUTPUT_FILE_JIC[i])
					{
						errors++;
						sendToDebug_uint32(decoded, true);
						sendToDebug_str(": ");
						sendToDebug_hex((uint8_t)(__RBIT(data) >> 24), true);
						sendToDebug_hex(FILES_OUTPUT_FILE_JIC[i], true);
						sendToDebug_newline();
						sendToDebug_flush();
					}
					data = FPGA_spi_continue_command(0xFF);
				}
				decoded++;
				i++;
			}
		}
		else //повторы
		{
			uint8_t count = ((int8_t)FILES_OUTPUT_FILE_JIC[i]);
			i++;
			for (uint8_t p = 0; p < count; p++)
			{
				if((decoded - file_offset) >=0 )
				{
					if(i < sizeof(FILES_OUTPUT_FILE_JIC) && (uint8_t)(__RBIT(data) >> 24) != FILES_OUTPUT_FILE_JIC[i])
					{
						errors++;
						sendToDebug_uint32(decoded, true);
						sendToDebug_str(": ");
						sendToDebug_hex((uint8_t)(__RBIT(data) >> 24), true);
						sendToDebug_hex(FILES_OUTPUT_FILE_JIC[i], true);
						sendToDebug_newline();
						sendToDebug_flush();
					}
					data = FPGA_spi_continue_command(0xFF);
				}
				decoded++;
			}
			i++;
		}
		HAL_IWDG_Refresh(&hiwdg1);
		if (!full && decoded > (file_offset + 2048))
			break;
		if (decoded >= (flash_size + file_offset))
			break;
		if (errors > 3)
			break;
	}
	//
	
	FPGA_bus_stop = false;
	if (errors>0)
	{
		sendToDebug_strln("[ERR] FPGA Flash verification failed");
		return false;
	}
	else
	{
		sendToDebug_strln("[OK] FPGA Flash verification compleated");
		return true;
	}
}
#endif
