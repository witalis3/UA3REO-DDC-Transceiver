#include "bands.h"
#include "functions.h"
#include <stdlib.h>
#include "settings.h"

SRAM4 BAND_MAP BANDS[BANDS_COUNT] =
	{
		// 2200METERS
		{
			.name = "2200m",
			.selectable = true,
			.startFreq = 135700,
			.endFreq = 137800,
			.regions = (const REGION_MAP[]){
				{.startFreq = 135700, .endFreq = 137800, .mode = TRX_MODE_CW},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// Long Wave Broadcast
		{
			.name = "LWBR",
			.selectable = false,
			.startFreq = 148500,
			.endFreq = 283500,
			.regions = (const REGION_MAP[]){
				{.startFreq = 148500, .endFreq = 283500, .mode = TRX_MODE_SAM},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// Medium Wave Broadcast
		{
			.name = "MWBR",
			.selectable = false,
			.startFreq = 526500,
			.endFreq = 1606500,
			.regions = (const REGION_MAP[]){
				{.startFreq = 526500, .endFreq = 1606500, .mode = TRX_MODE_SAM},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// 160METERS
		{
			.name = "160m",
			.selectable = true,
			.startFreq = 1810000,
			.endFreq = 2000000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 1810000, .endFreq = 1838000, .mode = TRX_MODE_CW},
				{.startFreq = 1838000, .endFreq = 1843000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 1843000, .endFreq = 2000000, .mode = TRX_MODE_LSB},
			},
			.regionsCount = 3,
			.channels = NULL,
			.channelsCount = 0,
		},
		// Shortwave Wave Broadcast
		{
			.name = "BR 2.4",
			.selectable = false,
			.startFreq = 2300000,
			.endFreq = 2468000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 2300000, .endFreq = 2468000, .mode = TRX_MODE_SAM},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// Shortwave Wave Broadcast
		{
			.name = "BR 3.3",
			.selectable = false,
			.startFreq = 3200000,
			.endFreq = 3400000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 3200000, .endFreq = 3400000, .mode = TRX_MODE_SAM},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// 80METERS
		{
			.name = "80m",
			.selectable = true,
			.startFreq = 3500000,
			.endFreq = 3800000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 3500000, .endFreq = 3570000, .mode = TRX_MODE_CW},
				{.startFreq = 3570000, .endFreq = 3600000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 3600000, .endFreq = 3800000, .mode = TRX_MODE_LSB},
			},
			.regionsCount = 3,
			.channels = NULL,
			.channelsCount = 0,
		},
		// Shortwave Wave Broadcast
		{
			.name = "BR 4.0",
			.selectable = false,
			.startFreq = 3950000,
			.endFreq = 4000000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 3950000, .endFreq = 4000000, .mode = TRX_MODE_SAM},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// Shortwave Wave Broadcast
		{
			.name = "BR 4.8",
			.selectable = false,
			.startFreq = 4750000,
			.endFreq = 4995000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 4750000, .endFreq = 4995000, .mode = TRX_MODE_SAM},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// Shortwave Wave Broadcast
		{
			.name = "BR 5.0",
			.selectable = false,
			.startFreq = 5005000,
			.endFreq = 5060000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 5005000, .endFreq = 5060000, .mode = TRX_MODE_SAM},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// 60METERS
		{
			.name = "60m",
			.selectable = false,
			.startFreq = 5330500,
			.endFreq = 5403500,
			.regions = (const REGION_MAP[]){
				{.startFreq = 5330500, .endFreq = 5403500, .mode = TRX_MODE_USB},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// Shortwave Wave Broadcast
		{
			.name = "BR 6.0",
			.selectable = false,
			.startFreq = 5900000,
			.endFreq = 6200000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 5900000, .endFreq = 6200000, .mode = TRX_MODE_SAM},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// 40METERS
		{
			.name = "40m",
			.selectable = true,
			.startFreq = 7000000,
			.endFreq = 7200000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 7000000, .endFreq = 7040000, .mode = TRX_MODE_CW},
				{.startFreq = 7040000, .endFreq = 7060000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 7060000, .endFreq = 7074000, .mode = TRX_MODE_LSB},
				{.startFreq = 7074000, .endFreq = 7080000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 7080000, .endFreq = 7200000, .mode = TRX_MODE_LSB},
			},
			.regionsCount = 5,
			.channels = NULL,
			.channelsCount = 0,
		},
		// Shortwave Wave Broadcast
		{
			.name = "BR 7.3",
			.selectable = false,
			.startFreq = 7200000,
			.endFreq = 7450000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 7200000, .endFreq = 7450000, .mode = TRX_MODE_SAM},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// Shortwave Wave Broadcast
		{
			.name = "BR 9.6",
			.selectable = false,
			.startFreq = 9400000,
			.endFreq = 9900000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 9400000, .endFreq = 9900000, .mode = TRX_MODE_SAM},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// 30METERS
		{
			.name = "30m",
			.selectable = true,
			.startFreq = 10100000,
			.endFreq = 10150000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 10100000, .endFreq = 10130000, .mode = TRX_MODE_CW},
				{.startFreq = 10130000, .endFreq = 10150000, .mode = TRX_MODE_DIGI_U},
			},
			.regionsCount = 2,
			.channels = NULL,
			.channelsCount = 0,
		},
		// Shortwave Wave Broadcast
		{
			.name = "BR 11.9",
			.selectable = false,
			.startFreq = 11600000,
			.endFreq = 12100000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 11600000, .endFreq = 12100000, .mode = TRX_MODE_SAM},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// Shortwave Wave Broadcast
		{
			.name = "BR 13.7",
			.selectable = false,
			.startFreq = 13570000,
			.endFreq = 13870000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 13570000, .endFreq = 13870000, .mode = TRX_MODE_SAM},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// 20METERS
		{
			.name = "20m",
			.selectable = true,
			.startFreq = 14000000,
			.endFreq = 14350000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 14000000, .endFreq = 14070000, .mode = TRX_MODE_CW},
				{.startFreq = 14070000, .endFreq = 14112000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 14112000, .endFreq = 14350000, .mode = TRX_MODE_USB},
			},
			.regionsCount = 3,
			.channels = NULL,
			.channelsCount = 0,
		},
		// Shortwave Wave Broadcast
		{
			.name = "BR 15.5",
			.selectable = false,
			.startFreq = 15100000,
			.endFreq = 15800000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 15100000, .endFreq = 15800000, .mode = TRX_MODE_SAM},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// Shortwave Wave Broadcast
		{
			.name = "BR 17.7",
			.selectable = false,
			.startFreq = 17480000,
			.endFreq = 17900000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 17480000, .endFreq = 17900000, .mode = TRX_MODE_SAM},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// 17METERS
		{
			.name = "17m",
			.selectable = true,
			.startFreq = 18068000,
			.endFreq = 18168000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 18068000, .endFreq = 18095000, .mode = TRX_MODE_CW},
				{.startFreq = 18095000, .endFreq = 18120000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 18120000, .endFreq = 18168000, .mode = TRX_MODE_USB},
			},
			.regionsCount = 3,
			.channels = NULL,
			.channelsCount = 0,
		},
		// Shortwave Wave Broadcast
		{
			.name = "BR 19.0",
			.selectable = false,
			.startFreq = 18900000,
			.endFreq = 19020000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 18900000, .endFreq = 19020000, .mode = TRX_MODE_SAM},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// 15METERS
		{
			.name = "15m",
			.selectable = true,
			.startFreq = 21000000,
			.endFreq = 21450000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 21000000, .endFreq = 21070000, .mode = TRX_MODE_CW},
				{.startFreq = 21070000, .endFreq = 21149000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 21149000, .endFreq = 21450000, .mode = TRX_MODE_USB},
			},
			.regionsCount = 4,
			.channels = NULL,
			.channelsCount = 0,
		},
		// Shortwave Wave Broadcast
		{
			.name = "BR 21.6",
			.selectable = false,
			.startFreq = 21450000,
			.endFreq = 21850000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 21450000, .endFreq = 21850000, .mode = TRX_MODE_SAM},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// 12METERS
		{
			.name = "12m",
			.selectable = true,
			.startFreq = 24890000,
			.endFreq = 24990000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 24890000, .endFreq = 24915000, .mode = TRX_MODE_CW},
				{.startFreq = 24915000, .endFreq = 24940000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 24940000, .endFreq = 24990000, .mode = TRX_MODE_USB},
			},
			.regionsCount = 4,
			.channels = NULL,
			.channelsCount = 0,
		},
		// Shortwave Wave Broadcast
		{
			.name = "BR 25.8",
			.selectable = false,
			.startFreq = 25670000,
			.endFreq = 26100000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 25670000, .endFreq = 26100000, .mode = TRX_MODE_SAM},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// CB
		{
			.name = "CB",
			.selectable = true,
			.startFreq = 26960000,
			.endFreq = 27410000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 26960000, .endFreq = 27410000, .mode = TRX_MODE_NFM},
			},
			.regionsCount = 1,
			.channels = (const CHANNEL_MAP[]){
				{.name = "1", .rxFreq = 26965000, .txFreq = 26965000},
				{.name = "2", .rxFreq = 26975000, .txFreq = 26975000},
				{.name = "3", .rxFreq = 26985000, .txFreq = 26985000},
				{.name = "4", .rxFreq = 27005000, .txFreq = 27005000},
				{.name = "5", .rxFreq = 27015000, .txFreq = 27015000},
				{.name = "6", .rxFreq = 27025000, .txFreq = 27025000},
				{.name = "7", .rxFreq = 27035000, .txFreq = 27035000},
				{.name = "8", .rxFreq = 27055000, .txFreq = 27055000},
				{.name = "9", .rxFreq = 27065000, .txFreq = 27065000},
				{.name = "10", .rxFreq = 27075000, .txFreq = 27075000},
				{.name = "11", .rxFreq = 27085000, .txFreq = 27085000},
				{.name = "12", .rxFreq = 27105000, .txFreq = 27105000},
				{.name = "13", .rxFreq = 27115000, .txFreq = 27115000},
				{.name = "14", .rxFreq = 27125000, .txFreq = 27125000},
				{.name = "15", .rxFreq = 27135000, .txFreq = 27135000},
				{.name = "16", .rxFreq = 27155000, .txFreq = 27155000},
				{.name = "17", .rxFreq = 27165000, .txFreq = 27165000},
				{.name = "18", .rxFreq = 27175000, .txFreq = 27175000},
				{.name = "19", .rxFreq = 27185000, .txFreq = 27185000},
				{.name = "20", .rxFreq = 27205000, .txFreq = 27205000},
				{.name = "21", .rxFreq = 27215000, .txFreq = 27215000},
				{.name = "22", .rxFreq = 27225000, .txFreq = 27225000},
				{.name = "23", .rxFreq = 27255000, .txFreq = 27255000},
				{.name = "24", .rxFreq = 27235000, .txFreq = 27235000},
				{.name = "25", .rxFreq = 27245000, .txFreq = 27245000},
				{.name = "26", .rxFreq = 27265000, .txFreq = 27265000},
				{.name = "27", .rxFreq = 27275000, .txFreq = 27275000},
				{.name = "28", .rxFreq = 27285000, .txFreq = 27285000},
				{.name = "29", .rxFreq = 27295000, .txFreq = 27295000},
				{.name = "30", .rxFreq = 27305000, .txFreq = 27305000},
				{.name = "31", .rxFreq = 27315000, .txFreq = 27315000},
				{.name = "32", .rxFreq = 27325000, .txFreq = 27325000},
				{.name = "33", .rxFreq = 27335000, .txFreq = 27335000},
				{.name = "34", .rxFreq = 27345000, .txFreq = 27345000},
				{.name = "35", .rxFreq = 27355000, .txFreq = 27355000},
				{.name = "36", .rxFreq = 27365000, .txFreq = 27365000},
				{.name = "37", .rxFreq = 27375000, .txFreq = 27375000},
				{.name = "38", .rxFreq = 27385000, .txFreq = 27385000},
				{.name = "39", .rxFreq = 27395000, .txFreq = 27395000},
				{.name = "40", .rxFreq = 27405000, .txFreq = 27405000},
			},
			.channelsCount = 40,
		},
		// 10METERS
		{
			.name = "10m",
			.selectable = true,
			.startFreq = 28000000,
			.endFreq = 29700000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 28000000, .endFreq = 28070000, .mode = TRX_MODE_CW},
				{.startFreq = 28070000, .endFreq = 28190000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 28190000, .endFreq = 28300000, .mode = TRX_MODE_CW},
				{.startFreq = 28300000, .endFreq = 28320000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 28320000, .endFreq = 29100000, .mode = TRX_MODE_USB},
				{.startFreq = 29100000, .endFreq = 29200000, .mode = TRX_MODE_NFM},
				{.startFreq = 29200000, .endFreq = 29300000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 29300000, .endFreq = 29520000, .mode = TRX_MODE_USB},
				{.startFreq = 29520000, .endFreq = 29700000, .mode = TRX_MODE_NFM},
			},
			.regionsCount = 9,
			.channels = NULL,
			.channelsCount = 0,
		},
		// 6METERS
		{
			.name = "6m",
			.selectable = true,
			.startFreq = 50000000,
			.endFreq = 54000000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 50000000, .endFreq = 50100000, .mode = TRX_MODE_CW},
				{.startFreq = 50100000, .endFreq = 50300000, .mode = TRX_MODE_USB},
				{.startFreq = 50300000, .endFreq = 50350000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 50350000, .endFreq = 50600000, .mode = TRX_MODE_USB},
				{.startFreq = 50600000, .endFreq = 51000000, .mode = TRX_MODE_DIGI_U},
				{.startFreq = 51000000, .endFreq = 51100000, .mode = TRX_MODE_USB},
				{.startFreq = 51100000, .endFreq = 54000000, .mode = TRX_MODE_NFM},
			},
			.regionsCount = 7,
			.channels = NULL,
			.channelsCount = 0,
		},
		// FM RADIO
		{
			.name = "FM",
			.selectable = true,
			.startFreq = 65900000,
			.endFreq = 108000000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 65900000, .endFreq = 108000000, .mode = TRX_MODE_WFM},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// AIR BAND
		{
			.name = "AIR",
			.selectable = false,
			.startFreq = 108000000,
			.endFreq = 137000000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 108000000, .endFreq = 137000000, .mode = TRX_MODE_AM},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// 2 meter
		{
			.name = "2m",
			.selectable = true,
			.startFreq = 144000000,
			.endFreq = 146000000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 144000000, .endFreq = 144110000, .mode = TRX_MODE_CW},
				{.startFreq = 144110000, .endFreq = 144491000, .mode = TRX_MODE_USB},
				{.startFreq = 144491000, .endFreq = 146000000, .mode = TRX_MODE_NFM},
			},
			.regionsCount = 3,
			.channels = (const CHANNEL_MAP[]){
				{.name = "V17", .rxFreq = 145212500, .txFreq = 145212500},
				{.name = "V18", .rxFreq = 145225000, .txFreq = 145225000},
				{.name = "V19", .rxFreq = 145237500, .txFreq = 145237500},
				{.name = "V20", .rxFreq = 145250000, .txFreq = 145250000},
				{.name = "V21", .rxFreq = 145262500, .txFreq = 145262500},
				{.name = "V22", .rxFreq = 145275000, .txFreq = 145275000},
				{.name = "V23", .rxFreq = 145287500, .txFreq = 145287500},
				{.name = "V24", .rxFreq = 145300000, .txFreq = 145300000},
				{.name = "V25", .rxFreq = 145312500, .txFreq = 145312500},
				{.name = "V26", .rxFreq = 145325000, .txFreq = 145325000},
				{.name = "V27", .rxFreq = 145337500, .txFreq = 145337500},
				{.name = "V28", .rxFreq = 145350000, .txFreq = 145350000},
				{.name = "V29", .rxFreq = 145362500, .txFreq = 145362500},
				{.name = "V30", .rxFreq = 145375000, .txFreq = 145375000},
				{.name = "V31", .rxFreq = 145387500, .txFreq = 145387500},
				{.name = "V32", .rxFreq = 145400000, .txFreq = 145400000},
				{.name = "V33", .rxFreq = 145412500, .txFreq = 145412500},
				{.name = "V34", .rxFreq = 145425000, .txFreq = 145425000},
				{.name = "V35", .rxFreq = 145437500, .txFreq = 145437500},
				{.name = "V36", .rxFreq = 145450000, .txFreq = 145450000},
				{.name = "V37", .rxFreq = 145462500, .txFreq = 145462500},
				{.name = "V38", .rxFreq = 145475000, .txFreq = 145475000},
				{.name = "V39", .rxFreq = 145487500, .txFreq = 145487500},
				{.name = "V40", .rxFreq = 145500000, .txFreq = 145500000},
				{.name = "V41", .rxFreq = 145512500, .txFreq = 145512500},
				{.name = "V42", .rxFreq = 145525000, .txFreq = 145525000},
				{.name = "V43", .rxFreq = 145537500, .txFreq = 145537500},
				{.name = "V44", .rxFreq = 145550000, .txFreq = 145550000},
				{.name = "V45", .rxFreq = 145562500, .txFreq = 145562500},
				{.name = "V46", .rxFreq = 145575000, .txFreq = 145575000},
				{.name = "V47", .rxFreq = 145587500, .txFreq = 145587500},
				{.name = "RV48", .rxFreq = 145000000, .txFreq = 145600000},
				{.name = "RV49", .rxFreq = 145012500, .txFreq = 145612500},
				{.name = "RV50", .rxFreq = 145025000, .txFreq = 145625000},
				{.name = "RV51", .rxFreq = 145037500, .txFreq = 145637500},
				{.name = "RV52", .rxFreq = 145050000, .txFreq = 145650000},
				{.name = "RV53", .rxFreq = 145062500, .txFreq = 145662500},
				{.name = "RV54", .rxFreq = 145075000, .txFreq = 145675000},
				{.name = "RV55", .rxFreq = 145087500, .txFreq = 145687500},
				{.name = "RV56", .rxFreq = 145100000, .txFreq = 145700000},
				{.name = "RV57", .rxFreq = 145112500, .txFreq = 145712500},
				{.name = "RV58", .rxFreq = 145125000, .txFreq = 145725000},
				{.name = "RV59", .rxFreq = 145137500, .txFreq = 145737500},
				{.name = "RV60", .rxFreq = 145150000, .txFreq = 145750000},
				{.name = "RV61", .rxFreq = 145162500, .txFreq = 145762500},
				{.name = "RV62", .rxFreq = 145175000, .txFreq = 145775000},
				{.name = "RV63", .rxFreq = 145187500, .txFreq = 145787500},
				{.name = "RV65", .rxFreq = 145200000, .txFreq = 145800000},
			},
			.channelsCount = 48,
		},
		// VHF Marine band
		{
			.name = "Marine",
			.selectable = false,
			.startFreq = 156000000,
			.endFreq = 174000000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 156000000, .endFreq = 174000000, .mode = TRX_MODE_NFM},
			},
			.regionsCount = 3,
			.channels = (const CHANNEL_MAP[]){
				{.name = "1", .rxFreq = 160650000, .txFreq = 156050000},
				{.name = "1A", .rxFreq = 156050000, .txFreq = 156050000},
				{.name = "2", .rxFreq = 160070000, .txFreq = 156100000},
				{.name = "3", .rxFreq = 160750000, .txFreq = 156150000},
				{.name = "4", .rxFreq = 160800000, .txFreq = 156200000},
				{.name = "4A", .rxFreq = 156200000, .txFreq = 156200000},
				{.name = "5", .rxFreq = 160850000, .txFreq = 156250000},
				{.name = "5A", .rxFreq = 156250000, .txFreq = 156250000},
				{.name = "6", .rxFreq = 156300000, .txFreq = 156300000},
				{.name = "7", .rxFreq = 160950000, .txFreq = 156350000},
				{.name = "7A", .rxFreq = 156350000, .txFreq = 156350000},
				{.name = "8", .rxFreq = 156400000, .txFreq = 156400000},
				{.name = "9", .rxFreq = 156450000, .txFreq = 156450000},
				{.name = "10", .rxFreq = 156500000, .txFreq = 156500000},
				{.name = "11", .rxFreq = 156550000, .txFreq = 156550000},
				{.name = "12", .rxFreq = 156600000, .txFreq = 156600000},
				{.name = "13", .rxFreq = 156650000, .txFreq = 156650000},
				{.name = "14", .rxFreq = 156700000, .txFreq = 156700000},
				{.name = "15", .rxFreq = 156750000, .txFreq = 156750000},
				{.name = "16", .rxFreq = 156800000, .txFreq = 156800000},
				{.name = "17", .rxFreq = 156850000, .txFreq = 156850000},
				{.name = "18", .rxFreq = 161500000, .txFreq = 156900000},
				{.name = "18A", .rxFreq = 156900000, .txFreq = 156900000},
				{.name = "19", .rxFreq = 161550000, .txFreq = 156950000},
				{.name = "19A", .rxFreq = 156950000, .txFreq = 156950000},
				{.name = "20", .rxFreq = 161600000, .txFreq = 157000000},
				{.name = "20A", .rxFreq = 157000000, .txFreq = 157000000},
				{.name = "21", .rxFreq = 161650000, .txFreq = 157050000},
				{.name = "21A", .rxFreq = 157050000, .txFreq = 157050000},
				{.name = "22", .rxFreq = 161700000, .txFreq = 157100000},
				{.name = "22A", .rxFreq = 157100000, .txFreq = 157100000},
				{.name = "23", .rxFreq = 161750000, .txFreq = 157150000},
				{.name = "23A", .rxFreq = 157150000, .txFreq = 157150000},
				{.name = "24", .rxFreq = 161800000, .txFreq = 157200000},
				{.name = "25", .rxFreq = 161850000, .txFreq = 157250000},
				{.name = "26", .rxFreq = 161900000, .txFreq = 157300000},
				{.name = "27", .rxFreq = 161950000, .txFreq = 157350000},
				{.name = "28", .rxFreq = 162000000, .txFreq = 157400000},
				{.name = "60", .rxFreq = 160625000, .txFreq = 156025000},
				{.name = "61", .rxFreq = 160675000, .txFreq = 156075000},
				{.name = "61A", .rxFreq = 156075000, .txFreq = 156075000},
				{.name = "62", .rxFreq = 160725000, .txFreq = 156125000},
				{.name = "62A", .rxFreq = 156125000, .txFreq = 156125000},
				{.name = "63", .rxFreq = 160775000, .txFreq = 156175000},
				{.name = "63A", .rxFreq = 156175000, .txFreq = 156175000},
				{.name = "64", .rxFreq = 160825000, .txFreq = 156225000},
				{.name = "64A", .rxFreq = 156225000, .txFreq = 156225000},
				{.name = "65", .rxFreq = 160875000, .txFreq = 156275000},
				{.name = "65A", .rxFreq = 156275000, .txFreq = 156275000},
				{.name = "66", .rxFreq = 160925000, .txFreq = 156325000},
				{.name = "66A", .rxFreq = 156325000, .txFreq = 156325000},
				{.name = "67", .rxFreq = 156375000, .txFreq = 156375000},
				{.name = "68", .rxFreq = 156425000, .txFreq = 156425000},
				{.name = "69", .rxFreq = 156475000, .txFreq = 156475000},
				{.name = "70", .rxFreq = 156525000, .txFreq = 156525000},
				{.name = "71", .rxFreq = 156575000, .txFreq = 156575000},
				{.name = "72", .rxFreq = 156625000, .txFreq = 156625000},
				{.name = "73", .rxFreq = 156675000, .txFreq = 156675000},
				{.name = "74", .rxFreq = 156725000, .txFreq = 156725000},
				{.name = "75", .rxFreq = 156775000, .txFreq = 156775000},
				{.name = "76", .rxFreq = 156825000, .txFreq = 156825000},
				{.name = "77", .rxFreq = 156875000, .txFreq = 156875000},
				{.name = "78", .rxFreq = 161525000, .txFreq = 156925000},
				{.name = "78A", .rxFreq = 156925000, .txFreq = 156925000},
				{.name = "79", .rxFreq = 161575000, .txFreq = 156975000},
				{.name = "79A", .rxFreq = 156975000, .txFreq = 156975000},
				{.name = "80", .rxFreq = 161625000, .txFreq = 157025000},
				{.name = "80A", .rxFreq = 157025000, .txFreq = 157025000},
				{.name = "81", .rxFreq = 161675000, .txFreq = 157075000},
				{.name = "81A", .rxFreq = 157075000, .txFreq = 157075000},
				{.name = "82", .rxFreq = 161725000, .txFreq = 157125000},
				{.name = "82A", .rxFreq = 157125000, .txFreq = 157125000},
				{.name = "83", .rxFreq = 161775000, .txFreq = 157175000},
				{.name = "83A", .rxFreq = 157175000, .txFreq = 157175000},
				{.name = "84", .rxFreq = 161825000, .txFreq = 157225000},
				{.name = "85", .rxFreq = 161875000, .txFreq = 157275000},
				{.name = "86", .rxFreq = 161925000, .txFreq = 157325000},
				{.name = "87", .rxFreq = 157375000, .txFreq = 157375000},
				{.name = "88", .rxFreq = 157425000, .txFreq = 157425000},
				{.name = "W01", .rxFreq = 162550000, .txFreq = 162550000},
				{.name = "W02", .rxFreq = 162400000, .txFreq = 162400000},
				{.name = "W03", .rxFreq = 162475000, .txFreq = 162475000},
				{.name = "W04", .rxFreq = 162425000, .txFreq = 162425000},
				{.name = "W05", .rxFreq = 162450000, .txFreq = 162450000},
				{.name = "W06", .rxFreq = 162500000, .txFreq = 162500000},
				{.name = "W07", .rxFreq = 162525000, .txFreq = 162525000},
				//{.name = "W08", .rxFreq = 161750000, .txFreq =	161750000},
				//{.name = "W09", .rxFreq = 161775000, .txFreq =	161775000},
				{.name = "W10", .rxFreq = 163275000, .txFreq = 163275000},
			},
			.channelsCount = 87,
		},
		// 70cm
		{
			.name = "70cm",
			.selectable = true,
			.startFreq = 430000000,
			.endFreq = 446200000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 430000000, .endFreq = 432000000, .mode = TRX_MODE_NFM},
				{.startFreq = 432000000, .endFreq = 432100000, .mode = TRX_MODE_CW},
				{.startFreq = 432100000, .endFreq = 432400000, .mode = TRX_MODE_USB},
				{.startFreq = 432400000, .endFreq = 432500000, .mode = TRX_MODE_CW},
				{.startFreq = 432500000, .endFreq = 434000000, .mode = TRX_MODE_NFM},
				{.startFreq = 434000000, .endFreq = 434100000, .mode = TRX_MODE_CW},
				{.startFreq = 434100000, .endFreq = 446200000, .mode = TRX_MODE_NFM},
			},
			.regionsCount = 7,
			.channels = (const CHANNEL_MAP[]){
				{.name = "L1", .rxFreq = 433075000, .txFreq = 433075000},
				{.name = "L2", .rxFreq = 433100000, .txFreq = 433100000},
				{.name = "L3", .rxFreq = 433125000, .txFreq = 433125000},
				{.name = "L4", .rxFreq = 433150000, .txFreq = 433150000},
				{.name = "L5", .rxFreq = 433175000, .txFreq = 433175000},
				{.name = "L6", .rxFreq = 433200000, .txFreq = 433200000},
				{.name = "L7", .rxFreq = 433225000, .txFreq = 433225000},
				{.name = "L8", .rxFreq = 433250000, .txFreq = 433250000},
				{.name = "L9", .rxFreq = 433275000, .txFreq = 433275000},
				{.name = "L10", .rxFreq = 433300000, .txFreq = 433300000},
				{.name = "L11", .rxFreq = 433325000, .txFreq = 433325000},
				{.name = "L12", .rxFreq = 433350000, .txFreq = 433350000},
				{.name = "L13", .rxFreq = 433375000, .txFreq = 433375000},
				{.name = "L14", .rxFreq = 433400000, .txFreq = 433400000},
				{.name = "L15", .rxFreq = 433425000, .txFreq = 433425000},
				{.name = "L16", .rxFreq = 433450000, .txFreq = 433450000},
				{.name = "L17", .rxFreq = 433475000, .txFreq = 433475000},
				{.name = "L18", .rxFreq = 433500000, .txFreq = 433500000},
				{.name = "L19", .rxFreq = 433525000, .txFreq = 433525000},
				{.name = "L20", .rxFreq = 433550000, .txFreq = 433550000},
				{.name = "L21", .rxFreq = 433575000, .txFreq = 433575000},
				{.name = "L22", .rxFreq = 433600000, .txFreq = 433600000},
				{.name = "L23", .rxFreq = 433625000, .txFreq = 433625000},
				{.name = "L24", .rxFreq = 433650000, .txFreq = 433650000},
				{.name = "L25", .rxFreq = 433675000, .txFreq = 433675000},
				{.name = "L26", .rxFreq = 433700000, .txFreq = 433700000},
				{.name = "L27", .rxFreq = 433725000, .txFreq = 433725000},
				{.name = "L28", .rxFreq = 433750000, .txFreq = 433750000},
				{.name = "L29", .rxFreq = 433775000, .txFreq = 433775000},
				{.name = "L30", .rxFreq = 433800000, .txFreq = 433800000},
				{.name = "L31", .rxFreq = 433825000, .txFreq = 433825000},
				{.name = "L32", .rxFreq = 433850000, .txFreq = 433850000},
				{.name = "L33", .rxFreq = 433875000, .txFreq = 433875000},
				{.name = "L34", .rxFreq = 433900000, .txFreq = 433900000},
				{.name = "L35", .rxFreq = 433925000, .txFreq = 433925000},
				{.name = "L36", .rxFreq = 433950000, .txFreq = 433950000},
				{.name = "L37", .rxFreq = 433975000, .txFreq = 433975000},
				{.name = "L38", .rxFreq = 434000000, .txFreq = 434000000},
				{.name = "L39", .rxFreq = 434025000, .txFreq = 434025000},
				{.name = "L40", .rxFreq = 434050000, .txFreq = 434050000},
				{.name = "L41", .rxFreq = 434075000, .txFreq = 434075000},
				{.name = "L42", .rxFreq = 434100000, .txFreq = 434100000},
				{.name = "L43", .rxFreq = 434125000, .txFreq = 434125000},
				{.name = "L44", .rxFreq = 434150000, .txFreq = 434150000},
				{.name = "L45", .rxFreq = 434175000, .txFreq = 434175000},
				{.name = "L46", .rxFreq = 434200000, .txFreq = 434200000},
				{.name = "L47", .rxFreq = 434225000, .txFreq = 434225000},
				{.name = "L48", .rxFreq = 434250000, .txFreq = 434250000},
				{.name = "L49", .rxFreq = 434275000, .txFreq = 434275000},
				{.name = "L50", .rxFreq = 434300000, .txFreq = 434300000},
				{.name = "L51", .rxFreq = 434325000, .txFreq = 434325000},
				{.name = "L52", .rxFreq = 434350000, .txFreq = 434350000},
				{.name = "L53", .rxFreq = 434375000, .txFreq = 434375000},
				{.name = "L54", .rxFreq = 434400000, .txFreq = 434400000},
				{.name = "L55", .rxFreq = 434425000, .txFreq = 434425000},
				{.name = "L56", .rxFreq = 434450000, .txFreq = 434450000},
				{.name = "L57", .rxFreq = 434475000, .txFreq = 434475000},
				{.name = "L58", .rxFreq = 434500000, .txFreq = 434500000},
				{.name = "L59", .rxFreq = 434525000, .txFreq = 434525000},
				{.name = "L60", .rxFreq = 434550000, .txFreq = 434550000},
				{.name = "L61", .rxFreq = 434575000, .txFreq = 434575000},
				{.name = "L62", .rxFreq = 434600000, .txFreq = 434600000},
				{.name = "L63", .rxFreq = 434625000, .txFreq = 434625000},
				{.name = "L64", .rxFreq = 434650000, .txFreq = 434650000},
				{.name = "L65", .rxFreq = 434675000, .txFreq = 434675000},
				{.name = "L66", .rxFreq = 434700000, .txFreq = 434700000},
				{.name = "L67", .rxFreq = 434725000, .txFreq = 434725000},
				{.name = "L68", .rxFreq = 434750000, .txFreq = 434750000},
				{.name = "L69", .rxFreq = 434775000, .txFreq = 434775000},
				{.name = "P1", .rxFreq = 446006250, .txFreq = 446006250},
				{.name = "P2", .rxFreq = 446018750, .txFreq = 446018750},
				{.name = "P3", .rxFreq = 446031250, .txFreq = 446031250},
				{.name = "P4", .rxFreq = 446043750, .txFreq = 446043750},
				{.name = "P5", .rxFreq = 446056250, .txFreq = 446056250},
				{.name = "P6", .rxFreq = 446068750, .txFreq = 446068750},
				{.name = "P7", .rxFreq = 446081250, .txFreq = 446081250},
				{.name = "P8", .rxFreq = 446093750, .txFreq = 446093750},
				{.name = "P9", .rxFreq = 446106250, .txFreq = 446106250},
				{.name = "P10", .rxFreq = 446118750, .txFreq = 446118750},
				{.name = "P11", .rxFreq = 446131250, .txFreq = 446131250},
				{.name = "P12", .rxFreq = 446143750, .txFreq = 446143750},
				{.name = "P13", .rxFreq = 446156250, .txFreq = 446156250},
				{.name = "P14", .rxFreq = 446168750, .txFreq = 446168750},
				{.name = "P15", .rxFreq = 446181250, .txFreq = 446181250},
				{.name = "P16", .rxFreq = 446193750, .txFreq = 446193750},
			},
			.channelsCount = 85,
		},
		// 23cm
		{
			.name = "23cm",
			.selectable = false,
			.startFreq = 1260000000,
			.endFreq = 1300000000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 1260000000, .endFreq = 1300000000, .mode = TRX_MODE_USB},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// 13cm
		{
			.name = "13cm",
			.selectable = false,
			.startFreq = 2320000000,
			.endFreq = 2450000000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 2320000000, .endFreq = 2450000000, .mode = TRX_MODE_USB},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// 6cm
		{
			.name = "6cm",
			.selectable = false,
			.startFreq = 5650000000,
			.endFreq = 5850000000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 5650000000, .endFreq = 5850000000, .mode = TRX_MODE_USB},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		// 3cm
		{
			.name = "3cm",
			.selectable = false,
			.startFreq = 10000000000,
			.endFreq = 10500000000,
			.regions = (const REGION_MAP[]){
				{.startFreq = 10000000000, .endFreq = 10500000000, .mode = TRX_MODE_USB},
			},
			.regionsCount = 1,
			.channels = NULL,
			.channelsCount = 0,
		},
		//
};

// band number from frequency
int8_t getBandFromFreq(uint64_t freq, bool nearest)
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
uint_fast8_t getModeFromFreq(uint64_t freq)
{
	uint_fast8_t ret = 0;
	ret = CurrentVFO->Mode;

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

int8_t getChannelbyFreq(uint64_t freq, bool txfreq)
{
	int8_t band = getBandFromFreq(freq, false);
	if (band != -1)
	{
		for (uint8_t ind = 0; ind < BANDS[band].channelsCount; ind++)
		{
			if (BANDS[band].channels[ind].rxFreq == freq && !txfreq)
				return ind;
			if (BANDS[band].channels[ind].txFreq == freq && txfreq)
				return ind;
		}
	}
	return -1;
}
