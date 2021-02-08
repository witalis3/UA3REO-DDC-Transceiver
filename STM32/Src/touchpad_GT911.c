#include "touchpad.h"
#include "i2c.h"
#include "lcd.h"
#if (defined(TOUCHPAD_GT911))

GT911_Dev GT911 = {0};
static uint8_t gt911_i2c_addr = GT911_I2C_ADDR_1;

static uint8_t GT911_Config[] = {
	0x81, 0x00, 0x04, 0x58, 0x02, 0x0A, 0x0C, 0x20, 0x01, 0x08, 0x28, 0x05, 0x50, // 0x8047 - 0x8053
	0x3C, 0x0F, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x8054 - 0x8060
	0x00, 0x89, 0x2A, 0x0B, 0x2D, 0x2B, 0x0F, 0x0A, 0x00, 0x00, 0x01, 0xA9, 0x03, // 0x8061 - 0x806D
	0x2D, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, // 0x806E - 0x807A
	0x59, 0x94, 0xC5, 0x02, 0x07, 0x00, 0x00, 0x04, 0x93, 0x24, 0x00, 0x7D, 0x2C, // 0x807B - 0x8087
	0x00, 0x6B, 0x36, 0x00, 0x5D, 0x42, 0x00, 0x53, 0x50, 0x00, 0x53, 0x00, 0x00, // 0x8088	- 0x8094
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x8095 - 0x80A1
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80A2 - 0x80AD
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, // 0x80AE - 0x80BA
	0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, // 0x80BB - 0x80C7
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80C8 - 0x80D4
	0x02, 0x04, 0x06, 0x08, 0x0A, 0x0F, 0x10, 0x12, 0x16, 0x18, 0x1C, 0x1D, 0x1E, // 0x80D5 - 0x80E1
	0x1F, 0x20, 0x21, 0x22, 0x24, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, // 0x80E2 - 0x80EE
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80EF - 0x80FB
	0x00, 0x00, 0xD6, 0x01};													  // 0x80FC - 0x8100

uint8_t GT911_WR_Reg(uint16_t reg, uint8_t *buf, uint8_t len)
{
	uint8_t i;
	uint8_t ret = 0;
	i2c_beginTransmission_u8(&I2C_TOUCHPAD, gt911_i2c_addr);
	i2c_write_u8(&I2C_TOUCHPAD, (reg >> 8) & 0xFF);
	i2c_write_u8(&I2C_TOUCHPAD, reg & 0xFF);
	for (i = 0; i < len; i++)
	{
		i2c_write_u8(&I2C_TOUCHPAD, buf[i]);
	}
	ret = i2c_endTransmission(&I2C_TOUCHPAD);
	return ret;
}

void GT911_RD_RegOneByte(uint16_t reg, uint8_t *buf)
{
	uint8_t i;
	i2c_beginTransmission_u8(&I2C_TOUCHPAD, gt911_i2c_addr);
	i2c_write_u8(&I2C_TOUCHPAD, (reg >> 8) & 0xFF);
	i2c_write_u8(&I2C_TOUCHPAD, reg & 0xFF);
	uint8_t res = i2c_endTransmission(&I2C_TOUCHPAD);
	if (res == 0)
	{
		if (i2c_beginReceive_u8(&I2C_TOUCHPAD, gt911_i2c_addr))
		{
			*buf = i2c_Read_Byte(&I2C_TOUCHPAD, 0);
			i2c_stop(&I2C_TOUCHPAD);
		}
	}
	else
	{
		sendToDebug_str("no touchpad found on i2c bus");
		
		//try new i2c addr if failed
		if(gt911_i2c_addr == GT911_I2C_ADDR_1)
			gt911_i2c_addr = GT911_I2C_ADDR_2;
		else
			gt911_i2c_addr = GT911_I2C_ADDR_1;
	}
}

