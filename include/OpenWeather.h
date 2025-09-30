#pragma once
#include <Arduino.h>
#include <HardwareSerial.h>
#include "Location.h"

// TODO: Move this to a secure configuration file or environment variable
#define OPENWEATHER_API_KEY "b17b25fe1a3d45d35c650828fce5ca2d"

class OpenWeather {
public:
    OpenWeather();
    bool updateWeather(const LocationData& location);
    String getWeatherIcon();
    float getTemperature();
    
private:
    // API key is defined as OPENWEATHER_API_KEY
    String _weatherIcon;
    float _temperature;
    
    bool parseWeatherData(String& response);
};