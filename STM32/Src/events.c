#include "events.h"
#include "main.h"
#include "system_menu.h"
#include "lcd.h"
#include "wspr.h"
#include "wifi.h"
#include "trx_manager.h"
#include "swr_analyzer.h"
#include "fft.h"
#include "usbd_debug_if.h"
#include "usbd_cat_if.h"
#include "usbd_audio_if.h"
#include "usbd_ua3reo.h"
#include "FT8\FT8_main.h"
#include "front_unit.h"
#include "rf_unit.h"
#include "fpga.h"
#include "codec.h"
#include "audio_processor.h"
#include "agc.h"
#include "fft.h"
#include "settings.h"
#include "fpga.h"
#include "profiler.h"
#include "audio_filters.h"
#include "vocoder.h"
#include "bootloader.h"
#include "decoder.h"
#include "sd.h"
#include "cw.h"
#include "functions.h"
#include "snap.h"

void EVENTS_do_WSPR(void) // 1,4648 hz
{
	if (SYSMENU_wspr_opened)
    WSPR_DoFastEvents();
}

void EVENTS_do_WIFI(void) // 1000 hz
{
	static uint16_t wifi_start_timeout = 0;
	
	//process wifi
  if (wifi_start_timeout < 10)
  {
    wifi_start_timeout++;
  }
  else
  {
		#if HRDW_HAS_WIFI
    // we work with WiFi by timer, or send it if it is turned off (to turn it on, we need a restart)
    if (TRX.WIFI_Enabled)
      WIFI_Process();
    else
      WIFI_GoSleep();
		#endif
  }
}

void EVENTS_do_FFT(void) // 1000 hz
{
	if (SYSMENU_spectrum_opened || SYSMENU_swr_opened)
  {
    SYSMENU_drawSystemMenu(false, false);
    return;
  }

  if (FFT_need_fft)
    FFT_doFFT();

  ua3reo_dev_cat_parseCommand();
}

void EVENTS_do_AUDIO_PROCESSOR(void) // 20 000 hz
{
	if (!Processor_NeedTXBuffer && !Processor_NeedRXBuffer)
    return;

  if (TRX_on_TX)
  {
		processTxAudio();
  }
  else
  {
    processRxAudio();
  }
	
  //in the spectrum analyzer mode, we raise its processing to priority, performing together with the audio processor
  if (SYSMENU_spectrum_opened)
    LCD_doEvents();
	
	#if FT8_SUPPORT
	if(FT8_DecodeActiveFlg) {
		TRX_Inactive_Time = 0;
		MenagerFT8();
	}
	#endif
}

void EVENTS_do_USB_FIFO(void) // 1000 hz
{
	print_flush(); // send data to debug from the buffer

  // unmute after transition process end
  if (TRX_Temporary_Mute_StartTime > 0 && (HAL_GetTick() - TRX_Temporary_Mute_StartTime) > 100)
  {
    CODEC_UnMute();
    TRX_Temporary_Mute_StartTime = 0;
  }
}

void EVENTS_do_PERIPHERAL(void) // 1000 hz
{
	#if HRDW_HAS_SD
	if (SD_BusyByUSB)
    return;
	#endif

  //FRONT PANEL SPI
  static uint16_t front_slowler = 0;
  front_slowler++;
  if (front_slowler > 20)
  {
    FRONTPANEL_Process();
    front_slowler = 0;
  }

  //EEPROM SPI
  if (NeedSaveCalibration) // save calibration data to EEPROM
    SaveCalibration();

	#if HRDW_HAS_SD
  //SD-Card SPI
  SD_Process();
	#endif
}

