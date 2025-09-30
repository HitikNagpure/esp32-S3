#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include "DisplayManager.h"

class ContentManager {
public:
    static void downloadContentBMP();
    static void displayContent();
    
private:
    static const char* CONTENT_BMP_URL;
    static void drawBMPFromFile(const char* filename);
};
