#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "hil_usb_cdc.h"
#include "hil_usb_cdc_port.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

USBD_HandleTypeDef hUsbDeviceFS;
static uint32_t s_irq_mask;
static uint32_t s_rx_arm_count;
static uint32_t s_tx_count;
static uint8_t s_rx_arm_result;
static uint8_t s_tx_result;
static uint8_t *s_last_tx;
static uint16_t s_last_tx_length;

uint32_t __get_PRIMASK(void) { return s_irq_mask; }
void __disable_irq(void) { s_irq_mask = 1U; }
void __set_PRIMASK(uint32_t value) { s_irq_mask = value; }

uint8_t CDC_Transmit_FS(uint8_t *data, uint16_t length)
{
  assert(s_irq_mask == 1U);
  ++s_tx_count;
  s_last_tx = data;
  s_last_tx_length = length;
  return s_tx_result;
}

uint8_t USBD_CDC_ReceivePacket(USBD_HandleTypeDef *device)
{
  assert(s_irq_mask == 1U);
  assert(device == &hUsbDeviceFS);
  ++s_rx_arm_count;
  return s_rx_arm_result;
}

static void Configure(void)
{
  s_rx_arm_count = 0U;
  s_tx_count = 0U;
  s_rx_arm_result = USBD_OK;
  s_tx_result = USBD_OK;
  s_irq_mask = 0U;
  hUsbDeviceFS.dev_state = USBD_STATE_CONFIGURED;
  HilUsbCdc_Init();
  HilUsbCdc_OnConfigured();
}

int main(void)
{
  uint8_t packet[64];
  uint8_t received[64];
  for (uint32_t i = 0U; i < sizeof(packet); ++i)
  {
    packet[i] = (uint8_t)i;
  }

  HilUsbCdc_Init();
  assert(!HilUsbCdc_IsConfigured());
  assert(!HilUsbCdc_CanWrite());
  assert(HilUsbCdc_Process() == HIL_USB_CDC_NOT_READY);
  assert(HilUsbCdc_Write(packet, 1U) == HIL_USB_CDC_NOT_READY);
  assert(HilUsbCdc_Read(received, sizeof(received)) == 0U);

  Configure();
  HilUsbCdc_OnReceive(packet, sizeof(packet));
  assert(HilUsbCdc_Read(received, 17U) == 17U);
  assert(HilUsbCdc_Process() == HIL_USB_CDC_OK);
  assert(s_rx_arm_count == 0U);
  assert(HilUsbCdc_Read(&received[17], 47U) == 47U);
  assert(memcmp(packet, received, sizeof(packet)) == 0);
  assert(HilUsbCdc_Read(received, sizeof(received)) == 0U);
  assert(HilUsbCdc_Process() == HIL_USB_CDC_OK);
  assert(s_rx_arm_count == 1U);
  assert(HilUsbCdc_Process() == HIL_USB_CDC_OK);
  assert(s_rx_arm_count == 1U);

  HilUsbCdc_OnReceive(packet, 0U);
  assert(HilUsbCdc_Process() == HIL_USB_CDC_OK);
  assert(s_rx_arm_count == 2U);

  HilUsbCdc_OnReceive(packet, 3U);
  s_rx_arm_result = USBD_FAIL;
  assert(HilUsbCdc_Read(received, sizeof(received)) == 3U);
  assert(HilUsbCdc_Process() == HIL_USB_CDC_IO_ERROR);
  s_rx_arm_result = USBD_OK;
  assert(HilUsbCdc_Process() == HIL_USB_CDC_OK);
  assert(s_rx_arm_count == 4U);

  Configure();
  assert(HilUsbCdc_Write(packet, sizeof(packet)) == HIL_USB_CDC_OK);
  assert(s_last_tx != packet);
  assert(s_last_tx_length == sizeof(packet));
  assert(!HilUsbCdc_CanWrite());
  packet[0] = 0xEEU;
  assert(s_last_tx[0] == 0U);
  assert(HilUsbCdc_Write(packet, 1U) == HIL_USB_CDC_BUSY);
  assert(s_tx_count == 1U);
  assert(s_last_tx[0] == 0U);
  HilUsbCdc_OnTransmitComplete();
  assert(HilUsbCdc_CanWrite());
  assert(HilUsbCdc_Write(packet, 1U) == HIL_USB_CDC_OK);
  assert(s_last_tx[0] == 0xEEU);
  HilUsbCdc_OnTransmitComplete();

  s_tx_result = USBD_BUSY;
  assert(HilUsbCdc_Write(packet, 1U) == HIL_USB_CDC_BUSY);
  assert(HilUsbCdc_CanWrite());
  s_tx_result = USBD_FAIL;
  assert(HilUsbCdc_Write(packet, 1U) == HIL_USB_CDC_IO_ERROR);
  assert(HilUsbCdc_CanWrite());
  assert(HilUsbCdc_Write(NULL, 1U) == HIL_USB_CDC_INVALID_ARGUMENT);
  assert(HilUsbCdc_Write(packet, 0U) == HIL_USB_CDC_INVALID_ARGUMENT);
  assert(HilUsbCdc_Write(packet, 65U) == HIL_USB_CDC_INVALID_ARGUMENT);

  Configure();
  HilUsbCdc_OnReceive(packet, 4U);
  assert(HilUsbCdc_Write(packet, 4U) == HIL_USB_CDC_OK);
  HilUsbCdc_OnDeInit();
  assert(!HilUsbCdc_IsConfigured());
  assert(HilUsbCdc_Read(received, sizeof(received)) == 0U);
  assert(HilUsbCdc_Process() == HIL_USB_CDC_NOT_READY);
  HilUsbCdc_OnConfigured();
  assert(HilUsbCdc_CanWrite());
  assert(HilUsbCdc_Read(received, sizeof(received)) == 0U);
  assert(HilUsbCdc_Process() == HIL_USB_CDC_OK);
  assert(s_rx_arm_count == 0U);

  hUsbDeviceFS.dev_state = USBD_STATE_DEFAULT;
  assert(!HilUsbCdc_IsConfigured());
  assert(HilUsbCdc_Write(packet, 1U) == HIL_USB_CDC_NOT_READY);
  hUsbDeviceFS.dev_state = USBD_STATE_CONFIGURED;
  s_irq_mask = 1U;
  assert(HilUsbCdc_IsConfigured());
  assert(HilUsbCdc_CanWrite());
  assert(HilUsbCdc_Read(received, sizeof(received)) == 0U);
  assert(HilUsbCdc_Process() == HIL_USB_CDC_OK);
  assert(HilUsbCdc_Write(packet, 1U) == HIL_USB_CDC_OK);
  assert(s_irq_mask == 1U);

  puts("PASS: RX backpressure, partial reads, ZLP, rearm retry, TX ownership/busy/error, disconnect and IRQ restoration");
  return 0;
}
