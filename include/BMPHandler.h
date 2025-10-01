#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "DisplayManager.h"

class BMPHandler {
public:
    static bool downloadAndDisplayBMP(const String& url, int16_t x, int16_t y, uint16_t width, uint16_t height) {
        HTTPClient http;
        bool success = false;
        
        Serial.println("📥 Downloading BMP from: " + url);
        http.begin(url);
        
        try {
            int httpCode = http.GET();
            Serial.printf("HTTP Response code: %d\n", httpCode);

            if (httpCode == HTTP_CODE_OK) {
                int contentLength = http.getSize();
                Serial.printf("Content length: %d bytes\n", contentLength);

                if (contentLength > 0) {
                    WiFiClient* stream = http.getStreamPtr();
                    if (stream) {
                        success = processBMPStream(stream, x, y, width, height);
                    } else {
                        Serial.println("❌ Failed to get stream pointer");
                    }
                } else {
                    Serial.println("❌ Invalid content length");
                }
            } else {
                Serial.println("⚠️ HTTP error: " + String(httpCode));
            }
        } catch (const std::exception& e) {
            Serial.printf("❌ Exception: %s\n", e.what());
        }
        
        http.end();
        return success;
    }

private:
    static bool processBMPStream(WiFiClient* stream, int16_t x, int16_t y, uint16_t width, uint16_t height) {
        // Read and validate BMP header (54 bytes)
        uint8_t header[54] = {0};
        size_t headerBytesRead = stream->readBytes(header, 54);
        
        if (headerBytesRead != 54 || header[0] != 'B' || header[1] != 'M') {
            Serial.println("❌ Invalid BMP header");
            return false;
        }

        // Read image data
        display.fillScreen(GxEPD_WHITE);
        display.setPartialWindow(x, y, width, height);
        
        uint8_t buffer[800] = {0};  // One row at a time
        bool dataError = false;
        
        for (int row = 0; row < height && !dataError; row++) {
            size_t bytesRead = stream->readBytes(buffer, width);
            if (bytesRead == width) {
                for (int col = 0; col < width; col++) {
                    display.drawPixel(x + col, y + row, 
                        buffer[col] < 128 ? GxEPD_BLACK : GxEPD_WHITE);
                }
            } else {
                Serial.printf("❌ Data read error at row %d: got %d bytes\n", row, bytesRead);
                dataError = true;
            }
        }
        
        if (!dataError) {
            display.display(false);
            Serial.println("✅ BMP processed and displayed successfully");
            return true;
        }
        
        return false;
    }
};