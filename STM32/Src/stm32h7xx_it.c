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

//EXTI0 - KEY DASH
//EXTI1 - KEY DOT
//EXTI2 - ENC_CLK
//EXTI4 - PTT_IN
//EXTI10 - 48K_Clock
//EXTI11 - PWR_button
//EXTI13 - ENC2_CLK
	
//TIM3 - WIFI
//TIM4 - расчёт FFT
//TIM5 - аудио-процессор
//TIM6 - каждые 50мс, различные действия
//TIM7 - USB FIFO
//TIM15 - EEPROM / передняя панель

//DMA1-0 - получение данных с аудио-кодека
//DMA1-1 - получение данных из WiFi по UART
//DMA1-5 - отсылка данных в аудио-кодек
//DMA2-0 - отправка буфера аудио-процессора в буффер кодека - A
//DMA2-1 - отправка буфера аудио-процессора в буффер кодека - B
//DMA2-3 - копирование аудио-буфферов по 32бит
//DMA2-4 - DMA для копирования 16 битных массивов
//DMA2-5 - DMA видео-драйвера, для заливки, 16 бит без инкремента
//DMA2-6 - отрисовка водопада по 16бит, инкремент
//DMA2-7 - смещение водопада вниз

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
#include "peripheral.h"
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

static uint32_t ms50_counter = 0;
static uint32_t tim6_delay = 0;
static uint32_t eeprom_save_timestamp = 0;
static uint32_t powerdown_start_delay = 0;
/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream0;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream1;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream7;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream6;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream5;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream3;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream4;
extern DMA_HandleTypeDef hdma_spi3_tx;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim15;
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

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    LCD_showError("Hard Fault", true);
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    LCD_showError("Memory Fault", true);
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    LCD_showError("Bus Fault", true);
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    LCD_showError("Usage Fault", true);
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
	if(wifi_start_timeout<10)
	{
		wifi_start_timeout++;
	}
	else
	{
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
  if (sysmenu_spectrum_opened)
  {
    drawSystemMenu(false);
    return;
  }
  ua3reo_dev_cat_parseCommand();
  if (FFT_need_fft)
    FFT_doFFT();
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
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
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
	if(!Processor_NeedTXBuffer && !Processor_NeedRXBuffer) return;
  if (TRX_on_TX())
  {
    if (CurrentVFO()->Mode != TRX_MODE_NO_TX)
      processTxAudio();
  }
  else
  {
    processRxAudio();
  }
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
	HAL_IWDG_Refresh(&hiwdg1);
  /* USER CODE END TIM6_DAC_IRQn 0 */
  HAL_TIM_IRQHandler(&htim6);
  /* USER CODE BEGIN TIM6_DAC_IRQn 1 */
  ms50_counter++;

  if (TRX_Key_Timeout_est > 0 && !TRX_key_serial && !TRX_key_dot_hard && !TRX_key_dash_hard)
  {
    TRX_Key_Timeout_est -= 50;
    if (TRX_Key_Timeout_est == 0)
    {
      LCD_UpdateQuery.StatusInfoGUI = true;
      FPGA_NeedSendParams = true;
      TRX_Restart_Mode();
    }
  }

  if (NeedSaveSettings)
    FPGA_NeedSendParams = true;

  if (NeedReinitNotch)
    InitNotchFilter();

	//Process SWR / Power meter
  if (TRX_on_TX() && CurrentVFO()->Mode != TRX_MODE_NO_TX)
    PERIPH_ProcessSWRMeter();
	
  if ((ms50_counter % 2) == 0) // every 100ms
  {
    FPGA_NeedGetParams = true;

    //S-Meter Calculate
    TRX_DBMCalculate();
  }
		
  if (ms50_counter == 21) // every 1 sec
  {
    ms50_counter = 0;
    TRX_DoAutoGain(); //Process AutoGain feature

    if (!TRX_SNMP_Synced) //Sync time from internet
      WIFI_GetSNMPTime(NULL);

		CPULOAD_Calc(); // Calculate CPU load
		TRX_STM32_TEMPERATURE = TRX_getSTM32H743Temperature();
		TRX_STM32_VREF = TRX_getSTM32H743vref();
		
    if (TRX.Debug_Console)
    {
      //Save Debug variables
      uint32_t dbg_FPGA_samples = FPGA_samples;
      uint32_t dbg_WM8731_DMA_samples = WM8731_DMA_samples / 2;
      uint32_t dbg_AUDIOPROC_TXA_samples = AUDIOPROC_TXA_samples;
      uint32_t dbg_AUDIOPROC_TXB_samples = AUDIOPROC_TXB_samples;
      uint32_t dbg_tim6_delay = HAL_GetTick() - tim6_delay;
      float32_t dbg_ALC_need_gain = ALC_need_gain;
      float32_t dbg_FPGA_Audio_Buffer_I_tmp = FPGA_Audio_Buffer_RX1_I[0];
      float32_t dbg_FPGA_Audio_Buffer_Q_tmp = FPGA_Audio_Buffer_RX1_Q[0];
      if (TRX_on_TX())
      {
        dbg_FPGA_Audio_Buffer_I_tmp = FPGA_Audio_SendBuffer_I[0];
        dbg_FPGA_Audio_Buffer_Q_tmp = FPGA_Audio_SendBuffer_Q[0];
      }
      uint32_t dbg_RX_USB_AUDIO_SAMPLES = RX_USB_AUDIO_SAMPLES;
      uint32_t dbg_TX_USB_AUDIO_SAMPLES = TX_USB_AUDIO_SAMPLES;
			uint32_t cpu_load = (uint32_t)CPU_LOAD.Load;
      //Print Debug info
      sendToDebug_str("FPGA Samples: ");
      sendToDebug_uint32(dbg_FPGA_samples, false); //~48000
      sendToDebug_str("Audio DMA samples: ");
      sendToDebug_uint32(dbg_WM8731_DMA_samples, false); //~48000
      sendToDebug_str("Audioproc cycles A: ");
      sendToDebug_uint32(dbg_AUDIOPROC_TXA_samples, false); //~120
      sendToDebug_str("Audioproc cycles B: ");
      sendToDebug_uint32(dbg_AUDIOPROC_TXB_samples, false); //~120
			sendToDebug_str("CPU Load: ");
      sendToDebug_uint32(cpu_load, false);
			sendToDebug_str("STM32 Temperature: ");
      sendToDebug_float32(TRX_STM32_TEMPERATURE, false);
			sendToDebug_str("STM32 Voltage: ");
      sendToDebug_float32(TRX_STM32_VREF, false);
			sendToDebug_str("TIM6 delay: ");
      sendToDebug_uint32(dbg_tim6_delay, false);
      sendToDebug_str("TX Autogain: ");
      sendToDebug_float32(dbg_ALC_need_gain, false);
      sendToDebug_str("First byte of RX-FPGA I: ");
      sendToDebug_float32(dbg_FPGA_Audio_Buffer_I_tmp, false); //first byte of I
      sendToDebug_str("First byte of RX-FPGA Q: ");
      sendToDebug_float32(dbg_FPGA_Audio_Buffer_Q_tmp, false); //first byte of Q
      sendToDebug_str("USB Audio RX samples: ");
      sendToDebug_uint32(dbg_RX_USB_AUDIO_SAMPLES, false); //~48000
      sendToDebug_str("USB Audio TX samples: ");
      sendToDebug_uint32(dbg_TX_USB_AUDIO_SAMPLES, false); //~48000
      sendToDebug_str("ADC MIN Amplitude: ");
      sendToDebug_int16(TRX_ADC_MINAMPLITUDE, false);
      sendToDebug_str("ADC MAX Amplitude: ");
      sendToDebug_int16(TRX_ADC_MAXAMPLITUDE, false);
      sendToDebug_str("WIFI State: ");
      sendToDebug_int16(WIFI_State, false);
      sendToDebug_newline();
			//PrintProfilerResult();
    }
		
    tim6_delay = HAL_GetTick();
    FPGA_samples = 0;
    AUDIOPROC_samples = 0;
    AUDIOPROC_TXA_samples = 0;
    AUDIOPROC_TXB_samples = 0;
    WM8731_DMA_samples = 0;
    RX_USB_AUDIO_SAMPLES = 0;
    TX_USB_AUDIO_SAMPLES = 0;
    TRX_Time_InActive++;
    WM8731_Buffer_underrun = false;
    FPGA_Buffer_underrun = false;
    RX_USB_AUDIO_underrun = false;
    FPGA_NeedSendParams = true;
  }

  if (TRX_on_TX() && CurrentVFO()->Mode != TRX_MODE_LOOPBACK)
  {
    TRX_Fan_Timeout += 3; //дуем в 2 раза больше чем работаем на передачу
    if (TRX_Fan_Timeout > 120)
      TRX_Fan_Timeout = 120; //но не более 2х минут
  }
  if (TRX_ptt_cat != TRX_old_ptt_cat)
    TRX_ptt_change();
  if (TRX_key_serial != TRX_old_key_serial)
    TRX_key_change();
  PERIPH_RF_UNIT_UpdateState(false);
  LCD_doEvents();
  FFT_printFFT();
  //power off sequence
  if ((HAL_GPIO_ReadPin(PWR_ON_GPIO_Port, PWR_ON_Pin) == GPIO_PIN_RESET) && ((HAL_GetTick() - powerdown_start_delay) > POWERDOWN_TIMEOUT))
	{
		TRX_Inited = false;
		LCD_busy = true;
		HAL_Delay(10);
		LCDDriver_Fill(COLOR_BLACK);
		LCDDriver_printTextFont("POWER OFF", 100, LCD_HEIGHT / 2, COLOR_WHITE, COLOR_BLACK, FreeSans12pt7b);
		SaveSettings();
		HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_RESET);
		WM8731_TX_mode(); //mute
		WM8731_CleanBuffer();
		sendToDebug_flush();
		while (true)
		{
		}
	}
	//
	if(TRX_Inited && (USB_LastActiveTime + USB_RESTART_TIMEOUT < HAL_GetTick())) 
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
  sendToDebug_flush();
  /* USER CODE END TIM7_IRQn 1 */
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
  FFT_printWaterfallDMA();
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
  * @brief This function handles USB On The Go FS End Point 1 Out global interrupt.
  */
