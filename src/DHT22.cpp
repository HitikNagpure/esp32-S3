#include "DHT22.h"
#include "DHT.h"

DHT dht(DHT22Manager::DHT_PIN, DHT22);

float DHT22Manager::temperature = 0.0;
float DHT22Manager::humidity = 0.0;
unsigned long DHT22Manager::lastReadTime = 0;

bool DHT22Manager::begin() {
    dht.begin();
    delay(2000);  // DHT22 requires 2s for initial reading
    return update();
}

bool DHT22Manager::update() {
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime >= READ_INTERVAL) {
        float newTemp = dht.readTemperature();
        float newHumid = dht.readHumidity();
        
        if (!isnan(newTemp) && !isnan(newHumid)) {
            temperature = newTemp;
            humidity = newHumid;
            lastReadTime = currentTime;
            return true;
        }
    }
    return false;
}

float DHT22Manager::getTemperature() {
    update();
    return temperature;
}

float DHT22Manager::getHumidity() {
    update();
    return humidity;
}
