#include <Ticker.h>
#include <esp.h>

Ticker secondTick;
uint8_t oldpin = LOW;
uint8_t resetTime = 10;
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
  Serial.println("Start watchdog");
  Serial.print("222Reset watchdog countdown to ");
  Serial.println(resetTime);
  secondTick.attach(resetTime, resetMaster);
  digitalWrite(0, HIGH);
}

void loop() {
  uint8_t readpin = digitalRead(2);
  if( (pinLowTime != 0) and (millis() - pinLowTime > 150) ) {
    digitalWrite(0, HIGH);
    pinLowTime=0;
    Serial.print("111Reset watchdog countdown to ");
    Serial.println(resetTime);
    secondTick.attach(resetTime, resetMaster);    
  }
  
  if ((readpin == LOW) && (oldpin==HIGH)) {
    Serial.println("Watchdog reset pulse reseived.");
    Serial.print("Reset watchdog countdown to ");
    Serial.println(resetTime);
    secondTick.attach(resetTime, resetMaster);
  }
oldpin=readpin;  
}


