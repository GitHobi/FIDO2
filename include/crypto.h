#pragma once

#include <Arduino.h>

#include <uECC.h>
#include "be.h"

#define CREDENTIAL_ID_LENGTH 32

#pragma pack(push, 1)
union AuthenticatorDataFlags
{
    struct flag_bitfields
    {
        bool userPresent : 1;
        uint8_t rfu1 : 1;
        bool userVerified : 1;
        uint8_t rfu2 : 3;
        bool attestationData : 1;
        bool extensions : 1;
    } f;
    uint8_t val;
};

struct AttestedCredentialData
{
    uint8_t aaguid[16];
    be_uint16_t credentialIdLen;
    uint8_t credentialId[CREDENTIAL_ID_LENGTH];
    uint8_t publicKey[77];
};

struct AuthenticatorData
{
    uint8_t rpIdHash[32];
    AuthenticatorDataFlags flags;
    uint32_t signCount;
    AttestedCredentialData attestedCredentialData;
};
#pragma pack(pop)

class Crypto
{

public:
    Crypto();

    bool SHA256_hash(const uint8_t *data, const size_t length, uint8_t *sha);
};

struct PrivateKey
{
    uint8_t key[32];
};
struct PublicKey
{
    uint8_t x[32];
    uint8_t y[32];
};

class ECDSA
{

    const struct uECC_Curve_t *_es256_curve = uECC_secp256r1();

    const PrivateKey privateKey = {
        .key = {0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD,
                0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD},
    };

    // extern const struct uECC_Curve_t *_es256_curve;
public:


    void getPublicKey(PublicKey *publicKey);

    void sign(const uint8_t *hash, uint8_t *signature);

    void encodeSignature(const uint8_t *signature, uint8_t *encodedSignature, size_t *encodedSize);

    void derivePublicKey(const PrivateKey *privateKey, PublicKey *publicKey);
};