void GT911_RD_Reg(uint16_t reg, uint8_t *buf, uint8_t len)
{
	for (uint8_t i = 0; i < len; i++)
	{
		GT911_RD_RegOneByte(reg + i, &buf[i]);
	}
	/*uint8_t i;
	i2c_beginTransmission_u8(&I2C_TOUCHPAD, GT911_I2C_ADDR);
	i2c_write_u8(&I2C_TOUCHPAD, (reg >> 8) & 0xFF);
	i2c_write_u8(&I2C_TOUCHPAD, reg & 0xFF);
	uint8_t res = i2c_endTransmission(&I2C_TOUCHPAD);
	if (res == 0)
	{
		if (i2c_beginReceive_u8(&I2C_TOUCHPAD, GT911_I2C_ADDR))
		{
			for (i = 0; i < len; i++)
			{
				buf[i] = i2c_Read_Byte(&I2C_TOUCHPAD, i == (len - 1) ? 0 : 1);
			}
			i2c_stop(&I2C_TOUCHPAD);
		}
	}
	else sendToDebug_str("no dev");*/
}

void GT911_ReadStatus(void)
{
	uint8_t buf[4] = {0};
	GT911_RD_Reg(GT911_PRODUCT_ID_REG, (uint8_t *)&buf[0], 3);
	GT911_RD_Reg(GT911_CONFIG_REG, (uint8_t *)&buf[3], 1);

	static IRAM2 char str[64] = {0};
	sprintf(str, "TouchPad_ID:%d,%d,%d\r\nTouchPad_Config_Version:%2x", buf[0], buf[1], buf[2], buf[3]);
	sendToDebug_strln(str);
}

void GT911_ReadFirmwareVersion(void)
{
	uint8_t buf[2] = {0};
	GT911_RD_Reg(GT911_FIRMWARE_VERSION_REG, buf, 2);

	static IRAM2 char str[128] = {0};
	sprintf(str, "FirmwareVersion:%2x", (((uint16_t)buf[1] << 8) + buf[0]));
	sendToDebug_strln(str);
}

void GT911_Init(void)
{
	/*GT911_Config[1] = 800 & 0x00FF; //X_Resolution
	GT911_Config[2] = (800 >> 8) & 0x00FF; 
	GT911_Config[3] = 480 & 0x00FF; //Y_Resolution
	GT911_Config[4] = (480 >> 8) & 0x00FF;
	GT911_Config[5] = GT911_MAX_TOUCH; //Number_Of_Touch_Support
	GT911_Config[6] = 0;
	GT911_Config[6] |= true << 7; //ReverseY
	GT911_Config[6] |= true << 6; //ReverseX
	GT911_Config[6] |= true << 3; //SwithX2Y
	GT911_Config[6] |= true << 2; //SoftwareNoiseReduction
	
	//GT911_CalculateCheckSum
	GT911_Config[184] = 0;
	for(uint8_t i = 0 ; i < 184 ; i++){
		GT911_Config[184] += GT911_Config[i];
	}
	GT911_Config[184] = (~GT911_Config[184]) + 1;
	//GT911_SendConfig
	GT911_WR_Reg(GT911_CONFIG_REG, GT911_Config, sizeof(GT911_Config));
	HAL_Delay(100);*/

	uint8_t buf[1] = {0};
	buf[0] = GOODIX_CMD_BASEUPDATE;
	GT911_WR_Reg(GT911_COMMAND_REG, buf, 1);
	HAL_Delay(100);
	buf[0] = GOODIX_CMD_CALIBRATE;
	GT911_WR_Reg(GT911_COMMAND_REG, buf, 1);
	HAL_Delay(100);
	/*buf[0] = GOODIX_CMD_SOFTRESET;
	GT911_WR_Reg(GT911_COMMAND_REG, buf, 1);
	HAL_Delay(100);*/
	/*buf[0] = GOODIX_CMD_READ;
	GT911_WR_Reg(GT911_COMMAND_REG, buf, 1);
	HAL_Delay(100);
	
	uint8_t rbuf[4] = {0};
	GT911_RD_Reg(0x8048, (uint8_t *)&rbuf[0], 2);
	sendToDebug_uint8(rbuf[0],false);
	sendToDebug_uint8(rbuf[1],false);
	sendToDebug_uint8(rbuf[2],false);
	sendToDebug_uint8(rbuf[3],false);
	sendToDebug_newline();
	GT911_RD_Reg(0x8049, (uint8_t *)&rbuf[0], 4);
	sendToDebug_uint8(rbuf[0],false);
	sendToDebug_uint8(rbuf[1],false);
	sendToDebug_uint8(rbuf[2],false);
	sendToDebug_uint8(rbuf[3],false);*/

	sendToDebug_strln("Touchpad calibrated");
}

