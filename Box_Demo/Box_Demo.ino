#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

#include <Bounce2.h>
#include <Servo.h>
#include "secrets.h" //Create your own secrets.h from the example

Servo boxservo;

int pos;

int openendCounter;

void sendOpenedInfo () {
    
}

void setup () {
  Serial.begin(115200);
  boxservo.attach(D2);
  pinMode(D3, INPUT_PULLUP);
  WiFi.begin(WIFI_SSID, WIFI_PSWD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting....");
  }

  pos = 0;
  boxservo.write(pos);

}

void loop () {
  
  int value = digitalRead(D3);



 if(value == LOW) {
  Serial.println("Nappi painettu");
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