void EVENTS_do_ENC(void) // 20 0000 hz
{
	// Update watchdog
	HAL_IWDG_Refresh(&HRDW_IWDG);
	
  // Poll an additional encoder by timer, because interrupt hangs in line with FPGA
  static uint8_t ENC2lastClkVal = 0;
  static bool ENC2first = true;
  uint8_t ENCODER2_CLKVal = HAL_GPIO_ReadPin(ENC2_CLK_GPIO_Port, ENC2_CLK_Pin);
  if (ENC2first)
  {
    ENC2lastClkVal = ENCODER2_CLKVal;
    ENC2first = false;
  }
  if (ENC2lastClkVal != ENCODER2_CLKVal)
  {
    if (TRX_Inited)
      FRONTPANEL_ENCODER2_checkRotate();
    ENC2lastClkVal = ENCODER2_CLKVal;
  }
	
#ifdef HAS_TOUCHPAD
  static bool TOUCH_Int_Last = true;
  bool TOUCH_Int_Now = HAL_GPIO_ReadPin(ENC2SW_AND_TOUCHPAD_GPIO_Port, ENC2SW_AND_TOUCHPAD_Pin);
  if (TOUCH_Int_Last != TOUCH_Int_Now)
  {
    TOUCH_Int_Last = TOUCH_Int_Now;
    if (TOUCH_Int_Now)
      TOUCHPAD_reserveInterrupt();
  }
  return;
#endif
}

void EVENTS_do_PREPROCESS(void) // 1000 hz
{
	//audio buffer RX preprocessor
  if (!TRX_on_TX)
    preProcessRxAudio();

  if (FFT_new_buffer_ready)
    FFT_bufferPrepare();

  if (NeedProcessDecoder)
    DECODER_Process();
}

