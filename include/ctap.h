#pragma once

#include "commdata.h"

#define CTAPHID_INIT 0x06
#define CTAPHID_CBOR 0x10
#define CTAPHID_PING 0x01
#define CTAPHID_MSG 0x03

#define HID_VENDOR_REPORT_SIZE 64

class CTAP
{
public:
    CTAP(void);
    bool processBuffer ( CommData<uint8_t> &data, CommData<uint8_t> &response);

    bool cmdINIT (CommData<uint8_t> &data, CommData<uint8_t> &response);
    bool cmdCBOR (CommData<uint8_t> &data, CommData<uint8_t> &response);
    bool cmdMSG (CommData<uint8_t> &data, CommData<uint8_t> &response);
};
