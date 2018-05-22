#include <Ticker.h>

//ESP-01
#define pulsePin = 2;
#define resetPin = 0;

//Wemos
//#define pulsePin = D5;
//#define resetPin = D1;

Ticker secondTick, msTick;
uint8_t oldpin = LOW;
uint8_t resetTime = 70; //sec

void startMasterReset() {
    digitalWrite(0, LOW);
    Serial.println("Watchdog time out. Reset master.");
    msTick.once_ms(120, stopMasterReset );
}

void stopMasterReset() {
    digitalWrite(0, HIGH);
    Serial.print("Watchdog countdown reset to ");
    Serial.println(resetTime);
    secondTick.attach(resetTime, startMasterReset);    
}

void setup() {
  Serial.begin(115200);
  pinMode(2, INPUT_PULLUP);
  pinMode(0, OUTPUT);
  Serial.println("Started to follow pulses from master board");
  Serial.print("Watchdog countdown set to ");
  Serial.println(resetTime);
  secondTick.attach(resetTime, startMasterReset);
  digitalWrite(0, HIGH);
}

void loop() {    
  uint8_t readpin = digitalRead(2);
  if ((readpin == LOW) && (oldpin==HIGH)) {
    Serial.print("Pulse from master reseived. Watchdog countdown reset to ");
    Serial.print("");
    Serial.println(resetTime);
    secondTick.attach(resetTime, startMasterReset);
  }
oldpin=readpin;  
}