void EVENTS_do_EVERY_10ms(void) // 100 hz
{
	static uint32_t powerdown_start_delay = 0;
	static bool prev_pwr_state = true;
	static uint32_t ms10_10_counter = 0;
	static uint32_t ms10_30_counter = 0;
	ms10_10_counter++;
	ms10_30_counter++;
	
	//power off sequence
  if (prev_pwr_state == true && HAL_GPIO_ReadPin(PWR_ON_GPIO_Port, PWR_ON_Pin) == GPIO_PIN_RESET)
  {
    powerdown_start_delay = HAL_GetTick();
  }
  prev_pwr_state = HAL_GPIO_ReadPin(PWR_ON_GPIO_Port, PWR_ON_Pin);

  if ((HAL_GPIO_ReadPin(PWR_ON_GPIO_Port, PWR_ON_Pin) == GPIO_PIN_RESET) && ((HAL_GetTick() - powerdown_start_delay) > POWERDOWN_TIMEOUT) 
		&& ((!NeedSaveCalibration && !HRDW_SPI_Locked && !EEPROM_Busy && !LCD_busy) || ((HAL_GetTick() - powerdown_start_delay) > POWERDOWN_FORCE_TIMEOUT)))
  {
    TRX_Inited = false;
    LCD_busy = true;
    HAL_Delay(10);
    CODEC_Mute();
    CODEC_CleanBuffer();
    LCDDriver_Fill(COLOR_BLACK);
    LCD_showInfo("GOOD BYE!", false);
    SaveSettings();
    SaveSettingsToEEPROM();
    print_flush();
    while (HAL_GPIO_ReadPin(PWR_ON_GPIO_Port, PWR_ON_Pin) == GPIO_PIN_RESET)
    {
    }
    HAL_Delay(500);
    HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_RESET);
    //SCB->AIRCR = 0x05FA0004; // software reset
    while (true)
    {
    }
  }

  //Process SWR, Power meter, ALC, Thermal sensors, Fan, ...
  RF_UNIT_ProcessSensors();

  //TRX protector
  if (TRX_on_TX)
  {
		TRX_TX_EndTime = HAL_GetTick();
		
    if (TRX_RF_Temperature > CALIBRATE.TRX_MAX_RF_TEMP)
    {
      TRX_Tune = false;
      TRX_ptt_hard = false;
      TRX_ptt_soft = false;
      LCD_UpdateQuery.StatusInfoGUIRedraw = true;
			LCD_UpdateQuery.StatusInfoBarRedraw = true;
      LCD_UpdateQuery.TopButtons = true;
      NeedSaveSettings = true;
      TRX_Restart_Mode();
      println("RF temperature too HIGH!");
      LCD_showTooltip("RF temperature too HIGH!");
    }
    /*if (TRX_SWR_SMOOTHED > CALIBRATE.TRX_MAX_SWR && !TRX_Tune && TRX_PWR_Forward > CALIBRATE.TUNE_MAX_POWER)
    {
      TRX_Tune = false;
      TRX_ptt_hard = false;
      TRX_ptt_soft = false;
      LCD_UpdateQuery.StatusInfoGUIRedraw = true;
			LCD_UpdateQuery.StatusInfoBarRedraw = true;
      LCD_UpdateQuery.TopButtons = true;
      NeedSaveSettings = true;
      TRX_Restart_Mode();
      println("SWR too HIGH!");
      LCD_showTooltip("SWR too HIGH!");
    }*/
		if (TRX_SWR_SMOOTHED > CALIBRATE.TRX_MAX_SWR && !TRX_Tune && TRX_PWR_Forward > 1.0f && !TRX_SWR_PROTECTOR)
    {
			TRX_SWR_PROTECTOR = true;
      LCD_showTooltip("SWR HIGH!");
    }
  }

  // transmission release time after key signal
  if (CW_Key_Timeout_est > 0 && !CW_key_serial && !CW_key_dot_hard && !CW_key_dash_hard)
  {
    CW_Key_Timeout_est -= 10;
    if (CW_Key_Timeout_est == 0)
    {
      FPGA_NeedSendParams = true;
      TRX_Restart_Mode();
    }
  }
	
	//VOX
	APROC_doVOX();
	
  // if the settings have changed, update the parameters in the FPGA
  if (NeedSaveSettings)
    FPGA_NeedSendParams = true;

  // there was a request to reinitialize audio and notch filters
  if (NeedReinitNotch)
    InitNotchFilter();
  if (NeedReinitAudioFilters)
    ReinitAudioFilters();

  //Process touchpad frequency changing
  TRX_setFrequencySlowly_Process();

  // emulate PTT over CAT/Software
  if (TRX_ptt_soft != TRX_old_ptt_soft)
    TRX_ptt_change();

  // emulate the key via the COM port
  if (CW_key_serial != CW_old_key_serial)
    CW_key_change();

  // update the state of the RF-Unit board
  RF_UNIT_UpdateState(false);

// check touchpad events
#ifdef HAS_TOUCHPAD
  TOUCHPAD_ProcessInterrupt();
#endif
	
	static bool needLCDDoEvents = true;
  if (ms10_30_counter >= 3) // every 30ms
  {
		ms10_30_counter = 0;
    LCD_UpdateQuery.StatusInfoBar = true;
    // update information on LCD
    needLCDDoEvents = true;
  }
  else if (LCD_UpdateQuery.FreqInfo) //Redraw freq fast
    needLCDDoEvents = true;

  if (needLCDDoEvents && LCD_doEvents())
    needLCDDoEvents = false;

  static uint8_t needPrintFFT = 0;
  if (needPrintFFT < 10 && (ms10_10_counter % (6 - TRX.FFT_Speed)) == 0) // every x msec
	{
		ms10_10_counter = 0;
    needPrintFFT++;
	}

  if (needPrintFFT > 0 && !LCD_UpdateQuery.Background && FFT_printFFT()) // draw FFT
    needPrintFFT--;
	
	// restart USB if there is no activity (off) to find a new connection
  if (TRX_Inited && ((USB_LastActiveTime + USB_RESTART_TIMEOUT < HAL_GetTick()))) // || (USB_LastActiveTime == 0)
    USBD_Restart();
}

