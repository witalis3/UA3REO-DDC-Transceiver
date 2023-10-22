#include "RFFC2072.h"
#include "i2c.h"

#if HRDW_HAS_RFFC2072_MIXER

static void RFFC2072_shift_out(I2C_DEVICE *dev, uint32_t val, uint8_t size);
static void RFFC2072_SDA_OUT(I2C_DEVICE *dev);
static void RFFC2072_SDA_IN(I2C_DEVICE *dev);
static uint16_t RFFC2072_Read_HalfWord(uint8_t reg_addr);
static void RFFC2072_Write_HalfWord(uint8_t reg_addr, uint16_t value);
static uint64_t RFFC2072_freq_calc(uint64_t lo_MHz);

uint16_t RFFC2072_reg[RFFC2072_REGS_NUM] = {
    0xbefa, /* 00 LF - Loop Filter Configuration */
    0x4064, /* 01 XO - Crystal Oscillator Configuration */
    0x9055, /* 02 CAL_TIME - Calibration Timing */
    0x2d02, /* 03 VCO_CTRL - Calibration Control */
    0xacbf, /* 04 CT_CAL1 - Path 1 Coarse Tuning Calibration */
    0xacbf, /* 05 CT_CAL2 - Path 2 Coarse Tuning Calibration */
    0x0028, /* 06 PLL_CAL1 - Path 1 PLL Calibration */
    0x0028, /* 07 PLL_CAL2 - Path 2 PLL Calibration */
    0xff00, /* 08 VCO_AUTO - Auto VCO select control */
    0x8220, /* 09 PLL_CTRL - PLL Contro */
    0x0202, /* 0A PLL_BIAS - PLL Bias Settings */
    0x7E00, // /* 0B MIX_CONT - Mixer Control (0x4800 default)*/
    0x1a94, /* 0C P1_FREQ1 - Path 1 Frequency 1 */
    0xd89d, /* 0D P1_FREQ2 - Path 1 Frequency 2 */
    0x8900, /* 0E P1_FREQ3 - Path 1 Frequency 3 */
    0x1e84, /* 0F P2_FREQ1 - Path 2 Frequency 1 */
    0x89d8, /* 10 P2_FREQ2 - Path 2 Frequency 2 */
    0x9d00, /* 11 P2_FREQ3 - Path 2 Frequency 3 */
    0x2a20, /* 12 FN_CTRL - Frac-N Control  (0x2A80 default)*/
    0x0000, /* 13 EXT_MOD - Frequency modulation control 1 */
    0x0000, /* 14 FMOD - Frequency modulation control 2 */
    0x0000, /* 15 SDI_CTRL - SDI Control */
    0x0000, /* 16 GPO - General Purpose Outputs */
    0x4900, /* 17 T_VCO - Temperature Compensation VCO Curves */
    0x0281, /* 18 IQMOD1 – Modulator Calibration */
    0xf00f, /* 19 IQMOD2 – Modulator Control */
    0x0000, /* 1A IQMOD3 - Modulator Buffer Control */
    0x0000, /* 1B IQMOD4– Modulator Core Control */
    0xc840, /* 1C TEMPC_CTRL – Temperature compensation control */
    0x1000, /* 1D DEV_CTRL - Readback register and RSM Control */
    0x0005, /* 1E TEST - Test register */
            // 0x0000  /* 1F READBACK – Readback Register */
};

void RFMIXER_Init(void) {
	if (I2C_SHARED_BUS.locked) {
		return;
	}

	I2C_SHARED_BUS.locked = true;

	for (uint8_t i = 0; i < RFFC2072_REGS_NUM; i++) {
		RFFC2072_Write_HalfWord(i, RFFC2072_reg[i]);
	}

	// set ENBL and MODE to be configured via 3-wire IF
	RFFC2072_reg[0x15] |= (1 << 15) + (1 << 13);
	RFFC2072_Write_HalfWord(0x15, RFFC2072_reg[0x15]);

	// GPOs are active at all time and send LOCK flag to GPO4
	RFFC2072_reg[0x16] |= (1 << 1) + (1 << 0);
	RFFC2072_Write_HalfWord(0x16, RFFC2072_reg[0x16]);

	RFMIXER_enable();

	RFMIXER_Freq_Set(145000000);

	I2C_SHARED_BUS.locked = false;
}

