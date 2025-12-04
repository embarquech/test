#include <Arduino.h>
#include "PN532I2C.h"

#define SDA_PIN A4 /**< I2C SDA pin */
#define SCL_PIN A5 /**< I2C SCL pin */

/** 
 * @brief Pointer to the PN532 NFC/RFID interface.
 * 
 * Uses the abstract base class PN532Base to allow swapping interfaces easily.
 */
PN532Base* nfc = new PN532I2C(SDA_PIN, SCL_PIN);

/**
 * @brief Arduino setup function.
 * 
 * Initializes serial communication, starts the PN532 module,
 * and reads its firmware version.
 */
void setup() {
    Serial.begin(115200);

    /** 
     * @brief Initialize the PN532 module.
     * 
     * If the module is not detected, the program halts in an infinite loop.
     */
    if (!nfc->begin()) {
        Serial.println("Failed to start PN532!");
        while (1);
    }

    /** 
     * @brief Read and print the PN532 firmware version.
     */
    uint32_t fw;
    if (nfc->getFirmwareVersion(fw)) {
        Serial.print("PN532 Firmware: 0x");
        Serial.println(fw, HEX);
        Serial.print("Version: ");
        Serial.print((fw >> 16) & 0xFF); // IC version
        Serial.print(".");
        Serial.print((fw >> 8) & 0xFF);  // version rev
        Serial.print(".");
        Serial.println(fw & 0xFF);       // revision
    }
}

/**
 * @brief Arduino loop function.
 * 
 * Continuously checks for NFC cards, reads their UID, and prints it to Serial.
 */
void loop() {
    uint8_t uid[7];      /**< Buffer to store UID */
    uint8_t uidLength;   /**< Length of UID read */

    /** 
     * @brief Check if an NFC card is present.
     * 
     * If a card is detected, print its UID in hexadecimal format.
     */
    if (nfc->readUID(uid, uidLength)) {
        Serial.print("UID: ");
        for (uint8_t i = 0; i < uidLength; i++) {
            Serial.print(" 0x"); Serial.print(uid[i], HEX);
        }
        Serial.println();
    }

    delay(1000); /**< Wait 1 second between reads */
}
