# FIDO2 Implementation


## Basics

Implementation is done based on espressifs native UBS support. \
Sample project to start is _libraries/USB/examples/CustomHIDDevice/CustomHIDDevice.ino_

FIDO2 compliant report descriptor looks like this:

#define HID_VENDOR_REPORT_SIZE 64

```cpp
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
```

Pitfal: Do not include a HID_REPORT_ID tag - the operating system will not accept the device!

Important specifications can be found here:
* [Client to Authenticator Protocol (CTAP)](https://fidoalliance.org/specs/fido-v2.0-ps-20190130/fido-client-to-authenticator-protocol-v2.0-ps-20190130.html)
* [Client to Authenticator Protocol (CTAP)](https://fidoalliance.org/specs/fido-v2.1-ps-20210615/fido-client-to-authenticator-protocol-v2.1-ps-errata-20220621.html)
* [Web Authentication](https://www.w3.org/TR/webauthn-1/#conforming-all-classes)
* [https://webauthn.io/](https://webauthn.io/)
* [https://webauthn.me/](https://webauthn.me/)

## compressed OTA

Data flow

### CTAPHID_INIT (0x06) 

06:47:51.950 > Input: \
06:47:51.950 > <span style="color:green">**FF FF FF FF**</span> <span style="color:red">86</span> <span style="color:yellow">**00 08**</span> <span style="color:lightblue">60 E3 C0 14 89 27 91 D1</span> _00_ \
06:47:51.950 > _00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00_ \
06:47:51.950 > _00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00_ \
06:47:51.962 > _00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00_ \
06:47:51.962 > \
06:47:51.962 > Response: \
06:47:51.962 > <span style="color:green">**FF FF FF FF**</span> <span style="color:red">86</span> <span style="color:yellow">**00 11**</span> <span style="color:lightblue">60 E3 C0 14 89 27 91 D1</span> <span style="color:lime">CB</span> \
06:47:51.973 > <span style="color:lime">**4E 3E A4**</span> <span style="color:orange">02 01 01 01 05</span> _00 00 00 00 00 00 00 00_ \
06:47:51.973 > _00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00_ \
06:47:51.973 > _00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00_ \
06:47:51.980 > 

The different colors are associated with the content:
* <span style="color:green">CID </span> FF FF FF FF - is the broadcast CID, used to open a connection
* <span style="color:red">CMD </span> 06 - with bit 8 set
* <span style="color:yellow">BCNT </span> - length of the following data package
* <span style="color:lightblue">Data - 8 byte nonce</span>
* <span style="color:lime">4 byte channel ID</span> - CID, created by the authenticator, to be used in following comminication.
* <span style="color:orange">Device info</span>
    * <span style="color:orange">0x02 - CTAPHID protocol version identifier</span>
    * <span style="color:orange">0x01 - Major device version number</span>
    * <span style="color:orange">0x01 - Minor device version number</span>
    * <span style="color:orange">0x01 - Build device version number</span>
    * <span style="color:orange">0x05 - Capabilities flags = CTAPHID_WINK + CTAPHID_CBOR + CTAPHID_MSG</span>

### CTABHID_CBOR 0x10

```
19:12:35.670 > Input: 
19:12:35.670 > CB 4E 3E A4 90 00 01 04 00 00 00 00 00 00 00 00 
19:12:35.681 > 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
19:12:35.681 > 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
19:12:35.681 > 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
```


## Uses third party software

* [JSON for Modern C++](https://github.com/nlohmann/json) (MIT License)
