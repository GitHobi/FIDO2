#pragma once

#include <Arduino.h>

class utils
{
public:
    static void serialDumpBuffer(const uint8_t *buffer, const size_t len, const char *prefix = nullptr)
    {
        uint16_t addr = 0;
        ESP_LOGI("UTILS", "len=%i buffer=%p", len, buffer);
        try
        {
            const uint16_t pageSize = 16;
            uint16_t pages = (len / pageSize) + ((len % pageSize) ? 1 : 0);

            for (auto page = 0; page < pages; page++)
            {
                if (prefix != nullptr)
                {
                    Serial.print(prefix);
                    Serial.print(": ");
                }
                Serial.printf ( "%04X: ", addr); 
                addr += pageSize;

                for (auto i = 0; i < pageSize; i++)
                {
                    const uint16_t pos = page * pageSize + i;
                    if (pos >= len)
                        break;

                    Serial.printf("%02X ", buffer[pos]);
                }
                Serial.println("");
            }
        }
        catch (const std::exception &e)
        {
            ESP_LOGE("", "%s", e.what());
        }
    }
};