void OTG_FS_EP1_OUT_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_FS_EP1_OUT_IRQn 0 */
	CPULOAD_WakeUp();
  /* USER CODE END OTG_FS_EP1_OUT_IRQn 0 */
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
  /* USER CODE BEGIN OTG_FS_EP1_OUT_IRQn 1 */

  /* USER CODE END OTG_FS_EP1_OUT_IRQn 1 */
}

/**
  * @brief This function handles USB On The Go FS End Point 1 In global interrupt.
  */
void OTG_FS_EP1_IN_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_FS_EP1_IN_IRQn 0 */
	CPULOAD_WakeUp();
  /* USER CODE END OTG_FS_EP1_IN_IRQn 0 */
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
  /* USER CODE BEGIN OTG_FS_EP1_IN_IRQn 1 */

  /* USER CODE END OTG_FS_EP1_IN_IRQn 1 */
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

  /* USER CODE END TIM15_IRQn 0 */
  HAL_TIM_IRQHandler(&htim15);
  /* USER CODE BEGIN TIM15_IRQn 1 */
	PERIPH_ProcessFrontPanel();
	if (NeedSaveSettings)
	{
		if ((HAL_GetTick() - eeprom_save_timestamp) > EEPROM_WRITE_INTERVAL) //Запись в EEPROM не чаще, чем раз в X секунд (против износа)
		{
			SaveSettings();
			eeprom_save_timestamp = HAL_GetTick();
		}
	}
  /* USER CODE END TIM15_IRQn 1 */
}

