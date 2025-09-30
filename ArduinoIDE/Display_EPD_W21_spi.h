#ifndef _DISPLAY_EPD_W21_SPI_
#define _DISPLAY_EPD_W21_SPI_
#include "Arduino.h"

// ESP32 DevKit V1 Pin definitions (safe pins)
// Using standard ESP32 GPIO pins compatible with DevKit V1 and not boot-strap sensitive
#define RST_PIN   22   // GPIO22 - Reset pin
#define DC_PIN    21   // GPIO21 - Data/Command pin
#define CS_PIN    5    // GPIO5  - Chip Select pin
#define BUSY_PIN  4    // GPIO4  - Busy pin
#define PWR_PIN   25   // GPIO25 - Power Enable pin
#define MOSI_PIN  23   // GPIO23 - SPI MOSI (default ESP32 SPI)
#define SCK_PIN   18   // GPIO18 - SPI SCK (default ESP32 SPI)

//IO settings for ESP32 DevKit V1
#define isEPD_W21_BUSY digitalRead(BUSY_PIN)  //BUSY
#define EPD_W21_RST_0 digitalWrite(RST_PIN,LOW)  //RES
#define EPD_W21_RST_1 digitalWrite(RST_PIN,HIGH)
#define EPD_W21_DC_0  digitalWrite(DC_PIN,LOW) //DC
#define EPD_W21_DC_1  digitalWrite(DC_PIN,HIGH)
#define EPD_W21_CS_0 digitalWrite(CS_PIN,LOW) //CS
#define EPD_W21_CS_1 digitalWrite(CS_PIN,HIGH)
#define EPD_W21_PWR_0 digitalWrite(PWR_PIN,LOW) //PWR
#define EPD_W21_PWR_1 digitalWrite(PWR_PIN,HIGH)


void SPI_Write(unsigned char value);
void EPD_W21_WriteDATA(unsigned char datas);
void EPD_W21_WriteCMD(unsigned char command);


#endif 
