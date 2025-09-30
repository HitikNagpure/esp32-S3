#include "NFC.h"

NFCManager::NFCManager(uint8_t sda, uint8_t scl) : 
    nfc(PN532_IRQ, PN532_RESET),
    initialized(false) {
    Wire.begin(sda, scl);
}

void NFCManager::printHex(const uint8_t *data, uint32_t numBytes) {
    for (uint32_t i = 0; i < numBytes; i++) {
        if (data[i] < 0x10) Serial.print("0");
        Serial.print(data[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

bool NFCManager::writePages(uint8_t startPage, const uint8_t *data, uint16_t length) {
    uint16_t pages = length / 4;
    for (uint16_t p = 0; p < pages; p++) {
        uint8_t pageBuf[4];
        memcpy(pageBuf, data + (p * 4), 4);
        if (!nfc.ntag2xx_WritePage(startPage + p, pageBuf)) {
            Serial.print("Failed writing page "); Serial.println(startPage + p);
            return false;
        }
        delay(5);
    }
    return true;
}

bool NFCManager::readPages(uint8_t startPage, uint8_t numPages, uint8_t *outBuffer) {
    const uint8_t maxRetries = 3;
    
    for (uint8_t p = 0; p < numPages; p++) {
        uint8_t pageBuf[4];
        bool pageRead = false;
        
        for (uint8_t retry = 0; retry < maxRetries && !pageRead; retry++) {
            if (retry > 0) {
                delay(50);  // Wait between retries
                Serial.print("Retry "); Serial.print(retry); Serial.print(" for page "); Serial.println(startPage + p);
            }
            
            if (nfc.ntag2xx_ReadPage(startPage + p, pageBuf)) {
                pageRead = true;
            }
        }
        
        if (!pageRead) {
            Serial.print("Failed reading page "); Serial.println(startPage + p);
            return false;
        }
        
        memcpy(outBuffer + (p * 4), pageBuf, 4);
        delay(10);  // Small delay between pages
    }
    return true;
}

bool NFCManager::begin() {
    if (initialized) return true;

    nfc.begin();

    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata) {
        Serial.println("Didn't find PN532 board - check wiring.");
        return false;
    }
    
    Serial.print("Found PN532 with firmware version: 0x"); 
    Serial.println(versiondata, HEX);

    nfc.SAMConfig();
    Serial.println("PN532 configured successfully");
    
    initialized = true;
    return true;
}

bool NFCManager::isTagPresent(uint32_t timeout) {
    uint8_t uid[7];
    uint8_t uidLength;
    return nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, timeout);
}

bool NFCManager::writeURLOnce(const char* url, uint32_t maxAttempts) {
    if (!initialized) {
        if (!begin()) return false;
    }

    uint8_t uid[7];
    uint8_t uidLength;
    uint32_t attempts = 0;

    Serial.println("Place your NFC tag to write...");
    
    for (attempts = 0; attempts < maxAttempts; attempts++) {
        Serial.print("Attempt ");
        Serial.print(attempts + 1);
        Serial.print(" of ");
        Serial.println(maxAttempts);

        // Wait for tag with 2 second timeout
        if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 2000)) {
            Serial.println("No tag detected.");
            if (attempts < maxAttempts - 1) {
                delay(1000);  // Wait a second before next attempt
            }
            continue;
        }

    Serial.print("Tag detected! UID: ");
    printHex(uid, uidLength);

    // First, try to read existing URL
    String current_url;
    if (readTag(current_url)) {
        if (current_url == url) {
            Serial.println("✅ Tag already contains the correct URL!");
            return true;
        }
        Serial.println("Tag contains a different URL, overwriting...");
    }

    // Build an NDEF URI record
    const uint8_t urlLen = strlen(url);
    const uint8_t uriIdCode = 0x00; // No prefix

    uint8_t payloadLen = 1 + urlLen;     // identifier code + url bytes
    uint8_t ndefHeaderLen = 4;
    uint8_t tlvHeaderLen = 2;
    uint16_t totalPayload = tlvHeaderLen + ndefHeaderLen + payloadLen + 1;
    uint16_t paddedTotal = totalPayload;
    if (paddedTotal % 4 != 0) paddedTotal += (4 - (paddedTotal % 4));

    uint8_t buf[paddedTotal];
    memset(buf, 0x00, paddedTotal);

    uint16_t idx = 0;
    buf[idx++] = 0x03;              // TLV tag - NDEF
    buf[idx++] = (uint8_t)(ndefHeaderLen + payloadLen);
    buf[idx++] = 0xD1;              // NDEF header
    buf[idx++] = 0x01;              // Type Length = 1
    buf[idx++] = payloadLen;        // Payload Length
    buf[idx++] = 'U';               // Type 'U' for URI
    buf[idx++] = uriIdCode;         // URI Identifier Code
    
    memcpy(buf + idx, url, urlLen);
    idx += urlLen;
    buf[idx++] = 0xFE;              // Terminator TLV

    if (!writePages(4, buf, paddedTotal)) {
        Serial.println("Failed to write NDEF message, retrying...");
        attempts++;
        delay(1000);
        continue;
    }

    // Add delay after writing before verification
    delay(100);  // Give the tag time to stabilize

    if (!verifyURL(url)) {
        Serial.println("Failed to verify written URL, retrying...");
        attempts++;
        delay(1000);
        continue;
    }

    Serial.println("✅ URL successfully written and verified!");
    return true;
    }

    Serial.println("⚠️ Failed to write URL after maximum attempts");
    return false;
}

bool NFCManager::readTag(String& url) {
    if (!initialized) {
        if (!begin()) return false;
    }

    uint8_t uid[7];
    uint8_t uidLength;

    if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 1000)) {
        return false;
    }

    uint8_t header[16];
    if (!readPages(4, 4, header)) {
        return false;
    }

    if (header[0] != 0x03) {
        return false;
    }

    uint8_t ndefLength = header[1];
    uint8_t totalToRead = ndefLength + 2;
    uint8_t pagesNeeded = (totalToRead + 3) / 4;

    uint8_t data[pagesNeeded * 4];
    if (!readPages(4, pagesNeeded, data)) {
        return false;
    }

    if (data[2] == 0xD1 && data[5] == 'U') {
        uint8_t payloadLength = data[4];
        uint8_t uriIdCode = data[6];
        
        url = "";
        for (uint8_t i = 0; i < payloadLength - 1; i++) {
            url += (char)data[7 + i];
        }
        
        return true;
    }

    return false;
}

bool NFCManager::verifyURL(const char* written_url) {
    String read_url;
    const uint8_t maxVerifyRetries = 3;
    
    for (uint8_t retry = 0; retry < maxVerifyRetries; retry++) {
        if (retry > 0) {
            delay(100);  // Wait between verification attempts
            Serial.print("Verification retry "); Serial.println(retry);
        }
        
        if (readTag(read_url)) {
            if (read_url == written_url) {
                Serial.println("✅ URL verified successfully!");
                return true;
            }
        }
    }
    
    Serial.println("❌ Failed to read tag for verification");

    if (read_url == written_url) {
        Serial.println("✅ URL verified successfully!");
        return true;
    } else {
        Serial.println("❌ URL verification failed!");
        Serial.print("Expected: ");
        Serial.println(written_url);
        Serial.print("Read: ");
        Serial.println(read_url);
        return false;
    }
}

