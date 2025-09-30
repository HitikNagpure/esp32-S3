#pragma once
#include <Arduino.h>
#include <HardwareSerial.h>

struct LocationData {
    String city;
    String country;
    String region;
    float latitude;
    float longitude;
};

class LocationManager {
public:
    LocationManager();
    bool updateLocation();
    String getLocationString();
    LocationData getCurrentLocation();
    
private:
    LocationData _currentLocation;
    bool _locationValid;
    
    bool parseIPGeolocation(String& response);
};