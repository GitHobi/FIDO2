#include "ctap.h"

#include <memory>
#include "YACL.h"
#include "crypto.h"
#include "storage.h"
#include "authenticator.h"

CTAP::CTAP(void)
{
}

bool CTAP::processBuffer(CommData<uint8_t> &data, CommData<uint8_t> &response)
{
    bool result = false;

    // CommData<uint8_t> data;
    // data.addArray(buffer, len);
    ESP_LOGI("CTAP", "Processing data with len %i", data.getDataSize());
    data.print("Input (processBuffer): ");

    auto cmd = data.peek(4);

    switch (cmd)
    {
    case (0x80 + CTAPHID_INIT):
        result = cmdINIT(data, response);
        break;
    case (0x80 + CTAPHID_CBOR):
        result = cmdCBOR(data, response);
        break;
    case (0x80 + CTAPHID_MSG):
        result = cmdMSG(data, response);
        break;
    default:
        ESP_LOGI("MAIN", "Unexpected command: %02X", cmd);
        break;
    }

    return result;
}

bool CTAP::cmdINIT(CommData<uint8_t> &data, CommData<uint8_t> &response)
{
    auto adr = data.getSubArray(0, 4);
    auto cmd = data.getSubArray(4, 1);
    auto nonce = data.getSubArray(7, 8);
    auto cid = data.getRandomVector(4);

    response.clear();
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

    return true;
}

bool CTAP::cmdCBOR(CommData<uint8_t> &data, CommData<uint8_t> &response)
{
    CBORPair resp = CBORPair(100);
    resp.append(0x01, "FIDO_2_0");
    char aaguid[17] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x00};
    resp.append(0x03, aaguid);
    const uint8_t *cbor_encoded = resp.to_CBOR();

    response.clear();
    auto adr = data.getSubArray(0, 4);
    auto cmd = data.getSubArray(4, 1);
    // auto nonce = data.getSubArray(7, 8);
    // auto cid = data.getRandomVector(4);
    response.addArray(adr);
    response.addArray(cmd);
    response.add(00);
    response.add(resp.length() + 2);
    response.add(0x90);
    response.add(0x00);
    for (size_t i = 0; i < resp.length(); ++i)
    {
        response.add(cbor_encoded[i]);
    }

    return true;
}

void encodePublicKey(PublicKey *publicKey, uint8_t *encodedKey)
{
    // use external buffer?
    CBORPair cborPair;

    // kty: EC2 key type
    cborPair.append(1, 2);

    // alg: ES256 signature algorithm
    cborPair.append(3, -7);

    // crv: P-256 curve
    cborPair.append(-1, 1);

    // x-coordinate as byte string 32 bytes in length
    CBOR cborX;
    cborX.encode(publicKey->x, 32);
    cborPair.append(-2, cborX);

    // y-coordinate as byte string 32 bytes in length
    CBOR cborY;
    cborY.encode(publicKey->y, 32);
    cborPair.append(-3, cborY);

    memcpy(encodedKey, cborPair.to_CBOR(), cborPair.length());
}

