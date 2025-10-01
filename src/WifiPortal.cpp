#include "WifiPortal.h"
#include <Arduino.h>

bool startWifiPortal(bool forcePortal) {
  WiFiManager wm;
  //wm.resetSettings();
  wm.setDebugOutput(true);
  
  // Configure portal timeout
  wm.setConfigPortalTimeout(120); // 2 minute timeout
  
  // Configure AP settings
  WiFi.softAP("ThumbstackTech"); // Open network for easy access
  
  if (forcePortal) {
    Serial.println("Starting forced configuration portal...");
    return wm.startConfigPortal("ThumbstackTech");
  } else {
    Serial.println("Attempting to connect using stored credentials...");
    if (wm.autoConnect("ThumbstackTech")) {
      Serial.println("✅ Connected to WiFi!");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
      return true;
    } else {
      Serial.println("⚠️ Failed to connect using stored credentials");
      return false;
    }
  }
}

// #include "WifiPortal.h"
// #include <WebServer.h>
// #include <DNSServer.h>

// DNSServer dnsServer;
// WebServer server(80);

// // ✅ Captive portal “success” page for iOS/Android
// void handleCaptive() {
//   server.send(200, "text/html",
//               "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>");
// }

// // ✅ Root handler (WiFiManager portal page)
// void handleRoot() {
//   server.send(200, "text/html",
//               "<html><head><title>Thumbstack WiFi Setup</title></head>"
//               "<body><h1>Welcome to Thumbstack WiFi Setup</h1>"
//               "<p>Use this page to configure your WiFi.</p></body></html>");
// }

// void startWifiPortal() {
//   WiFiManager wm;

//   // Comment this out if you want to remember last WiFi
//   wm.resetSettings();

//   wm.setDebugOutput(true);

//   // Start AP
//   if (!wm.autoConnect("ESP32-Setup", "setup1234")) {
//     Serial.println("⚠️ Failed to connect, staying in AP mode...");
//   } else {
//     Serial.println("✅ Connected to WiFi!");
//     Serial.print("IP Address: ");
//     Serial.println(WiFi.localIP());
//     return; // exit if connected
//   }

//   // If still in AP mode → start DNS + web server
//   dnsServer.start(53, "*", WiFi.softAPIP());

//   // iOS captive portal triggers
//   server.on("/hotspot-detect.html", handleCaptive);
//   server.on("/generate_204", handleCaptive);  // Android
//   server.on("/library/test/success.html", handleCaptive);
//   server.on("/", handleRoot);
//   server.onNotFound(handleCaptive);

//   server.begin();
//   Serial.println("🌐 Captive portal started");

//   // Keep DNS + webserver alive
//   while (true) {
//     dnsServer.processNextRequest();
//     server.handleClient();
//     delay(10);
//   }
// }

