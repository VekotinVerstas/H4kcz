#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <Bounce2.h>
#include <Servo.h>
#include "secrets.h" //Create your own secrets.h from the example


#define SENSOR_TYPE "Box sensor"

static char esp_id[16];

int EveryOpenCounter;

Servo boxservo;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

int pos;

int addr = 0;


void setup () {
  Serial.begin(115200);
  EEPROM.begin(512);

  sprintf(esp_id, "%08X", ESP.getChipId());
  Serial.print("ESP ID: ");
  Serial.println(esp_id);

  boxservo.attach(D2);
  pinMode(D3, INPUT_PULLUP);
  pinMode(D7, OUTPUT);
  pinMode(D5, OUTPUT);
  
  WiFi.begin(WIFI_SSID, WIFI_PSWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Connecting....");
  }
  
  Serial.println("Connected to WiFi");
  pos = 0;
  boxservo.write(pos);

}

void loop () {
  int openedCounter;
  
  int Button_value = digitalRead(D3);



 if(Button_value == LOW) {
  Serial.println("Nappi painettu");
  for (pos = 0; pos <= 180; pos += 1){
    boxservo.write(pos);
    delay(15);
    
  }
   digitalWrite(D7, HIGH);
   delay(1000);
   digitalWrite(D5, HIGH);
   delay(1000);
   digitalWrite(D7, LOW);
   delay(1000);
   digitalWrite(D5, LOW);
   delay(1000);
  
   for (pos = 180; pos >= 0;pos -= 1) {
    boxservo.write(pos);
    delay(15);
  }
  openedCounter++;
  EveryOpenCounter++;

  EEPROM.write(addr, EveryOpenCounter);

  addr = addr + 1;
  if (addr= 512) {
    addr = 0;
    EEPROM.commit();
  }
  Serial.println(EveryOpenCounter);
  
  mqtt_send(MQTT_TOPIC, openedCounter);
 }

}
static void mqtt_send(const char *topic, int value) {
  if (!mqttClient.connected()) {
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.connect(esp_id, MQTT_USER, MQTT_PASSWORD);
  }
  if (mqttClient.connected()) {
    char jsonMessage[256];
    DynamicJsonBuffer jsonBuffer(256);
    JsonObject& root = jsonBuffer.createObject();
    root["chipid"] = esp_id;
    root["sensor"] = "Box";
    root["millis"] = millis();

    JsonObject& data = root.createNestedObject("data");
    data["Opened"] = value;

    Serial.print("Publishing: ");
    root.prettyPrintTo(Serial);
    root.printTo(jsonMessage);
    Serial.print(" to ");
    Serial.print(topic);
    Serial.print("...");
    int result = mqttClient.publish(topic, jsonMessage, true);
    Serial.println(result ? "OK" : "FAIL");
  }
}

void writeLedLow(int ledpin) {
  digitalWrite(ledpin, HIGH);
}

void writeLedHigh(int ledpin) {
  digitalWrite(ledpin, LOW);  
}

