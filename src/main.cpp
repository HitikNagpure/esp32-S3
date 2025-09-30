#include <Arduino.h>
#include <HardwareSerial.h>
#include "DisplayManager.h"
#include "calender.h"
#include "800x420.h"
#include "800x480.h"
#include "QRCodeManager.h"
#include "WifiPortal.h"
#include "Location.h"
#include "NTP.h"
#include "OpenWeather.h"
#include "NFC.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

// Constants for refresh intervals
const unsigned long MIN_REFRESH_INTERVAL = 20000;     // Minimum time between any refresh (20s)

// Global instances
LocationManager locationManager;
NTPClient ntpClient;
OpenWeather weather;
NFCManager nfcManager;
DHTManager dhtManager(4);  // DHT22 on pin 4

// No global variables needed for basic display functionality

// No helper functions needed for basic display functionality

void updateDashboardBMP() {
    // Step 1: Update location
    if (!locationManager.updateLocation()) {
        Serial.println("❌ Failed to get location");
        return;
    }

    LocationData loc = locationManager.getCurrentLocation();
    Serial.println("🌍 Location detected: " + locationManager.getLocationString());

    // Step 2: Build dashboard URL
    String dashboardURL = "http://192.168.1.4:5000/dashboard?lat=" + String(loc.latitude, 6) +
                          "&lon=" + String(loc.longitude, 6);

    Serial.println("📥 Downloading BMP from: " + dashboardURL);

    // Step 3: HTTP GET request to fetch BMP
    HTTPClient http;
    http.begin(dashboardURL);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        WiFiClient* stream = http.getStreamPtr();
        // TODO: Read the BMP stream and send to E-Ink display
        Serial.println("✅ Dashboard BMP downloaded successfully!");
    } else {
        Serial.println("⚠️ Failed to download BMP, HTTP code: " + String(httpCode));
    }
    http.end();
}

void updateWakeImageBMP(String imageURL) {
    // Fetch wake-up image (800x420)
    String wakeURL = "http://192.168.1.4:5000/wake_image?url=" + imageURL;
    Serial.println("📥 Downloading wake-up BMP from: " + wakeURL);

    HTTPClient http;
    http.begin(wakeURL);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        WiFiClient* stream = http.getStreamPtr();
        // TODO: Read the BMP stream and send to E-Ink display
        Serial.println("✅ Wake-up BMP downloaded successfully!");
    } else {
        Serial.println("⚠️ Failed to download wake BMP, HTTP code: " + String(httpCode));
    }
    http.end();
}

