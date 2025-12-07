#include "CryptnoxWallet.h"
#include <Arduino.h>
#include "uECC.h"

#define RESPONSE_LENGTH_IN_BYTES 64
#define RANDOM_BYTES 8

/* Main NFC handler:
 * - If ISO-DEP card detected → select app, request certificate, open secure channel.
 * - Otherwise → try reading UID of simple NFC tag.
 */
bool CryptnoxWallet::processCard() {
    bool ret = false;

    /* Check for ISO-DEP capable target (APDU-capable card) */
    if (driver.inListPassiveTarget()) {
        /* Try selecting Cryptnox app */
        if (selectApdu()) {
            /* Get certificate and establish secure channel */
            getCardCertificate();
            openSecureChannel();
            ret = true;
        }
    }
    else {
        /* Basic tag: read its UID */
        uint8_t uid[7];
        uint8_t uidLength;
        if (driver.readUID(uid, uidLength)) {
            Serial.print(F("Card UID: "));
            for (uint8_t i = 0; i < uidLength; i++) {
                if (uid[i] < 16) Serial.print(F("0"));
                Serial.print(uid[i], HEX);
                Serial.print(F(" "));
            }
            Serial.println();
        }
    }

    return ret;
}

/* Simple forward to PN532 driver for UID read */
bool CryptnoxWallet::readUID(uint8_t* uidBuffer, uint8_t &uidLength) {
    return driver.readUID(uidBuffer, uidLength);
}

/* Print PN532 firmware version via driver */
bool CryptnoxWallet::printPN532FirmwareVersion() {
    return driver.printFirmwareVersion();
}

/* SELECT APDU to activate Cryptnox application */
bool CryptnoxWallet::selectApdu() {
    bool ret = false;

    /* Application AID selection command */
    uint8_t selectApdu[] = { 
        0x00, 0xA4, 0x04, 0x00, 0x07, 0xA0, 0x00, 0x00, 0x10, 0x00, 0x01, 0x12
    };

    /* Response buffer on stack */
    uint8_t response[RESPONSE_LENGTH_IN_BYTES];
    uint8_t responseLength = sizeof(response);

    Serial.println(F("Sending Select APDU..."));

    /* Send SELECT command */
    if (driver.sendAPDU(selectApdu, sizeof(selectApdu), response, responseLength)) {
        ret = true;
    } else {
        Serial.println(F("APDU select failed."));
    }

    return ret;
}

/* Request card certificate (with random nonce appended) */
bool CryptnoxWallet::getCardCertificate() {
    bool ret = false;

    /* APDU template (last 8 bytes replaced by random nonce) */
    uint8_t getCardCertificateApdu[] = { 
        0x80, 0xF8, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    
    /* Local response buffer */
    uint8_t response[RESPONSE_LENGTH_IN_BYTES];
    uint8_t responseLength = sizeof(response);

    /* Fill last RANDOM_BYTES with random values */
    randomSeed(analogRead(0));
    for (int i = sizeof(getCardCertificateApdu) - RANDOM_BYTES; i < sizeof(getCardCertificateApdu); i++) {
        getCardCertificateApdu[i] = random(0, 256);
    }

    /* Debug print APDU */
    Serial.print(F("APDU to send: "));
    for (int i = 0; i < sizeof(getCardCertificateApdu); i++) {
        if (getCardCertificateApdu[i] < 16) Serial.print(F("0"));
        Serial.print(getCardCertificateApdu[i], HEX);
        Serial.print(F(" "));
    }
    Serial.println();

    Serial.println(F("Sending getCardCertificate APDU..."));

    /* Send APDU */
    if (driver.sendAPDU(getCardCertificateApdu, sizeof(getCardCertificateApdu), response, responseLength)) {
        Serial.println(F("APDU exchange successful!"));
        ret = true;
    } else {
        Serial.println(F("APDU getCardCertificate failed."));
    }

    return ret;
}

/* Establish secure channel using ECC keypair exchange */
bool CryptnoxWallet::openSecureChannel() {
    bool ret = false;

    /* Keys allocated on stack to save global RAM */
    uint8_t privateKey[32];
    uint8_t publicKey[64];

    /* ECC setup and random generation */
    randomSeed(analogRead(0));
    uECC_set_rng(&uECC_RNG);
    const uECC_Curve_t *curve = uECC_secp256r1();

    /* Generate keypair */
    bool eccSuccess = uECC_make_key(publicKey, privateKey, curve);

    /* Abort if ECC fails */
    if (!eccSuccess) {
        Serial.println(F("ECC key generation failed."));
        return false;
    }

    /* APDU header for OPEN SECURE CHANNEL */
    uint8_t opcApduHeader[] = {
        0x80,  /* CLA */
        0x10,  /* INS : OPEN SECURE CHANNEL */
        0xFF,  /* P1 : pairing slot index */
        0x00,  /* P2 */
        0x41,  /* Lc : 1 format byte + 64 public key bytes */
        0x04   /* ECC uncompressed public key format */
    };

    /* Construct final APDU */
    uint8_t fullApdu[sizeof(opcApduHeader) + sizeof(publicKey)];
    memcpy(fullApdu, opcApduHeader, sizeof(opcApduHeader));
    memcpy(fullApdu + sizeof(opcApduHeader), publicKey, sizeof(publicKey));

    /* Response buffer */
    uint8_t response[255];
    uint8_t responseLength = sizeof(response);

    /* Debug print APDU */
    Serial.print(F("OPC APDU to send: "));
    for (uint8_t i = 0; i < sizeof(fullApdu); i++) {
        if (fullApdu[i] < 16) Serial.print(F("0"));
        Serial.print(fullApdu[i], HEX);
        Serial.print(F(" "));
    }
    Serial.println();

    Serial.println(F("Sending OPEN SECURE CHANNEL APDU..."));

    /* Send OPC request */
    if (driver.sendAPDU(fullApdu, sizeof(fullApdu), response, responseLength)) {
        Serial.println(F("OPC APDU exchange successful!"));
        ret = true;
    } else {
        Serial.println(F("OPC APDU exchange failed."));
    }

    return ret;
}

/* RNG callback for micro-ecc library */
int CryptnoxWallet::uECC_RNG(uint8_t *dest, unsigned size) {
    for (unsigned i = 0; i < size; i++) {
        dest[i] = random(0, 256);
    }
    return 1;
}
