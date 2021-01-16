/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32h7xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

// EXTI0 - KEY DASH
// EXTI1 - KEY DOT
// EXTI2 - ENC_CLK
// EXTI4 - PTT_IN
// EXTI10 - 48K_Clock
// EXTI11 - PWR_button
// EXTI13 - ENC2_CLK

// TIM3 - WIFI
// TIM4 - FFT calculation
// TIM5 - audio processor
// TIM6 - every 10ms, different actions
// TIM7 - USB FIFO
// TIM15 - EEPROM / front panel
// TIM16 - Interrogation of the auxiliary encoder, because it hangs on the same interrupt as the FPGA
// TIM17 - Digital CW decoding, ...

// DMA1-0 - receiving data from the audio codec
// DMA1-1 - receiving data from WiFi via UART
// DMA1-5 - sending data to audio codec
// DMA2-4 - DMA for copying 16 bit arrays
// DMA2-5 - draw the fft at 16 bits, increment
// DMA2-6 - draw the waterfall at 16 bits, increment
// DMA2-7 - move the waterfall down

// MDMA-0 - copy audio buffers at 32bit
// MDMA-1 - send audio processor buffer to codec buffer - A
// MDMA-2 - send audio processor buffer to codec buffer - B

/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32h7xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#include "functions.h"
#include "front_unit.h"
#include "rf_unit.h"
#include "fpga.h"
#include "lcd.h"
#include "wm8731.h"
#include "audio_processor.h"
#include "agc.h"
#include "fft.h"
#include "settings.h"
#include "fpga.h"
#include "profiler.h"
#include "usbd_debug_if.h"
#include "usbd_cat_if.h"
#include "usbd_audio_if.h"
#include "usbd_ua3reo.h"
#include "trx_manager.h"
#include "audio_filters.h"
#include "wifi.h"
#include "system_menu.h"
#include "bootloader.h"
#include "decoder.h"
#include "swr_analyzer.h"
#include "sd.h"
#include "wspr.h"

static uint32_t ms10_counter = 0;
static uint32_t tim6_delay = 0;
static uint32_t powerdown_start_delay = 0;
uint32_t dbg_FPGA_samples = 0;
/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream7;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream6;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream4;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream5;
extern DMA2D_HandleTypeDef hdma2d;
extern DMA_HandleTypeDef hdma_spi3_tx;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim15;
extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim17;
extern DMA_HandleTypeDef hdma_usart6_rx;
extern UART_HandleTypeDef huart6;
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
		HAL_MPU_Disable();
    LCD_showError("Hard Fault", false);
    static uint32_t i = 0;
    while (i < 99999999)
    {
      i++;
      __asm("nop");
    }
    HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_RESET);
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
		HAL_MPU_Disable();
    LCD_showError("Memory Fault", false);
    static uint32_t i = 0;
    while (i < 99999999)
    {
      i++;
      __asm("nop");
    }
    HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_RESET);
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    LCD_showError("Bus Fault", false);
    static uint32_t i = 0;
    while (i < 99999999)
    {
      i++;
      __asm("nop");
    }
    HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_RESET);
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    LCD_showError("Usage Fault", false);
    static uint32_t i = 0;
    while (i < 99999999)
    {
      i++;
      __asm("nop");
    }
    HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_RESET);
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32H7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32h7xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles EXTI line0 interrupt.
  */
void EXTI0_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI0_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END EXTI0_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
  /* USER CODE BEGIN EXTI0_IRQn 1 */

  /* USER CODE END EXTI0_IRQn 1 */
}

/**
  * @brief This function handles EXTI line1 interrupt.
  */
void EXTI1_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI1_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END EXTI1_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
  /* USER CODE BEGIN EXTI1_IRQn 1 */

  /* USER CODE END EXTI1_IRQn 1 */
}

/**
  * @brief This function handles EXTI line2 interrupt.
  */
void EXTI2_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI2_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END EXTI2_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
  /* USER CODE BEGIN EXTI2_IRQn 1 */

  /* USER CODE END EXTI2_IRQn 1 */
}

/**
  * @brief This function handles EXTI line4 interrupt.
  */
void EXTI4_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI4_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END EXTI4_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
  /* USER CODE BEGIN EXTI4_IRQn 1 */

  /* USER CODE END EXTI4_IRQn 1 */
}

