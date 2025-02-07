/*
    HTTP REST API POST/GET over TLS (HTTPS)
    Created By: Zachary Hunter / zachary.hunter@gmail.com
    Created On: April 2, 2024
    Verified Working on NodeMCU & NodeMCU MMINI on ESP8266
    Inside Orange 3D Printed Box with Hole for Light and Micro USB
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Arduino_JSON.h>
#include "certs.h"

#define SensorPIN 16
int SensorState = 0;

// ESP8266 WIFI Setup
const char* ssid = "AutumnHillsGuest";
const char* password = "Hunter2023";
const char* serverName = "https://www.zachhunter.net/wp-json/wp-arduino-api/v1/arduino";

// Timer set to 1 seconds (1000)
unsigned long timerDelay = (1000 * 60 * 1);

// Time Last Run
unsigned long lastTime = millis() - timerDelay;

X509List cert(cert_ISRG_Root_X1);

void setup() {
  pinMode(SensorPIN, INPUT);

  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // Set time via NTP, as required for x.509 validation
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));

  Serial.print("NodeMCU POST Sensor Value Every ");
  Serial.print(timerDelay / 1000);
  Serial.println(" seconds.");
}

void loop() {
  int currentState = digitalRead(SensorPIN);

  if(SensorState != currentState) {
    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      String SensorStatus = "Off";

      if (currentState == 0) {
        SensorStatus = "On";
      } else {
        SensorStatus = "Off";
      }

      SensorState = currentState;

      Serial.println(SensorStatus);

      // ========================================================
      // POST Sensor Values to the DB via REST API POST Method
      // ========================================================
      JSONVar myObject;
      String jsonString;
      String sensorWriting;

      myObject["sendorValue"] = WiFi.localIP().toString();
      myObject["sensorType"] = "LightSensor";
      myObject["sensorValue"] = SensorStatus;
      jsonString = JSON.stringify(myObject);
      sensorWriting = httpPOSTRequest(serverName, jsonString);
      Serial.println("==============================================================================");

    } else {
      Serial.println("WiFi Disconnected");
    }
  }
}

// POST JSON Data to REST API
String httpPOSTRequest(const char* serverName, String httpRequestData) {
  WiFiClientSecure client;
  HTTPClient http;

  client.setTrustAnchors(&cert);

  // Enable if your having certificate issues
  //client.setInsecure();

  Serial.println("Secure POST Request to: " + String(serverName));
  Serial.println("Payload: " + httpRequestData);

  http.begin(client, serverName);
  http.addHeader("Authorization", "QT1kIG0Dt9u090ODH6bHXvGU");
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(httpRequestData);

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  Serial.println();

  http.end();

  return payload;
}

// GET Request to REST API
String httpGETRequest(const char* serverName) {
  WiFiClientSecure client;
  HTTPClient http;

  client.setTrustAnchors(&cert);

  // Enable if your having certificate issues
  //client.setInsecure();

  Serial.println("Secure GET Request to: " + String(serverName));

  http.begin(client, serverName);
  http.addHeader("Authorization", "QT1kIG0Dt9u090ODH6bHXvGU");

  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  Serial.println();

  http.end();

  return payload;
}