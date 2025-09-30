#include "DHT.h"
#include <Arduino.h>

DHTManager::DHTManager(uint8_t pin) : dht(pin, DHT_TYPE), initialized(false) {}

bool DHTManager::begin() {
    if (initialized) return true;
    
    dht.begin();
    delay(2000);  // DHT22 needs 2 seconds to initialize
    
    // Try to read the sensor to verify it's working
    if (readSensor()) {
        initialized = true;
        return true;
    }
    return false;
}

bool DHTManager::readSensor() {
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    
    return !isnan(humidity) && !isnan(temperature);
}

float DHTManager::getTemperature() {
    return temperature;
}

float DHTManager::getHumidity() {
    return humidity;
}

String DHTManager::getTemperatureString() {
    return String(temperature, 1) + "°C";
}

String DHTManager::getHumidityString() {
    return String(humidity, 0) + "%";
}