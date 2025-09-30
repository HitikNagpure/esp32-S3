#ifndef DHT_H
#define DHT_H

#include <Arduino.h>
#include <DHT.h>
#include <DHT_U.h>

class DHTManager {
public:
    DHTManager(uint8_t pin = 4);  // Default to pin 4
    bool begin();
    bool readSensor();
    float getTemperature();
    float getHumidity();
    String getTemperatureString();
    String getHumidityString();

private:
    DHT dht;
    float temperature;
    float humidity;
    bool initialized;
    static const uint8_t DHT_TYPE = DHT22;
};

#endif // DHT_H
