#include <WiFi.h>
#include "time.h"

const char* ssid = "ssid";
const char* password = "PASSWORD";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800; // India UTC+5:30
const int daylightOffset_sec = 0;

HardwareSerial mySerial(2);  // UART2

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600, SERIAL_8N1, 16, 17); 
  // RX=16, TX=17

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }

  char timeString[9];
  strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeinfo);

  mySerial.println(timeString);
  delay(1000);
}