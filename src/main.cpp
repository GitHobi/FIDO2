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
// #include "USBHIDVendor.h"

#include "YACL.h"
#include "commdata.h"

// #include "json.hpp"
// using json = nlohmann::json;

USBHID HID;

#include "fidodevice.h"

#include "crypto.h"
#include "utils.h"

FidoDevice myDevice(&HID);

static void vendorEventCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  ESP_LOGI("MAIN", "event_id=%u", event_id);
}

void setup()
{

  Serial.begin(115200);
  Serial.println();
  // Serial.setDebugOutput(true);
/*
  uint8_t data[32] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

  CommData<uint8_t> application;
  application.addArray ( data, 32);

  uint8_t buffer[32];
  application.getBytes(buffer, sizeof(buffer));


  AuthenticatorData *authData = new AuthenticatorData();
  authData->flags.val = 0;
  authData->flags.f.userPresent = true;
  authData->flags.f.userVerified = true;
  authData->flags.f.attestationData = true;

  uint8_t target[32];
  ESP_LOGI("", "preHash - source=%p, target=%p", buffer, target);

  ESP_LOGI("", "Buffer: ");
  utils::serialDumpBuffer(buffer, 32, "Buffer");

  ESP_LOGI("", "Target: ");
  utils::serialDumpBuffer(authData->rpIdHash, 32, "Target");

  Crypto c;
  c.SHA256_hash(buffer, 32, authData->rpIdHash); // application
  String rpID = String(authData->rpIdHash, 32);

  ESP_LOGI("", "Target: ");
  utils::serialDumpBuffer(authData->rpIdHash, 32, "Target");
*/
  /*
  CommData <uint8_t> data;
  data.addArray ( testdata, sizeof (testdata) );
  data.print ( "Daten: ");
  CommData <uint8_t> sub;
  data.moveData ( 0,5, sub);
  sub.print ( "Submenge: ");
  data.print ( "Daten: ");
*/
  //return;

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