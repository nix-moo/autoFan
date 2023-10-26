
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <LittleFS.h>
#include <NTPClient.h>
#include <SPI.h>
#include <Time.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#include "DHT.h"

#define HOSTNAME "blobFan"
#define DHTPIN 14
#define DHTTYPE DHT22

ESP8266WebServer server(80);
DHT dht(DHTPIN, DHTTYPE);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

const char *ssid = "Verizon_NQ6GYX";
const char *password = "adverb-fee7-jug";
const char *t = "/data/temp.txt";
const char *b = "/data/batt.txt";
const char timeServer[] = "time.nist.gov";

void readFile(const char *path) {
  Serial.printf("\nReading file: %s\n", path);

  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.println("\nFailed to open file for reading");
    return;
  }
  while (file.available()) { Serial.write(file.read()); }
  file.close();
};

void appendFile(const char *path, String message) {
  Serial.print("\nAppending: ");

  File file = LittleFS.open(path, "a+");
  if (!file) {
    Serial.print(" - FAILED: file open");
    return;
  }
  if (file) {
    file.print(message);
    Serial.print(" - SUCCESS");
  } else {
    Serial.print("- FAILED: Append");
  }
  file.close();
};

void getTemp() {
  Serial.print("\ngetTemp() - ");
  float h = dht.readHumidity();
  float f = dht.readTemperature(true);
  delay(5000);
  delay(5000);
  if (isnan(h) || isnan(f)) {
    Serial.print("FAILED: read from DHT sensor\n");
    return;
  }

  float hif = dht.computeHeatIndex(f, h, true);
  Serial.printf("SUCCESS: %3.0f", f);
  appendFile(t, String(timeClient.getEpochTime()));
  appendFile(t, ",");
  appendFile(t, String(f));
  appendFile(t, ",");
  appendFile(t, String(h));
  appendFile(t, "\n");
};

void getBattery() {
  int raw = analogRead(A0);
  int level = map(raw, 580, 774, 0, 100);
  appendFile(b, String(timeClient.getEpochTime()));
  appendFile(b, ",");
  appendFile(b, String(raw));
  appendFile(b, ",");
  appendFile(b, String(level));
  appendFile(b, "\n");
}

void setup() {
  Serial.begin(115200);
  delay(10000);

  // Connect to wifi
  Serial.print("\nConnecting");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  };

  delay(20000);

  // Get current time
  Serial.print("\nCurrent time: ");
  timeClient.begin();
  timeClient.update();
  Serial.print(timeClient.getFormattedTime());
  Serial.print(" - ");
  Serial.print(String(timeClient.getEpochTime(), DEC));

  // Mount filesystem
  Serial.println("\nMounting LittleFS");
  if (!LittleFS.begin()) {
    Serial.println(" - mount failed");
    return;
  };

  // Check for log files
  if (LittleFS.exists(t)) {
    Serial.print("\nFound temp file");
  } else {
    Serial.println("\nFile not found: Creating new file");
    File file = LittleFS.open(t, "a+");
    file.write("time,F,H\n");
    file.close();

    readFile(t);

    if (LittleFS.exists(b)) {
      Serial.print("\nFound batt file");
    } else {
      Serial.println("\nFile not found: Creating new file");
      File battFile = LittleFS.open(b, "a+");
      battFile.write("time,raw,percent\n");
      battFile.close();
    };

    readFile(b);
  };
};

void loop() {
  delay(20000);

  // getTemp();
  // readFile(t);
  // getBattery();
  // readFile(b);
  Serial.print("\nNOW:");
  Serial.print(timeClient.getDay());
  Serial.print(":");
  Serial.print(timeClient.getHours());
  Serial.print(":");
  Serial.print(timeClient.getMinutes());
  Serial.print(":");
  Serial.print(timeClient.getSeconds());
  Serial.print(" - ");
  Serial.print(timeClient.getEpochTime());
};