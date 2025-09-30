#include "DisplayManager.h"
#include <Arduino.h>
#include "NTP.h"
#include "Location.h"
#include "OpenWeather.h"
#include "DHT.h"
#include <WiFi.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <SPIFFS.h>

// External reference to refresh interval defined in main.cpp
extern const unsigned long FULL_REFRESH_INTERVAL;

#ifdef PANEL_VARIANT_Z08
GxEPD2_3C<GxEPD2_750c_Z08, 16> display(GxEPD2_750c_Z08(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY));
#else
GxEPD2_3C<GxEPD2_750c_Z90, 16> display(GxEPD2_750c_Z90(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY));
#endif

// External references to shared objects
extern NTPClient ntpClient;
extern LocationManager locationManager;
extern OpenWeather weather;
extern DHTManager dhtManager;

// Define refresh tracking variables
unsigned long lastFullRefresh = 0;
unsigned long lastTimeRefresh = 0;

void setupPowerEnable() {
  if (POWER_EN_PIN >= 0) {
    Serial.println("setupPowerEnable(): enabling panel power");
    pinMode(POWER_EN_PIN, OUTPUT);
    digitalWrite(POWER_EN_PIN, HIGH);
    delay(100);  // Allow power to stabilize
  Serial.println("E-Paper power enabled");
  }
}

void initDisplay() {
  Serial.println("initDisplay(): configuring control pins");

  // Configure control pins for a clean hardware reset
  pinMode(PIN_CS, OUTPUT);
  pinMode(PIN_DC, OUTPUT);
  pinMode(PIN_RST, OUTPUT);
  pinMode(PIN_BUSY, INPUT);
  digitalWrite(PIN_CS, HIGH);
  digitalWrite(PIN_DC, HIGH);
  digitalWrite(PIN_RST, HIGH);
  delay(10);

    Serial.println("🔌 initDisplay(): begin");

    // Prepare control pins and do a hard reset prior to library init
    pinMode(PIN_CS, OUTPUT);   digitalWrite(PIN_CS, HIGH);
    pinMode(PIN_DC, OUTPUT);   digitalWrite(PIN_DC, HIGH);
    pinMode(PIN_RST, OUTPUT);  digitalWrite(PIN_RST, HIGH);
    pinMode(PIN_BUSY, INPUT);

  Serial.printf("BUSY (pre-reset): %d\n", digitalRead(PIN_BUSY));
  // Hard reset pulse (some Waveshare boards need a longer reset)
  digitalWrite(PIN_RST, LOW);  delay(200);
  digitalWrite(PIN_RST, HIGH); delay(400);
    Serial.printf("BUSY (post-reset): %d\n", digitalRead(PIN_BUSY));

  // Initialize SPI for ESP32 DevKit V1 with standard SPI pins
  Serial.println("initDisplay(): starting SPI");
  SPI.begin(PIN_CLK, -1, PIN_MOSI, PIN_CS);

  // Initialize display (may block if BUSY is wrong). Set serial diag for visibility.
  Serial.printf("initDisplay(): calling display.init() with pins CS=%d, DC=%d, RST=%d, BUSY=%d, CLK=%d, MOSI=%d\n",
                PIN_CS, PIN_DC, PIN_RST, PIN_BUSY, PIN_CLK, PIN_MOSI);
#ifdef PANEL_VARIANT_Z08
  Serial.println("Panel variant: GxEPD2_750c_Z08 (older V1)");
#else
  Serial.println("Panel variant: GxEPD2_750c_Z90 (V2)");
#endif
    display.init(115200, true, 50, false);
  Serial.println("initDisplay(): display.init() returned");
  delay(100);  // Give display time to stabilize

  // Additional init parameters
  display.setRotation(0);
  display.setFullWindow();

  // Perform initial clean
    display.firstPage();
    do {
      display.fillScreen(GxEPD_WHITE);
      // Bold test frame
      display.drawRect(5, 5, display.width() - 10, display.height() - 10, GxEPD_BLACK);
      display.drawRect(8, 8, display.width() - 16, display.height() - 16, GxEPD_RED);
    } while (display.nextPage());

  Serial.println("E-ink display initialized");
}

// void showWelcomeMessage() {
//   display.firstPage();
//   do {
//     display.fillScreen(GxEPD_WHITE);

//     // ---- Line 1: "Welcome" ----
//     display.setFont(&FreeSerifBoldItalic24pt7b);
//     display.setTextColor(GxEPD_BLACK);

//     const char* line1 = "Welcome";
//     int16_t x1, y1;
//     uint16_t w1, h1;
//     display.getTextBounds(line1, 0, 0, &x1, &y1, &w1, &h1);

//     int16_t cursorX1 = (display.width() - w1) / 2;
//     int16_t cursorY1 = (display.height() / 2) - 20; // shift up a little

//     display.setCursor(cursorX1, cursorY1);
//     display.print(line1);

//     // ---- Line 2: "Thumbstack Technologies" ----
//     display.setFont(&FreeSerifBoldItalic18pt7b);
//     display.setTextColor(GxEPD_RED);

