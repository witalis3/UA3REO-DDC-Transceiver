#include "INA226_PWR_monitor.h"
#include "i2c.h"
#include "lcd.h"

static float Voltage = 0;
static float Current = 0;

uint16_t INA226_Read2Byte(uint8_t reg_addr) {
#ifdef HAS_TOUCHPAD
	uint16_t reg_data = 0;
	uint16_t temp = 0;

	uint8_t NumBytes = 5;
	// uint8_t buf[6] = {0};

	i2c_beginTransmission_u8(&I2C_TOUCHPAD, INA226_ADDR);
	i2c_write_u8(&I2C_TOUCHPAD, reg_addr);
	uint8_t res = i2c_endTransmission(&I2C_TOUCHPAD);
	if (res == 0) {
		if (i2c_beginReceive_u8(&I2C_TOUCHPAD, INA226_ADDR)) {
			reg_data = i2c_Read_Word(&I2C_TOUCHPAD);
			i2c_stop(&I2C_TOUCHPAD);
		}
	}
	return reg_data;
#else
	return 0;
#endif
}

uint8_t INA226_Write2Byte(uint8_t reg_addr, uint16_t reg_data) {
#ifdef HAS_TOUCHPAD
	uint8_t data_high = (uint8_t)((reg_data & 0xFF00) >> 8);
	uint8_t data_low = (uint8_t)reg_data & 0x00FF;

	i2c_beginTransmission_u8(&I2C_TOUCHPAD, INA226_ADDR);
	i2c_write_u8(&I2C_TOUCHPAD, reg_addr);
	i2c_write_u8(&I2C_TOUCHPAD, data_high);
	i2c_write_u8(&I2C_TOUCHPAD, data_low);

	uint8_t res = i2c_endTransmission(&I2C_TOUCHPAD);

	return res;
#else
	return 0;
#endif
}

void INA226_Init(void) {
	// INA226_Write2Byte(Config_Reg, 0x4527);//0100_010_100_100_111 //16 times average, 1.1ms, 1.1ms, continuous measurement of shunt voltage and
	// bus voltage
	INA226_Write2Byte(Config_Reg,
	                  0x4AD7); // 0100_101_011_010_111 //256 times average, 588�s, 332�s, continuous measurement of shunt voltage and bus voltage
	// New VBUS data is ready every 150,5ms
	// New VSHC (current) data is ready every 85ms

	// INA226_Write2Byte(Config_Reg, 0x4F27);//0100_111_100_100_111 //1024 times average, 1.1ms, 1.1ms, continuous measurement of shunt voltage and
	// bus voltage INA226_Write2Byte(Config_Reg, 0x4127);//0100_001_100_100_111 //1 times average, 1.1ms, 1.1ms, continuous measurement of shunt
	// voltage and bus voltage

	// Write the calibration byte (used for the current calculation)
	INA226_Write2Byte(Calib_Reg, 0x0800); // Current_LSB is selected 500�A R_Shunt is 5mOhm
	                                      // INA226_Write2Byte(Calib_Reg, 0x0A00);
	                                      // INA226_Write2Byte(Calib_Reg, 0x0034);
}

// Read the INA226 Voltage and Current data
void Read_INA226_Data(void) {
#define INA226_Read_Tm 6
	static uint8_t Rd_Count;

	Rd_Count++;

	// not ot overload the I2C bus, take the data oproximately every 210ms
	if (Rd_Count > INA226_Read_Tm) {
		Rd_Count = 0;

		Voltage = INA226_Read2Byte(Bus_V_Reg) * 0.00125;
		Current = INA226_Read2Byte(Current_Reg) * CALIBRATE.INA226_CurCalc *
		          0.001; // multiply the Current register value with the calibration coefficient (mA/Bit)
	}
}

// Return the INA226 Bus Voltage
float Get_INA226_Voltage(void) { return Voltage; }

// Return the INA226 Schunt Current
float Get_INA226_Current(void) { return Current; }
