#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial espSerial(10, 11);

bool timeSynced = false;
bool alarmSynced = false;

String currentTime = "00:00:00";
int currentDay = 0;


int alarmHour = 0;
int alarmMinute = 0;
bool alarmSet = false;

String alarmNote = "ALARM";

bool ringing = false;
unsigned long ringStart = 0;
bool alarmTriggeredThisMinute = false;
unsigned long ringDurationMs = 10000;

/* slack*/

bool slackNotify = false;
unsigned long slackStart = 0;

String slackUser = "";
String slackMessage = "";

String lastLine1 = "";
String lastLine2 = "";

/* blick/scroll*/

bool blinkState = false;
unsigned long lastBlink = 0;

unsigned long lastScroll = 0;
int scrollIndex = 0;

/*buzzer*/

bool beepState = false;
unsigned long lastBeep = 0;

/*load animation*/

unsigned long lastLoadUpdate = 0;
int loadProgress = 0;

/* animation */

bool animationMode = false;
char animationType = 'C';

unsigned long lastAnimFrame = 0;
int animFrame = 0;

int cactusX = 15;
bool dinoFrame = false;



const int buzzerPin = 8;

String days[7] = {"SU","MO","TU","WE","TH","FR","SA"};

/* custom characters */

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

byte waveChar[8] = {
B00000,
B00000,
B00000,
B00100,
B01110,
B11111,
B00000,
B00000
};

byte dino1[8] = {
B00110,
B01111,
B11110,
B11111,
B01110,
B01110,
B01010,
B00000
};

byte dino2[8] = {
B00110,
B01111,
B11110,
B11111,
B01110,
B01110,
B00110,
B00000
};

byte cactus[8] = {
B00100,
B00110,
B10101,
B10101,
B10101,
B00110,
B00100,
B00000
};

byte ground[8] = {
B00000,
B00000,
B00000,
B00000,
B00000,
B00000,
B00100,
B00000
};

String serialBuffer = "";


void resetLCDCache(){
  lastLine1 = "";
  lastLine2 = "";
}



void setup(){

  Serial.begin(9600);
  espSerial.begin(9600);

  lcd.init();
  lcd.backlight();

  lcd.createChar(0, bell);
  lcd.createChar(1, waveChar);
  lcd.createChar(2, dino1);
  lcd.createChar(3, dino2);
  lcd.createChar(4, cactus);
  lcd.createChar(5, ground);

  pinMode(buzzerPin, OUTPUT);
}


void loop(){

  while (espSerial.available()){

    char c = espSerial.read();

    if (c == '\n'){
      processLine(serialBuffer);
      serialBuffer = "";
    }
    else{
      serialBuffer += c;
    }
  }


  if(animationMode){

    if(animationType=='W') waveAnimation();
    if(animationType=='G') dinoAnimation();

    return;
  }


  if(!(timeSynced && alarmSynced)){

    if(millis() - lastLoadUpdate >= 300){

      lastLoadUpdate = millis();
      loadProgress++;

      if(loadProgress>10)
        loadProgress = 0;
    }

    String line1 = " CONNECTING... ";

    String line2 = "[";

    for(int i=0;i<10;i++){

      if(i < loadProgress)
        line2 += (char)255;
      else
        line2 += " ";
    }

    line2 += "]     ";

    resetLCDCache();
    updateLCD(line1,line2);

    return;
  }

  /* slack notify*/

  if(slackNotify){

    if(millis() - slackStart >= 8000){

      slackNotify=false;
      scrollIndex=0;

      resetLCDCache();
      return;
    }

    String line1="SLACK MESSAGE";

    while(line1.length()<16)
      line1+=" ";

    String msg = slackUser + ": " + slackMessage;

    String padded = "   " + msg + "   ";

    if(millis() - lastScroll >= 250){

      lastScroll = millis();
      scrollIndex++;

      if(scrollIndex >= padded.length())
        scrollIndex = 0;
    }

    String view = padded + padded;

    String line2 = view.substring(scrollIndex,scrollIndex+16);

    updateLCD(line1,line2);

    return;
  }

  int hour24 = currentTime.substring(0,2).toInt();
  int minute = currentTime.substring(3,5).toInt();
  int second = currentTime.substring(6,8).toInt();

  int displayHour = hour24;

  String period="AM";

  if(hour24>=12) period="PM";

  if(hour24==0)
    displayHour=12;

  else if(hour24>12)
    displayHour-=12;

  /* ring mode*/

  if(ringing){

    if(millis()-lastBlink>=400){

      lastBlink=millis();
      blinkState=!blinkState;
    }

    String line1;

    if(blinkState){

      char buffer[17];

      sprintf(buffer,"%02d:%02d:%02d %s",
              displayHour,minute,second,period.c_str());

      line1 = String(buffer);

      while(line1.length()<15)
        line1+=" ";

      line1 += (char)0;
    }
    else{
      line1="                ";
    }

    String padded = "   " + alarmNote + "   ";

    if(millis()-lastScroll>=250){

      lastScroll=millis();
      scrollIndex++;

      if(scrollIndex>=padded.length())
        scrollIndex=0;
    }

    String view = padded + padded;

    String line2 = view.substring(scrollIndex,scrollIndex+16);

    updateLCD(line1,line2);

    if(millis()-lastBeep>=200){

      lastBeep=millis();

      beepState=!beepState;

      digitalWrite(buzzerPin,beepState);
    }

    if(millis()-ringStart>=ringDurationMs){

      digitalWrite(buzzerPin,LOW);

      ringing=false;

      scrollIndex=0;
    }

    return;
  }

  /*normal mode clock*/

  char buffer[17];

  sprintf(buffer,"%02d:%02d:%02d %s",
          displayHour,minute,second,period.c_str());

  String line1 = String(buffer);

  while(line1.length()<14)
    line1+=" ";

  line1 += days[currentDay];

  String line2;

  if(alarmSet){

    int alarmDisplay = alarmHour;

    String alarmPeriod="AM";

    if(alarmHour>=12)
      alarmPeriod="PM";

    if(alarmHour==0)
      alarmDisplay=12;

    else if(alarmHour>12)
      alarmDisplay-=12;

    char buf2[17];

    sprintf(buf2,"%02d:%02d %s",
            alarmDisplay,alarmMinute,alarmPeriod.c_str());

    line2 = String(buf2);

    while(line2.length()<15)
      line2+=" ";

    line2 += (char)0;
  }
  else{

    line2="--:--          ";
  }

  updateLCD(line1,line2);

  if(alarmSet && !ringing){

    if(hour24==alarmHour && minute==alarmMinute){

      if(!alarmTriggeredThisMinute){

        ringing=true;

        ringStart=millis();

        alarmTriggeredThisMinute=true;
      }
    }
    else{

      alarmTriggeredThisMinute=false;
    }
  }
}



