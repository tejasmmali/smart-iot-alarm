#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27,16,2);
SoftwareSerial esp(10,11);

String data="";
bool ready=false;

int alarmH=0;
int alarmM=0;

void setup(){
lcd.init();
lcd.backlight();
esp.begin(9600);

lcd.print("Internet Alarm");
}

void loop(){

while(esp.available()){
char c=esp.read();

if(c=='<') data="";
else if(c=='>'){ ready=true; break; }
else data+=c;
}

if(ready){
ready=false;

int colon=data.indexOf(':');
alarmH=data.substring(0,colon).toInt();
alarmM=data.substring(colon+1).toInt();

lcd.clear();
lcd.print("Alarm Set:");
lcd.setCursor(0,1);
lcd.print(data);
}

}