/**
  * @brief This function handles DMA1 stream1 global interrupt.
  */
void DMA1_Stream1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream1_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END DMA1_Stream1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart6_rx);
  /* USER CODE BEGIN DMA1_Stream1_IRQn 1 */

  /* USER CODE END DMA1_Stream1_IRQn 1 */
}

/**
  * @brief This function handles DMA1 stream5 global interrupt.
  */
void DMA1_Stream5_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream5_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END DMA1_Stream5_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_spi3_tx);
  /* USER CODE BEGIN DMA1_Stream5_IRQn 1 */

  /* USER CODE END DMA1_Stream5_IRQn 1 */
}

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */
  if (SYSMENU_wspr_opened)
    WSPR_DoFastEvents();
  /* USER CODE END TIM2_IRQn 1 */
}

/**
  * @brief This function handles TIM3 global interrupt.
  */
void TIM3_IRQHandler(void)
{
  /* USER CODE BEGIN TIM3_IRQn 0 */
  static uint16_t wifi_start_timeout = 0;
  CPULOAD_WakeUp();
  /* USER CODE END TIM3_IRQn 0 */
  HAL_TIM_IRQHandler(&htim3);
  /* USER CODE BEGIN TIM3_IRQn 1 */

  //process wifi
  if (wifi_start_timeout < 10)
  {
    wifi_start_timeout++;
  }
  else
  {
    // we work with WiFi by timer, or send it if it is turned off (to turn it on, we need a restart)
    if (TRX.WIFI_Enabled)
      WIFI_Process();
    else
      WIFI_GoSleep();
  }
  /* USER CODE END TIM3_IRQn 1 */
}

/**
  * @brief This function handles TIM4 global interrupt.
  */
void TIM4_IRQHandler(void)
{
  /* USER CODE BEGIN TIM4_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END TIM4_IRQn 0 */
  HAL_TIM_IRQHandler(&htim4);
  /* USER CODE BEGIN TIM4_IRQn 1 */
  if (SYSMENU_spectrum_opened || SYSMENU_swr_opened)
  {
    SYSMENU_drawSystemMenu(false);
    return;
  }
  
  if (FFT_need_fft)
    FFT_doFFT();
	
	ua3reo_dev_cat_parseCommand();
  /* USER CODE END TIM4_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[15:10] interrupts.
  */
void EXTI15_10_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI15_10_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END EXTI15_10_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
  /* USER CODE BEGIN EXTI15_10_IRQn 1 */

  /* USER CODE END EXTI15_10_IRQn 1 */
}

/**
  * @brief This function handles TIM5 global interrupt.
  */
void TIM5_IRQHandler(void)
{
  /* USER CODE BEGIN TIM5_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END TIM5_IRQn 0 */
  HAL_TIM_IRQHandler(&htim5);
  /* USER CODE BEGIN TIM5_IRQn 1 */
  //StartProfilerUs();
  if (!Processor_NeedTXBuffer && !Processor_NeedRXBuffer)
    return;

  if (TRX_on_TX())
  {
    if (CurrentVFO()->Mode != TRX_MODE_NO_TX)
      processTxAudio();
  }
  else
  {
    processRxAudio();
  }
  //in the spectrum analyzer mode, we raise its processing to priority, performing together with the audio processor
  if (SYSMENU_spectrum_opened)
    LCD_doEvents();
  //EndProfilerUs(true);
  /* USER CODE END TIM5_IRQn 1 */
}

/**
  * @brief This function handles TIM6 global interrupt, DAC1_CH1 and DAC1_CH2 underrun error interrupts.
  */
