// PN532I2C.h
#ifndef PN532I2C_H
#define PN532I2C_H

#include "PN532Base.h"
#include <Wire.h>
#include <Adafruit_PN532.h>

class PN532I2C : public PN532Base {
public:
    PN532I2C(uint8_t sdaPin, uint8_t sclPin);
    bool begin() override;
    bool readUID(uint8_t* uidBuffer, uint8_t &uidLength) override;
    bool getFirmwareVersion(uint32_t &version) override; // version ajoutée

private:
    Adafruit_PN532 nfc;
};

#endif
