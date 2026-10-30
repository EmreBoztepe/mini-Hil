#ifndef HIL_USB_CDC_PORT_H
#define HIL_USB_CDC_PORT_H

#include <stdint.h>

/* USB class callbacks only; application code uses hil_usb_cdc.h. */
void HilUsbCdc_OnConfigured(void);
void HilUsbCdc_OnDeInit(void);
void HilUsbCdc_OnReceive(const uint8_t *data, uint32_t length);
void HilUsbCdc_OnTransmitComplete(void);

#endif /* HIL_USB_CDC_PORT_H */
