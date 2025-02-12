/*
  Ali Safarli made with AI
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>

#include <WiFiManager.h> // WiFiManager library

#define MOTION_SENSOR_PIN 14 // GPIO pin for motion sensor D5
#define USE_SERIAL Serial

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
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
    // Initialize serial communication
    USE_SERIAL.begin(115200);
    USE_SERIAL.println();

    // Initialize motion sensor pin
    pinMode(MOTION_SENSOR_PIN, INPUT);

    // WiFi Manager setup
    
    wifiManager.autoConnect("MotionSensorAP");

    // Start WebSocket server
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    // Web server to serve the web interface
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
                    "<form action=\"/reset\" method=\"POST\">"
                    "<input type=\"submit\" value=\"Reset settings\" />"
                    "</form>"
                    "</body>"
                    "</html>");
    });

    server.on("/reset", handleReset);
    

    server.begin();
}

void loop() {
    // Handle WebSocket communication
    webSocket.loop();

    // Handle web server requests
    server.handleClient();

    // Check motion sensor status
    bool currentStatus = digitalRead(MOTION_SENSOR_PIN);
    if (currentStatus != motionDetected) {
        motionDetected = currentStatus;
        String status = motionDetected ? "Motion Detected" : "No Motion";
        webSocket.broadcastTXT(status); // Broadcast status to all connected clients
    }
}

void handleReset() {
  String html = "<h1>Resetting...</h1>";
  html += "<p>Please go to the Access point</p>";
  server.send(200, "text/html", html);
  wifiManager.resetSettings();
  ESP.restart();
  wifiManager.autoConnect("MotionSensorAP");
  }