void RFMIXER_disable(void) {
	// clear ENBL BIT
	RFFC2072_reg[0x15] &= ~(1 << 14);
	RFFC2072_Write_HalfWord(0x15, RFFC2072_reg[0x15]);
}

uint64_t RFMIXER_Freq_Set(uint64_t lo_freq_Hz) {
	RFMIXER_disable();
	uint64_t set_freq = RFFC2072_freq_calc(lo_freq_Hz);
	RFMIXER_enable();

	println("RFMixer LO Freq: ", set_freq);
	return set_freq;
}

void RFMIXER_enable(void) {
	// set ENBL BIT
	RFFC2072_reg[0x15] |= (1 << 14);
	RFFC2072_Write_HalfWord(0x15, RFFC2072_reg[0x15]);
}

static uint64_t RFFC2072_freq_calc(uint64_t lo_freq_Hz) {
	uint8_t n_lo = log2(RFFC2072_LO_MAX / lo_freq_Hz);
	uint8_t lodiv = 1 << n_lo; // 2^n_lo
	uint64_t fvco = lodiv * lo_freq_Hz;
	uint8_t fbkdiv;

	if (fvco > 3200000000) {
		fbkdiv = 4;
		RFFC2072_reg[0x0] &= ~(7);
		RFFC2072_reg[0x0] |= 3;
		RFFC2072_Write_HalfWord(0x0, RFFC2072_reg[0x0]);
	} else {
		fbkdiv = 2;
		RFFC2072_reg[0x0] &= ~(7);
		RFFC2072_reg[0x0] |= 2;
		RFFC2072_Write_HalfWord(0x0, RFFC2072_reg[0x0]);
	}

	float64_t n_div = (float64_t)fvco / (float64_t)fbkdiv / RFFC2072_REF_FREQ;
	uint8_t n = n_div;
	uint16_t nummsb = (1 << 16) * (n_div - n);
	uint8_t numlsb = (1 << 8) * ((1 << 16) * (n_div - n) - nummsb);
	// println(n_lo, " ", lodiv, " ", fvco, " ", fbkdiv, " ", n_div, " ", n, " ", nummsb, " ", numlsb);

	/* Path 1 */

	// p1_freq1
	RFFC2072_reg[0x0C] &= ~(0x3fff << 2);
	RFFC2072_reg[0x0C] |= (n << 7) + (n_lo << 4) + ((fbkdiv >> 1) << 2);
	RFFC2072_Write_HalfWord(0x0C, RFFC2072_reg[0x0C]);

	// p1_freq2
	RFFC2072_reg[0x0D] = nummsb;
	RFFC2072_Write_HalfWord(0x0D, RFFC2072_reg[0x0D]);

	// p1_freq3
	RFFC2072_reg[0x0E] = (numlsb << 8);
	RFFC2072_Write_HalfWord(0x0E, RFFC2072_reg[0x0E]);

	/* Path 2 */

	// p2_freq1
	RFFC2072_reg[0x0f] &= ~(0x3fff << 2);
	RFFC2072_reg[0x0f] |= (n << 7) + (n_lo << 4) + ((fbkdiv >> 1) << 2);
	RFFC2072_Write_HalfWord(0x0f, RFFC2072_reg[0x0f]);

	// p2_freq2
	RFFC2072_reg[0x10] = nummsb;
	RFFC2072_Write_HalfWord(0x10, RFFC2072_reg[0x10]);

	// p2_freq3
	RFFC2072_reg[0x11] = (numlsb << 8);
	RFFC2072_Write_HalfWord(0x11, RFFC2072_reg[0x11]);

	// calculate result freq
	uint64_t tune_freq_hz = RFFC2072_REF_FREQ * fbkdiv * ((float64_t)n + ((float64_t)((nummsb << 8) | numlsb) / (1 << 24))) / lodiv;
	return tune_freq_hz;
}