void TIM6_DAC_IRQHandler(void)
{
  /* USER CODE BEGIN TIM6_DAC_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END TIM6_DAC_IRQn 0 */
  HAL_TIM_IRQHandler(&htim6);
  /* USER CODE BEGIN TIM6_DAC_IRQn 1 */
  ms10_counter++;
  // transmission release time after key signal
  if (TRX_Key_Timeout_est > 0 && !TRX_key_serial && !TRX_key_dot_hard && !TRX_key_dash_hard)
  {
    TRX_Key_Timeout_est -= 10;
    if (TRX_Key_Timeout_est == 0)
    {
      LCD_UpdateQuery.StatusInfoGUIRedraw = true;
      FPGA_NeedSendParams = true;
      TRX_Restart_Mode();
    }
  }

  //every 10ms

  // if the settings have changed, update the parameters in the FPGA
  if (NeedSaveSettings)
    FPGA_NeedSendParams = true;

  // there was a request to reinitialize audio and notch filters
  if (NeedReinitNotch)
    InitNotchFilter();
  if (NeedReinitAudioFilters)
    ReinitAudioFilters();

  //Process SWR, Power meter, ALC, Thermal sensors, Fan, ...
  RF_UNIT_ProcessSensors();

  // emulate PTT over CAT/Software
  if (TRX_ptt_soft != TRX_old_ptt_soft)
    TRX_ptt_change();

  // emulate the key via the COM port
  if (TRX_key_serial != TRX_old_key_serial)
    TRX_key_change();

  // update the state of the RF-Unit board
  RF_UNIT_UpdateState(false);

// check touchpad events
#ifdef HAS_TOUCHPAD
  TOUCHPAD_ProcessInterrupt();
#endif

  if ((ms10_counter % 10) == 0) // every 100ms
  {
    // every 100ms we receive data from FPGA (amplitude, ADC overload, etc.)
    FPGA_NeedGetParams = true;

    //S-Meter Calculate
    TRX_DBMCalculate();

    //Detect FPGA stuck error
    static float32_t old_FPGA_Audio_Buffer_RX1_I = 0;
    static float32_t old_FPGA_Audio_Buffer_RX1_Q = 0;
    static uint16_t fpga_stuck_errors = 0;
    if (FPGA_Audio_Buffer_RX1_I[0] == old_FPGA_Audio_Buffer_RX1_I || FPGA_Audio_Buffer_RX1_Q[0] == old_FPGA_Audio_Buffer_RX1_Q)
      fpga_stuck_errors++;
    else
      fpga_stuck_errors = 0;
    if (fpga_stuck_errors > 3 && !TRX_on_TX() && !TRX.ADC_SHDN)
    {
      sendToDebug_strln("[ERR] IQ stuck error, restart");
      fpga_stuck_errors = 0;
      FPGA_NeedRestart = true;
    }
    old_FPGA_Audio_Buffer_RX1_I = FPGA_Audio_Buffer_RX1_I[0];
    old_FPGA_Audio_Buffer_RX1_Q = FPGA_Audio_Buffer_RX1_Q[0];

    //Process AutoGain feature
    TRX_DoAutoGain();

    //Process Scaner
    if (TRX_ScanMode)
      TRX_ProcessScanMode();

    // reset error flags
    WM8731_Buffer_underrun = false;
    FPGA_Buffer_underrun = false;
    RX_USB_AUDIO_underrun = false;
  }

  if ((ms10_counter % 3) == 0) // every 30ms
  {
    LCD_UpdateQuery.StatusInfoBar = true;
    // update information on LCD
    LCD_doEvents();
  }
  else if (LCD_UpdateQuery.FreqInfo) //Redraw freq fast
    LCD_doEvents();

	static bool needPrintFFT = false;
  if ((ms10_counter % (6 - TRX.FFT_Speed)) == 0) // every x msec
		needPrintFFT = true;
  
	if(needPrintFFT && !LCD_UpdateQuery.Background && FFT_printFFT()) // draw FFT
		needPrintFFT = false;

  if (ms10_counter == 101) // every 1 sec
  {
    ms10_counter = 0;

    //Detect FPGA IQ phase error
    static bool phase_restarted = false;
    if (fabsf(TRX_IQ_phase_error) > 0.1f && !TRX_on_TX() && !phase_restarted && !TRX.ADC_SHDN)
    {
      sendToDebug_str("[ERR] IQ phase error, restart | ");
      sendToDebug_float32(TRX_IQ_phase_error, false);
      FPGA_NeedRestart = true;
      phase_restarted = true;
    }

    if (!WIFI_IP_Gotted) //Get resolved IP
      WIFI_GetIP(NULL);
    uint32_t mstime = HAL_GetTick();
    if (TRX_SNTP_Synced == 0 || (mstime > (SNTP_SYNC_INTERVAL * 1000) && TRX_SNTP_Synced < (mstime - SNTP_SYNC_INTERVAL * 1000))) //Sync time from internet
      WIFI_GetSNTPTime(NULL);
    if (TRX.WIFI_CAT_SERVER && !WIFI_CAT_server_started)
      WIFI_StartCATServer(NULL);

    CPULOAD_Calc(); // Calculate CPU load
    TRX_STM32_TEMPERATURE = TRX_getSTM32H743Temperature();
		TRX_STM32_VREF = TRX_getSTM32H743vref();
		
    if (TRX.Debug_Console)
    {
      //Save Debug variables
      uint32_t dbg_tim6_delay = HAL_GetTick() - tim6_delay;
      float32_t dbg_coeff = 1000.0f / (float32_t)dbg_tim6_delay;
      dbg_FPGA_samples = (uint32_t)((float32_t)FPGA_samples * dbg_coeff);
      uint32_t dbg_WM8731_DMA_samples = (uint32_t)((float32_t)WM8731_DMA_samples / 2.0f * dbg_coeff);
      uint32_t dbg_AUDIOPROC_samples = (uint32_t)((float32_t)AUDIOPROC_samples * dbg_coeff);
      float32_t dbg_FPGA_Audio_Buffer_I_tmp = FPGA_Audio_Buffer_RX1_I[0];
      float32_t dbg_FPGA_Audio_Buffer_Q_tmp = FPGA_Audio_Buffer_RX1_Q[0];
      if (TRX_on_TX())
      {
        dbg_FPGA_Audio_Buffer_I_tmp = FPGA_Audio_SendBuffer_I[0];
        dbg_FPGA_Audio_Buffer_Q_tmp = FPGA_Audio_SendBuffer_Q[0];
      }
      uint32_t dbg_RX_USB_AUDIO_SAMPLES = (uint32_t)((float32_t)RX_USB_AUDIO_SAMPLES * dbg_coeff);
      uint32_t dbg_TX_USB_AUDIO_SAMPLES = (uint32_t)((float32_t)TX_USB_AUDIO_SAMPLES * dbg_coeff);
      uint32_t cpu_load = (uint32_t)CPU_LOAD.Load;
      //Print Debug info
      sendToDebug_str("FPGA Samples: ");
      sendToDebug_uint32(dbg_FPGA_samples, false); //~96000
      sendToDebug_str("Audio DMA samples: ");
      sendToDebug_uint32(dbg_WM8731_DMA_samples, false); //~48000
      sendToDebug_str("Audioproc blocks: ");
      sendToDebug_uint32(dbg_AUDIOPROC_samples, false);
      sendToDebug_str("CPU Load: ");
      sendToDebug_uint32(cpu_load, false);
      sendToDebug_str("RF/STM32 Temperature: ");
      sendToDebug_int16((int16_t)TRX_RF_Temperature, true);
      sendToDebug_str(" / ");
      sendToDebug_int16((int16_t)TRX_STM32_TEMPERATURE, false);
      sendToDebug_str("STM32 Voltage: ");
      sendToDebug_float32(TRX_STM32_VREF, false);
      sendToDebug_str("TIM6 delay: ");
      sendToDebug_uint32(dbg_tim6_delay, false);
      sendToDebug_str("FFT FPS: ");
      sendToDebug_uint32(FFT_FPS, false);
      sendToDebug_str("First byte of RX-FPGA I/Q: ");
      sendToDebug_float32(dbg_FPGA_Audio_Buffer_I_tmp, true); //first byte of I
      sendToDebug_str(" / ");
      sendToDebug_float32(dbg_FPGA_Audio_Buffer_Q_tmp, false); //first byte of Q
      sendToDebug_str("IQ Phase error: ");
      sendToDebug_float32(TRX_IQ_phase_error, false); //first byte of Q
      sendToDebug_str("USB Audio RX/TX samples: ");
      sendToDebug_uint32(dbg_RX_USB_AUDIO_SAMPLES, true); //~48000
      sendToDebug_str(" / ");
      sendToDebug_uint32(dbg_TX_USB_AUDIO_SAMPLES, false); //~48000
      sendToDebug_str("ADC MIN/MAX Amplitude: ");
      sendToDebug_int16(TRX_ADC_MINAMPLITUDE, true);
      sendToDebug_str(" / ");
      sendToDebug_int16(TRX_ADC_MAXAMPLITUDE, false);
      sendToDebug_str("VCXO Error: ");
      sendToDebug_int32(TRX_VCXO_ERROR, false);
      sendToDebug_str("WIFI State: ");
      sendToDebug_int16(WIFI_State, false);
      sendToDebug_newline();
      PrintProfilerResult();
    }

    //Save Settings to Backup Memory
    if (NeedSaveSettings && (HAL_GPIO_ReadPin(PWR_ON_GPIO_Port, PWR_ON_Pin) == GPIO_PIN_SET))
      SaveSettings();

    //Reset counters
    tim6_delay = HAL_GetTick();
    FPGA_samples = 0;
    AUDIOPROC_samples = 0;
    WM8731_DMA_samples = 0;
		FFT_FPS_Last = FFT_FPS;
    FFT_FPS = 0;
    RX_USB_AUDIO_SAMPLES = 0;
    TX_USB_AUDIO_SAMPLES = 0;
    FPGA_NeedSendParams = true;

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

  //power off sequence
  if ((HAL_GPIO_ReadPin(PWR_ON_GPIO_Port, PWR_ON_Pin) == GPIO_PIN_RESET) && ((HAL_GetTick() - powerdown_start_delay) > POWERDOWN_TIMEOUT) && !NeedSaveCalibration && !SPI_process && !EEPROM_Busy)
  {
    TRX_Inited = false;
    LCD_busy = true;
    HAL_Delay(10);
    WM8731_Mute();
    WM8731_CleanBuffer();
    LCDDriver_Fill(COLOR_BLACK);
    LCD_showInfo("GOOD BYE!", false);
    SaveSettings();
    SaveSettingsToEEPROM();
    sendToDebug_flush();
    while (HAL_GPIO_ReadPin(PWR_ON_GPIO_Port, PWR_ON_Pin) == GPIO_PIN_RESET) { }
    HAL_Delay(500);
		HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_RESET);
    //SCB->AIRCR = 0x05FA0004; // software resetw
    while (true) { }
  }

  //TRX protector
  if (TRX_on_TX())
  {
    if (TRX_RF_Temperature > TRX_MAX_RF_TEMP)
    {
      TRX_Tune = false;
      TRX_ptt_hard = false;
      TRX_ptt_soft = false;
      LCD_UpdateQuery.StatusInfoGUI = true;
      LCD_UpdateQuery.TopButtons = true;
      NeedSaveSettings = true;
      TRX_Restart_Mode();
      sendToDebug_strln("RF temperature too HIGH!");
      LCD_showTooltip("RF temperature too HIGH!");
    }
    if (TRX_SWR > TRX_MAX_SWR && !TRX_Tune)
    {
      TRX_Tune = false;
      TRX_ptt_hard = false;
      TRX_ptt_soft = false;
      LCD_UpdateQuery.StatusInfoGUI = true;
      LCD_UpdateQuery.TopButtons = true;
      NeedSaveSettings = true;
      TRX_Restart_Mode();
      sendToDebug_strln("SWR too HIGH!");
      LCD_showTooltip("SWR too HIGH!");
    }
  }

  // restart USB if there is no activity (off) to find a new connection
  if (TRX_Inited && ((USB_LastActiveTime + USB_RESTART_TIMEOUT < HAL_GetTick()))) // || (USB_LastActiveTime == 0)
    USBD_Restart();

  /* USER CODE END TIM6_DAC_IRQn 1 */
}

