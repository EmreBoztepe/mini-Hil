#ifndef HIL_USB_CDC_H
#define HIL_USB_CDC_H

#include <stdbool.h>
#include <stdint.h>

#define HIL_USB_CDC_PACKET_SIZE 64U

typedef enum
{
  HIL_USB_CDC_OK,
  HIL_USB_CDC_BUSY,
  HIL_USB_CDC_NOT_READY,
  HIL_USB_CDC_INVALID_ARGUMENT,
  HIL_USB_CDC_IO_ERROR
} HilUsbCdc_Status;

/* Main-loop API. Call Init before MX_USB_DEVICE_Init. */
void HilUsbCdc_Init(void);
HilUsbCdc_Status HilUsbCdc_Process(void);
bool HilUsbCdc_IsConfigured(void);
bool HilUsbCdc_CanWrite(void);

/* Copies up to capacity bytes; returns the number copied, or zero if empty. */
uint32_t HilUsbCdc_Read(uint8_t *data, uint32_t capacity);

/* Copies 1..64 bytes into owned storage. OK means accepted, not PC receipt. */
HilUsbCdc_Status HilUsbCdc_Write(const uint8_t *data, uint32_t length);

#endif /* HIL_USB_CDC_H */
