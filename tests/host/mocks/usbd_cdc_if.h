#ifndef TEST_USBD_CDC_IF_H
#define TEST_USBD_CDC_IF_H
#include "usb_device.h"
#define CDC_DATA_FS_MAX_PACKET_SIZE 64U
#define USBD_OK 0U
#define USBD_BUSY 1U
#define USBD_FAIL 2U
uint8_t CDC_Transmit_FS(uint8_t *data, uint16_t length);
uint8_t USBD_CDC_ReceivePacket(USBD_HandleTypeDef *device);
#endif