void updateLCD(String l1,String l2){

  while(l1.length()<16) l1+=" ";
  while(l2.length()<16) l2+=" ";

  if(l1!=lastLine1){

    lcd.setCursor(0,0);
    lcd.print(l1);

    lastLine1=l1;
  }

  if(l2!=lastLine2){

    lcd.setCursor(0,1);
    lcd.print(l2);

    lastLine2=l2;
  }
}

/* wave animation */

void waveAnimation(){

  if(millis()-lastAnimFrame < 200) return;

  lastAnimFrame=millis();

  lcd.setCursor(0,0);
  lcd.print("                ");

  lcd.setCursor(0,1);
  lcd.print("                ");

  for(int i=0;i<16;i++){

    int y=(sin((i+animFrame)*0.5)+1)*2;

    lcd.setCursor(i,y>2?1:0);
    lcd.write(byte(1));
  }

  animFrame++;
}

/* dino animation */

void dinoAnimation(){

  if(millis()-lastAnimFrame < 200) return;

  lastAnimFrame=millis();

  lcd.setCursor(0,0);
  lcd.print("                ");

  lcd.setCursor(0,1);
  lcd.print("                ");

  for(int i=0;i<16;i++){

    if((i+animFrame)%2==0){
      lcd.setCursor(i,1);
      lcd.write(byte(5));
    }
  }

  lcd.setCursor(2,0);

  if(dinoFrame)
    lcd.write(byte(2));
  else
    lcd.write(byte(3));

  dinoFrame=!dinoFrame;

  lcd.setCursor(cactusX,1);
  lcd.write(byte(4));

  cactusX--;

  if(cactusX < -1)
    cactusX = 16;

  animFrame++;
}



void processLine(String input){

  input.trim();

  if(input=="W"){

    animationMode=true;
    animationType='W';

    lcd.clear();
    resetLCDCache();

    return;
  }

  if(input=="G"){

    animationMode=true;
    animationType='G';

    lcd.clear();
    resetLCDCache();

    return;
  }

  if(input=="C"){

    animationMode=false;

    lcd.clear();
    resetLCDCache();

    return;
  }

  if(input.indexOf(",")>0){

    int commaIndex=input.indexOf(",");

    currentTime=input.substring(0,commaIndex);
    currentDay=input.substring(commaIndex+1).toInt();

    timeSynced=true;
  }

  if(input=="ACLEAR"){

    alarmSet=false;
    alarmSynced=true;

    ringing=false;

    digitalWrite(buzzerPin,LOW);
  }

  else if(input.startsWith("A")){

    alarmHour=input.substring(1,3).toInt();
    alarmMinute=input.substring(4,6).toInt();

    alarmSet=true;
    alarmSynced=true;
  }

  if(input.startsWith("N"))
    alarmNote=input.substring(1);

  if(input.startsWith("D")){

    int sec=input.substring(1).toInt();

    if(sec<5) sec=5;
    if(sec>20) sec=20;

    ringDurationMs=(unsigned long)sec*1000UL;
  }

  if(input.startsWith("S"))
    slackUser=input.substring(1);

  if(input.startsWith("M")){

    slackMessage=input.substring(1);

    slackNotify=true;

    slackStart=millis();

    scrollIndex=0;
  }
}