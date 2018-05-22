#include <Ticker.h>

//ESP-01
#define pulsePin = 2;
#define resetPin = 0;

//Wemos
//#define pulsePin = D5;
//#define resetPin = D1;

Ticker secondTick;
uint8_t oldpin = LOW;
uint8_t resetTime = 70;
uint32_t pinLowTime=0;

void resetMaster() {
    digitalWrite(0, LOW);
    pinLowTime = millis();
    Serial.println("Watchdog time out. Reset master.");
}

void setup() {
  Serial.begin(115200);
  pinMode(2, INPUT_PULLUP);
  pinMode(0, OUTPUT);
  Serial.println("Started to follow pulses from master board");
  Serial.print("Watchdog countdown set to ");
  Serial.println(resetTime);
  secondTick.attach(resetTime, resetMaster);
  digitalWrite(0, HIGH);
}

void loop() {
  uint8_t readpin = digitalRead(2);
  if( (pinLowTime != 0) and (millis() - pinLowTime > 150) ) {
    digitalWrite(0, HIGH);
    pinLowTime=0;
    Serial.print("Watchdog countdown reset to ");
    Serial.println(resetTime);
    secondTick.attach(resetTime, resetMaster);    
  }
  
  if ((readpin == LOW) && (oldpin==HIGH)) {
    Serial.print("Pulse from master reseived. Watchdog countdown reset to ");
    Serial.print("");
    Serial.println(resetTime);
    secondTick.attach(resetTime, resetMaster);
  }
oldpin=readpin;  
}


