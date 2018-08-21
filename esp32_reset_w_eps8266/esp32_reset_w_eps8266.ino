#include <Ticker.h>

/* Define WEMOS for WEMOS and WEMOS pro boards. Comment define out for ESP-01 boards.*/
//#define WEMOS

#ifdef WEMOS
uint8_t pulsePin = D5;
uint8_t resetPin = D1;
#else
// Use generic ESP8266 board ESP-01
uint8_t pulsePin = 2;
uint8_t resetPin = 0;
#endif

Ticker secondTick, msTick;
uint8_t oldpin = LOW;
uint16_t resetTime = 13*60; // 13*60 sec = 13 min

void startMasterReset() {
  digitalWrite(resetPin, LOW);
  Serial.println("Watchdog time out. Reset master.");
  msTick.once_ms(120, stopMasterReset );
}

void stopMasterReset() {
  digitalWrite(resetPin, HIGH);
  Serial.print("Watchdog countdown reset to ");
  Serial.println(resetTime);
  secondTick.attach(resetTime, startMasterReset);
}

void setup() {
  Serial.begin(115200);
  pinMode(pulsePin, INPUT_PULLUP);
  pinMode(resetPin, OUTPUT);
  Serial.println("Started to follow pulses from master board");
  Serial.print("Watchdog countdown set to ");
  Serial.println(resetTime);
  secondTick.attach(resetTime, startMasterReset);
  digitalWrite(resetPin, HIGH);
}

void loop() {
  uint8_t readpin = digitalRead(pulsePin);
  if ((readpin == LOW) && (oldpin == HIGH)) {
    Serial.print("Pulse from master reseived. Watchdog countdown reset to ");
    Serial.print("");
    Serial.println(resetTime);
    secondTick.attach(resetTime, startMasterReset);
  }
  oldpin = readpin;
}

