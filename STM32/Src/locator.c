#include "locator.h"
#include "main.h"
#include "lcd_driver.h"
#include "trx_manager.h"
#include "functions.h"
#include "fpga.h"
#include "lcd.h"
#include "fonts.h"

// Public variables
bool SYSMENU_locator_info_opened = false;

// Private Variables
static char entered_locator[32] = {0};

// Prototypes

// start
void LOCINFO_Start(void)
{
	LCD_busy = true;

	memset(entered_locator, 0x00, sizeof(entered_locator));

	// draw the GUI
	LCDDriver_Fill(BG_COLOR);

	LCD_busy = false;
	LCD_UpdateQuery.SystemMenu = true;
}

// stop
void LOCINFO_Stop(void)
{
	LCD_hideKeyboard();
}

static void LOCINFO_keyboardHandler(uint32_t parameter)
{
	char str[2] = {0};
	str[0] = parameter;
	if (parameter == '<') // backspace
	{
		if (strlen(entered_locator) > 0)
			entered_locator[strlen(entered_locator) - 1] = 0;
	}
	else if (strlen(entered_locator) < 8)
		strcat(entered_locator, str);

	LCD_UpdateQuery.SystemMenuRedraw = true;
}

// draw
void LOCINFO_Draw(void)
{
	if (LCD_busy)
	{
		LCD_UpdateQuery.SystemMenuRedraw = true;
		return;
	}
	LCD_busy = true;

	float32_t lat = LOCINFO_get_latlon_from_locator(entered_locator, true);
	float32_t lon = LOCINFO_get_latlon_from_locator(entered_locator, false);
	float32_t my_lat = LOCINFO_get_latlon_from_locator(TRX.LOCATOR, true);
	float32_t my_lon = LOCINFO_get_latlon_from_locator(TRX.LOCATOR, false);
	uint16_t distance = LOCINFO_distanceInKmBetweenEarthCoordinates(my_lat, my_lon, lat, lon);
	int16_t azimuth = LOCINFO_azimuthFromCoordinates(my_lat, my_lon, lat, lon);

	char tmp[64] = {0};

	sprintf(tmp, "My Locator: %s", TRX.LOCATOR);
	addSymbols(tmp, tmp, 15 + 8, " ", true);
	LCDDriver_printText(tmp, 10, 30, FG_COLOR, BG_COLOR, 2);

	sprintf(tmp, "Target Locator: %s", entered_locator);
	addSymbols(tmp, tmp, 15 + 8, " ", true);
	LCDDriver_printText(tmp, 10, 50, FG_COLOR, BG_COLOR, 2);

	sprintf(tmp, "LAT: %f", lat);
	addSymbols(tmp, tmp, 15 + 8, " ", true);
	LCDDriver_printText(tmp, 10, 70, FG_COLOR, BG_COLOR, 2);

	sprintf(tmp, "LON: %f", lon);
	addSymbols(tmp, tmp, 15 + 8, " ", true);
	LCDDriver_printText(tmp, 10, 90, FG_COLOR, BG_COLOR, 2);

	sprintf(tmp, "Distance: %d km", distance);
	addSymbols(tmp, tmp, 15 + 8, " ", true);
	LCDDriver_printText(tmp, 10, 110, FG_COLOR, BG_COLOR, 2);

	sprintf(tmp, "Azimuth: %d deg", azimuth);
	addSymbols(tmp, tmp, 15 + 8, " ", true);
	LCDDriver_printText(tmp, 10, 130, FG_COLOR, BG_COLOR, 2);

	LCD_printKeyboard(LOCINFO_keyboardHandler, false);

	LCD_busy = false;
}

// events to the encoder
void LOCINFO_EncRotate(int8_t direction)
{
	/*if (LCD_busy)
		return;
	LCD_busy = true;

	LCD_busy = false;*/
}

float32_t LOCINFO_get_latlon_from_locator(char *locator, bool return_lat)
{
	// origin
	float32_t lon = -180.0f; // Positive: East, negative: West.
	float32_t lat = -90.0f;	 // Positive: North, negative: South.

	if (strlen(locator) >= 1)
		lon += (locator[0] - 65) * 20.0f;
	if (strlen(locator) >= 2)
		lat += (locator[1] - 65) * 10.0f;
	if (strlen(locator) >= 3)
		lon += charToInt(locator[2]) * 2.0f;
	if (strlen(locator) >= 4)
		lat += charToInt(locator[3]) * 1.0f;
	if (strlen(locator) >= 5)
		lon += (locator[4] - 65) * (5.0f / 60.0f);
	if (strlen(locator) >= 6)
		lat += (locator[5] - 65) * (2.5f / 60.0f);

	// averaging
	if (strlen(locator) >= 5)
	{
		lon += (2.5f / 60.0f);
		lat += (1.25f / 60.0f);
	}
	else if (strlen(locator) >= 3)
	{
		// averaging
		lon += 1.0f;
		lat += 0.5f;
	}
	else if (strlen(locator) >= 1)
	{
		// averaging
		lon += 10.0f;
		lat += 5.0f;
	}

	if (return_lat)
		return lat;
	else
		return lon;
}

float32_t LOCINFO_distanceInKmBetweenEarthCoordinates(float32_t lat1, float32_t lon1, float32_t lat2, float32_t lon2)
{
	float32_t earthRadiusKm = 6371.0f;

	float32_t dLat = DEG2RAD(lat2 - lat1);
	float32_t dLon = DEG2RAD(lon2 - lon1);

	lat1 = DEG2RAD(lat1);
	lat2 = DEG2RAD(lat2);

	float32_t a = arm_sin_f32(dLat / 2.0f) * arm_sin_f32(dLat / 2.0f) + arm_sin_f32(dLon / 2.0f) * arm_sin_f32(dLon / 2.0f) * arm_cos_f32(lat1) * arm_cos_f32(lat2);
	float32_t angle;
	arm_atan2_f32(sqrtf(a), sqrtf(1.0f - a), &angle);
	float32_t c = 2.0f * angle;

	return earthRadiusKm * c;
}

float32_t LOCINFO_azimuthFromCoordinates(float32_t lat1, float32_t lon1, float32_t lat2, float32_t lon2)
{
	float32_t dLat = DEG2RAD(lat2 - lat1);
	float32_t dLon = DEG2RAD(lon2 - lon1);

	lat1 = DEG2RAD(lat1);
	lat2 = DEG2RAD(lat2);

	float32_t azimuth;
	arm_atan2_f32((arm_sin_f32(dLon) * arm_cos_f32(lat2)), (arm_cos_f32(lat1) * arm_sin_f32(lat2) - arm_sin_f32(lat1) * arm_cos_f32(lat2) * arm_cos_f32(dLon)), &azimuth);

	azimuth = RAD2DEG(azimuth);
	while (azimuth < 0)
		azimuth += 360.0f;

	return azimuth;
}
