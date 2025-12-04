// PN532Base.h
#ifndef PN532BASE_H
#define PN532BASE_H

#include <Arduino.h>

class PN532Base {
public:
    virtual bool begin() = 0;
    virtual bool readUID(uint8_t* uidBuffer, uint8_t &uidLength) = 0;
    virtual bool getFirmwareVersion(uint32_t &version) = 0;

    virtual ~PN532Base() {}
};

#endif
