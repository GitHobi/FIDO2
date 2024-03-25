#pragma once

#include <Arduino.h>

#include <memory>
#include <vector>

// #include "cred-storage/storage.h"

#include "fixedbuffer.h"

struct Credential
{
    FixedBuffer32 id;
    String rpId;
    FixedBuffer64 userId;
};

class CredentialsStorage
{

public:
    std::vector<std::unique_ptr<Credential>> credentials;

    FixedBuffer64 userID;
    FixedBuffer64 getUserID()
    {
        if (userID.length == 0)
        {
            userID.alloc(4);
            userID.value[0] = 'u';
            userID.value[0] = 's';
            userID.value[0] = 'e';
            userID.value[0] = 'r';
        }
        return userID;
    }

    void reset();

    bool getCredential(const FixedBuffer32 &credentialId, Credential **credential);

    bool findCredential(const String &rpId, const FixedBuffer64 &userId, Credential **credential);

    bool createCredential(const String &rpId, const FixedBuffer64 &userId, Credential **credential);

    // bool storeCredential(const String &rpId, const FixedBuffer64 &userId);
};