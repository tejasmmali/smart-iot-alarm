#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial espSerial(10, 11); // RX, TX

String currentTime = "00:00:00";

int alarmHour = 0;     // 24h internally
int alarmMinute = 0;
bool alarmSet = false;
bool alarmTriggered = false;

const int buzzerPin = 8;

// Custom bell character
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

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, bell);

  pinMode(buzzerPin, OUTPUT);
}

void loop() {

  // Receive time from ESP32 (24h format)
  if (espSerial.available()) {
    String input = espSerial.readStringUntil('\n');
    if (input.length() >= 8 && input.charAt(2) == ':') {
      currentTime = input;
    }
  }

  // Set alarm from Serial Monitor
  // Format: A07:30PM
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');

    if (cmd.startsWith("A")) {

      int hour12 = cmd.substring(1,3).toInt();
      alarmMinute = cmd.substring(4,6).toInt();
      String period = cmd.substring(6);

      if (period == "PM" && hour12 != 12) hour12 += 12;
      if (period == "AM" && hour12 == 12) hour12 = 0;

      alarmHour = hour12;

      alarmSet = true;
      alarmTriggered = false;

      Serial.println("Alarm Set!");
    }
  }

  // Extract 24h time
  int hour24 = currentTime.substring(0,2).toInt();
  int minute = currentTime.substring(3,5).toInt();
  int second = currentTime.substring(6,8).toInt();

  // Convert to 12h format
  int displayHour = hour24;
  String period = "AM";

  if (hour24 >= 12) period = "PM";
  if (hour24 == 0) displayHour = 12;
  else if (hour24 > 12) displayHour = hour24 - 12;

  // TOP ROW → Current Time
  lcd.setCursor(0,0);
  printTwo(displayHour);
  lcd.print(":");
  printTwo(minute);
  lcd.print(":");
  printTwo(second);
  lcd.print(" ");
  lcd.print(period);
  lcd.print("   "); // clear extra chars

  // Convert alarm to 12h for display
  int alarmDisplay = alarmHour;
  String alarmPeriod = "AM";

  if (alarmHour >= 12) alarmPeriod = "PM";
  if (alarmHour == 0) alarmDisplay = 12;
  else if (alarmHour > 12) alarmDisplay = alarmHour - 12;

  // BOTTOM ROW → Alarm + Bell
  lcd.setCursor(0,1);
  printTwo(alarmDisplay);
  lcd.print(":");
  printTwo(alarmMinute);
  lcd.print(" ");
  lcd.print(alarmPeriod);

  if (alarmSet) {
    lcd.setCursor(14,1);
    lcd.write(byte(0)); // bell symbol
  }

  // Alarm Trigger
  if (alarmSet &&
      hour24 == alarmHour &&
      minute == alarmMinute &&
      second == 0 &&
      !alarmTriggered) {

    alarmTriggered = true;

    for(int i=0;i<10;i++){
      digitalWrite(buzzerPin, HIGH);
      delay(500);
      digitalWrite(buzzerPin, LOW);
      delay(500);
    }
  }
}

void printTwo(int number) {
  if (number < 10) lcd.print("0");
  lcd.print(number);
}