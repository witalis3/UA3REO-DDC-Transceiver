#ifndef __USBD_CDC_CAT_IF_H__
#define __USBD_CDC_CAT_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_ua3reo.h"

#if HRDW_HAS_USB_CAT
extern USBD_CAT_ItfTypeDef USBD_CAT_fops_FS;
extern void ua3reo_dev_cat_parseCommand(void);
extern void CAT_SetWIFICommand(char *data, uint32_t length, uint32_t link_id);

typedef enum
{
    FT817_SET_FREQ      = 0x01,
    FT817_GET_FREQ      = 0x03,
    FT817_SPLIT_ON      = 0x02,
    FT817_SPLIT_OFF     = 0x82,
    FT817_PTT_ON        = 0x08,
    FT817_PTT_OFF       = 0x88,
    FT817_MODE_SET      = 0x07,
    FT817_PWR_ON        = 0x0f,
    FT817_TOGGLE_VFO    = 0x81,
    FT817_A7            = 0xa7,
    FT817_EEPROM_READ   = 0xbb,
    FT817_EEPROM_WRITE  = 0xbc,
    FT817_READ_TX_STATE = 0xbd,
    FT817_READ_RX_STATE = 0xe7,
    FT817_PTT_STATE     = 0xf7,
    FT817_NOOP          = 0xff,

    UHSDR_ID            = 0x42, // this command is not known to the FT817 so we can use this to identify a UHSDR
} Ft817_CatCmd_t;

#endif

#ifdef __cplusplus
}
#endif

#endif
