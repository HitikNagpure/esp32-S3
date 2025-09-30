# ESP32 E-Paper Display Configuration

## Pin Connections

Connect your GDEY075Z08 E-Paper display to ESP32 using the following pins:

| E-Paper Pin | ESP32 DevKit V1 Pin | Function |
|-------------|-------------|----------|
| RST         | GPIO 22    | Reset    |
| DC          | GPIO 21    | Data/Command |
| CS          | GPIO 5     | Chip Select |
| BUSY        | GPIO 4     | Busy Status |
| PWR         | GPIO 25    | Power Control |
| DIN/MOSI    | GPIO 23    | SPI Data (MOSI) |
| CLK/SCK     | GPIO 18    | SPI Clock |
| GND         | GND        | Ground |
| VCC         | 3.3V       | Power Supply |

## Changes Made for ESP32 DevKit V1 Compatibility

1. **Pin Definitions**: Updated `Display_EPD_W21_spi.h` with ESP32 DevKit V1 compatible GPIO pins
2. **SPI Configuration**: Using ESP32's standard SPI pins (GPIO23 for MOSI, GPIO18 for SCK)
3. **Safe GPIO Selection**: Chosen pins that don't conflict with internal flash/PSRAM
4. **Power Management**: Using GPIO15 for PWR_PIN control
5. **Serial Debug**: Added Serial output for debugging

## Usage Notes

- Using ESP32 DevKit V1's standard SPI pins for better compatibility
- All GPIO pins chosen are safe for general use and don't interfere with boot or flash operations
- MISO pin is not used by the e-paper display
- PWR_PIN can be used to control display power if your hardware supports it
- All original functionality (full refresh, fast refresh, partial refresh) is preserved

## Compilation

Make sure to select "ESP32 Dev Module" or your specific ESP32 board in the Arduino IDE board manager.

## Testing

The display should initialize and show the demo images as before. Check the Serial Monitor at 115200 baud for initialization confirmation.