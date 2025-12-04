// PN532I2C.cpp
#include "PN532I2C.h"
#include <Arduino.h>

PN532I2C::PN532I2C(uint8_t sdaPin, uint8_t sclPin)
    : nfc(sdaPin, sclPin) {}

bool PN532I2C::begin() {
    nfc.begin();
    uint32_t versiondata;
    if (!getFirmwareVersion(versiondata)) {
        Serial.println("PN532 I2C not found!");
        return false;
    }
    Serial.print("PN532 I2C detected. Firmware version: 0x");
    Serial.println(versiondata, HEX);

    nfc.SAMConfig();
    return true;
}

bool PN532I2C::readUID(uint8_t* uidBuffer, uint8_t &uidLength) {
    return nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uidBuffer, &uidLength);
}

bool PN532I2C::getFirmwareVersion(uint32_t &version) {
    version = nfc.getFirmwareVersion();
    return (version != 0);
}