//     const char* line2 = "Thumbstack Technologies";
//     int16_t x2, y2;
//     uint16_t w2, h2;
//     display.getTextBounds(line2, 0, 0, &x2, &y2, &w2, &h2);

//     int16_t cursorX2 = (display.width() - w2) / 2;
//     int16_t cursorY2 = cursorY1 + h1 + 40; // place below first line

//     display.setCursor(cursorX2, cursorY2);
//     display.print(line2);

//   } while (display.nextPage());
// }


// Store welcome strings in PROGMEM to save RAM
const char WELCOME_LINE1[] PROGMEM = "Welcome";
const char WELCOME_LINE2[] PROGMEM = "Thumbstack Technologies";
const char WELCOME_LINE3[] PROGMEM = "Initializing...";

void showWelcomeMessage() {
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    
    // Draw a decorative border
    display.drawRect(10, 10, display.width() - 20, display.height() - 20, GxEPD_BLACK);
    display.drawRect(15, 15, display.width() - 30, display.height() - 30, GxEPD_RED);

    // ---- Line 1: "Welcome" ----
    display.setFont(&FreeMonoBold12pt7b);  // Larger font for welcome
    display.setTextColor(GxEPD_BLACK);

    int16_t x1, y1;
    uint16_t w1, h1;
    display.getTextBounds(FPSTR(WELCOME_LINE1), 0, 0, &x1, &y1, &w1, &h1);
    int16_t cursorX1 = (display.width() - w1) / 2;
    int16_t cursorY1 = (display.height() / 3);  // Position at 1/3 from top

    display.setCursor(cursorX1, cursorY1);
    display.print(FPSTR(WELCOME_LINE1));

    // ---- Line 2: Company name ----
    display.setTextColor(GxEPD_RED);
    display.setFont(&FreeMonoBold12pt7b);

    int16_t x2, y2;
    uint16_t w2, h2;
    display.getTextBounds(FPSTR(WELCOME_LINE2), 0, 0, &x2, &y2, &w2, &h2);
    int16_t cursorX2 = (display.width() - w2) / 2;
    int16_t cursorY2 = cursorY1 + h1 + 40;  // Add more spacing between lines

    display.setCursor(cursorX2, cursorY2);
    display.print(FPSTR(WELCOME_LINE2));

    // ---- Line 3: Initializing... ----
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeSans9pt7b);  // Smaller font for status

    int16_t x3, y3;
    uint16_t w3, h3;
    display.getTextBounds(FPSTR(WELCOME_LINE3), 0, 0, &x3, &y3, &w3, &h3);
    int16_t cursorX3 = (display.width() - w3) / 2;
    int16_t cursorY3 = display.height() - 50;  // Position near bottom

    display.setCursor(cursorX3, cursorY3);
    display.print(FPSTR(WELCOME_LINE3));

  } while (display.nextPage());

  // ---- Print to Serial too ----
  Serial.println(F("\n=== Boot Greetings ==="));
  Serial.println(FPSTR(WELCOME_LINE1));
  Serial.println(FPSTR(WELCOME_LINE2));
  Serial.println(F("======================"));
}

void updateMainContent(const uint8_t* bitmap, int16_t x, int16_t y, int16_t w, int16_t h, bool useFullRefresh) {
  Serial.println("\n=== Updating Main Content Area ===");
  Serial.printf("Position: (%d, %d)\n", x, y);
  Serial.printf("Size: %dx%d pixels\n", w, h);
  
  // Ensure coordinates are within main content area
  if (y < STATUS_BAR_HEIGHT) {
    h -= (STATUS_BAR_HEIGHT - y);
    y = STATUS_BAR_HEIGHT;
  Serial.println("Adjusted for status bar area");
  Serial.printf("New position: (%d, %d)\n", x, y);
  Serial.printf("New height: %d pixels\n", h);
  }
  
  if (h <= 0) {
  Serial.println("❌ Invalid height, nothing to draw");
    return;
  }
  
  // Wait for any previous updates to complete
  delay(500);  // Increased delay
  
  // Perform full refresh if requested to reduce ghosting
  if (useFullRefresh) {
    performFullRefresh();
  }
  
  // Set window to only update main content area
  display.setFullWindow(); // Using full window for better refresh
  Serial.println("Set full window for content update");
  
  // Update the display with timeout handling
  unsigned long startTime = millis();
  const unsigned long timeout = 10000; // 10 second timeout
  
  display.firstPage();
  int pageCount = 0;
  
  do {
    pageCount++;
  Serial.printf("Drawing page %d...\n", pageCount);
    
    display.fillScreen(GxEPD_WHITE);  // Clear the update area
  Serial.println("Cleared screen");
    
    display.drawBitmap(x, y, bitmap, w, h, GxEPD_BLACK);  // Draw the bitmap normally
  Serial.println("Drew bitmap");
    
    // Check for timeout
    if (millis() - startTime > timeout) {
  Serial.println("❌ Display update timeout!");
      return;
    }
    
    delay(100); // Increased delay between iterations
  } while (display.nextPage());
  
  Serial.printf("✅ Main content updated successfully (%d pages)\n", pageCount);
  Serial.println("===============================\n");
  
  delay(1000); // Longer delay to ensure display settles
}

