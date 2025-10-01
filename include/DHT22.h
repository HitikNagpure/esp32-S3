#ifndef DHT22_H
#define DHT22_H

#include <Arduino.h>
#include <DHT.h>

#define DHT_PIN 4       // DHT22 connected to GPIO4
#define DHT_TYPE DHT22  // DHT22 (AM2302) sensor type

class DHT22Manager {
public:
    static bool begin();
    static float getTemperature();
    static float getHumidity();
    static bool isAvailable();
    static void update();
    static String getTemperatureString();
    static String getHumidityString();

private:
    static DHT* dht;
    static bool initialized;
    static float lastTemp;
    static float lastHumidity;
    static unsigned long lastReadTime;
    static const unsigned long READ_INTERVAL = 2000; // Read every 2 seconds
};

#endif // DHT22_H