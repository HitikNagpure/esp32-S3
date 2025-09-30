#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include "DisplayManager.h"

class CalendarManager {
public:
    static void downloadCalendarBMP();
    static void displayCalendar();
    
private:
    static const char* CALENDAR_BMP_URL;
    static void drawBMPFromFile(const char* filename);
};