void updateTimeDisplay() {
    String timeStr = ntpClient.getTimeString();
    
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        
        // Draw time in the top-right corner
        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(display.width() - 100, 40);
        display.print(timeStr);
        
        // Redraw the rest of the status bar
        updateStatusBar(false); // false means don't trigger display update
        
    } while (display.nextPage());
    
    lastTimeRefresh = millis();
    Serial.println("Full display refresh with updated time");
}

void updateStatusBar(bool refreshDisplay) {
    String dateStr = ntpClient.getDateString();
    String dayStr = ntpClient.getDayString();
    String locationStr = locationManager.getLocationString();
    float temp = weather.getTemperature();
    
    if (refreshDisplay) {
        display.setFullWindow();
        display.firstPage();
    }
    
    // Draw status bar content
    display.setTextColor(GxEPD_BLACK);
    
    // Draw battery symbol (far right)
    display.fillRect(display.width() - 25, 10, 15, 25, GxEPD_BLACK); // battery body
    display.fillRect(display.width() - 21, 7, 7, 3, GxEPD_BLACK);    // battery tip

    // Draw WiFi symbol between battery and time
    if (WiFi.status() == WL_CONNECTED) {
        int16_t wx = display.width() - 45; // Position between battery and time
        int16_t wy = 22; // Vertical center
        
        // Draw concentric arcs using filled rectangles and circles
        // Outer arc
        display.fillRect(wx - 8, wy - 6, 16, 3, GxEPD_BLACK);
        display.fillRect(wx - 9, wy - 5, 2, 2, GxEPD_BLACK);
        display.fillRect(wx + 7, wy - 5, 2, 2, GxEPD_BLACK);
        
        // Middle arc
        display.fillRect(wx - 5, wy - 2, 10, 2, GxEPD_BLACK);
        display.fillRect(wx - 6, wy - 1, 2, 2, GxEPD_BLACK);
        display.fillRect(wx + 4, wy - 1, 2, 2, GxEPD_BLACK);
        
        // Inner arc
        display.fillRect(wx - 2, wy + 2, 4, 2, GxEPD_BLACK);
        
        // Center dot
        display.fillCircle(wx, wy + 5, 1, GxEPD_BLACK);
    }

    // Draw time (right of WiFi symbol)
    String timeStr = ntpClient.getTimeString();
    display.setFont(&FreeSansBold12pt7b);
    int16_t x1, y1;
    uint16_t w1, h1;
    display.getTextBounds(timeStr.c_str(), 0, 0, &x1, &y1, &w1, &h1);
    display.setCursor(display.width() - w1 - 75, 30); // Moved left to make room for battery and WiFi
    display.print(timeStr);
    
    // Draw date and day (center)
    display.setFont(&FreeSans9pt7b);
    display.setCursor(300, 25);
    display.print(dayStr);
    display.setCursor(300, 45);
    display.print(dateStr);
    
    // Draw location (left)
    display.setCursor(10, 40);
    display.print(locationStr);
    
    // Draw indoor temperature and humidity (between location and date)
    if (dhtManager.readSensor()) {
        display.setCursor(180, 40);  // Position after location
        display.printf("%s | %s", dhtManager.getTemperatureString().c_str(), 
                                  dhtManager.getHumidityString().c_str());
    }
    
    // Draw outdoor temperature (if available)
    if (temp != 0.0) {
        display.setCursor(display.width() - 200, 40);
        display.printf("%.1f°C", temp);
    }
    
    if (refreshDisplay) {
        while (display.nextPage());
        Serial.println("Status bar updated with full refresh");
    }
}

void performFullRefresh() {
  Serial.println("\n=== Performing Full Refresh ===");
    display.setFullWindow();
    
    // Single white refresh
    display.fillScreen(GxEPD_WHITE);
    display.display(false);  // false = full update
    delay(2000);  // Give display time to complete
    
    lastFullRefresh = millis();
  Serial.println("Full refresh completed");
}

void checkAndRefresh() {
    unsigned long currentTime = millis();
    
    // Check if it's time for a time update (includes full refresh)
    if (currentTime - lastTimeRefresh >= TIME_REFRESH_INTERVAL) {
        Serial.println("Time update with full refresh triggered");
        updateTimeDisplay();
    }
    // Check if it's time for a periodic full refresh
    else if (currentTime - lastFullRefresh >= FULL_REFRESH_INTERVAL) {
        Serial.println("Periodic full refresh triggered");
        performFullRefresh();
        showDashboard();
    }
}

// Simple status bar only page
void showDashboard() {
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        
        // Draw just the status bar
        updateStatusBar(false);
        
    } while (display.nextPage());
    
    Serial.println("Status bar page displayed");
}




