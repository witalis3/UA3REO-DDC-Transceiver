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

	volatile uint32_t addr = 0x1FF00000;
	if (TRX_Inited)
		LCD_showError("Flash DFU mode", false);
	//prepare cpu
	MX_USB_DevDisconnect();
	hiwdg1.Init.Reload = 0;
	for (uint8_t i = 0; i < 255; i++)
		HAL_NVIC_DisableIRQ((IRQn_Type)i);
	HAL_RCC_DeInit();
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;
	//go to bootloader
	//SYSCFG->MEMRMP = 0x01;
	SysMemBootJump = (void(*)(void)) (*((uint32_t *)(addr + 4)));
	__set_MSP(*(uint32_t *)addr);
	SysMemBootJump();
}
