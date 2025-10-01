#include "DHT22.h"

// Initialize static members
DHT* DHT22Manager::dht = nullptr;
bool DHT22Manager::initialized = false;
float DHT22Manager::lastTemp = 0.0f;
float DHT22Manager::lastHumidity = 0.0f;
unsigned long DHT22Manager::lastReadTime = 0;

bool DHT22Manager::begin() {
    if (!initialized) {
        dht = new DHT(DHT_PIN, DHT_TYPE);
        dht->begin();
        initialized = true;
        update(); // Initial reading
        Serial.println("DHT22 sensor initialized");
    }
    return initialized;
}

void DHT22Manager::update() {
    if (!initialized) return;
    
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime >= READ_INTERVAL) {
        float newTemp = dht->readTemperature();
        float newHumidity = dht->readHumidity();
        
        if (!isnan(newTemp) && !isnan(newHumidity)) {
            lastTemp = newTemp;
            lastHumidity = newHumidity;
            lastReadTime = currentTime;
        }
    }
}

float DHT22Manager::getTemperature() {
    update();
    return lastTemp;
}

float DHT22Manager::getHumidity() {
    update();
    return lastHumidity;
}

bool DHT22Manager::isAvailable() {
    return initialized && !isnan(lastTemp) && !isnan(lastHumidity);
}

String DHT22Manager::getTemperatureString() {
    if (!isAvailable()) return "T=--.-";
    return "T=" + String(lastTemp, 1);
}

String DHT22Manager::getHumidityString() {
    if (!isAvailable()) return "H=--.-";
    return "H=" + String(lastHumidity, 1);
}