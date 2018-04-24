#include <Ticker.h>


Ticker secondTick;
      int wt = 0;
void wtl(){
  wt++;
  if (wt == 70){
     digitalWrite(D1, LOW);
  delay(2000);
  Serial.println("wt reset");
  digitalWrite(D1, HIGH);
  delay(2000);

    
  }
}

void rst(){
  
  if (digitalRead(D5)== LOW){
    Serial.println("pulssi vastaanotetu");
    secondTick.attach(1,wtl);
  }
}

void setup() {
  pinMode(D5, INPUT_PULLUP);
  pinMode(D1, OUTPUT);
  Serial.begin(115200);
  secondTick.attach(1,wtl);
 digitalWrite(D1, HIGH);
}

void loop() {
   rst();

}
