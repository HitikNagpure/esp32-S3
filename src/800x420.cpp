#include "800x420.h"
#include "DisplayManager.h"
#include <SPIFFS.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ContentManager::CONTENT_BMP_URL = "http://192.168.1.4:3000/image_800x420.bmp";

// BMP header structure
struct BMPHeader {
    uint16_t signature;
    uint32_t fileSize;
    uint32_t reserved;
    uint32_t dataOffset;
    uint32_t headerSize;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
} __attribute__((packed));

void ContentManager::downloadContentBMP() {
    Serial.println("\n=== Downloading Content Image ===");
    Serial.printf("🔗 URL: %s\n", CONTENT_BMP_URL);
    
    HTTPClient http;
    http.setTimeout(15000);
    
    if (!http.begin(CONTENT_BMP_URL)) {
        Serial.println("❌ Failed to begin HTTP request");
        return;
    }
    
    Serial.println("📡 Sending GET request...");
    int httpCode = http.GET();
    Serial.printf("📡 Response Code: %d\n", httpCode);
    
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("❌ HTTP Error: %d\n", httpCode);
        http.end();
        return;
    }
    
    int contentLength = http.getSize();
    Serial.printf("📄 Content Length: %d bytes (%.1f KB)\n", 
                 contentLength, contentLength/1024.0);
    
    // Remove old file
    if (SPIFFS.exists("/content.bmp")) {
        SPIFFS.remove("/content.bmp");
    }
    
    // Create new file
    File file = SPIFFS.open("/content.bmp", "w");
    if (!file) {
        Serial.println("❌ Failed to create file");
        http.end();
        return;
    }
    
    // Download
    WiFiClient* stream = http.getStreamPtr();
    uint8_t buf[1024];
    size_t totalBytes = 0;
    
    Serial.println("⬇️ Downloading...");
    while (http.connected() && (contentLength <= 0 || totalBytes < contentLength)) {
        size_t available = stream->available();
        if (available) {
            size_t readBytes = stream->readBytes(buf, min(sizeof(buf), available));
            if (readBytes == 0) break;
            
            file.write(buf, readBytes);
            totalBytes += readBytes;
            
            // Progress
            if (contentLength > 0) {
                int progress = (totalBytes * 100) / contentLength;
                if (progress % 25 == 0) {
                    Serial.printf("⬇️ Progress: %d%%\n", progress);
                }
            }
        } else {
            delay(10);
        }
    }
    
    file.close();
    http.end();
    
    Serial.printf("✅ Downloaded: %u bytes\n", totalBytes);
}

void ContentManager::displayContent() {
    Serial.println("\n=== Displaying Content Below Status Bar ===");
    
    // First update the status bar
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        updateStatusBar(false);  // Update status bar without refresh
        
        // Draw content below status bar
        if (SPIFFS.exists("/content.bmp")) {
            drawBMPFromFile("/content.bmp");
        } else {
            display.setTextColor(GxEPD_BLACK);
            display.setCursor(10, STATUS_BAR_HEIGHT + 30);
            display.print("Content image not available");
        }
    } while (display.nextPage());
    
    Serial.println("✅ Content displayed!");
}

void ContentManager::drawBMPFromFile(const char* filename) {
    File file = SPIFFS.open(filename, "r");
    if (!file) {
        Serial.println("❌ Failed to open BMP file");
        return;
    }
    
    BMPHeader header;
    if (file.size() < sizeof(BMPHeader)) {
        Serial.println("❌ Invalid BMP file size");
        file.close();
        return;
    }
    
    file.read((uint8_t*)&header, sizeof(BMPHeader));
    
    if (header.signature != 0x4D42) {
        Serial.printf("❌ Invalid BMP signature: 0x%04X\n", header.signature);
        file.close();
        return;
    }
    
    Serial.printf("🖼️ BMP: %dx%d, %d-bit\n", 
                 header.width, header.height, header.bitsPerPixel);
    
    // Verify image dimensions match the available area
    if (header.width != 800 || header.height != 420) {
        Serial.printf("⚠️ Warning: Content BMP should be 800x420, got %dx%d\n", 
                     header.width, header.height);
    }
    
    // Seek to image data
    file.seek(header.dataOffset);
    
    // Calculate row size (padded to 4 bytes)
    int rowSize = ((header.width * header.bitsPerPixel + 31) / 32) * 4;
    uint8_t* rowBuffer = (uint8_t*)malloc(rowSize);
    
    if (!rowBuffer) {
        Serial.println("❌ Memory allocation failed");
        file.close();
        return;
    }
    
    // Draw BMP (bottom-to-top)
    for (int y = header.height - 1; y >= 0; y--) {
        file.read(rowBuffer, rowSize);
        
        for (int x = 0; x < header.width; x++) {
            if (header.bitsPerPixel == 1) {
                int byteIndex = x / 8;
                int bitIndex = 7 - (x % 8);
                bool pixel = (rowBuffer[byteIndex] >> bitIndex) & 1;
                
                uint16_t color = pixel ? GxEPD_WHITE : GxEPD_BLACK;
                // Draw pixel below status bar
                display.drawPixel(x, STATUS_BAR_HEIGHT + y, color);
            }
        }
    }
    
    free(rowBuffer);
    file.close();
}
