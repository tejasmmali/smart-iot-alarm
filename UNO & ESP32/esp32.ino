#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "time.h"



const char* ssid = "BAPPA 3";
const char* password = "tej@$m@li7122007@@$";



const char* firebaseURL =
"https://YOUR_FIREBASE_PROJECT.firebaseio.com/alarms.json";

const char* slackURL =
"https://YOUR_FIREBASE_PROJECT.firebaseio.com/slack_messages.json";

const char* animationURL =
"https://YOUR_FIREBASE_PROJECT.firebaseio.com/animation/mode.json";

/* BUTTONS */

const int modeButton = 25;
const int stopButton = 26;

bool lastModeState = HIGH;
bool lastStopState = HIGH;
unsigned long lastButtonTime = 0;

/* VARIABLES */

String lastSlackID = "";
String lastAnimationMode = "off";

unsigned long lastSlackFetch = 0;
unsigned long lastAlarmFetch = 0;
unsigned long lastAnimationFetch = 0;
unsigned long lastTimeSend = 0;

/* NTP */

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

/* SERIAL */

HardwareSerial mySerial(2);



void setup(){

Serial.begin(115200);

mySerial.begin(9600, SERIAL_8N1, 16, 17);

pinMode(modeButton, INPUT_PULLUP);
pinMode(stopButton, INPUT_PULLUP);

WiFi.begin(ssid, password);

Serial.print("Connecting WiFi");

while(WiFi.status()!=WL_CONNECTED){
delay(500);
Serial.print(".");
}

Serial.println("\nWiFi Connected");

configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}



void loop(){

handleButtons();

unsigned long now = millis();

/* SEND TIME */

if(now-lastTimeSend>=1000){
lastTimeSend=now;
sendTimeToUNO();
}

/* FETCH ALARMS */

if(now-lastAlarmFetch>=10000){
lastAlarmFetch=now;
fetchAlarmFromFirebase();
}

/* FETCH SLACK */

if(now-lastSlackFetch>=8000){
lastSlackFetch=now;
fetchSlackMessage();
}

/* FETCH ANIMATION */

if(now-lastAnimationFetch>=3000){
lastAnimationFetch=now;
fetchAnimationState();
}

}

/* btn control*/

void handleButtons(){

bool modeState=digitalRead(modeButton);
bool stopState=digitalRead(stopButton);

if(millis()-lastButtonTime>80){

/* mode btn*/

if(modeState==LOW && lastModeState==HIGH){

Serial.println("MODE BUTTON");

if(lastAnimationMode=="wave"){

lastAnimationMode="dino";
mySerial.println("G");
updateFirebaseAnimation("dino");

}
else if(lastAnimationMode=="dino"){

lastAnimationMode="off";
mySerial.println("C");
updateFirebaseAnimation("off");

}
else{

lastAnimationMode="wave";
mySerial.println("W");
updateFirebaseAnimation("wave");

}
}

/* sto btn */

if(stopState==LOW && lastStopState==HIGH){

Serial.println("STOP BUTTON");

mySerial.println("ACLEAR");

}

lastButtonTime=millis();
}

lastModeState=modeState;
lastStopState=stopState;
}


void updateFirebaseAnimation(String mode){

HTTPClient http;

http.begin(animationURL);

http.addHeader("Content-Type","application/json");

String payload="\""+mode+"\"";

http.PUT(payload);

http.end();

}



void sendTimeToUNO(){

struct tm timeinfo;

if(!getLocalTime(&timeinfo)){
Serial.println("NTP Failed");
return;
}

char buffer[20];

strftime(buffer,sizeof(buffer),"%H:%M:%S",&timeinfo);

mySerial.print(buffer);
mySerial.print(",");
mySerial.println(timeinfo.tm_wday);
}


void fetchAnimationState(){

if(WiFi.status()!=WL_CONNECTED) return;

HTTPClient http;

http.begin(animationURL);

int httpCode=http.GET();

if(httpCode!=200){
http.end();
return;
}

String mode=http.getString();
mode.replace("\"","");

if(mode==lastAnimationMode){
http.end();
return;
}

lastAnimationMode=mode;

Serial.print("Animation Mode: ");
Serial.println(mode);

if(mode=="wave"){
mySerial.println("W");
}
else if(mode=="dino"){
mySerial.println("G");
}
else{
mySerial.println("C");
}

http.end();
}



void fetchAlarmFromFirebase(){

if(WiFi.status()!=WL_CONNECTED) return;

HTTPClient http;

http.begin(firebaseURL);

int httpCode=http.GET();

if(httpCode!=200){
http.end();
return;
}

String payload=http.getString();

DynamicJsonDocument doc(2048);

if(deserializeJson(doc,payload)){
http.end();
return;
}

JsonArray alarms=doc.as<JsonArray>();

struct tm timeinfo;

if(!getLocalTime(&timeinfo)){
http.end();
return;
}

int nowMinutes=timeinfo.tm_hour*60+timeinfo.tm_min;

JsonObject bestAlarm;

int bestDiff=1441;

bool found=false;

for(JsonObject alarm:alarms){

bool enabled=alarm["enabled"]|false;
if(!enabled) continue;

int hour=alarm["hour"]|-1;
int minute=alarm["minute"]|-1;

if(hour<0 || minute<0) continue;

int alarmMinutes=hour*60+minute;
int diff=(alarmMinutes-nowMinutes+1440)%1440;

if(diff<bestDiff){
bestDiff=diff;
bestAlarm=alarm;
found=true;
}

}

if(found){

int hour=bestAlarm["hour"];
int minute=bestAlarm["minute"];

String name=bestAlarm["name"]|"ALARM";
int duration=bestAlarm["durationSec"]|10;

mySerial.print("A");

if(hour<10) mySerial.print("0");
mySerial.print(hour);
mySerial.print(":");

if(minute<10) mySerial.print("0");
mySerial.println(minute);

mySerial.print("N");
mySerial.println(name);

mySerial.print("D");
mySerial.println(duration);

}
else{

mySerial.println("ACLEAR");

}

http.end();
}

/* slack message*/

void fetchSlackMessage(){

if(WiFi.status()!=WL_CONNECTED) return;

HTTPClient http;

http.begin(slackURL);

int httpCode=http.GET();

if(httpCode!=200){
http.end();
return;
}

String payload=http.getString();

DynamicJsonDocument doc(4096);

if(deserializeJson(doc,payload)){
http.end();
return;
}

JsonObject obj=doc.as<JsonObject>();

String latestKey="";

for(JsonPair kv:obj){
latestKey=kv.key().c_str();
}

if(latestKey=="" || latestKey==lastSlackID){
http.end();
return;
}

lastSlackID=latestKey;

JsonObject msg=obj[latestKey];

String user=msg["user"]|"Slack";
String text=msg["text"]|"";

if(text==""){
http.end();
return;
}

mySerial.print("S");
mySerial.println(user);

mySerial.print("M");
mySerial.println(text);

Serial.println("Slack Notification Sent");

http.end();
}