bool CTAP::cmdMSG(CommData<uint8_t> &data, CommData<uint8_t> &response)
{
    uint16_t size = data.peek(5) * 256 + data.peek(6);

    uint8_t pos = 7;
    uint8_t CLA = data.peek(pos++);
    uint8_t INS = data.peek(pos++);
    uint8_t P1 = data.peek(pos++);
    uint8_t P2 = data.peek(pos++);
    uint8_t Lc = data.peek(pos++);

    if (INS == 0x03)
    {
        ESP_LOGI("U2F", "Found command VERSION (CLA = %02X, INS = %02X)", CLA, INS);
        uint8_t txt[] = "U2F_V2";

        response.clear();
        auto adr = data.getSubArray(0, 4);
        auto cmd = data.getSubArray(4, 1);
        response.addArray(adr);
        response.addArray(cmd);
        response.add(0);
        response.add(8);
        pos = 0;
        response.add(txt[pos++]); // U
        response.add(txt[pos++]); // 2
        response.add(txt[pos++]); // F
        response.add(txt[pos++]); // _
        response.add(txt[pos++]); // V
        response.add(txt[pos++]); // 2

        response.add(0x90);
        response.add(0x00);

        return true;
    }

    if (INS == 0x02)
    {
        ESP_LOGI("U2F", "Found command AUTHENTICATE  (CLA = %02X, INS = %02X P1=%i)", CLA, INS, P1);
        // If P1 == 0x07 (check only), the device only verifies if the Application Parameter and Key Handle are valid for this U2F device.
        // Otherwise, if P1 == 0x03 (enforce user presence and sign)
        // or P1 == 0x08 (don’t enforce user presence and sign)
        // the U2F device then signs a payload in the following format:
        //
        // U2F AUTHENTICATE Signature Field	| Size (Bytes)
        // Application Parameter	        | 32
        // User Presence Flags	            | 1
        // Counter	                        | 4
        // Challenge Parameter	            | 32
    }

    if (INS == 0x01)
    {
        // The REGISTER command takes no input parameters. The input data is in the following format:
        //
        // U2F REGISTER Request Field	    | Size (Bytes)
        // Challenge Parameter	            | 32
        // Application Parameter	        | 32

        ESP_LOGI("U2F", "Found command REGISTER (CLA = %02X, INS = %02X)", CLA, INS);

        CommData<uint8_t> challenge(data, 14, 32);
        challenge.print("Challenge Parameter: ");

        uint8_t clientDataHash[32];
        challenge.getBytes(clientDataHash, sizeof(clientDataHash));

        CommData<uint8_t> application(data, 14 + 32, 32);
        application.print("Application Parameter: ");

        // Data generation

        AuthenticatorData *authData = new AuthenticatorData();
        //std::unique_ptr<AuthenticatorData> authData = std::unique_ptr<AuthenticatorData>(new AuthenticatorData());        
        authData->flags.val = 0;
        authData->flags.f.userPresent = true;
        authData->flags.f.userVerified = true;
        authData->flags.f.attestationData = true;

        uint8_t buffer[32];
        application.getBytes(buffer, sizeof(buffer));

        ESP_LOGI ( "", "preHash - source=%p, target=%p", buffer, authData->rpIdHash);

        Crypto c;
        c.SHA256_hash(&buffer[0], 32, authData->rpIdHash); // application
        String rpID = String(authData->rpIdHash, 32);

        // 2. public key
        ECDSA ecdsa;
        PublicKey publicKey;
        ecdsa.getPublicKey(&publicKey);
        CommData<uint8_t> pKey;
        pKey.addArray(publicKey.x, 32);
        pKey.addArray(publicKey.y, 32);
        pKey.print ( "\e[1;33mPublicKey: \e[1;0m");

        // 3. key handle length
        // buffer[length++] = CREDENTIAL_ID_LENGTH;

        // 4. key handle
        //Credential *credential = nullptr;
        CredentialsStorage cs;
        Credential 
        // String rpID = "webauthn.me";
        // if (!CredentialsStorage::findCredential(rpID, getUserID(), &credential)){
        cs.createCredential(rpID, cs.getUserID(), &credential);
        //}
        // memcpy(&buffer[length], credential->id.value, CREDENTIAL_ID_LENGTH);
        // length += CREDENTIAL_ID_LENGTH;
        CommData<uint8_t> cred;
        cred.addArray((uint8_t *)credential, CREDENTIAL_ID_LENGTH);
        

        // 5. attestation certificate
        Authenticator a;
        /*
        memcpy(&buffer[length], FIDO2::Authenticator::certificate, FIDO2::Authenticator::certificateSize);
        length += FIDO2::Authenticator::certificateSize;
        */
        CommData<uint8_t> attest;
        attest.addArray(a.getCertificate(), a.getCertificateSize());
        // 6. signature

        authData->attestedCredentialData.credentialIdLen = CREDENTIAL_ID_LENGTH;
        // memcpy(authData->attestedCredentialData.credentialId, credential->id.value, CREDENTIAL_ID_LENGTH);
        encodePublicKey(&publicKey, authData->attestedCredentialData.publicKey);
        size_t signatureLength = 0;
        //uint8_t signature[1024];
        uint8_t *signature = new uint8_t[1024];
        //a.sign(authData.get(), clientDataHash, signature, &signatureLength);
        a.sign(authData, clientDataHash, signature, &signatureLength);
        CommData<uint8_t> sig;
        sig.addArray(signature, signatureLength);
        // length += signatureLength;
        /**/
        delete[] signature;

        // status SW_NO_ERROR
        // buffer[length++] = 0x90;
        // buffer[length++] = 0x00;

        // The U2F device will create a new P-256 signing keypair used for future authentications and will return back the following data:
        //
        // U2F REGISTER Reply Field	Size    | (Bytes)
        // Reserved (0x05)	                | 1
        // Public Key	                    | 65
        // Key Handle Length	            | 1
        // Key Handle	                    | 0-255
        // Attestation Certificate	        | X.509 (DER)
        // ECDSA (P-256) Signature	        | 71-73
        response.clear();
        auto adr = data.getSubArray(0, 4);
        auto cmd = data.getSubArray(4, 1);

        size_t l = 1 + 1 + pKey.getData().size() + 1 + cred.getData().size() + attest.getData().size() + sig.getData().size() + 1 + 1;
        response.addArray(adr);
        response.addArray(cmd);
        response.add((l >> 8) & 0xFF); // Size higher
        response.add(l & 0xFF);        // Size lower

        response.add(0x05);

        response.add(0x04);
        response.addArray(pKey.getData());

        response.add(CREDENTIAL_ID_LENGTH);
        response.addArray(cred.getData());

        response.addArray(attest.getData());

        response.addArray(sig.getData());

        response.add(0x90);
        response.add(0x00);

        response.print ( "Complete Response: ");

        return true;
    }

    return false;
}
