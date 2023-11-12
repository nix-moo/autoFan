
#include "config.h"

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>


DHT_Unified dht(DHTPIN, DHTTYPE);

AdafruitIO_Feed *temperature = io.feed("temperature");
AdafruitIO_Feed *humidity = io.feed("humidity");
AdafruitIO_Feed *battery = io.feed("battery");
AdafruitIO_Feed *fan = io.feed("fan");


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

void getTemp() {
  sensors_event_t event;

  dht.temperature().getEvent(&event);
  float c = event.temperature;
  float f = (c * 1.8) + 32;
  temperature->save(f);

  dht.humidity().getEvent(&event);
  humidity->save(event.relative_humidity);

  delay(5000);

};

void getBattery() {
  int raw = analogRead(A0);
  battery->save(raw);
}

void toggleFan() {
  digitalWrite(FANBLUE, HIGH);
  Serial.println("ON");
  int blueValue = digitalRead(FANBLUE);
  fan->save(blueValue);
  delay(10000);

  digitalWrite(FANBLUE, LOW);
  Serial.println("OFF");
  blueValue = digitalRead(FANBLUE);
  fan->save(blueValue);
  delay(1000);
}

Timer batt(2400000);
Timer temp(300000);

void setup() {
  Serial.begin(115200);
  delay(5000);

  dht.begin();
  io.connect();

  // wait for a connection
  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());

  pinMode(FANGREEN, INPUT);
  pinMode(FANBLUE, OUTPUT);
};


void loop() {
  io.run();
  batt.Update(getBattery);
  temp.Update(getTemp);

  toggleFan();
  delay(2000);
};