/**
  * @brief This function handles TIM7 global interrupt.
  */
void TIM7_IRQHandler(void)
{
  /* USER CODE BEGIN TIM7_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END TIM7_IRQn 0 */
  HAL_TIM_IRQHandler(&htim7);
  /* USER CODE BEGIN TIM7_IRQn 1 */

  sendToDebug_flush(); // send data to debug from the buffer

  // unmute after transition process end
  if (TRX_Temporary_Mute_StartTime > 0 && (HAL_GetTick() - TRX_Temporary_Mute_StartTime) > 10)
  {
    WM8731_UnMute();
    TRX_Temporary_Mute_StartTime = 0;
  }

  /* USER CODE END TIM7_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream5 global interrupt.
  */
void DMA2_Stream5_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream5_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END DMA2_Stream5_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_memtomem_dma2_stream5);
  /* USER CODE BEGIN DMA2_Stream5_IRQn 1 */
  FFT_afterPrintFFT();
  /* USER CODE END DMA2_Stream5_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream6 global interrupt.
  */
void DMA2_Stream6_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream6_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END DMA2_Stream6_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_memtomem_dma2_stream6);
  /* USER CODE BEGIN DMA2_Stream6_IRQn 1 */
  FFT_printWaterfallDMA(); // display the waterfall
  /* USER CODE END DMA2_Stream6_IRQn 1 */
}