/* USER CODE BEGIN 1 */

void DMA1_Stream0_IRQHandler(void)
{
	CPULOAD_WakeUp();
	HAL_DMA_IRQHandler(&hdma_spi3_rx);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == GPIO_PIN_10) //FPGA BUS
  {
    //StartProfilerUs();
    if (!FPGA_busy)
      FPGA_fpgadata_iqclock();
    if (!FPGA_busy)
      FPGA_fpgadata_stuffclock();
    //EndProfilerUs(true);
  }
  else if (GPIO_Pin == GPIO_PIN_2) //Main encoder
  {
    TRX_Time_InActive = 0;
    if (TRX_Inited)
      PERIPH_ENCODER_checkRotate();
  }
  else if (GPIO_Pin == GPIO_PIN_13) //Secondary encoder
  {
    TRX_Time_InActive = 0;
    if (TRX_Inited)
      PERIPH_ENCODER2_checkRotate();
  }
  else if (GPIO_Pin == GPIO_PIN_4) //PTT
  {
    TRX_Time_InActive = 0;
    if (TRX_Inited && CurrentVFO()->Mode != TRX_MODE_NO_TX)
      TRX_ptt_change();
  }
  else if (GPIO_Pin == GPIO_PIN_1) //KEY DOT
  {
    TRX_Time_InActive = 0;
    TRX_key_change();
  }
  else if (GPIO_Pin == GPIO_PIN_0) //KEY DASH
  {
    TRX_Time_InActive = 0;
    TRX_key_change();
  }
  else if (GPIO_Pin == GPIO_PIN_11) //POWER OFF
  {
    powerdown_start_delay = HAL_GetTick();
  }
}
/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
