#ifndef WIRE_h
#define WIRE_h

#include "main.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_gpio.h"
#include <stdbool.h>

#define WIRE_BUFSIZ 101

/* return codes from endTransmission() */
#define SUCCESS 0   /* transmission was successful */
#define EDATA 1     /* too much data */
#define ENACKADDR 2 /* received nack on transmit of address */
#define ENACKTRNS 3 /* received nack on transmit of data */
#define EOTHER 4    /* other error */

#define I2C_WRITE 0
#define I2C_READ 1
#define I2C_DELAY                                     \
    for (uint16_t wait_i = 0; wait_i < 300; wait_i++) \
    {                                                 \
        __asm("nop");                                 \
    };

typedef struct
{
	GPIO_TypeDef *SDA_PORT;
	uint16_t SDA_PIN;
	GPIO_TypeDef *SCK_PORT;
	uint16_t SCK_PIN;
	uint8_t i2c_tx_addr;				  /* address transmitting to */
	uint8_t i2c_tx_buf[WIRE_BUFSIZ]; /* transmit buffer */
	uint8_t i2c_tx_buf_idx;			  /* next idx available in tx_buf, -1 overflow */
	bool i2c_tx_buf_overflow;
} I2C_DEVICE;

static I2C_DEVICE I2C_WM8731 = {
	.SDA_PORT = WM8731_SDA_GPIO_Port,
	.SDA_PIN = WM8731_SDA_Pin,
	.SCK_PORT = WM8731_SCK_GPIO_Port,
	.SCK_PIN = WM8731_SCK_Pin,
	.i2c_tx_addr = 0,
	.i2c_tx_buf = {0},
	.i2c_tx_buf_idx = 0,
	.i2c_tx_buf_overflow = false,
};

extern void i2c_begin(I2C_DEVICE *dev);
extern void i2c_beginTransmission_u8(I2C_DEVICE *dev, uint8_t);
extern void i2c_write_u8(I2C_DEVICE *dev, uint8_t);
extern uint8_t i2c_endTransmission(I2C_DEVICE *dev);

#endif
