

#define WIFI_SSID "Verizon_NQ6GYX"
#define WIFI_PASS "adverb-fee7-jug"
#define HOSTNAME "blobfan"

#define IO_USERNAME  "nik_nax"
#define IO_KEY       "1ecbdc80f434dd13ddb91eba2f30b8e86f6c437d"

#include "AdafruitIO_WiFi.h"

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

#define DEEPSLEEP_DURATION 20e6

#define DHTPIN 14
#define DHTTYPE DHT22

#define FANBLUE 15
#define FANGREEN 16