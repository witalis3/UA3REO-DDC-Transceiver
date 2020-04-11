#include "bootloader.h"
#include "usb_device.h"
#include "main.h"
#include "lcd.h"

//перехов в DFU-режим булодера
void JumpToBootloader(void)
{
	void (*SysMemBootJump)(void);

	volatile uint32_t BootAddr = 0x1FF09800;
	LCD_busy=true;
	TRX_Inited=false;
	LCD_showError("Flash DFU mode", false);
	MX_USB_DevDisconnect();
	HAL_Delay(1000);
	//prepare cpu
	hiwdg1.Init.Reload = 0;
	__disable_irq(); //Disable all interrupts
	SysTick->CTRL = 0; //Disable Systick timer
	for (uint8_t i = 0; i < 255; i++) //Disable all interrupts
		HAL_NVIC_DisableIRQ((IRQn_Type)i);
	HAL_RCC_DeInit(); //Set the clock to the default state 
	for (uint8_t i=0;i<5;i++) //Clear Interrupt Enable Register & Interrupt Pending Register 
	{
		NVIC->ICER[i]=0xFFFFFFFF;
		NVIC->ICPR[i]=0xFFFFFFFF;
	}
	__enable_irq(); //Re-enable all interrupts
	//go to bootloader
	SysMemBootJump = (void(*)(void)) (*((uint32_t *)(BootAddr + 4)));
	__set_MSP(*(uint32_t *)BootAddr);
	SysMemBootJump();
	while(true);
}
