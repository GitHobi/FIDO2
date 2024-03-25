#ifndef ARDUINO_USB_MODE
#error This ESP32 SoC has no Native USB interface
#elif ARDUINO_USB_MODE == 1
#warning This sketch should be used when USB is in OTG mode
#else
#include "fidodevice.h"

FidoDevice::FidoDevice(USBHID *HID)
{
    ESP_LOGI("MAIN", "", "");
    static bool initialized = false;
    if (!initialized)
    {
        _HID = HID;
        initialized = true;
        _HID->addDevice(this, sizeof(report_descriptor));
    }
}

void FidoDevice::begin(void)
{
    ESP_LOGI("MAIN", "", "");
    _HID->begin();
}

uint16_t FidoDevice::_onGetDescriptor(uint8_t *buffer)
{
    ESP_LOGI("MAIN", "", "");
    memcpy(buffer, report_descriptor, sizeof(report_descriptor));
    return sizeof(report_descriptor);
}

bool FidoDevice::send()
{
    ESP_LOGI("MAIN", "dataToSend=%i", dataToSend);

    if (!dataToSend)
        return false;

    uint8_t d[64];
    memcpy(d, (const void *)response.getData().data(), response.getData().size());
    return send(d, sizeof(d));
}

bool FidoDevice::send(uint8_t *buffer, size_t len)
{
    ESP_LOGI("MAIN", "len=%i", len);
    int i = 0;
    while (!_HID->ready() && (i < 100))
    {
        delay(10);
        i++;
    }
    ESP_LOGI("MAIN", "i=%i", i);
    dataToSend = false;
    return _HID->SendReport(0, buffer, len, 1000u);
}

void FidoDevice::_onOutput(uint8_t report_id, const uint8_t *buffer, uint16_t len)
{
    ESP_LOGI("MAIN", "report_id=%i, buffer_len=%i, cmd=%i", report_id, len, buffer[4]);

    CommData<uint8_t> debugBuffer;
    debugBuffer.addArray(buffer, len);
    debugBuffer.print("Debug Input:");

    CommData<uint8_t> response;

    bool processInputBuffer = false;

    if (buffer[4] > 0x80)
    {
        size_t dataLen = buffer[5] * 256 + buffer[6];

        ESP_LOGI("MAIN", "Creating new inputBuffer for command %02X with len=%i", buffer[4], dataLen);
        // a new command starts.
        inputBuffer.clear();
        inputBuffer.addArray(buffer, len);
        inputBuffer.setDataSize(dataLen);

        if (dataLen < (len - 4))
        {
            // we have enough data ...
            processInputBuffer = true;
        }
    }
    else
    {
        // we got a continuation package!
        ESP_LOGI("MAIN", "Adding Sequence %i to buffer", buffer[4]);
        inputBuffer.addArray(buffer + 5, len - 5);
        ESP_LOGI("MAIN", "targetSize %i - size=%i, size-4=%i", inputBuffer.getDataSize(), inputBuffer.getData().size(), inputBuffer.getData().size() - 4);
        if (inputBuffer.getDataSize() < inputBuffer.getData().size() - 4)
        {
            processInputBuffer = true;
        }
    }

    if (!processInputBuffer)
        return;

    bool result = ctap.processBuffer(inputBuffer, response);

    if (result)
    {
        auto responseSize = response.getData().size();
        ESP_LOGI ( "MAIN", "RepsonsSize=%i / %i", responseSize, HID_VENDOR_REPORT_SIZE);
        if (responseSize < HID_VENDOR_REPORT_SIZE) // Stimmt nicht ganz!
        {
            response.blowUp(HID_VENDOR_REPORT_SIZE - 1);
            response.print("Response: ");
            uint8_t d[64];
            memcpy(d, (const void *)response.getData().data(), response.getData().size());
            _HID->SendReport(0, d, sizeof(d), 1000u);
        }
        else
        {
            // We have more data to send as in one package fits.
            // neet to create severl transmission packages.
            uint8_t buffer[64];
            uint8_t seqNr = 0;
            bool done = false;
            auto adr = response.getSubArray(0, 4);
            while (!done)
            {
                CommData<uint8_t> transmissionPackage;

                if (seqNr == 0)
                {
                    // First frame
                    response.moveData(0, 64, transmissionPackage);   
                }
                else
                {
                    ESP_LOGI ( "MAIN", "Adding Package %i (data=%i)", seqNr, response.getData().size());
                    transmissionPackage.addArray ( adr );
                    transmissionPackage.add(seqNr-1);
                    response.moveData(0, 64-5, transmissionPackage);
                }

                seqNr++;

                if ( transmissionPackage.getData().size() < 64 )
                {
                    transmissionPackage.blowUp(HID_VENDOR_REPORT_SIZE - 1);
                    done = true;
                }
             
                transmissionPackage.print("Package: ");
                
                memcpy(buffer, (const void *)transmissionPackage.getData().data(), transmissionPackage.getData().size());
                _HID->SendReport(0, buffer, sizeof(buffer), 1000u);
                delay ( 100 );
            }
        }
    }
}

#endif