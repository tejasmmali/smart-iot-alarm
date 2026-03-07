#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial espSerial(10, 11);

/* ================= STATUS ================= */

bool timeSynced = false;
bool alarmSynced = false;

/* ================= TIME ================= */

String currentTime = "00:00:00";
int currentDay = 0;

/* ================= ALARM ================= */

int alarmHour = 0;
int alarmMinute = 0;
bool alarmSet = false;

String alarmNote = "ALARM";

bool ringing = false;
unsigned long ringStart = 0;
bool alarmTriggeredThisMinute = false;

/* ================= SLACK ================= */

bool slackNotify = false;
unsigned long slackStart = 0;

String slackUser = "";
String slackMessage = "";

/* ===== DISPLAY STATE TRACKING ===== */

String lastLine1 = "";
String lastLine2 = "";

/* ===== Blink + Scroll ===== */

bool blinkState = false;
unsigned long lastBlink = 0;

unsigned long lastScroll = 0;
int scrollIndex = 0;

/* ===== Buzzer ===== */

bool beepState = false;
unsigned long lastBeep = 0;

/* ================= CONNECTING ================= */

unsigned long lastLoadUpdate = 0;
int loadProgress = 0;

/* ================= HARDWARE ================= */

const int buzzerPin = 8;

String days[7] = {"SU","MO","TU","WE","TH","FR","SA"};

byte bell[8] = {
  B00000,
  B00100,
  B01110,
  B01110,
  B01110,
  B11111,
  B00100,
  B00000
};

String serialBuffer = "";

/* ================================================= */

void setup() {

  Serial.begin(9600);
  espSerial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, bell);

  pinMode(buzzerPin, OUTPUT);
}

/* ================================================= */

void loop() {

  /* ===== READ SERIAL FROM ESP ===== */

  while (espSerial.available()) {
    char c = espSerial.read();

    if (c == '\n') {
      processLine(serialBuffer);
      serialBuffer = "";
    }
    else {
      serialBuffer += c;
    }
  }

  /* ===== CONNECTING SCREEN ===== */

  if (!(timeSynced && alarmSynced)) {

    if (millis() - lastLoadUpdate >= 300) {
      lastLoadUpdate = millis();
      loadProgress++;
      if (loadProgress > 10) loadProgress = 0;
    }

    String line1 = " CONNECTING... ";
    String line2 = "[";

    for (int i=0;i<10;i++) {
      if (i < loadProgress) line2 += (char)255;
      else line2 += " ";
    }

    line2 += "]     ";

    updateLCD(line1,line2);
    return;
  }

  /* ================= SLACK DISPLAY ================= */

  if (slackNotify) {

    if (millis() - slackStart >= 5000) {
      slackNotify = false;
      lastLine1 = "";
      lastLine2 = "";
      scrollIndex = 0;
      return;
    }

    String line1 = "SLACK MESSAGE";

    while (line1.length() < 16)
      line1 += " ";

    String msg = slackUser + ": " + slackMessage;
    String padded = "   " + msg + "   ";

    if (millis() - lastScroll >= 250) {
      lastScroll = millis();
      scrollIndex++;

      if (scrollIndex >= padded.length())
        scrollIndex = 0;
    }

    String view = padded + padded;
    String line2 = view.substring(scrollIndex, scrollIndex + 16);

    updateLCD(line1,line2);

    return;
  }

  /* ===== PARSE TIME ===== */

  int hour24 = currentTime.substring(0,2).toInt();
  int minute = currentTime.substring(3,5).toInt();
  int second = currentTime.substring(6,8).toInt();

  int displayHour = hour24;
  String period = "AM";

  if (hour24 >= 12) period = "PM";
  if (hour24 == 0) displayHour = 12;
  else if (hour24 > 12) displayHour -= 12;

  /* ================= RINGING MODE ================= */

  if (ringing) {

    if (millis() - lastBlink >= 400) {
      lastBlink = millis();
      blinkState = !blinkState;
    }

    String line1;

    if (blinkState) {

      char buffer[17];

      sprintf(buffer,"%02d:%02d:%02d %s",
              displayHour,minute,second,period.c_str());

      line1 = String(buffer);

      while (line1.length() < 15)
        line1 += " ";

      line1 += (char)0;
    }
    else {
      line1 = "                ";
    }

    String padded = "   " + alarmNote + "   ";

    if (millis() - lastScroll >= 250) {
      lastScroll = millis();
      scrollIndex++;
      if (scrollIndex >= padded.length())
        scrollIndex = 0;
    }

    String view = padded + padded;
    String line2 = view.substring(scrollIndex,scrollIndex+16);

    updateLCD(line1,line2);

    if (millis() - lastBeep >= 200) {
      lastBeep = millis();
      beepState = !beepState;
      digitalWrite(buzzerPin,beepState);
    }

    if (millis() - ringStart >= 10000) {
      digitalWrite(buzzerPin,LOW);
      ringing = false;
      scrollIndex = 0;
    }

    return;
  }

  /* ================= NORMAL MODE ================= */

  char buffer[17];

  sprintf(buffer,"%02d:%02d:%02d %s",
          displayHour,minute,second,period.c_str());

  String line1 = String(buffer);

  while (line1.length() < 14)
    line1 += " ";

  line1 += days[currentDay];

  String line2;

  if (alarmSet) {

    int alarmDisplay = alarmHour;
    String alarmPeriod = "AM";

    if (alarmHour >= 12) alarmPeriod = "PM";
    if (alarmHour == 0) alarmDisplay = 12;
    else if (alarmHour > 12) alarmDisplay -= 12;

    char buf2[17];

    sprintf(buf2,"%02d:%02d %s",
            alarmDisplay,alarmMinute,alarmPeriod.c_str());

    line2 = String(buf2);

    while (line2.length() < 15)
      line2 += " ";

    line2 += (char)0;
  }
  else {
    line2 = "--:--          ";
  }

  updateLCD(line1,line2);

  /* ===== SAFE MINUTE TRIGGER ===== */

  if (alarmSet && !ringing) {

    if (hour24 == alarmHour && minute == alarmMinute) {

      if (!alarmTriggeredThisMinute) {
        ringing = true;
        ringStart = millis();
        alarmTriggeredThisMinute = true;
      }

    }
    else {
      alarmTriggeredThisMinute = false;
    }
  }
}

/* ================================================= */

void updateLCD(String l1,String l2) {

  while (l1.length() < 16) l1 += " ";
  while (l2.length() < 16) l2 += " ";

  if (l1 != lastLine1) {
    lcd.setCursor(0,0);
    lcd.print(l1);
    lastLine1 = l1;
  }

  if (l2 != lastLine2) {
    lcd.setCursor(0,1);
    lcd.print(l2);
    lastLine2 = l2;
  }
}

/* ================================================= */

void processLine(String input) {

  input.trim();

  if (input.indexOf(",") > 0) {

    int commaIndex = input.indexOf(",");

    currentTime = input.substring(0,commaIndex);
    currentDay = input.substring(commaIndex+1).toInt();

    timeSynced = true;
  }

  if (input.startsWith("A")) {

    alarmHour = input.substring(1,3).toInt();
    alarmMinute = input.substring(4,6).toInt();

    alarmSet = true;
    alarmSynced = true;
  }

  if (input.startsWith("N")) {
    alarmNote = input.substring(1);
  }

  if (input.startsWith("S")) {
    slackUser = input.substring(1);
  }

  if (input.startsWith("M")) {
    slackMessage = input.substring(1);
    slackNotify = true;
    slackStart = millis();
    scrollIndex = 0;
  }
}