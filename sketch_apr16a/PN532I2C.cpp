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
