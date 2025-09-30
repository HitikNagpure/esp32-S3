#include "OpenWeather.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

OpenWeather::OpenWeather() : _temperature(0.0) {}

bool OpenWeather::updateWeather(const LocationData& location) {
    const int maxRetries = 3;
    int retryCount = 0;
    HTTPClient http;
    
    if (!location.latitude || !location.longitude) {
        Serial.println("⚠️ Invalid location for weather update");
        return false;
    }
    
    String url = "http://api.openweathermap.org/data/2.5/weather?";
    url += "lat=" + String(location.latitude, 6);
    url += "&lon=" + String(location.longitude, 6);
    url += "&appid=" + String(OPENWEATHER_API_KEY);
    url += "&units=metric";  // Use Celsius
    
    while (retryCount < maxRetries) {
        http.begin(url);
        int httpCode = http.GET();
        
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            http.end();
            return parseWeatherData(payload);
        }
        
        Serial.printf("⚠️ Weather API request failed (attempt %d/%d)\n", retryCount + 1, maxRetries);
        Serial.printf("HTTP Code: %d\n", httpCode);
        http.end();
        retryCount++;
        
        if (retryCount < maxRetries) {
            delay(1000);  // Wait before retry
            continue;
        }
        
        Serial.println("❌ Failed to get weather after all retries");
        return false;
    }
    
    return false;
}

bool OpenWeather::parseWeatherData(String& response) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
        Serial.println("⚠️ Weather JSON parsing failed");
        return false;
    }
    
    _temperature = doc["main"]["temp"].as<float>();
    _weatherIcon = doc["weather"][0]["icon"].as<String>();
    
    return true;
}

String OpenWeather::getWeatherIcon() {
    return _weatherIcon;
}

float OpenWeather::getTemperature() {
    return _temperature;
}