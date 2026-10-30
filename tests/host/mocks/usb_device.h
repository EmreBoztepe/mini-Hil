#ifndef TEST_USB_DEVICE_H
#define TEST_USB_DEVICE_H
#include <stdint.h>
#define USBD_STATE_DEFAULT 1U
#define USBD_STATE_CONFIGURED 3U
typedef struct
{
  uint8_t dev_state;
} USBD_HandleTypeDef;
#endif