void setup() {
  Serial.begin(115200);
  delay(200);

  // Initialize DHT22
  if (!dhtManager.begin()) {
    Serial.println("⚠️ DHT22 initialization failed");
  } else {
    Serial.println("✅ DHT22 initialized");
  }

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)) {
      Serial.println("❌ SPIFFS Mount Failed");
      return;
  }
  Serial.println("✅ SPIFFS initialized");
  
  Serial.println("Starting E-ink Display Setup");
  
  // Initialize hardware
  setupPowerEnable();
  delay(100); // Allow power to stabilize
  initDisplay();

  // Step 1: Show boot message
  showWelcomeMessage();
  delay(3000); // Keep welcome message visible for 3 seconds

  // Step 2: WiFi setup and QR code page
  Serial.println("🌐 Attempting to connect to saved WiFi...");
  if (!startWifiPortal()) {
    // If connection failed, show QR code for setup
    Serial.println("⚠️ Failed to connect to saved WiFi, showing QR code for setup...");
    showQRCode("WIFI:T:WPA;S:ESP32-Setup;P:setup1234;;");
    delay(3000); // Show QR code for 3 seconds
    
    // Start portal in AP mode for configuration
    Serial.println("🌐 Starting WiFi setup portal...");
    startWifiPortal(true); // Force portal mode
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("✅ WiFi connected: ");
    Serial.println(WiFi.SSID());

    delay(1000); // stabilize

    // Initialize NFC first after WiFi
    Serial.println("Initializing NFC...");
    if (!nfcManager.begin()) {
      Serial.println("⚠️ NFC initialization failed");
      return;  // Don't proceed if NFC init fails
    }
    Serial.println("✅ NFC initialized");
    
    // Write URL to the first detected tag - 3 attempts only
    const char* url = "http://192.168.3.120:3000/Bcard";
    String current_url;
    Serial.println("Waiting for NFC tag to write URL (3 attempts)...");
    
    // First try to read the current URL
    if (nfcManager.readTag(current_url)) {
      if (current_url == url) {
        Serial.println("✅ Tag already contains the correct URL - proceeding with setup");
      } else {
        // URL is different, try to write the new one
        if (!nfcManager.writeURLOnce(url, 3)) {  // Try 3 times only
          Serial.println("⚠️ Failed to write/verify URL after 3 attempts. System cannot proceed.");
          return;  // Don't proceed if we can't write the tag
        }
      }
    } else {
      // Couldn't read the tag, try to write the new URL
      if (!nfcManager.writeURLOnce(url, 3)) {  // Try 3 times only
        Serial.println("⚠️ Failed to write/verify URL after 3 attempts. System cannot proceed.");
        return;  // Don't proceed if we can't write the tag
      }
    }

    delay(1000); // stabilize

    // Initialize NTP client for IST (GMT+5:30)
    Serial.println("⏰ Initializing NTP...");
    ntpClient.begin("pool.ntp.org", 19800, 0);
    delay(1000);

    if (ntpClient.update()) {
      Serial.println("✅ Time synchronized");
    } else {
      Serial.println("⚠️ Initial time sync failed, will retry in loop");
    }

    // Location + weather once on boot
    Serial.println("📍 Getting location...");
    if (locationManager.updateLocation()) {
      Serial.println("✅ Location obtained");
      Serial.println("🌤️ Getting weather...");
      LocationData loc = locationManager.getCurrentLocation();
      if (weather.updateWeather(loc)) {
        Serial.println("✅ Weather obtained");
      }
    }

    // Show dashboard with content image below status bar (3rd page)
    ContentManager::downloadContentBMP();  // Download the 800x420 image
    ContentManager::displayContent();      // Display status bar and content image
    delay(60000);  // Show for 60 seconds

    // Download and show calendar as the fourth page
    Serial.println("Loading calendar page...");
    CalendarManager::downloadCalendarBMP();
    CalendarManager::displayCalendar();
    delay(60000);  // Show calendar for 60 seconds

    // Download and show full screen image as the fifth page
    Serial.println("Loading full screen page...");
    FullScreenManager::downloadFullScreenBMP();
    FullScreenManager::displayFullScreen();
  } else {
    Serial.println("⚠️ Not connected to WiFi; QR screen will remain visible until connected.");
  }

  Serial.println("Setup complete!");
}

void loop() {
    unsigned long currentMillis = millis();
    
    // Enforce minimum time between any refresh operations
    if (currentMillis - millis() < MIN_REFRESH_INTERVAL) {
        delay(100);
        return;
    }
    
    // Check if it's time for full refresh (every hour)
    checkAndRefresh();  // This function in DisplayManager handles the timing check
    
    // Additional periodic updates if needed
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Hourly refresh cycle starting...");
        
        // Update location first
        if (locationManager.updateLocation()) {
            Serial.println("✅ Location updated");
            
            // Then update weather using new location
            if (WiFi.status() == WL_CONNECTED) {
                LocationData loc = locationManager.getCurrentLocation();
                if (weather.updateWeather(loc)) {
                    Serial.println("✅ Weather updated");
                }
            }
            
            // Finally, refresh the display with new data
            showDashboard();
            lastFullRefresh = currentMillis;
            Serial.println("Location and weather updates completed");
        }
    }
    
    delay(100);  // Small delay for loop responsiveness
}
