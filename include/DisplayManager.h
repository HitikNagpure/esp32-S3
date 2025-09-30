#pragma once
#include <GxEPD2_3C.h>
#include <SPI.h>
#include <Fonts/FreeMonoBold9pt7b.h>

// Function declarations
void performFullRefresh();
void checkAndRefresh();
void updateStatusBar(bool refreshDisplay);  // Remove default argument from declaration
void updateTimeDisplay();
void showDashboard();

// Refresh intervals (in milliseconds)
const unsigned long TIME_REFRESH_INTERVAL = 60000;   // 1 minute
const unsigned long FULL_REFRESH_INTERVAL = 3600000; // 1 hour

// External declarations for refresh tracking
extern unsigned long lastFullRefresh;
extern unsigned long lastTimeRefresh;

// ---- E-Paper Pins per user specification ----
// #define RST_PIN   25
// #define DC_PIN    26
// #define CS_PIN    5
// #define BUSY_PIN  27
// #define PWR_PIN   33
// #define MOSI_PIN  23
// #define SCK_PIN   18
const int PIN_CS   = 5;     // Chip Select (GPIO5)
const int PIN_DC   = 26;    // Data/Command (GPIO26)
const int PIN_RST  = 25;    // Reset (GPIO25)
const int PIN_BUSY = 27;    // Busy (GPIO27)
const int PIN_CLK  = 18;    // SPI Clock (SCK)
const int PIN_MOSI = 23;    // SPI MOSI (DIN)
const int POWER_EN_PIN = 33; // Power Enable (GPIO33)

// ---- Display type (Waveshare 7.5" 3-color) ----
// Default to Z90 (V2). Define PANEL_VARIANT_Z08 in build flags to target older Z08.
#ifdef PANEL_VARIANT_Z08
using EpdPanel = GxEPD2_750c_Z08;  // 800x480, 3-color, older variant
#else
using EpdPanel = GxEPD2_750c_Z90;  // 800x480, 3-color, Waveshare V2
#endif
extern GxEPD2_3C<EpdPanel, 16> display;

void setupPowerEnable();
void initDisplay();
void showWelcomeMessage();
// High-level views for mode switching
void showDashboard(); // draws main dashboard under status bar

// Constants for display regions
const int STATUS_BAR_HEIGHT = 60;
const int MAIN_CONTENT_Y = STATUS_BAR_HEIGHT;
const int MAIN_CONTENT_HEIGHT = 420;  // 480 - 60

