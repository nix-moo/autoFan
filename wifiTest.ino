
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "DHT.h"
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <SPI.h>
#include <FS.h>
#include <LittleFS.h>
#include <time.h>

#define HOSTNAME "webserver"

ESP8266WebServer server(80);

#define DHTPIN 14
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

const char *ssid = "Verizon_NQ6GYX";
const char *password = "adverb-fee7-jug";
const char *path = "/data/log.txt";

void readFile(const char *path) {
  Serial.printf("Reading file: %s\n", path);

  File file = LittleFS.open(path, "a+");
  if (!file) {
    Serial.println("\nFailed to open file for reading");
    return;
  }

  Serial.print("\nRead from file: ");
  while (file.available()) { Serial.write(file.read()); }
  file.close();
};

void appendFile(const char *path, const char *message) {
  Serial.printf("\nAppending to file: %s\n", path);
  Serial.write(message);

  File file = LittleFS.open(path, "a+");
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file) {

    Serial.write(message);
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
};

void getTemp() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  Serial.print(F("\nHumidity"));
  Serial.print(h);

  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  Serial.print(F("\nTemp in F"));
  Serial.print(f);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h, true);
};


void setup() {
  Serial.begin(115200);
  delay(10000);

  // Start sensor
  dht.begin();

  // Connect to wifi
  Serial.print("\n\nConnecting");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  };

  delay(2000);

  // Serial.println("Formatting LittleFS filesystem");
  // LittleFS.format();
  Serial.println("\nMounting LittleFS");
  if (!LittleFS.begin()) {
    Serial.println("\nLittleFS mount failed");
    return;
  };

  if (LittleFS.exists(path)) {
    Serial.print("\nFound file");
  } else {
    Serial.println("Directory not found");
    Serial.println("Formatting directory.");
    if (!LittleFS.format()) {
      Serial.println("Formatting failed.");
    };
    Serial.println("Creating file");
    File file = LittleFS.open(path, "a+");
    file.write("|  F  |  H  | Comf |");
    file.close();

  };

  readFile(path);
};




void loop() {
  delay(5000);

  getTemp();
  readFile(path);
};