
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <LittleFS.h>
#include <NTPClient.h>
#include <SPI.h>
#include <WiFiUdp.h>
#include "config.h"
#include "DHT.h"
#include "index.h"

#define DHTPIN 14
#define DHTTYPE DHT22

AsyncWebServer server(80);
DHT dht(DHTPIN, DHTTYPE);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "us.pool.ntp.org", -14400, 60000);

const char* t = "/temp.txt";
const char* b = "/batt.txt";
// const char *f = "/fan.txt";


class Timer {
  long interval;
  long prevMillis = millis();
public:
  Timer(long intv) {
    interval = intv;
    prevMillis = 0;
  };
  void Update(void (*function)()) {
    long currentMillis = millis();
    if (currentMillis - prevMillis >= interval) {
      prevMillis = currentMillis;
      (*function)();
    };
  };
};

String printFile(const char* path) {
  File file = LittleFS.open(path, "r");
  String text = "";
  if (!file) {
    Serial.println("\nFailed to open file for reading");
    return String("ERROR");
  }
  while (file.available()) {
    Serial.print(file.readString());
    return file.readString();
  }
  return text;
};

void appendFile(const char* path, String message) {
  Serial.print("\nAppending");
  File file = LittleFS.open(path, "a+");
  if (!file) {
    Serial.print(" - FAILED: file open");
    return;
  };
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
  char buff[20];
  timeClient.update();
  sprintf(buff, "%15.0li", timeClient.getEpochTime());
  Serial.println(buff);
  Serial.println(timeClient.getFormattedTime());
  Serial.println(timeClient.getDay());

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
  // float hif = dht.computeHeatIndex(f, h, true);
  Serial.printf("SUCCESS: %3.0f", f);

  String result = "{";
  result += "time:" + getTime();
  result += ",\ntemp:" + String(f) + ",\nhumid:" + String(h) + "},\n";
  appendFile(t, result);
};

void getBattery() {
  int raw = analogRead(A0);
  int level = map(raw, 580, 774, 0, 100);

  String result = "{time:";
  result += getTime();
  result += ",\nraw:" + String(raw) + ",\nlevel:" + String(level) + "},\n";
  appendFile(b, result);
}

String handleFiles() {
  Dir dir = LittleFS.openDir("/");
  String result;
  result += "[\n";
  while (dir.next()) {
    if (result.length() > 4) {
      result += ",";
    }
    result += "  {";
    result += " \"name\": \"" + dir.fileName() + "\", ";
    result += " \"size\": " + String(dir.fileSize()) + ", ";
    result += " \"time\": " + String(dir.fileTime());
    result += " }\n";

  };  // while
  return result;
};  // handleListFiles()

void handleDataRequest(AsyncWebServerRequest *request, bool final){
  String processor(const String& var) {
  Serial.println(var);
  if (var == "TEMP_DATA") {
    String output = printFile(t);
    Serial.print(output);
    return output;
  } else if (var == "BATT_DATA") {
    Serial.print(printFile(b));
    return printFile(b);
  }
  return String();
};
}

Timer battery(300000);
Timer temp(300000);

void setup() {
  Serial.begin(115200);
  delay(5000);

  // Connect to wifi
  Serial.print("\nConnecting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  WiFi.setHostname(HOSTNAME);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  };

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse_P(200, "text/html", index_html, processor);
    request->send(response);
  });

  server.on("/batt", HTTP_GET, [](AsyncWebServerRequest* request) {
    String output = printFile(b);
    // request->send(200, "text/plain; charset=utf-8", output);
    request->send("text/html", 128, [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
  //Write up to "maxLen" bytes into "buffer" and return the amount written.
  //index equals the amount of bytes that have been already sent
  //You will not be asked for more bytes once the content length has been reached.
  //Keep in mind that you can not delay or yield waiting for more data!
  //Send what you currently have and you will be asked for more again
  return mySource.read(buffer, maxLen);
}, processor);
  });
  server.on("/temp", HTTP_GET, [](AsyncWebServerRequest* request) {
    String output = printFile(t);
    request->send(200, "text/plain; charset=utf-8", output);
  });

  // server.on(UriBraces("/files/{}"), []()
  //           {
  // String output = printFile(server.pathArg(0));
  // request->send(200, "text/plain; charset=utf-8", text);
  //  });
  server.begin();

  delay(20000);
  dht.begin();

  // Get current time
  Serial.print("\nCurrent time: ");
  timeClient.begin();
  Serial.print(timeClient.getFormattedTime());
  Serial.print(" - ");
  Serial.print(getTime());
  delay(20000);

  // Mount filesystem
  Serial.println("\nMounting LittleFS");
  if (!LittleFS.begin()) {
    Serial.println(" - mount failed");
    return;
  };

  // Check for log files
  if (LittleFS.exists(t)) {
    Serial.print("\nFound temp file");
    Serial.print(printFile(t));
  } else {
    Serial.println("\nFile not found: Creating new file");
    File file = LittleFS.open(t, "a+");
    file.close();
    Serial.print(printFile(t));

    if (LittleFS.exists(b)) {
      Serial.print("\nFound batt file");
    } else {
      Serial.println("\nFile not found: Creating new file");
      File battFile = LittleFS.open(b, "a+");
      battFile.close();
    };

    Serial.print(printFile(b));
  };
};

void loop() {
  battery.Update(getBattery);
  temp.Update(getTemp);
};