#include "bands.h"
#include "functions.h"
#include <stdlib.h>
#include "settings.h"

const BAND_MAP BANDS[BANDS_COUNT] =
	{
		//2200METERS
		{
			.name = "2200m",
			.selectable = true,
			.startFreq = 135700,
			.endFreq = 137800,
			.regions = (const REGION_MAP[]){
				{.startFreq = 135700, .endFreq = 137800, .mode = TRX_MODE_CW_L},
			},
			.regionsCount = 1,
		},
		//Long Wave Broadcast
		{
			.name = "LWBR",
			.selectable = false,
			.startFreq = 148500,
			.endFreq = 283500,
			.regions = (const REGION_MAP[]){
				{.startFreq = 148500, .endFreq = 283500, .mode = TRX_MODE_AM},
			},
			.regionsCount = 1,
		},
		//Medium Wave Broadcast
		{
			.name = "MWBR",
			.selectable = false,
			.startFreq = 526500,
			.endFreq = 1606500,
			.regions = (const REGION_MAP[]){
				{.startFreq = 526500, .endFreq = 1606500, .mode = TRX_MODE_AM},
			},
			.regionsCount = 1,
		},
		//160METERS
		{
			.name = "160m",
			.selectable = true,
			.startFreq = 1810000,
			.endFreq = 2000000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 1810000, .endFreq = 1838000, .mode = TRX_MODE_CW_L},
				{.startFreq = 1838000, .endFreq = 1843000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 1843000, .endFreq = 2000000, .mode = TRX_MODE_LSB},
			},
			.regionsCount = 3,
		},
		//Shortwave Wave Broadcast
		{
			.name = "SBR1",
			.selectable = false,
			.startFreq = 2300000,
			.endFreq = 2468000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 2300000, .endFreq = 2468000, .mode = TRX_MODE_AM},
			},
			.regionsCount = 1,
		},
		//Shortwave Wave Broadcast
		{
			.name = "SBR2",
			.selectable = false,
			.startFreq = 3200000,
			.endFreq = 3400000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 3200000, .endFreq = 3400000, .mode = TRX_MODE_AM},
			},
			.regionsCount = 1,
		},
		//80METERS
		{
			.name = "80m",
			.selectable = true,
			.startFreq = 3500000,
			.endFreq = 3800000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 3500000, .endFreq = 3570000, .mode = TRX_MODE_CW_L},
				{.startFreq = 3570000, .endFreq = 3620000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 3620000, .endFreq = 3800000, .mode = TRX_MODE_LSB},
			},
			.regionsCount = 3,
		},
		//Shortwave Wave Broadcast
		{
			.name = "SBR3",
			.selectable = false,
			.startFreq = 3950000,
			.endFreq = 4000000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 3950000, .endFreq = 4000000, .mode = TRX_MODE_AM},
			},
			.regionsCount = 1,
		},
		//Shortwave Wave Broadcast
		{
			.name = "SBR4",
			.selectable = false,
			.startFreq = 4750000,
			.endFreq = 4995000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 4750000, .endFreq = 4995000, .mode = TRX_MODE_AM},
			},
			.regionsCount = 1,
		},
		//Shortwave Wave Broadcast
		{
			.name = "SBR5",
			.selectable = false,
			.startFreq = 5005000,
			.endFreq = 5060000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 5005000, .endFreq = 5060000, .mode = TRX_MODE_AM},
			},
			.regionsCount = 1,
		},
		//60METERS
		{
			.name = "60m",
			.selectable = false,
			.startFreq = 5330500,
			.endFreq = 5403500,
			.regions = (const REGION_MAP[]){
				{.startFreq = 5330500, .endFreq = 5403500, .mode = TRX_MODE_USB},
			},
			.regionsCount = 1,
		},
		//Shortwave Wave Broadcast
		{
			.name = "SBR6",
			.selectable = false,
			.startFreq = 5900000,
			.endFreq = 6200000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 5900000, .endFreq = 6200000, .mode = TRX_MODE_AM},
			},
			.regionsCount = 1,
		},
		//40METERS
		{
			.name = "40m",
			.selectable = true,
			.startFreq = 7000000,
			.endFreq = 7200000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 7000000, .endFreq = 7040000, .mode = TRX_MODE_CW_L},
				{.startFreq = 7040000, .endFreq = 7060000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 7060000, .endFreq = 7074000, .mode = TRX_MODE_LSB},
				{.startFreq = 7074000, .endFreq = 7080000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 7080000, .endFreq = 7200000, .mode = TRX_MODE_LSB},
			},
			.regionsCount = 5,
		},
		//Shortwave Wave Broadcast
		{
			.name = "SBR7",
			.selectable = false,
			.startFreq = 7200000,
			.endFreq = 7450000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 7200000, .endFreq = 7450000, .mode = TRX_MODE_AM},
			},
			.regionsCount = 1,
		},
		//Shortwave Wave Broadcast
		{
			.name = "SBR8",
			.selectable = false,
			.startFreq = 9400000,
			.endFreq = 9900000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 9400000, .endFreq = 9900000, .mode = TRX_MODE_AM},
			},
			.regionsCount = 1,
		},
		//30METERS
		{
			.name = "30m",
			.selectable = true,
			.startFreq = 10100000,
			.endFreq = 10150000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 10100000, .endFreq = 10130000, .mode = TRX_MODE_CW_U},
				{.startFreq = 10130000, .endFreq = 10150000, .mode = TRX_MODE_DIGI_U},
			},
			.regionsCount = 2,
		},
		//Shortwave Wave Broadcast
		{
			.name = "SBR9",
			.selectable = false,
			.startFreq = 11600000,
			.endFreq = 12100000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 11600000, .endFreq = 12100000, .mode = TRX_MODE_AM},
			},
			.regionsCount = 1,
		},
		//Shortwave Wave Broadcast
		{
			.name = "SBR10",
			.selectable = false,
			.startFreq = 13570000,
			.endFreq = 13870000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 13570000, .endFreq = 13870000, .mode = TRX_MODE_AM},
			},
			.regionsCount = 1,
		},
		//20METERS
		{
			.name = "20m",
			.selectable = true,
			.startFreq = 14000000,
			.endFreq = 14350000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 14000000, .endFreq = 14070000, .mode = TRX_MODE_CW_U},
				{.startFreq = 14070000, .endFreq = 14112000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 14112000, .endFreq = 14350000, .mode = TRX_MODE_USB},
			},
			.regionsCount = 3,
		},
		//Shortwave Wave Broadcast
		{
			.name = "SBR11",
			.selectable = false,
			.startFreq = 15100000,
			.endFreq = 15800000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 15100000, .endFreq = 15800000, .mode = TRX_MODE_AM},
			},
			.regionsCount = 1,
		},
		//Shortwave Wave Broadcast
		{
			.name = "SBR12",
			.selectable = false,
			.startFreq = 17480000,
			.endFreq = 17900000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 17480000, .endFreq = 17900000, .mode = TRX_MODE_AM},
			},
			.regionsCount = 1,
		},
		//17METERS
		{
			.name = "17m",
			.selectable = true,
			.startFreq = 18068000,
			.endFreq = 18168000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 18068000, .endFreq = 18095000, .mode = TRX_MODE_CW_U},
				{.startFreq = 18095000, .endFreq = 18120000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 18120000, .endFreq = 18168000, .mode = TRX_MODE_USB},
			},
			.regionsCount = 3,
		},
		//Shortwave Wave Broadcast
		{
			.name = "SBR13",
			.selectable = false,
			.startFreq = 18900000,
			.endFreq = 19020000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 18900000, .endFreq = 19020000, .mode = TRX_MODE_AM},
			},
			.regionsCount = 1,
		},
		//15METERS
		{
			.name = "15m",
			.selectable = true,
			.startFreq = 21000000,
			.endFreq = 21450000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 21000000, .endFreq = 21070000, .mode = TRX_MODE_CW_U},
				{.startFreq = 21070000, .endFreq = 21149000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 21149000, .endFreq = 21450000, .mode = TRX_MODE_USB},
			},
			.regionsCount = 4,
		},
		//Shortwave Wave Broadcast
		{
			.name = "SBR14",
			.selectable = false,
			.startFreq = 21450000,
			.endFreq = 21850000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 21450000, .endFreq = 21850000, .mode = TRX_MODE_AM},
			},
			.regionsCount = 1,
		},
		//12METERS
		{
			.name = "12m",
			.selectable = true,
			.startFreq = 24890000,
			.endFreq = 24990000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 24890000, .endFreq = 24915000, .mode = TRX_MODE_CW_U},
				{.startFreq = 24915000, .endFreq = 24940000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 24940000, .endFreq = 24990000, .mode = TRX_MODE_USB},
			},
			.regionsCount = 4,
		},
		//Shortwave Wave Broadcast
		{
			.name = "SBR15",
			.selectable = false,
			.startFreq = 25670000,
			.endFreq = 26100000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 25670000, .endFreq = 26100000, .mode = TRX_MODE_AM},
			},
			.regionsCount = 1,
		},
		//CB
		{
			.name = "CB",
			.selectable = true,
			.startFreq = 26960000,
			.endFreq = 27410000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 26960000, .endFreq = 27410000, .mode = TRX_MODE_NFM},
			},
			.regionsCount = 1,
		},
		//10METERS
		{
			.name = "10m",
			.selectable = true,
			.startFreq = 28000000,
			.endFreq = 29700000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 28000000, .endFreq = 28070000, .mode = TRX_MODE_CW_U},
				{.startFreq = 28070000, .endFreq = 28190000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 28190000, .endFreq = 28300000, .mode = TRX_MODE_CW_U},
				{.startFreq = 28300000, .endFreq = 28320000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 28320000, .endFreq = 29100000, .mode = TRX_MODE_USB},
				{.startFreq = 29100000, .endFreq = 29200000, .mode = TRX_MODE_NFM},
				{.startFreq = 29200000, .endFreq = 29300000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 29300000, .endFreq = 29520000, .mode = TRX_MODE_USB},
				{.startFreq = 29520000, .endFreq = 29700000, .mode = TRX_MODE_NFM},
			},
			.regionsCount = 9,
		},
		//6METERS
		{
			.name = "6m",
			.selectable = true,
			.startFreq = 50000000,
			.endFreq = 54000000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 50000000, .endFreq = 50100000, .mode = TRX_MODE_CW_U},
				{.startFreq = 50100000, .endFreq = 50300000, .mode = TRX_MODE_USB},
				{.startFreq = 50300000, .endFreq = 50350000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 50350000, .endFreq = 50600000, .mode = TRX_MODE_USB},
				{.startFreq = 50600000, .endFreq = 51000000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 51000000, .endFreq = 51100000, .mode = TRX_MODE_USB},
				{.startFreq = 51100000, .endFreq = 54000000, .mode = TRX_MODE_NFM},
			},
			.regionsCount = 7,
		},
		//FM RADIO
		{
			.name = "FM",
			.selectable = true,
			.startFreq = 65900000,
			.endFreq = 74000000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 65900000, .endFreq = 108000000, .mode = TRX_MODE_WFM},
			},
			.regionsCount = 1,
		},
		//FM RADIO
		{
			.name = "AIR",
			.selectable = false,
			.startFreq = 108000000,
			.endFreq = 137000000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 108000000, .endFreq = 137000000, .mode = TRX_MODE_AM},
			},
			.regionsCount = 1,
		},
		//2 meter
		{
			.name = "VHF",
			.selectable = true,
			.startFreq = 144000000,
			.endFreq = 146000000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 144000000, .endFreq = 144110000, .mode = TRX_MODE_CW_U},
				{.startFreq = 144110000, .endFreq = 144491000, .mode = TRX_MODE_USB},
				{.startFreq = 144491000, .endFreq = 146000000, .mode = TRX_MODE_NFM},
			},
			.regionsCount = 3,
		},
		//70cm
		{
			.name = "UHF",
			.selectable = true,
			.startFreq = 430000000,
			.endFreq = 440000000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 430000000, .endFreq = 432000000, .mode = TRX_MODE_NFM},
				{.startFreq = 432000000, .endFreq = 432100000, .mode = TRX_MODE_CW_U},
				{.startFreq = 432100000, .endFreq = 432400000, .mode = TRX_MODE_USB},
				{.startFreq = 432400000, .endFreq = 432500000, .mode = TRX_MODE_CW_U},
				{.startFreq = 432500000, .endFreq = 434000000, .mode = TRX_MODE_NFM},
				{.startFreq = 434000000, .endFreq = 434100000, .mode = TRX_MODE_CW_U},
				{.startFreq = 434100000, .endFreq = 440000000, .mode = TRX_MODE_NFM},
			},
			.regionsCount = 7,
		},
		//
};

