#include <Arduino.h>

#ifndef ARDUINO_USB_MODE
#error This ESP32 SoC has no Native USB interface
#elif ARDUINO_USB_MODE == 1
#warning This sketch should be used when USB is in OTG mode
void setup() {}
void loop() {}
#else
#include "USB.h"
#include "USBHID.h"
//#include "USBHIDVendor.h"

#include "commdata.h"
#include "json.hpp"

USBHID HID;

#define HID_VENDOR_REPORT_SIZE 64

static const uint8_t report_descriptor[] = {HID_USAGE_PAGE_N(HID_USAGE_PAGE_FIDO, 2),
                                            HID_USAGE(HID_USAGE_FIDO_U2FHID),
                                            HID_COLLECTION(HID_COLLECTION_APPLICATION),
                                            HID_USAGE(HID_USAGE_FIDO_DATA_IN), /* Input */
                                            HID_LOGICAL_MIN(0x00),
                                            HID_LOGICAL_MAX(0xff),
                                            HID_REPORT_SIZE(8),
                                            HID_REPORT_COUNT(HID_VENDOR_REPORT_SIZE),
                                            HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
                                            HID_USAGE(HID_USAGE_FIDO_DATA_OUT), /* Output */
                                            HID_LOGICAL_MIN(0x00),
                                            HID_LOGICAL_MAX(0xff),
                                            HID_REPORT_SIZE(8),
                                            HID_REPORT_COUNT(HID_VENDOR_REPORT_SIZE),
                                            HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
                                            HID_COLLECTION_END
};

#define CTAPHID_INIT 0x06
#define CTAPHID_CBOR 0x10
#define CTAPHID_PING 0x01

class CustomHIDDevice : public USBHIDDevice
{
public:
  CommData<uint8_t> response;
  bool dataToSend = false;

  CustomHIDDevice(void)
  {
    ESP_LOGI("MAIN", "", "");
    static bool initialized = false;
    if (!initialized)
    {
      initialized = true;
      HID.addDevice(this, sizeof(report_descriptor));
    }
  }

  void begin(void)
  {
    ESP_LOGI("MAIN", "", "");
    HID.begin();
  }

  void _onOutput(uint8_t report_id, const uint8_t *buffer, uint16_t len)
  {
    ESP_LOGI("MAIN", "report_id=%i, buffer_len=%i, ", report_id, len);

    CommData<uint8_t> data;
    data.addArray(buffer, len);
    data.print("Input: ");

    if (data.peek(4) == (0x80 + CTAPHID_INIT))
    {
      auto adr = data.getSubArray(0, 4);
      auto cmd = data.getSubArray(4, 1);
      auto nonce = data.getSubArray(7, 8);
      auto cid = data.getRandomVector(4);
      response.addArray(adr);
      response.addArray(cmd);
      response.add(00);
      response.add(17);
      response.addArray(nonce);
      response.addArray(cid);
      response.add(2);           // CTAPHID protocol version identifier
      response.add(1);           // Major device version number
      response.add(1);           // Minor device version number
      response.add(1);           // Build device version number
      response.add(0x01 + 0x04); // Capabilities flags - WINK + CTAPHID_CBOR + CTAPHID_MSG
      response.blowUp(HID_VENDOR_REPORT_SIZE - 1);
      response.print("Response: ");

      // send (d, sizeof(d) );
      // dataToSend = true;
      uint8_t d[64];
      memcpy(d, (const void *)response.getData().data(), response.getData().size());
      HID.SendReport(0, d, sizeof(d), 1000u);
      return;
    }

    ESP_LOGI("MAIN", "Unexpected command: %02X", data.peek(4));
  }

  uint16_t _onGetDescriptor(uint8_t *buffer)
  {
    ESP_LOGI("MAIN", "", "");
    memcpy(buffer, report_descriptor, sizeof(report_descriptor));
    return sizeof(report_descriptor);
  }

  bool send()
  {
    ESP_LOGI("MAIN", "dataToSend=%i", dataToSend);

    if (!dataToSend)
      return false;

    uint8_t d[64];
    memcpy(d, (const void *)response.getData().data(), response.getData().size());
    return send(d, sizeof(d));
  }

  bool send(uint8_t *buffer, size_t len)
  {
    ESP_LOGI("MAIN", "len=%i", len);
    int i = 0;
    while (!HID.ready() && (i < 100))
    {
      delay(10);
      i++;
    }
    ESP_LOGI("MAIN", "i=%i", i);
    dataToSend = false;
    return HID.SendReport(0, buffer, len, 1000u);
  }
};

CustomHIDDevice myDevice;

static void vendorEventCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  ESP_LOGI("MAIN", "event_id=%u", event_id);
}

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println();
  // Serial.setDebugOutput(true);

  USB.onEvent(vendorEventCallback);
  USB.VID(0x0483);
  USB.PID(0xa2ca);
  USB.begin();

  HID.onEvent(vendorEventCallback);
  myDevice.begin();

  ESP_LOGI("MAIN", "ready to go!", "");
}

void loop()
{
  if (myDevice.dataToSend)
  {
    if (myDevice.send())
    {
      ESP_LOGI("MAIN", "Data sent!");
    }
  }

  delay(100);
}
#endif /* ARDUINO_USB_MODE */