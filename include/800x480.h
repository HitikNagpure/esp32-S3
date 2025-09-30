#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include "DisplayManager.h"

class FullScreenManager {
public:
    static void downloadFullScreenBMP();
    static void displayFullScreen();
    
private:
    static const char* FULLSCREEN_BMP_URL;
    static void drawBMPFromFile(const char* filename);
};
