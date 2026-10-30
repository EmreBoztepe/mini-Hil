#include "hil_usb_cdc.h"
#include "hil_usb_cdc_port.h"

#include <string.h>

#include "stm32f4xx.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

/* CubeMX defines the handle in usb_device.c without exporting it in its header. */
extern USBD_HandleTypeDef hUsbDeviceFS;

_Static_assert(HIL_USB_CDC_PACKET_SIZE == CDC_DATA_FS_MAX_PACKET_SIZE,
               "Platform buffers must match the USB FS CDC packet size");

static uint8_t s_rx_buffer[HIL_USB_CDC_PACKET_SIZE];
static uint8_t s_tx_buffer[HIL_USB_CDC_PACKET_SIZE];
static volatile uint32_t s_rx_length;
static volatile uint32_t s_rx_offset;
static volatile bool s_configured;
static volatile bool s_tx_busy;
static volatile bool s_rx_rearm;

/* Main-loop access is protected against USB callbacks, including disconnect. */
static bool IsConfigured(void)
{
  return s_configured && (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED);
}

void HilUsbCdc_OnDeInit(void)
{
  s_configured = false;
  s_tx_busy = false;
  s_rx_length = 0U;
  s_rx_offset = 0U;
  s_rx_rearm = false;
}

void HilUsbCdc_Init(void)
{
  uint32_t irq_state = __get_PRIMASK();
  __disable_irq();
  HilUsbCdc_OnDeInit();
  __set_PRIMASK(irq_state);
}

void HilUsbCdc_OnConfigured(void)
{
  HilUsbCdc_OnDeInit();
  s_configured = true;
  /* The ST class arms the first OUT reception after its Init callback. */
}

void HilUsbCdc_OnReceive(const uint8_t *data, uint32_t length)
{
  /* FS delivers at most one 64-byte packet; OUT stays unarmed until consumed. */
  memcpy(s_rx_buffer, data, length);
  s_rx_length = length;
  s_rx_offset = 0U;
  s_rx_rearm = (length == 0U);
}

void HilUsbCdc_OnTransmitComplete(void)
{
  s_tx_busy = false;
}

bool HilUsbCdc_IsConfigured(void)
{
  uint32_t irq_state = __get_PRIMASK();
  __disable_irq();
  bool configured = IsConfigured();
  __set_PRIMASK(irq_state);
  return configured;
}

bool HilUsbCdc_CanWrite(void)
{
  uint32_t irq_state = __get_PRIMASK();
  __disable_irq();
  bool ready = IsConfigured() && !s_tx_busy;
  __set_PRIMASK(irq_state);
  return ready;
}

HilUsbCdc_Status HilUsbCdc_Process(void)
{
  HilUsbCdc_Status result = HIL_USB_CDC_OK;
  uint32_t irq_state = __get_PRIMASK();
  __disable_irq();

  if (!IsConfigured())
  {
    result = HIL_USB_CDC_NOT_READY;
  }
  else if (s_rx_rearm)
  {
    if (USBD_CDC_ReceivePacket(&hUsbDeviceFS) == USBD_OK)
    {
      s_rx_rearm = false;
    }
    else
    {
      result = HIL_USB_CDC_IO_ERROR;
    }
  }

  __set_PRIMASK(irq_state);
  return result;
}

uint32_t HilUsbCdc_Read(uint8_t *data, uint32_t capacity)
{
  if ((data == NULL) || (capacity == 0U))
  {
    return 0U;
  }

  uint32_t irq_state = __get_PRIMASK();
  __disable_irq();
  uint32_t length = 0U;

  if (IsConfigured())
  {
    length = s_rx_length - s_rx_offset;
    if (length > capacity)
    {
      length = capacity;
    }

    if (length > 0U)
    {
      memcpy(data, &s_rx_buffer[s_rx_offset], length);
      s_rx_offset += length;
      if (s_rx_offset == s_rx_length)
      {
        s_rx_length = 0U;
        s_rx_offset = 0U;
        s_rx_rearm = true;
      }
    }
  }

  __set_PRIMASK(irq_state);
  return length;
}

HilUsbCdc_Status HilUsbCdc_Write(const uint8_t *data, uint32_t length)
{
  if ((data == NULL) || (length == 0U) || (length > HIL_USB_CDC_PACKET_SIZE))
  {
    return HIL_USB_CDC_INVALID_ARGUMENT;
  }

  HilUsbCdc_Status result;
  uint32_t irq_state = __get_PRIMASK();
  __disable_irq();

  if (!IsConfigured())
  {
    result = HIL_USB_CDC_NOT_READY;
  }
  else if (s_tx_busy)
  {
    result = HIL_USB_CDC_BUSY;
  }
  else
  {
    memcpy(s_tx_buffer, data, length);
    s_tx_busy = true;
    uint8_t status = CDC_Transmit_FS(s_tx_buffer, (uint16_t)length);
    if (status == USBD_OK)
    {
      result = HIL_USB_CDC_OK;
    }
    else
    {
      s_tx_busy = false;
      result = (status == USBD_BUSY) ? HIL_USB_CDC_BUSY : HIL_USB_CDC_IO_ERROR;
    }
  }

  __set_PRIMASK(irq_state);
  return result;
}
