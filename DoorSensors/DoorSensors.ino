/**************************************************************************************
   Sketch to report door magnetic sensor state via MQTT.
   Copyright 2017 Aki Salminen / Vekotinverstas / MIT license

   The idea is to read door1 and door2 ports and report any change to MQTT & serial and auto recover from connection failures.
 
 **************************************************************************************/

#include <stdio.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

#include "settings.h"

#define DOOR1 D2
#define DOOR2 D5
#define SENSOR_TYPE "door sensor"

static char esp_id[16];
uint8_t door_status = 0;

WiFiManager wifiManager;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("ESP ");
  Serial.println(SENSOR_TYPE);

  sprintf(esp_id, "%08X", ESP.getChipId());
  Serial.print("ESP ID: ");
  Serial.println(esp_id);

  pinMode(DOOR1, INPUT_PULLUP);        // sets the digital pin 0 as input
  pinMode(DOOR2, INPUT_PULLUP);        // sets the digital pin 0 as input
}

static void mqtt_send(const char *topic, int value, const char *unit)
{
		// Make sure we have wifi and if not try to get some wifi. If we do not have saved wifi settings create accespoint with esp_id and wifi_pw ( at first run login to ap and save wifi settings ).
    wifiManager.autoConnect(esp_id, WIFI_PASSWORD);   
    if (!mqttClient.connected()) {
        mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
        mqttClient.connect(esp_id, MQTT_USER, MQTT_PASSWORD);
    }
    if (mqttClient.connected()) {
        char jsonValue[256];
        //{"chipid":2057786,"sensor":"button","millis":964606330,"data":["door",25.21948,"_",0,"_",0]}
        snprintf(jsonValue, sizeof(jsonValue), "{\"chipid\":%s,\"sensor\":\"button\",\"millis\":%d,\"data\":[\"%s\",%d,\"_\",0,\"_\",0]}", esp_id, millis(), unit, value );
        Serial.print("Publishing ");
        Serial.print(jsonValue);
        Serial.print(" to ");
        Serial.print(topic);
        Serial.print("...");
        int result = mqttClient.publish(topic, jsonValue, true);
        Serial.println(result ? "OK" : "FAIL");
    }
}

void loop() {
  uint8_t door_status_new=0;
  door_status_new = digitalRead(DOOR1);
  door_status_new += digitalRead(DOOR2)*2;
  if( door_status != door_status_new ) {
    door_status = door_status_new;
    Serial.print("Ovien tila: " );
    Serial.println( door_status );
    mqtt_send(MQTT_TOPIC, door_status, "door");
  }
}
