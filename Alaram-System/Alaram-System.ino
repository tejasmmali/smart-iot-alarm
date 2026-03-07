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

/* ================= SLACK ================= */

const char* slackURL =
"https://smart-iot-alarm-2d459-default-rtdb.firebaseio.com/slack_messages.json";

String lastSlackID = "";
unsigned long lastSlackFetch = 0;

/* ================= NTP ================= */

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
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

  Serial.println("\nWiFi Connected");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

/* ================================================= */

void loop() {

  unsigned long now = millis();

  /* ===== SEND TIME EVERY SECOND ===== */

  if (now - lastTimeSend >= 1000) {
    lastTimeSend = now;
    sendTimeToUNO();
  }

  /* ===== FETCH ALARM EVERY 10s ===== */

  if (now - lastAlarmFetch >= 10000) {
    lastAlarmFetch = now;
    fetchAlarmFromFirebase();
  }

  /* ===== FETCH SLACK MESSAGE ===== */

  if (now - lastSlackFetch >= 5000) {
    lastSlackFetch = now;
    fetchSlackMessage();
  }
}

/* ================================================= */
/* ================= SEND TIME ===================== */
/* ================================================= */

void sendTimeToUNO() {

  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to get NTP time");
    return;
  }

  char buffer[20];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);

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

  if (httpCode != 200) {
    Serial.println("Alarm HTTP error");
    http.end();
    return;
  }

  String payload = http.getString();

  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.println("Alarm JSON parse failed");
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

      Serial.println("Alarm Sent to UNO");

      break;
    }
  }

  http.end();
}

/* ================================================= */
/* ================= FETCH SLACK =================== */
/* ================================================= */

void fetchSlackMessage() {

  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(slackURL);

  int httpCode = http.GET();

  if (httpCode != 200) {
    http.end();
    return;
  }

  String payload = http.getString();

  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.println("Slack JSON parse failed");
    http.end();
    return;
  }

  if (!doc.is<JsonObject>()) {
    http.end();
    return;
  }

  JsonObject obj = doc.as<JsonObject>();

  String latestKey = "";

  for (JsonPair kv : obj) {
    latestKey = kv.key().c_str();
  }

  if (latestKey == "" || latestKey == lastSlackID) {
    http.end();
    return;
  }

  lastSlackID = latestKey;

  JsonObject msg = obj[latestKey];

  String user = msg["user"] | "Slack";
  String text = msg["text"] | "";

  if (text == "") {
    http.end();
    return;
  }

  /* ===== SEND SLACK MESSAGE TO UNO ===== */

  mySerial.print("S");
  mySerial.println(user);

  mySerial.print("M");
  mySerial.println(text);

  Serial.println("Slack Notification Sent");
  Serial.println(user + ": " + text);

  http.end();
}