void EVENTS_do_EVERY_100ms(void) // 10 hz
{
	// every 100ms we receive data from FPGA (amplitude, ADC overload, etc.)
	FPGA_NeedGetParams = true;

	//Detect FPGA stuck error
	static float32_t old_FPGA_Audio_Buffer_RX1_I = 0;
	static float32_t old_FPGA_Audio_Buffer_RX1_Q = 0;
	float32_t *FPGA_Audio_Buffer_RX1_I_current = !FPGA_RX_Buffer_Current ? (float32_t *)&FPGA_Audio_Buffer_RX1_I_A : (float32_t *)&FPGA_Audio_Buffer_RX1_I_B;
  float32_t *FPGA_Audio_Buffer_RX1_Q_current = !FPGA_RX_Buffer_Current ? (float32_t *)&FPGA_Audio_Buffer_RX1_Q_A : (float32_t *)&FPGA_Audio_Buffer_RX1_Q_B;
	static uint16_t fpga_stuck_errors = 0;
	if (FPGA_Audio_Buffer_RX1_I_current[0] == old_FPGA_Audio_Buffer_RX1_I || FPGA_Audio_Buffer_RX1_Q_current[0] == old_FPGA_Audio_Buffer_RX1_Q)
		fpga_stuck_errors++;
	else
		fpga_stuck_errors = 0;
	if (fpga_stuck_errors > 5 && !TRX_on_TX && !TRX.ADC_SHDN && !FPGA_bus_stop && CurrentVFO->Mode != TRX_MODE_WFM) // && !SD_PlayInProcess
	{
		println("[ERR] IQ stuck error, restart disabled");
		fpga_stuck_errors = 0;
		//FPGA_NeedRestart_RX = true;
	}
	old_FPGA_Audio_Buffer_RX1_I = FPGA_Audio_Buffer_RX1_I_current[0];
	old_FPGA_Audio_Buffer_RX1_Q = FPGA_Audio_Buffer_RX1_Q_current[0];

	//Process AutoGain feature
	TRX_DoAutoGain();

	//Process Scaner
	if (TRX_ScanMode)
		TRX_ProcessScanMode();
	
	//Process LCD dimmer
#ifdef HAS_BRIGHTNESS_CONTROL
	static bool LCD_Dimmer_State = false;
	if(!LCD_busy)
	{
		LCD_busy = true;
		if(!LCD_Dimmer_State && TRX.LCD_SleepTimeout > 0 &&TRX_Inactive_Time >= TRX.LCD_SleepTimeout)
		{
			LCDDriver_setBrightness(IDLE_LCD_BRIGHTNESS);
			LCD_Dimmer_State = true;
		}
		if(LCD_Dimmer_State && TRX_Inactive_Time < TRX.LCD_SleepTimeout)
		{
			LCDDriver_setBrightness(TRX.LCD_Brightness);
			LCD_Dimmer_State = false;
		}
		LCD_busy = false;
	}
#endif

	// reset error flags
	CODEC_Buffer_underrun = false;
	FPGA_Buffer_underrun = false;
	RX_USB_AUDIO_underrun = false;
	APROC_IFGain_Overflow = false;
	TRX_MIC_BELOW_NOISEGATE = false;
	#if HRDW_HAS_SD
	SD_underrun = false;
	#endif
}

void EVENTS_do_EVERY_500ms(void) // 2 hz
{
	//Redraw menu infolines if needed
	LCD_UpdateQuery.SystemMenuInfolines = true;
}

