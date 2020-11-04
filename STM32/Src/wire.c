#include "stm32h7xx_hal.h"
#include "main.h"
#include "functions.h"
#include "wire.h"

/* low level conventions:
 * - SDA/SCL idle high (expected high)
 * - always start with i2c_delay rather than end
 */

static uint8_t i2c_writeOneByte(I2C_DEVICE *dev, uint8_t);
static void i2c_start(I2C_DEVICE *dev);
static void i2c_stop(I2C_DEVICE *dev);
static bool i2c_get_ack(I2C_DEVICE *dev);
static void i2c_shift_out(I2C_DEVICE *dev, uint8_t val);

static void i2c_start(I2C_DEVICE *dev)
{
	HAL_GPIO_WritePin(dev->SDA_PORT, dev->SDA_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(dev->SCK_PORT, dev->SCK_PIN, GPIO_PIN_SET);
	I2C_DELAY
	HAL_GPIO_WritePin(dev->SDA_PORT, dev->SDA_PIN, GPIO_PIN_RESET);
	I2C_DELAY
	HAL_GPIO_WritePin(dev->SCK_PORT, dev->SCK_PIN, GPIO_PIN_RESET);
}

static void i2c_stop(I2C_DEVICE *dev)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = dev->SDA_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(dev->SDA_PORT, &GPIO_InitStruct);

	HAL_GPIO_WritePin(dev->SDA_PORT, dev->SDA_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(dev->SCK_PORT, dev->SCK_PIN, GPIO_PIN_SET);
	I2C_DELAY
	I2C_DELAY
	I2C_DELAY
	I2C_DELAY
	HAL_GPIO_WritePin(dev->SDA_PORT, dev->SDA_PIN, GPIO_PIN_SET);
	I2C_DELAY
}

static bool i2c_get_ack(I2C_DEVICE *dev)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	int time = 0;
	I2C_DELAY
	HAL_GPIO_WritePin(dev->SDA_PORT, dev->SDA_PIN, GPIO_PIN_RESET);
	GPIO_InitStruct.Pin = dev->SDA_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(dev->SDA_PORT, &GPIO_InitStruct);

	I2C_DELAY
	HAL_GPIO_WritePin(dev->SCK_PORT, dev->SCK_PIN, GPIO_PIN_SET);
	I2C_DELAY
	I2C_DELAY

	while (HAL_GPIO_ReadPin(dev->SDA_PORT, dev->SDA_PIN))
	{
		time++;
		if (time > 50)
		{
			i2c_stop(dev);
			return false;
		}
		I2C_DELAY
	}

	GPIO_InitStruct.Pin = dev->SDA_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(dev->SDA_PORT, &GPIO_InitStruct);

	HAL_GPIO_WritePin(dev->SCK_PORT, dev->SCK_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(dev->SDA_PORT, dev->SDA_PIN, GPIO_PIN_RESET);

	//I2C_DELAY;
	//HAL_Delay(1);
	return true;
}

static void i2c_shift_out(I2C_DEVICE *dev, uint8_t val)
{
	int i;
	GPIO_InitTypeDef GPIO_InitStruct;
	for (i = 0; i < 8; i++)
	{

		I2C_DELAY
		HAL_GPIO_WritePin(dev->SDA_PORT, dev->SDA_PIN, (GPIO_PinState) !!(val & (1 << (7 - i))));
		GPIO_InitStruct.Pin = dev->SDA_PIN;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(dev->SDA_PORT, &GPIO_InitStruct);

		I2C_DELAY
		HAL_GPIO_WritePin(dev->SCK_PORT, dev->SCK_PIN, GPIO_PIN_SET);

		I2C_DELAY
		I2C_DELAY
		HAL_GPIO_WritePin(dev->SCK_PORT, dev->SCK_PIN, GPIO_PIN_RESET);
	}
}

/*
 * Joins I2C bus as master on given SDA and SCL pins.
 */
void i2c_begin(I2C_DEVICE *dev)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	HAL_GPIO_WritePin(dev->SCK_PORT, dev->SCK_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(dev->SDA_PORT, dev->SDA_PIN, GPIO_PIN_SET);

	GPIO_InitStruct.Pin = dev->SCK_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(dev->SCK_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = dev->SDA_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(dev->SDA_PORT, &GPIO_InitStruct);
}

void i2c_beginTransmission_u8(I2C_DEVICE *dev, uint8_t slave_address)
{
	dev->i2c_tx_addr = slave_address;
	dev->i2c_tx_buf_idx = 0;
	dev->i2c_tx_buf_overflow = false;
}

uint8_t i2c_endTransmission(I2C_DEVICE *dev)
{
	if (dev->i2c_tx_buf_overflow)
		return EDATA;
	i2c_start(dev);

	//I2C_DELAY;
	i2c_shift_out(dev, (uint8_t)((dev->i2c_tx_addr << 1) | I2C_WRITE));
	if (!i2c_get_ack(dev))
		return ENACKADDR;

	// shift out the address we're transmitting to
	for (uint8_t i = 0; i < dev->i2c_tx_buf_idx; i++)
	{
		uint8_t ret = i2c_writeOneByte(dev, dev->i2c_tx_buf[i]);
		if (ret)
			return ret; // SUCCESS is 0
	}
	I2C_DELAY
	I2C_DELAY
	i2c_stop(dev);

	dev->i2c_tx_buf_idx = 0;
	dev->i2c_tx_buf_overflow = false;
	return SUCCESS;
}

void i2c_write_u8(I2C_DEVICE *dev, uint8_t value)
{
	if (dev->i2c_tx_buf_idx == WIRE_BUFSIZ)
	{
		dev->i2c_tx_buf_overflow = true;
		return;
	}

	dev->i2c_tx_buf[dev->i2c_tx_buf_idx++] = value;
}

// private methods
static uint8_t i2c_writeOneByte(I2C_DEVICE *dev, uint8_t byte)
{
	i2c_shift_out(dev, byte);
	if (!i2c_get_ack(dev))
		return ENACKTRNS;
	return SUCCESS;
}
