#ifndef __XPD_RCC_H_
#define __XPD_RCC_H_

#include <xpd_common.h>

#define RCC_POS_USB_EN          __HAL_RCC_USB_CLK_ENABLE
#define RCC_POS_USB_DIS         __HAL_RCC_USB_CLK_DISABLE
#define RCC_POS_OTG_FS_EN       __HAL_RCC_USB_OTG_FS_CLK_ENABLE
#define RCC_POS_OTG_FS_DIS      __HAL_RCC_USB_OTG_FS_CLK_DISABLE
#define RCC_POS_OTG_HS_EN       __HAL_RCC_USB_OTG_HS_CLK_ENABLE
#define RCC_POS_OTG_HS_DIS      __HAL_RCC_USB_OTG_HS_CLK_DISABLE
#define RCC_POS_USBPHYC_EN      __HAL_RCC_OTGPHYC_CLK_ENABLE
#define RCC_POS_USBPHYC_DIS     __HAL_RCC_OTGPHYC_CLK_DISABLE
#define RCC_POS_OTG_HS_ULPI_EN  __HAL_RCC_USB_OTG_HS_ULPI_CLK_ENABLE
#define RCC_POS_OTG_HS_ULPI_DIS __HAL_RCC_USB_OTG_HS_ULPI_CLK_DISABLE

#define RCC_vClockEnable(A)     A##_EN()
#define RCC_vClockDisable(A)    A##_DIS()

#define RCC_CLKFREQ_HZ_HSE      HSE_VALUE
#define RCC_CLKFREQ_HZ_HCLK     HAL_RCC_GetHCLKFreq()
#define RCC_ulClockFreq_Hz(A)   RCC_CLKFREQ_HZ_##A

#endif /* __XPD_RCC_H_ */
