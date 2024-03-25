#pragma once

#include "Arduino.h"
#include <SparkFun_ATECCX08a_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#SparkFun_ATECCX08a
#include <Wire.h>

class cryptoHW
{

public:
    cryptoHW(uint8_t SDA = 10, uint8_t SDC = 11)
    {
        Wire.begin(SDA, SDC);
        Wire.setBufferSize ( 1024);
        if (atecc.begin() == true)
        {
            Serial.println("Successful wakeUp(). I2C connections are good.");
        }
    }

    void printInfo();

private:
    ATECCX08A atecc;

    
};