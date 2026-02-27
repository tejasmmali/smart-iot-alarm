#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid="BAPPA 3";
const char* password="tej@$m@li7122007@@$";

String api="https://your-project.vercel.app/api/alarm";

#define RXD2 16
#define TXD2 17

String lastAlarm="";

void setup(){
Serial.begin(115200);
Serial2.begin(9600,SERIAL_8N1,RXD2,TXD2);

WiFi.begin(ssid,password);

while(WiFi.status()!=WL_CONNECTED){
 delay(500);
 Serial.print(".");
}

Serial.println("Connected");
}

void loop(){

if(WiFi.status()==WL_CONNECTED){

HTTPClient http;
http.begin(api);

int code=http.GET();

if(code==200){
String payload=http.getString();

int hIndex=payload.indexOf("hour")+7;
int mIndex=payload.indexOf("minute")+9;

String h=payload.substring(hIndex,hIndex+2);
String m=payload.substring(mIndex,mIndex+2);

String alarm=h+":"+m;

if(alarm!=lastAlarm){
lastAlarm=alarm;

Serial2.print("<");
Serial2.print(alarm);
Serial2.println(">");
}
}

http.end();
}

delay(5000);
}