/**
  * @brief This function handles USART6 global interrupt.
  */
void USART6_IRQHandler(void)
{
  /* USER CODE BEGIN USART6_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END USART6_IRQn 0 */
  HAL_UART_IRQHandler(&huart6);
  /* USER CODE BEGIN USART6_IRQn 1 */

  /* USER CODE END USART6_IRQn 1 */
}

/**
  * @brief This function handles DMA2D global interrupt.
  */
void DMA2D_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2D_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END DMA2D_IRQn 0 */
  HAL_DMA2D_IRQHandler(&hdma2d);
  /* USER CODE BEGIN DMA2D_IRQn 1 */

  /* USER CODE END DMA2D_IRQn 1 */
}

/**
  * @brief This function handles USB On The Go FS global interrupt.
  */
void OTG_FS_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_FS_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END OTG_FS_IRQn 0 */
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
  /* USER CODE BEGIN OTG_FS_IRQn 1 */

  /* USER CODE END OTG_FS_IRQn 1 */
}

/**
  * @brief This function handles TIM15 global interrupt.
  */
void TIM15_IRQHandler(void)
{
  /* USER CODE BEGIN TIM15_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END TIM15_IRQn 0 */
  HAL_TIM_IRQHandler(&htim15);
  /* USER CODE BEGIN TIM15_IRQn 1 */

  //FRONT PANEL SPI
  FRONTPANEL_Process();

  //EEPROM SPI
  if (NeedSaveCalibration) // save calibration data to EEPROM
    SaveCalibration();

  //SD-Card SPI
  SD_Process();

  /* USER CODE END TIM15_IRQn 1 */
}

