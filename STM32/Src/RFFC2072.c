#include "RFFC2072.h"
#include "i2c.h"

#if HRDW_HAS_RFFC2072_MIXER

static void RFFC2072_shift_out(I2C_DEVICE *dev, uint32_t val, uint8_t size);
static void RFFC2072_SDA_OUT(I2C_DEVICE *dev);
static void RFFC2072_SDA_IN(I2C_DEVICE *dev);
static uint16_t RFFC2072_Read_HalfWord(I2C_DEVICE *dev, uint8_t reg_addr);
static void RFFC2072_Write_HalfWord(I2C_DEVICE *dev, uint8_t reg_addr, uint16_t value);

void RFMIXER_Init(void) {
	if (I2C_SHARED_BUS.locked) {
		return;
	}

	I2C_SHARED_BUS.locked = true;

	//

	I2C_SHARED_BUS.locked = false;
}

static void RFFC2072_Write_HalfWord(I2C_DEVICE *dev, uint8_t reg_addr, uint16_t value) {
	unsigned char i;
	uint16_t receive = 0;

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

static uint16_t RFFC2072_Read_HalfWord(I2C_DEVICE *dev, uint8_t reg_addr) {
	unsigned char i;
	uint16_t receive = 0;

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