void GT911_Scan(void)
{
	char str[64] = {0};
	uint8_t buf[41] = {0};
	uint8_t Clearbuf = 0;

	if (GT911.Touch == 1)
	{
		GT911.Touch = 0;
		GT911_RD_Reg(GT911_READ_XY_REG, buf, 1);

		if ((buf[0] & 0x80) == 0x00)
		{
			GT911_WR_Reg(GT911_READ_XY_REG, (uint8_t *)&Clearbuf, 1);

			//sprintf(str,"%d",buf[0]);
			//sendToDebug_strln(str);
			//HAL_Delay(10);
		}
		else
		{
			//sprintf(str, "bufstat:%d",(buf[0]&0x80));
			//sendToDebug_strln(str);

			GT911.TouchpointFlag = buf[0];
			GT911.TouchCount = buf[0] & 0x0f;
			if (GT911.TouchCount > 5 || GT911.TouchCount == 0)
			{
				GT911_WR_Reg(GT911_READ_XY_REG, (uint8_t *)&Clearbuf, 1);
				return;
			}
			GT911.TouchCount = 1; //LIMIT NOW
			GT911_RD_Reg(GT911_READ_XY_REG + 1, &buf[1], GT911.TouchCount * 8);
			GT911_WR_Reg(GT911_READ_XY_REG, (uint8_t *)&Clearbuf, 1);

			GT911.Touchkeytrackid[0] = buf[1];
			GT911.X[0] = (uint16_t)((uint16_t)buf[3] << 8) + buf[2];
			GT911.Y[0] = (uint16_t)(480 - (((uint16_t)buf[5] << 8) + buf[4]));
			GT911.S[0] = (uint16_t)((uint16_t)buf[7] << 8) + buf[6];

			GT911.Touchkeytrackid[1] = buf[9];
			GT911.X[1] = (uint16_t)((uint16_t)buf[11] << 8) + buf[10];
			GT911.Y[1] = (uint16_t)(480 - (((uint16_t)buf[13] << 8) + buf[12]));
			GT911.S[1] = (uint16_t)((uint16_t)buf[15] << 8) + buf[14];

			GT911.Touchkeytrackid[2] = buf[17];
			GT911.X[2] = (uint16_t)((uint16_t)buf[19] << 8) + buf[18];
			GT911.Y[2] = (uint16_t)(480 - (((uint16_t)buf[21] << 8) + buf[20]));
			GT911.S[2] = (uint16_t)((uint16_t)buf[23] << 8) + buf[22];

			GT911.Touchkeytrackid[3] = buf[25];
			GT911.X[3] = (uint16_t)((uint16_t)buf[27] << 8) + buf[26];
			GT911.Y[3] = (uint16_t)(480 - (((uint16_t)buf[29] << 8) + buf[28]));
			GT911.S[3] = (uint16_t)((uint16_t)buf[31] << 8) + buf[30];

			GT911.Touchkeytrackid[4] = buf[33];
			GT911.X[4] = (uint16_t)((uint16_t)buf[35] << 8) + buf[34];
			GT911.Y[4] = (uint16_t)(480 - (((uint16_t)buf[37] << 8) + buf[36]));
			GT911.S[4] = (uint16_t)((uint16_t)buf[39] << 8) + buf[38];

			for (uint8_t touch_id = 0; touch_id < GT911.TouchCount; touch_id++)
			{
				GT911.Y[touch_id] = 480 - GT911.Y[touch_id];

				if (GT911.Y[touch_id] < 5)
					GT911.Y[touch_id] = 5;
				if (GT911.Y[touch_id] > 475)
					GT911.Y[touch_id] = 475;

				if (GT911.X[touch_id] < 5)
					GT911.X[touch_id] = 5;
				if (GT911.X[touch_id] > 795)
					GT911.X[touch_id] = 795;

#if SCREEN_ROTATE
				GT911.X[touch_id] = LCD_WIDTH - GT911.X[touch_id];
				GT911.Y[touch_id] = LCD_HEIGHT - GT911.Y[touch_id];
#endif

				//sprintf(str, "%d,%d - %d / %d", GT911.X[touch_id], GT911.Y[touch_id], GT911.Touchkeytrackid[touch_id], GT911.TouchCount);
				//sendToDebug_strln(str);

				TOUCHPAD_processTouch(GT911.X[touch_id], GT911.Y[touch_id]);
			}
		}
	}
}

#endif
