/*
  Ali Safarli made with AI
*/
#include <ESP8266WiFi.h>
#include <WiFiManager.h> // WiFiManager library
#include <ESP8266WebServer.h> // WebServer library
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

ESP8266WebServer server(80); // Web server on port 80
WiFiManager wifiManager; // WiFiManager instance

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");
  
  // Connect to WiFi using WiFiManager
  wifiManager.autoConnect("ESP8266_AP");
  Serial.println("Connected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Start OTA updates
  ArduinoOTA.onStart([]() {
    Serial.println("Start updating...");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nUpdate complete");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA Ready");
  
  // Define web server routes
  server.on("/", handleRoot);
  server.on("/reset", handleReset);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient(); // Handle web server requests
  ArduinoOTA.handle(); // Handle OTA updates
}

// Root URL handler
void handleRoot() {
  String ssid = WiFi.SSID();
  String html = "<h1>Welcome to ESP8266 Web Server!</h1>";
  html += "<p>Connected to WiFi: " + ssid + "</p>";
  html += "<form action='/reset' method='POST'>";
  html += "<input type='submit' value='Reset settings' />";
  html += "</form>";
  server.send(200, "text/html", html);
}

// Reset WiFi settings handler
void handleReset() {
  String html = "<h1>Resetting...</h1>";
  html += "<p>Please go to the Access Point</p>";
  server.send(200, "text/html", html);
  wifiManager.resetSettings();
  ESP.restart();
}