// band number from frequency
int8_t getBandFromFreq(uint32_t freq, bool nearest)
{
	for (int8_t b = 0; b < BANDS_COUNT; b++)
		if (BANDS[b].startFreq <= freq && freq <= BANDS[b].endFreq)
			return b;

	if (nearest)
	{
		int8_t near_band = 0;
		int32_t near_diff = 999999999;
		for (int8_t b = 0; b < BANDS_COUNT; b++)
		{
			if (abs((int32_t)BANDS[b].startFreq - (int32_t)freq) < near_diff)
			{
				near_diff = abs((int32_t)BANDS[b].startFreq - (int32_t)freq);
				near_band = b;
			}
			if (abs((int32_t)BANDS[b].endFreq - (int32_t)freq) < near_diff)
			{
				near_diff = abs((int32_t)BANDS[b].endFreq - (int32_t)freq);
				near_band = b;
			}
		}
		return near_band;
	}

	return -1;
}

// mod from frequency
uint_fast8_t getModeFromFreq(uint32_t freq)
{
	uint_fast8_t ret = 0;
	ret = CurrentVFO()->Mode;

	for (uint_fast16_t b = 0; b < BANDS_COUNT; b++)
	{
		if (BANDS[b].startFreq <= freq && freq <= BANDS[b].endFreq)
			for (uint_fast16_t r = 0; r < BANDS[b].regionsCount; r++)
			{
				if (BANDS[b].regions[r].startFreq <= freq && freq < BANDS[b].regions[r].endFreq)
				{
					ret = BANDS[b].regions[r].mode;
					return ret;
				}
			}
	}
	return ret;
}
