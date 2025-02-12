/*
  Ali Safarli made with AI
*/
#include <ESP8266WiFi.h>
#include <WiFiManager.h> // Include the WiFiManager library
#include <ESP8266WebServer.h> // Include the WebServer library

ESP8266WebServer server(80); // Create a web server on port 80
WiFiManager wifiManager; // Create an instance of WiFiManager

void setup() {
  Serial.begin(115200); // Start the Serial communication
  delay(1000); // Give time for the Serial monitor to open

  // Try to connect to WiFi
  wifiManager.autoConnect("ESP8266_AP");

  // If connected, print the IP address
  Serial.println("Connected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Define the route for the root URL
  server.on("/", handleRoot);
  // Define the route for the WiFi Manager menu
  server.on("/reset", handleReset);

  // Start the server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient(); // Handle incoming client requests
}

// Function to handle the root URL
void handleRoot() {
  String ssid = WiFi.SSID(); // Get the SSID of the connected WiFi
  String html = "<h1>Welcome to ESP8266 Web Server!</h1>";
  html += "<p>Connected to WiFi: " + ssid + "</p>";
  html += "<form action=\"/reset\" method=\"POST\">";
  html += "<input type=\"submit\" value=\"Reset settings\" />";
  html += "</form>";
  server.send(200, "text/html", html); // Send the HTML response
}

// Function to handle the WiFi Manager menu
void handleReset() {
  String html = "<h1>Resetting...</h1>";
  html += "<p>Please go to the Access point</p>";
  server.send(200, "text/html", html);
  wifiManager.resetSettings();
  ESP.restart();
  wifiManager.autoConnect("ESP8266_AP");
  }
