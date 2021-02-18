#include <arduino.h>
#include <ESP8266WiFi.h>
#include "arduinosecrets.h"

// WIFI SSID & PW
const char* ssid     = SECRET_SSID;
const char* password = SECRET_PASS;

// Readings Store
float currentTemperature;
const char CELCIUS[4] = {' ', 176, 'C', '\0'};

// Create Wifi Server
WiFiServer server(80);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    // Report connecting status
    Serial.print("WIFISTATE Connecting to "); Serial.println(ssid);
    delay(500);
  }
  // Report Connected to status
  Serial.print("WIFISTATE Connected to "); Serial.println(ssid);

  // Report IP Addres
  Serial.print("WIFIADDRS "); Serial.println(WiFi.localIP());

  // Start HTTP Server
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  SerialComms();
  WebServer();
}

void SerialComms() {
    if (Serial.available()) {
      char commandBuffer[10] = "";
      Serial.readBytesUntil(' ', commandBuffer, 9);
      Serial.print("DEBUG Received '");
      Serial.print(commandBuffer);
      Serial.println("'");
      if (strcmp(commandBuffer, "TEMPINCEL") == 0) {
        // Receiving Temperature
        currentTemperature = Serial.parseFloat();
        Serial.print("DEBUG executing ");
        Serial.print(commandBuffer);
        Serial.print(": temperature reading ");
        Serial.print(currentTemperature);
        Serial.println(" Celsius stored"); 
      }
    }
}

void WebServer() {
  WiFiClient client = server.available();

  if (client) {
    const unsigned long timeoutTime = 2000;
    long currentTime = millis();
    long startTime = currentTime;

    String currentLine = "";
    String header = "";
    while (client.connected() && currentTime - startTime <= timeoutTime) {
      currentTime = millis();
      if (client.available())
      {
        char c = client.read();
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // After 2 \n characters, the headers are concluded and a response is requested
            ProcessRequest(client, header);
            break;
          }
          currentLine = "";
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    Serial.print("CLIENTSRV "); Serial.println(client.remoteIP());
    client.stop();
  }
}

void ProcessRequest(WiFiClient client, String header) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE html><html>");
  client.println("<head>");
  client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<link rel=\"icon\" href=\"data:,\">");
  client.println("</head><body>");
  client.println("<h1>John's Awesome Weather Station</h1>");
  client.print("<p>The current temperature is "); client.print(currentTemperature); client.print(CELCIUS); client.println("</p>");
  client.println("</body>");
  client.println();
}
