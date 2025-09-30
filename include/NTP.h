#pragma once
#include <Arduino.h>
#include <HardwareSerial.h>
#include <time.h>

class NTPClient {
public:
    NTPClient();
    void begin(const char* ntpServer = "pool.ntp.org", 
               long gmtOffset = 19800,    // Default GMT+5:30 for India
               int daylightOffset = 0);
    bool update();
    String getTimeString();
    String getDateString();
    String getDayString();
    bool isTimeValid();

private:
    bool timeReceived;
    const char* _ntpServer;
    long _gmtOffset;
    int _daylightOffset;
};