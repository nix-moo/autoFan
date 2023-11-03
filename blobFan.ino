
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <LittleFS.h>
#include <NTPClient.h>
#include <SPI.h>
#include <uri/UriBraces.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#include "DHT.h"
#include "index.h"

#define HOSTNAME "hostname"
#define DHTPIN 14
#define DHTTYPE DHT22

ESP8266WebServer server(80);
DHT dht(DHTPIN, DHTTYPE);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -14400, 60000);

const char *ssid = "Verizon_NQ6GYX";
const char *password = "adverb-fee7-jug";
const char *t = "/temp.txt";
const char *b = "/batt.txt";
// const char *f = "/fan.txt";

class Timer {

  long interval;
  long prevMillis = 0;

public:
  Timer(long intv) {
    interval = intv;
    prevMillis = 0;
  }
  void Update(void (*function)()) {
    unsigned long currentMillis = millis();
    if (currentMillis - prevMillis >= interval) {
      prevMillis = currentMillis;
      (*function)();
    }
  }
};

// void fan(){

// }


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

void printFile(const String path) {
  String fn = "/";
  File file = LittleFS.open(fn + path, "r");
  if (!file) {
    Serial.println("\nFailed to open file for reading");
    return;
  }
  while (file.available()) {
    String text = file.readString();
    Serial.print(text);
    server.sendHeader("Cache-Control", "no-cache");
    server.send(200, "text/plain; charset=utf-8", text);
  }
  file.close();
};

void appendFile(const char *path, String message) {
  Serial.print("\nAppending");

  File file = LittleFS.open(path, "a+");
  if (!file) {
    Serial.print(" - FAILED: file open");
    return;
  }
  if (file) {
    file.print(message);
    Serial.print(" - SUCCESS: ");
    Serial.print(message);
  } else {
    Serial.print("- FAILED: Append");
  }
  file.close();
};

String getTime() {
  char buff[15];
  timeClient.update();
  sprintf(buff, "%15.0i", timeClient.getEpochTime());
  Serial.println(buff);
  Serial.println(timeClient.getFormattedTime());
  return String(buff);
};

void getTemp() {
  Serial.print("\ngetTemp() - ");
  float h = dht.readHumidity();
  float f = dht.readTemperature(true);
  delay(5000);
  if (isnan(h) || isnan(f)) {
    Serial.print("FAILED: read from DHT sensor\n");
    return;
  }
  float hif = dht.computeHeatIndex(f, h, true);
  Serial.printf("SUCCESS: %3.0f", f);

  String result = getTime();
  result += ", " + String(f) + ", " + String(h) + "\n";

  appendFile(t, result);
};

void getBattery() {
  int raw = analogRead(A0);
  int level = map(raw, 580, 774, 0, 100);

  String result = getTime();
  result += ", " + String(raw) + ", " + String(level) + "\n";

  appendFile(b, result);
}

void handleFiles() {
  Dir dir = LittleFS.openDir("/");
  String result;

  result += "[\n";
  while (dir.next()) {
    if (result.length() > 4) { result += ","; }
    result += "  {";
    result += " \"name\": \"" + dir.fileName() + "\", ";
    result += " \"size\": " + String(dir.fileSize()) + ", ";
    result += " \"time\": " + String(dir.fileTime());
    result += " }\n";

    // jc.addProperty("size", dir.fileSize());
  }  // while

  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/javascript; charset=utf-8", result);
}  // handleListFiles()

Timer battery(60000);
Timer temp(10000);

void setup() {
  Serial.begin(115200);
  delay(5000);

  // Connect to wifi
  Serial.print("\nConnecting");
  WiFi.begin(ssid, password);
  WiFi.setHostname(HOSTNAME);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  };

  server.on("/", []() {
    server.send(200, "text/html", FPSTR(uploadContent));
  });
  server.on("/files", []() {
    handleFiles();
  });
  server.on(UriBraces("/files/{}"), []() {
    printFile(server.pathArg(0));
  });
  server.begin();

  delay(20000);
  dht.begin();

  // Get current time
  Serial.print("\nCurrent time: ");
  timeClient.begin();
  Serial.print(timeClient.getFormattedTime());
  Serial.print(" - ");
  Serial.print(getTime());

  // Mount filesystem
  Serial.println("\nMounting LittleFS");
  if (!LittleFS.begin()) {
    Serial.println(" - mount failed");
    return;
  };

  // Check for log files
  if (LittleFS.exists(t)) {
    Serial.print("\nFound temp file");
    readFile(t);
  } else {
    Serial.println("\nFile not found: Creating new file");
    File file = LittleFS.open(t, "a+");
    file.write("time, F, H\n");
    file.close();

    readFile(t);

    if (LittleFS.exists(b)) {
      Serial.print("\nFound batt file");
    } else {
      Serial.println("\nFile not found: Creating new file");
      File battFile = LittleFS.open(b, "a+");
      battFile.write("time, raw, percent\n");
      battFile.close();
    };

    readFile(b);
  };
};

void loop() {
  battery.Update(getBattery);
  temp.Update(getTemp);
  server.handleClient();
};