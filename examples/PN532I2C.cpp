// PN532I2C.cpp
#include "PN532I2C.h"
#include <Arduino.h>

/**
 * @brief Construct a new PN532I2C object.
 * 
 * @param sdaPin Arduino pin used for I2C SDA.
 * @param sclPin Arduino pin used for I2C SCL.
 */
PN532I2C::PN532I2C(uint8_t sdaPin, uint8_t sclPin)
    : nfc(sdaPin, sclPin) {}

/**
 * @brief Initialize the PN532 module over I2C.
 * 
 * Starts the module, reads firmware version, and configures it for normal operation.
 * Prints status messages to Serial.
 * 
 * @return true if the PN532 module was successfully initialized, false otherwise.
 */
bool PN532I2C::begin() {
    bool ret = false;
    uint32_t versiondata;

    nfc.begin();
    if (!getFirmwareVersion(versiondata)) {
        Serial.println("PN532 I2C not found!");
    } else {
        uint8_t ic        = (versiondata >> 24) & 0xFF;
        uint8_t verMajor  = (versiondata >> 16) & 0xFF;
        uint8_t verMinor  = (versiondata >>  8) & 0xFF;
        uint8_t flags     =  versiondata        & 0xFF;

        Serial.println("PN532 I2C detected");
        Serial.print(" ├─ Raw firmware: 0x");
        Serial.println(versiondata, HEX);

        Serial.print(" ├─ IC Chip:       ");
        Serial.println(ic == 0x32 ? "PN532" : "Unknown");

        Serial.print(" ├─ Firmware:      ");
        Serial.print(verMajor);
        Serial.print(".");
        Serial.println(verMinor);

        Serial.print(" └─ Features:      ");

        bool first = true;
        if (flags & 0x01) { Serial.print("MIFARE"); first = false; }
        if (flags & 0x02) { if (!first) Serial.print(" + "); Serial.print("ISO-DEP"); first = false; }
        if (flags & 0x04) { if (!first) Serial.print(" + "); Serial.print("FeliCa");  first = false; }

        if (first) {
            Serial.print("Unknown");
        }

        Serial.print("  (0x");
        Serial.print(flags, HEX);
        Serial.println(")");

        nfc.SAMConfig();
        ret = true;
    }
    
    return ret;
}

/**
 * @brief Read the UID of a detected NFC card.
 * 
 * @param uidBuffer Pointer to a buffer to store the UID.
 * @param uidLength Reference to a variable to store the UID length.
 * @return true if a card was detected and UID read successfully, false otherwise.
 */
bool PN532I2C::readUID(uint8_t* uidBuffer, uint8_t &uidLength) {
    return nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uidBuffer, &uidLength);
}

/**
 * @brief Get the firmware version of the PN532 module.
 * 
 * @param version Reference to a variable that will hold the firmware version as a 32-bit value.
 * @return true if the firmware version was successfully retrieved, false otherwise.
 */
bool PN532I2C::getFirmwareVersion(uint32_t &version) {
    version = nfc.getFirmwareVersion();
    return (version != 0);
}

/**
 * @brief Send an APDU command to an ISO14443-4 (Type 4) NFC tag.
 * 
 * @param apdu Pointer to the APDU buffer to send.
 * @param apduLength Length of the APDU buffer.
 * @param response Pointer to a buffer to store the response APDU.
 * @param responseLength Reference to a variable that will contain the response length.
 * @return true if the exchange succeeded, false otherwise.
 */
bool PN532I2C::sendAPDU(const uint8_t* apdu, uint8_t apduLength,
                        uint8_t* response, uint8_t &responseLength)
{
    bool success = nfc.inDataExchange(
        (uint8_t*)apdu,
        apduLength,
        response,
        &responseLength
    );

    if (!success) {
        Serial.println("APDU exchange failed!");
        return false;
    }

    Serial.print("APDU response (");
    Serial.print(responseLength);
    Serial.println(" bytes):");

    for (uint8_t i = 0; i < responseLength; i++) {
        Serial.print("0x");
        Serial.print(response[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    return true;
}

/**
 * @brief Detect an ISO14443-4 / ISO-DEP card and activate it.
 * 
 * This function wraps the Adafruit PN532 `inListPassiveTarget()` method.
 * It prepares the PN532 to communicate with ISO-DEP (Type 4) cards
 * so that APDUs can be exchanged.
 * 
 * @return true if a card was detected and activated successfully, false otherwise.
 */
bool PN532I2C::inListPassiveTarget() {
    return nfc.inListPassiveTarget();
}
