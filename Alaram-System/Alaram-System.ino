#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "time.h"

/* ================= WIFI ================= */

const char* ssid = "BAPPA 3";
const char* password = "tej@$m@li7122007@@$";

/* ================= FIREBASE ================= */

const char* firebaseURL =
"https://smart-iot-alarm-2d459-default-rtdb.firebaseio.com/alarms.json";

/* ================= NTP ================= */

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;   // India GMT+5:30
const int daylightOffset_sec = 0;

/* ================= SERIAL ================= */

HardwareSerial mySerial(2);   // TX2 = 17, RX2 = 16

unsigned long lastTimeSend = 0;
unsigned long lastAlarmFetch = 0;

/* ================================================= */

void setup() {

  Serial.begin(115200);
  mySerial.begin(9600, SERIAL_8N1, 16, 17);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

/* ================================================= */

void loop() {

  unsigned long now = millis();

  /* ===== SEND TIME EVERY 1 SECOND ===== */

  if (now - lastTimeSend >= 1000) {
    lastTimeSend = now;
    sendTimeToUNO();
  }

  /* ===== FETCH ALARM EVERY 10 SECONDS ===== */

  if (now - lastAlarmFetch >= 10000) {
    lastAlarmFetch = now;
    fetchAlarmFromFirebase();
  }
}

/* ================================================= */
/* ================= SEND TIME ===================== */
/* ================================================= */

void sendTimeToUNO() {

  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to get time");
    return;
  }

  char buffer[20];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);

  // Format example: 19:25:42,2
  mySerial.print(buffer);
  mySerial.print(",");
  mySerial.println(timeinfo.tm_wday);
}

/* ================================================= */
/* ================= FETCH ALARM =================== */
/* ================================================= */

void fetchAlarmFromFirebase() {

  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(firebaseURL);

  int httpCode = http.GET();

  if (httpCode == 200) {

    String payload = http.getString();

    Serial.println("Alarms JSON:");
    Serial.println(payload);

    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.println("JSON Parse Failed!");
      http.end();
      return;
    }

    JsonArray alarms = doc.as<JsonArray>();

    for (JsonObject alarm : alarms) {

      bool enabled = alarm["enabled"];

      if (enabled) {

        int hour = alarm["hour"];
        int minute = alarm["minute"];
        String name = alarm["name"] | "ALARM";

        /* ===== SEND ALARM TIME ===== */

        mySerial.print("A");
        if (hour < 10) mySerial.print("0");
        mySerial.print(hour);
        mySerial.print(":");
        if (minute < 10) mySerial.print("0");
        mySerial.println(minute);

        /* ===== SEND ALARM NOTE ===== */

        mySerial.print("N");
        mySerial.println(name);

        Serial.print("Sent Alarm: ");
        Serial.print(hour);
        Serial.print(":");
        Serial.println(minute);

        Serial.print("Sent Note: ");
        Serial.println(name);

        break;  // Only first enabled alarm
      }
    }
  }
  else {
    Serial.print("HTTP Error: ");
    Serial.println(httpCode);
  }

  http.end();
}