//#pragma once
#ifndef FIDODEVICE_H
#define FIDODEVICE_H


#include <Arduino.h>

#include "YACL.h"
#include "commdata.h"
#include "ctap.h"

#ifndef ARDUINO_USB_MODE
#error This ESP32 SoC has no Native USB interface
#elif ARDUINO_USB_MODE == 1
#warning This sketch should be used when USB is in OTG mode
#else
#include "USB.h"
#include "USBHID.h"







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
                                            HID_COLLECTION_END};



class FidoDevice : public USBHIDDevice
{
public:
    CTAP ctap;

    CommData<uint8_t> inputBuffer;
    CommData<uint8_t> response;
    bool dataToSend = false;
    USBHID *_HID;

    FidoDevice(USBHID *HID);

    void begin(void);

    void _onOutput(uint8_t report_id, const uint8_t *buffer, uint16_t len);

    uint16_t _onGetDescriptor(uint8_t *buffer);

    bool send();
    bool send(uint8_t *buffer, size_t len);
};

#endif
#endif // FIDODEVICE