void EVENTS_do_EVERY_1000ms(void) // 1 hz
{
	static uint32_t tim6_delay = 0;
	TRX_Inactive_Time++;

	//Detect FPGA IQ phase error
	if (fabsf(TRX_IQ_phase_error) > 0.1f && !TRX_on_TX && !TRX_phase_restarted && !TRX.ADC_SHDN && !FPGA_bus_stop && CurrentVFO->Mode != TRX_MODE_WFM)
	{
		println("[ERR] IQ phase error, restart disabled | ", TRX_IQ_phase_error);
		//FPGA_NeedRestart_RX = true;
		TRX_phase_restarted = true;
	}

	#if HRDW_HAS_WIFI
	bool maySendIQ = true;
	if (!WIFI_IP_Gotted) { //Get resolved IP
		WIFI_GetIP(NULL);
		maySendIQ = false;
	}
	uint32_t mstime = HAL_GetTick();
	if (TRX_SNTP_Synced == 0 || (mstime > (SNTP_SYNC_INTERVAL * 1000) && TRX_SNTP_Synced < (mstime - SNTP_SYNC_INTERVAL * 1000))) {//Sync time from internet
		WIFI_GetSNTPTime(NULL);
		maySendIQ = false;
	}
	if (TRX.WIFI_CAT_SERVER && !WIFI_CAT_server_started) { //start WiFi CAT Server
		WIFI_StartCATServer(NULL);
		maySendIQ = false;
	}
	if(CALIBRATE.OTA_update && !WIFI_NewFW_checked) {	//check OTA FW updates
		WIFI_checkFWUpdates();
		maySendIQ = false;
	}
	if(TRX.FFT_DXCluster && ((HAL_GetTick() - TRX_DXCluster_UpdateTime) > DXCLUSTER_UPDATE_TIME || TRX_DXCluster_UpdateTime == 0)) //get and show dx cluster
	{
		if(WIFI_getDXCluster_background())
			TRX_DXCluster_UpdateTime = HAL_GetTick();
		maySendIQ = false;
	}
	WIFI_maySendIQ = maySendIQ;
	#endif
	
	//Check vBAT
	static bool vbat_checked = false;
	if(!vbat_checked && TRX_Inited && !LCD_busy)
	{
		vbat_checked = true;
		if(TRX_VBAT_Voltage <= 2.5f)
		{
			LCD_showError("Replace BAT", true);
		}
	}
	
	//Auto Snap
	if (TRX.Auto_Snap) {
		SNAP_DoSnap(true);
	}
	
	CPULOAD_Calc(); // Calculate CPU load
	TRX_STM32_TEMPERATURE = HRDW_getCPUTemperature();
	TRX_STM32_VREF = HRDW_getCPUVref();

	//Save Debug variables
	uint32_t dbg_tim6_delay = HAL_GetTick() - tim6_delay;
	float32_t dbg_coeff = 1000.0f / (float32_t)dbg_tim6_delay;
	dbg_FPGA_samples = (uint32_t)((float32_t)FPGA_samples * dbg_coeff);

	if (TRX.Debug_Type == TRX_DEBUG_SYSTEM)
	{
		//Print debug
		uint32_t dbg_WM8731_DMA_samples = (uint32_t)((float32_t)CODEC_DMA_samples / 2.0f * dbg_coeff);
		uint32_t dbg_AUDIOPROC_samples = (uint32_t)((float32_t)AUDIOPROC_samples * dbg_coeff);
		float32_t *FPGA_Audio_Buffer_RX1_I_current = !FPGA_RX_Buffer_Current ? (float32_t *)&FPGA_Audio_Buffer_RX1_I_A : (float32_t *)&FPGA_Audio_Buffer_RX1_I_B;
		float32_t *FPGA_Audio_Buffer_RX1_Q_current = !FPGA_RX_Buffer_Current ? (float32_t *)&FPGA_Audio_Buffer_RX1_Q_A : (float32_t *)&FPGA_Audio_Buffer_RX1_Q_B;
		float32_t dbg_FPGA_Audio_Buffer_I_tmp = FPGA_Audio_Buffer_RX1_I_current[0];
		float32_t dbg_FPGA_Audio_Buffer_Q_tmp = FPGA_Audio_Buffer_RX1_Q_current[0];
		if (TRX_on_TX)
		{
			dbg_FPGA_Audio_Buffer_I_tmp = FPGA_Audio_SendBuffer_I[0];
			dbg_FPGA_Audio_Buffer_Q_tmp = FPGA_Audio_SendBuffer_Q[0];
		}
		uint32_t dbg_RX_USB_AUDIO_SAMPLES = (uint32_t)((float32_t)RX_USB_AUDIO_SAMPLES * dbg_coeff);
		uint32_t dbg_TX_USB_AUDIO_SAMPLES = (uint32_t)((float32_t)TX_USB_AUDIO_SAMPLES * dbg_coeff);
		uint32_t cpu_load = (uint32_t)CPU_LOAD.Load;
		//Print Debug info
		println("FPGA Samples: ", dbg_FPGA_samples);            //~96000
		println("Audio DMA samples: ", dbg_WM8731_DMA_samples); //~48000
		print_flush();
		println("Audioproc blocks: ", dbg_AUDIOPROC_samples);
		println("CPU Load: ", cpu_load);
		print_flush();
		println("RF/STM32 Temperature: ", (int16_t)TRX_RF_Temperature, " / ", (int16_t)TRX_STM32_TEMPERATURE);
		println("STM32 Voltage: ", TRX_STM32_VREF);
		print_flush();
		println("TIM6 delay: ", dbg_tim6_delay);
		println("FFT FPS: ", FFT_FPS);
		print_flush();
		println("First byte of RX-FPGA I/Q: ", dbg_FPGA_Audio_Buffer_I_tmp, " / ", dbg_FPGA_Audio_Buffer_Q_tmp); //first byte of IQ
		println("IQ Phase error: ", TRX_IQ_phase_error);                                                         //first byte of Q
		print_flush();
		println("USB Audio RX/TX samples: ", dbg_RX_USB_AUDIO_SAMPLES, " / ", dbg_TX_USB_AUDIO_SAMPLES);         //~48000
		println("ADC MIN/MAX Amplitude: ", TRX_ADC_MINAMPLITUDE, " / ", TRX_ADC_MAXAMPLITUDE);
		print_flush();
		println("VCXO Error: ", TRX_VCXO_ERROR);
		#if HRDW_HAS_WIFI
		println("WIFI State: ", WIFI_State);
		#endif
		println("");
		print_flush();
		PrintProfilerResult();
	}
	
	//Save Settings to Backup Memory
	if (NeedSaveSettings && (HAL_GPIO_ReadPin(PWR_ON_GPIO_Port, PWR_ON_Pin) == GPIO_PIN_SET))
		SaveSettings();

	//Reset counters
	tim6_delay = HAL_GetTick();
	FPGA_samples = 0;
	AUDIOPROC_samples = 0;
	CODEC_DMA_samples = 0;
	FFT_FPS_Last = FFT_FPS;
	FFT_FPS = 0;
	RX_USB_AUDIO_SAMPLES = 0;
	TX_USB_AUDIO_SAMPLES = 0;
	FPGA_NeedSendParams = true;
	CODEC_Beeping = false;

//redraw lcd to fix problem
#ifdef LCD_HX8357B
	static uint8_t HX8357B_BUG_redraw_counter = 0;
	HX8357B_BUG_redraw_counter++;
	if (HX8357B_BUG_redraw_counter == 60)
	{
		LCD_UpdateQuery.TopButtonsRedraw = true;
		LCD_UpdateQuery.StatusInfoBarRedraw = true;
		LCD_UpdateQuery.StatusInfoGUI = true;
	}
	else if (HX8357B_BUG_redraw_counter == 120)
	{
		LCD_UpdateQuery.FreqInfoRedraw = true;
		LCD_UpdateQuery.StatusInfoGUI = true;
	}
	else if (HX8357B_BUG_redraw_counter >= 180)
	{
		LCD_UpdateQuery.StatusInfoGUI = true;
		LCD_UpdateQuery.StatusInfoBarRedraw = true;
		HX8357B_BUG_redraw_counter = 0;
	}
#endif
}
