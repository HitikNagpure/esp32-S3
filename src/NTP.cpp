#include "NTP.h"
#include <Arduino.h>

NTPClient::NTPClient() : timeReceived(false) {}

void NTPClient::begin(const char* ntpServer, long gmtOffset, int daylightOffset) {
    _ntpServer = ntpServer;
    _gmtOffset = gmtOffset;
    _daylightOffset = daylightOffset;
    
    // Configure multiple NTP servers for better reliability
    configTime(_gmtOffset, _daylightOffset, 
              "pool.ntp.org",          // Primary
              "time.google.com",       // Backup 1
              "time.windows.com");     // Backup 2
              
    Serial.println("⏰ Configured NTP servers:");
    Serial.println(" - pool.ntp.org");
    Serial.println(" - time.google.com");
    Serial.println(" - time.windows.com");
}

bool NTPClient::update() {
    const int maxRetries = 3;
    int retryCount = 0;
    
    while (retryCount < maxRetries) {
        struct tm timeinfo;
        Serial.printf("⏰ Attempting to sync time (attempt %d/%d)...\n", retryCount + 1, maxRetries);
        
        if(getLocalTime(&timeinfo)) {
            char strftime_buf[64];
            strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
            Serial.print("✅ Time synchronized: ");
            Serial.println(strftime_buf);
            timeReceived = true;
            return true;
        }
        
        Serial.printf("⚠️ Failed to obtain time (attempt %d/%d)\n", retryCount + 1, maxRetries);
        retryCount++;
        delay(2000); // Wait 2 seconds before retry
        
        // Force NTP resync on retry
        configTime(_gmtOffset, _daylightOffset, 
                  "pool.ntp.org",
                  "time.google.com",
                  "time.windows.com");
    }
    
    Serial.println("❌ Failed to obtain time after all retries");
    return false;
}

String NTPClient::getTimeString() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return "00:00";
    
    char buffer[6];
    strftime(buffer, sizeof(buffer), "%H:%M", &timeinfo);
    return String(buffer);
}

String NTPClient::getDateString() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return "00-00-0000";
    
    char buffer[11];
    strftime(buffer, sizeof(buffer), "%d-%m-%Y", &timeinfo);
    return String(buffer);
}

String NTPClient::getDayString() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return "Unknown";
    
    char buffer[10];
    strftime(buffer, sizeof(buffer), "%A", &timeinfo);
    return String(buffer);
}

bool NTPClient::isTimeValid() {
    return timeReceived;
}