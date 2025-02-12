/*
  Ali Safarli made with AI
*/
#include <ESP8266WiFi.h>
#include <WiFiManager.h> // WiFiManager library
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WebSocketsServer.h>

#define MOTION_SENSOR_PIN 14 // GPIO pin for motion sensor (D5)
#define USE_SERIAL Serial

ESP8266WebServer server(80);
WebSocketsServer webSocket(81);
WiFiManager wifiManager;

bool motionDetected = false;

// WebSocket event handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
            // Send initial motion sensor status
            String status = motionDetected ? "Motion Detected" : "No Motion";
            webSocket.sendTXT(num, status);
        }
            break;
        case WStype_TEXT:
            USE_SERIAL.printf("[%u] Received: %s\n", num, payload);
            break;
    }
}

void setup() {
    USE_SERIAL.begin(115200);
    USE_SERIAL.println("Booting...");

    pinMode(MOTION_SENSOR_PIN, INPUT);

    // Connect to WiFi using WiFiManager
    wifiManager.autoConnect("ESP8266_AP");
    USE_SERIAL.println("Connected to WiFi!");
    USE_SERIAL.print("IP Address: ");
    USE_SERIAL.println(WiFi.localIP());

    // Start OTA updates
    ArduinoOTA.onStart([]() {
        USE_SERIAL.println("Start updating...");
    });
    ArduinoOTA.onEnd([]() {
        USE_SERIAL.println("\nUpdate complete");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        USE_SERIAL.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        USE_SERIAL.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) USE_SERIAL.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) USE_SERIAL.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) USE_SERIAL.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) USE_SERIAL.println("Receive Failed");
        else if (error == OTA_END_ERROR) USE_SERIAL.println("End Failed");
    });
    ArduinoOTA.begin();
    USE_SERIAL.println("OTA Ready");

    // Start WebSocket server
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    // Web server setup
    server.on("/", []() {
        server.send(200, "text/html",
                    "<html>"
                    "<head>"
                    "<script>"
                    "var connection = new WebSocket('ws://' + location.hostname + ':81/');"
                    "connection.onmessage = function (event) {"
                    "  document.getElementById('status').innerText = 'Motion Sensor Status: ' + event.data;"
                    "};"
                    "</script>"
                    "</head>"
                    "<body>"
                    "<h1>Motion Sensor Status</h1>"
                    "<p id='status'>Waiting for data...</p>"
                    "<form action='/reset' method='POST'>"
                    "<input type='submit' value='Reset settings' />"
                    "</form>"
                    "</body>"
                    "</html>");
    });

    server.on("/reset", handleReset);
    server.begin();
    USE_SERIAL.println("HTTP server started");
}

void loop() {
    server.handleClient();
    webSocket.loop();
    ArduinoOTA.handle();

    bool currentStatus = digitalRead(MOTION_SENSOR_PIN);
    if (currentStatus != motionDetected) {
        motionDetected = currentStatus;
        String status = motionDetected ? "Motion Detected" : "No Motion";
        webSocket.broadcastTXT(status);
    }
}

void handleReset() {
    String html = "<h1>Resetting...</h1>";
    html += "<p>Please go to the Access Point</p>";
    server.send(200, "text/html", html);
    wifiManager.resetSettings();
    ESP.restart();
}
