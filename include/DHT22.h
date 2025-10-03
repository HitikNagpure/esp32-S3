#ifndef DHT22_H
#define DHT22_H

#include <Arduino.h>

class DHT22Manager {
public:
    static const uint8_t DHT_PIN = 15;  // DHT22 data pin
    static bool begin();
    static float getTemperature();
    static float getHumidity();
    static bool update();

private:
    static float temperature;
    static float humidity;
    static unsigned long lastReadTime;
    static const unsigned long READ_INTERVAL = 2000;  // Read every 2 seconds
};

#endif // DHT22_H
