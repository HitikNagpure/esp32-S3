#include "Location.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

LocationManager::LocationManager() : _locationValid(false) {
    _currentLocation.city = "Unknown";
    _currentLocation.country = "Unknown";
    _currentLocation.region = "Unknown";
    _currentLocation.latitude = 0.0;
    _currentLocation.longitude = 0.0;
}

bool LocationManager::updateLocation() {
    const int maxRetries = 3;
    int retryCount = 0;
    String publicIP;
    HTTPClient http;
    
    // Get public IP with retries
    while (retryCount < maxRetries) {
        http.begin("http://api.ipify.org");
        int httpCode = http.GET();
        
        if (httpCode == HTTP_CODE_OK) {
            publicIP = http.getString();
            http.end();
            Serial.print("📍 Public IP: ");
            Serial.println(publicIP);
            break;  // Success, continue with location lookup
        }
        
        Serial.printf("⚠️ Failed to get public IP (attempt %d/%d)\n", retryCount + 1, maxRetries);
        Serial.printf("HTTP Code: %d\n", httpCode);
        http.end();
        retryCount++;
        
        if (retryCount < maxRetries) {
            delay(1000);  // Wait before retry
            continue;
        }
        
        Serial.println("❌ Failed to get public IP after all retries");
        return false;
    }
    
    // Reset retry counter for location lookup
    retryCount = 0;
    
    // Get location from IP with retries
    while (retryCount < maxRetries) {
        String geoUrl = "http://ipinfo.io/" + publicIP + "/json";
        http.begin(geoUrl);
        int httpCode = http.GET();
        
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println("Location API Response:");
            Serial.println(payload);
            http.end();
            
            if (!parseIPGeolocation(payload)) {
                Serial.println("⚠️ Failed to parse location data");
                return false;
            }
            
            _locationValid = true;
            return true;
        }
        
        Serial.printf("⚠️ Location API request failed (attempt %d/%d)\n", retryCount + 1, maxRetries);
        Serial.printf("HTTP Code: %d\n", httpCode);
        http.end();
        retryCount++;
        
        if (retryCount < maxRetries) {
            delay(1000);  // Wait before retry
            continue;
        }
        
        Serial.println("❌ Failed to get location after all retries");
        return false;
    }
    
    return false;
}

bool LocationManager::parseIPGeolocation(String& response) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
        Serial.println("⚠️ JSON parsing failed");
        Serial.println(error.c_str());
        return false;
    }
    
    // Check for error response
    if (doc.containsKey("error")) {
        Serial.println("⚠️ API Error: " + doc["error"].as<String>());
        return false;
    }
    
    if (!doc.containsKey("city") || !doc.containsKey("country")) {
        Serial.println("⚠️ Missing required location fields in response");
        return false;
    }
    
    _currentLocation.city = doc["city"].as<String>();
    _currentLocation.country = doc["country"].as<String>();
    _currentLocation.region = doc["region"].as<String>();
    
    // Parse location string "lat,lon" into separate values
    if (doc.containsKey("loc")) {
        String loc = doc["loc"].as<String>();
        int commaIndex = loc.indexOf(',');
        if (commaIndex > 0) {
            _currentLocation.latitude = loc.substring(0, commaIndex).toFloat();
            _currentLocation.longitude = loc.substring(commaIndex + 1).toFloat();
        }
    }
    
    return true;
}

String LocationManager::getLocationString() {
    if (!_locationValid) return "Location Unknown";
    return _currentLocation.city + ", " + _currentLocation.country;
}

LocationData LocationManager::getCurrentLocation() {
    return _currentLocation;
}