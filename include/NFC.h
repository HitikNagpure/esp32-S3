#ifndef NFC_H
#define NFC_H

#include <Wire.h>
#include <Adafruit_PN532.h>

class NFCManager {
public:
    NFCManager(uint8_t sda = 21, uint8_t scl = 22);
    bool begin();
    bool writeURLOnce(const char* url, uint32_t maxAttempts = 3);  // Write URL and verify it
    bool verifyURL(const char* written_url);  // Verify if URL matches
    bool readTag(String& url);
    bool isTagPresent(uint32_t timeout = 1000);  // Timeout in milliseconds
    
private:
    static const int I2C_SDA = 21;
    static const int I2C_SCL = 22;
    static const int PN532_IRQ = -1;
    static const int PN532_RESET = -1;
    
    Adafruit_PN532 nfc;
    bool initialized;
    
    bool writePages(uint8_t startPage, const uint8_t *data, uint16_t length);
    bool readPages(uint8_t startPage, uint8_t numPages, uint8_t *outBuffer);
    void printHex(const uint8_t *data, uint32_t numBytes);
};

#endif // NFC_H
