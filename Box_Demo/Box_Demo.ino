#include <Servo.h>

Servo boxservo;
#define BUTTONPIN D3

void setup () {
  boxservo.attach(D2);
  pinMode(BUTTONPIN, INPUT_PULLUP);
  
}

void loop () {
  int pos;
  int value = digitalRead(BUTTONPIN);

  pos = 0;
  boxservo.write(pos);
  delay(10000);
  

 if(value == LOW) {
  for (pos = 0; pos <= 180; pos += 1){
    boxservo.write(pos);
    delay(15);
  }
  for (pos = 180; pos >= 0;pos -= 1) {
    boxservo.write(pos);
    delay(15);
  }
  
 }

}