static void RFFC2072_Write_HalfWord(uint8_t reg_addr, uint16_t value) {
	unsigned char i;
	uint16_t receive = 0;
	I2C_DEVICE *dev = &I2C_SHARED_BUS;

	// before write
	SCK_CLR;
	I2C_DELAY
	SCK_SET;
	I2C_DELAY
	HAL_GPIO_WritePin(RFMIXER_CS_GPIO_Port, RFMIXER_CS_Pin, GPIO_PIN_RESET);

	RFFC2072_shift_out(dev, (I2C_WRITE << 8) | (I2C_WRITE << 7) | (reg_addr & 0x7F), 9);
	RFFC2072_shift_out(dev, value, 16);

	// after write
	HAL_GPIO_WritePin(RFMIXER_CS_GPIO_Port, RFMIXER_CS_Pin, GPIO_PIN_SET);
	SCK_CLR;
	I2C_DELAY
	SCK_SET;
	I2C_DELAY
}

static uint16_t RFFC2072_Read_HalfWord(uint8_t reg_addr) {
	unsigned char i;
	uint16_t receive = 0;
	I2C_DEVICE *dev = &I2C_SHARED_BUS;

	// before read
	SCK_CLR;
	I2C_DELAY
	SCK_SET;
	I2C_DELAY
	HAL_GPIO_WritePin(RFMIXER_CS_GPIO_Port, RFMIXER_CS_Pin, GPIO_PIN_RESET);

	RFFC2072_shift_out(dev, (I2C_READ << 8) | (I2C_READ << 7) | (reg_addr & 0x7F), 9);

	RFFC2072_SDA_IN(dev);

	for (i = 0; i < 18; i++) { // 2 bits for read waiting
		SCK_CLR;
		receive <<= 1;
		if (HAL_GPIO_ReadPin(dev->SDA_PORT, dev->SDA_PIN)) {
			receive++;
		}
		I2C_DELAY
		SCK_SET;
		I2C_DELAY
	}

	// after read
	HAL_GPIO_WritePin(RFMIXER_CS_GPIO_Port, RFMIXER_CS_Pin, GPIO_PIN_SET);
	SCK_CLR;
	I2C_DELAY
	SCK_SET;
	I2C_DELAY

	return receive;
}

static void RFFC2072_shift_out(I2C_DEVICE *dev, uint32_t val, uint8_t size) {
	RFFC2072_SDA_OUT(dev);
	for (uint8_t i = 0; i < size; i++) {
		SCK_CLR;
		HAL_GPIO_WritePin(dev->SDA_PORT, dev->SDA_PIN, (GPIO_PinState) !!(val & (1 << (size - 1 - i))));
		I2C_DELAY
		SCK_SET;
		I2C_DELAY
	}
}

static void RFFC2072_SDA_OUT(I2C_DEVICE *dev) {
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = dev->SDA_PIN;
	GPIO_InitStruct.Mode = I2C_OUTPUT_MODE;
	GPIO_InitStruct.Pull = I2C_PULLS;
	GPIO_InitStruct.Speed = I2C_GPIO_SPEED;
	HAL_GPIO_Init(dev->SDA_PORT, &GPIO_InitStruct);
}

static void RFFC2072_SDA_IN(I2C_DEVICE *dev) {
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = dev->SDA_PIN;
	GPIO_InitStruct.Mode = I2C_INPUT_MODE;
	GPIO_InitStruct.Pull = I2C_PULLS;
	GPIO_InitStruct.Speed = I2C_GPIO_SPEED;
	HAL_GPIO_Init(dev->SDA_PORT, &GPIO_InitStruct);
}

#endif
