
#include "QRCodeManager.h"
#include <Fonts/FreeMonoBold12pt7b.h>   // Title
#include <Fonts/FreeMonoBold9pt7b.h>    // Labels

void showQRCode(const char* text) {
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(6)];
  qrcode_initText(&qrcode, qrcodeData, 6, 0, text);

  int scale = 6;   // Reduced QR size
  int qrSize = qrcode.size * scale;
  int offsetX = (display.width()  - qrSize) / 2;
  int offsetY = (display.height() - qrSize) / 2 - 20;  // shift slightly up

  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);

    // ===== Title =====
    display.setFont(&FreeMonoBold12pt7b);
    display.setTextColor(GxEPD_BLACK);
    const char* title = "Thumbstack Technologies";
    int16_t tbx, tby; uint16_t tbw, tbh;
    display.getTextBounds(title, 0, 0, &tbx, &tby, &tbw, &tbh);
    display.setCursor((display.width() - tbw) / 2, 50);
    display.print(title);

    // ===== Scan to Setup WiFi =====
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    const char* label = "Scan to Setup WiFi";
    display.getTextBounds(label, 0, 0, &tbx, &tby, &tbw, &tbh);
    display.setCursor((display.width() - tbw) / 2, 75);
    display.print(label);

    // ===== QR Code =====
    for (int y = 0; y < qrcode.size; y++) {
      for (int x = 0; x < qrcode.size; x++) {
        if (qrcode_getModule(&qrcode, x, y)) {
          display.fillRect(offsetX + x * scale, offsetY + y * scale, scale, scale, GxEPD_BLACK);
        }
      }
    }

    // ===== OR Connect Manually =====
    display.setTextColor(GxEPD_RED);
    const char* altMsg = "OR Connect Manually";
    display.getTextBounds(altMsg, 0, 0, &tbx, &tby, &tbw, &tbh);
    display.setCursor((display.width() - tbw) / 2, offsetY + qrSize + 40);
    display.print(altMsg);

    // ===== SSID =====
  const char* ssid = "SSID: ESP32-Setup";
    display.getTextBounds(ssid, 0, 0, &tbx, &tby, &tbw, &tbh);
    display.setCursor((display.width() - tbw) / 2, offsetY + qrSize + 70);
    display.print(ssid);

    // ===== Password =====
  const char* pass = "PASS: setup1234";
    display.getTextBounds(pass, 0, 0, &tbx, &tby, &tbw, &tbh);
    display.setCursor((display.width() - tbw) / 2, offsetY + qrSize + 100);
    display.print(pass);

  } while (display.nextPage());
}

