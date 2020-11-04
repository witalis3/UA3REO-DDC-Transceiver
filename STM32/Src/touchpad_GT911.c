#include "touchpad.h"
#include "i2c.h"
#if (defined(TOUCHPAD_GT911))

GT911_Dev GT911 = {0};

uint8_t GT911_WR_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{
	uint8_t i;
	uint8_t ret=0;
	i2c_beginTransmission_u8(&I2C_TOUCHPAD, GT911_I2C_ADDR);	
	i2c_write_u8(&I2C_TOUCHPAD, (reg >> 8) & 0xFF);					  		   
	i2c_write_u8(&I2C_TOUCHPAD, reg & 0xFF);
	for(i=0;i<len;i++)
	{	   
		i2c_write_u8(&I2C_TOUCHPAD, buf[i]);
	}
  ret = i2c_endTransmission(&I2C_TOUCHPAD);				
	return ret; 
}

void GT911_RD_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{
	uint8_t i;
	i2c_beginTransmission_u8(&I2C_TOUCHPAD, GT911_I2C_ADDR);
	i2c_write_u8(&I2C_TOUCHPAD, (reg >> 8) & 0xFF);
	i2c_write_u8(&I2C_TOUCHPAD, reg & 0xFF);
	uint8_t res = i2c_endTransmission(&I2C_TOUCHPAD);
	if(res == 0)
	{
		if(i2c_beginReceive_u8(&I2C_TOUCHPAD, GT911_I2C_ADDR))
		{
			for(i=0;i<len;i++)
			{	   
				buf[i]=i2c_Read_Byte(&I2C_TOUCHPAD, i==(len-1) ? 0 : 1); 
			} 
			i2c_stop(&I2C_TOUCHPAD);
		}
	}
	//else sendToDebug_str("no dev");
}

void GT911_ReadStatus(void)
{
	uint8_t buf[4] = {0};
	GT911_RD_Reg(GT911_PRODUCT_ID_REG, (uint8_t *)&buf[0], 3);
	GT911_RD_Reg(GT911_CONFIG_REG, (uint8_t *)&buf[3], 1);
	
	char str[64] = {0};
	sprintf(str, "TouchPad_ID:%c,%c,%c\r\nTouchPad_Config_Version:%2x",buf[0],buf[1],buf[2],buf[3]);
	sendToDebug_strln(str);
}

void GT911_ReadFirmwareVersion(void)
{
	uint8_t buf[2] = {0};
	GT911_RD_Reg(GT911_FIRMWARE_VERSION_REG, buf, 2);

	char str[128] = {0};
	sprintf(str, "FirmwareVersion:%2x",(((uint16_t)buf[1] << 8) + buf[0]));
	sendToDebug_strln(str);
}

void GT911_Scan(void)
{
	char str[64] = {0};
	uint8_t buf[41] ={0};
  uint8_t Clearbuf = 0;
	
	if (GT911.Touch == 1)
	{
		GT911.Touch = 0;
		GT911_RD_Reg(GT911_READ_XY_REG, buf, 1);		

		if ((buf[0]&0x80) == 0x00)
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
			GT911_RD_Reg(GT911_READ_XY_REG+1, &buf[1], GT911.TouchCount*8);
			GT911_WR_Reg(GT911_READ_XY_REG, (uint8_t *)&Clearbuf, 1);
			
			GT911.Touchkeytrackid[0] = buf[1];
			GT911.X[0] = (uint16_t)((uint16_t)buf[3] << 8) + buf[2];
			GT911.Y[0] = (uint16_t)(480 - (((uint16_t)buf[5] << 8) + buf[4]));
			GT911.S[0] = (uint16_t)((uint16_t)buf[7] << 8) + buf[6];
			/*
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
			*/
			
			if(GT911.Y[0]<10) GT911.Y[0]=10;
			if(GT911.Y[0]>470) GT911.Y[0]=470;
			GT911.Y[0] = 480 - GT911.Y[0];
			if(GT911.X[0]<10) GT911.X[0]=10;
			if(GT911.X[0]>790) GT911.X[0]=790;	

			sprintf(str, "%d,%d ", GT911.X[0],GT911.Y[0]);
			sendToDebug_strln(str);
		}
	}
}

#endif
