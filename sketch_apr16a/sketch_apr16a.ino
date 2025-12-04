#include <Arduino.h>
#include "PN532I2C.h"

#define SDA_PIN A4
#define SCL_PIN A5

PN532Base* nfc = new PN532I2C(SDA_PIN, SCL_PIN);

void setup() {
    Serial.begin(115200);

    if (!nfc->begin()) {
        Serial.println("Impossible de démarrer le PN532 !");
        while (1);
    }

    uint32_t fw;
    if (nfc->getFirmwareVersion(fw)) {
        Serial.print("Firmware PN532: 0x");
        Serial.println(fw, HEX);
        Serial.print("Version: ");
        Serial.print((fw >> 16) & 0xFF); // IC version
        Serial.print(".");
        Serial.print((fw >> 8) & 0xFF);  // version rev
        Serial.print(".");
        Serial.println(fw & 0xFF);       // revision
    }
}

void loop() {
    uint8_t uid[7];
    uint8_t uidLength;
    
    if (nfc->readUID(uid, uidLength)) {
        Serial.print("UID: ");
        for (uint8_t i = 0; i < uidLength; i++) {
            Serial.print(" 0x"); Serial.print(uid[i], HEX);
        }
        Serial.println();
    }
    delay(1000);
}
