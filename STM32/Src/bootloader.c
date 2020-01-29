#include "bootloader.h"
#include "usb_device.h"
#include "main.h"
#include "lcd.h"

/**
 * Function to perform jump to system memory boot from user application
 *
 * Call function when you want to jump to system memory
 */
void JumpToBootloader(void)
{
	void (*SysMemBootJump)(void);
	/**
	 * Step: Set system memory address.
	 *       For STM32F429, system memory is on 0x1FFF 0000
	 *       For other families, check AN2606 document table 110 with descriptions of memory addresses
	 */
	/*
	volatile uint32_t addr = 0x1FFF0000;
	if (TRX_Inited)
		LCD_showError("Flash DFU mode", false);
	//turn on LCD backlight
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = LCD_BACKLIGT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(LCD_BACKLIGT_GPIO_Port, &GPIO_InitStruct);
	HAL_GPIO_WritePin(LCD_BACKLIGT_GPIO_Port, LCD_BACKLIGT_Pin, GPIO_PIN_RESET);
	//prepare cpu
	MX_USB_DevDisconnect();
	hiwdg.Init.Reload = 0;
	for (uint8_t i = 0; i < 255; i++)
		HAL_NVIC_DisableIRQ((IRQn_Type)i);
	HAL_RCC_DeInit();
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;
	//go to bootloader
	SYSCFG->MEMRMP = 0x01;
	SysMemBootJump = (void(*)(void)) (*((uint32_t *)(addr + 4)));
	__set_MSP(*(uint32_t *)addr);
	SysMemBootJump();
	*/
}