/**
  * @brief This function handles TIM16 global interrupt.
  */
void TIM16_IRQHandler(void)
{
  /* USER CODE BEGIN TIM16_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END TIM16_IRQn 0 */
  HAL_TIM_IRQHandler(&htim16);
  /* USER CODE BEGIN TIM16_IRQn 1 */
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
  /* USER CODE END TIM16_IRQn 1 */
}

/**
  * @brief This function handles TIM17 global interrupt.
  */
void TIM17_IRQHandler(void)
{
  /* USER CODE BEGIN TIM17_IRQn 0 */
  CPULOAD_WakeUp();
  /* USER CODE END TIM17_IRQn 0 */
  HAL_TIM_IRQHandler(&htim17);
  /* USER CODE BEGIN TIM17_IRQn 1 */
	if (FFT_new_buffer_ready)
    FFT_bufferPrepare();
	
  if (TRX.CWDecoder)
    DECODER_Process();
  /* USER CODE END TIM17_IRQn 1 */
}

/* USER CODE BEGIN 1 */

void DMA1_Stream0_IRQHandler(void)
{
  CPULOAD_WakeUp();
  HAL_DMA_IRQHandler(&hdma_spi3_rx);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  CPULOAD_WakeUp();
  if (GPIO_Pin == GPIO_PIN_10) //FPGA BUS
  {
    if (!WM8731_Buffer_underrun)
      FPGA_fpgadata_iqclock();  // IQ data
    FPGA_fpgadata_stuffclock(); // parameters and other services
  }
  else if (GPIO_Pin == GPIO_PIN_2) //Main encoder
  {
    if (TRX_Inited)
      FRONTPANEL_ENCODER_checkRotate();
  }
  else if (GPIO_Pin == GPIO_PIN_4) //PTT
  {
    if (TRX_Inited && CurrentVFO()->Mode != TRX_MODE_NO_TX)
      TRX_ptt_change();
  }
  else if (GPIO_Pin == GPIO_PIN_1) //KEY DOT
  {
    TRX_key_change();
  }
  else if (GPIO_Pin == GPIO_PIN_0) //KEY DASH
  {
    TRX_key_change();
  }
  else if (GPIO_Pin == GPIO_PIN_11) //POWER OFF
  {
    powerdown_start_delay = HAL_GetTick();
  }
}
/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
