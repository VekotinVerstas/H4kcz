#include <ArduinoJson.h>
#include <stdio.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Bounce2.h>

#include "settings.h"

#define DOOR1 D7
#define DOOR2 D6
#define DOOR3 D5
#define LED_PIN D1
#define SENSOR_TYPE "door sensor"

static char esp_id[16];
uint8_t door_status = 0;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
Bounce debouncer1 = Bounce();
Bounce debouncer2 = Bounce();
Bounce debouncer3 = Bounce();

void connectWifi() {
  int count = 0;
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    count++;
    Serial.print(".");
    if ( count > 20 ) {
      ESP.restart();
    }
  }
  Serial.println("Connected to wifi");
}

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
  pinMode(DOOR3, INPUT_PULLUP);
  debouncer1.attach(DOOR1);
  debouncer1.interval(200); // interval in ms
  debouncer2.attach(DOOR2);
  debouncer2.interval(200); // interval in ms
  debouncer3.attach(DOOR3);
  debouncer3.interval(30); // interval in ms

  pinMode(LED_PIN, OUTPUT);

  WiFi.mode(WIFI_STA);
  connectWifi();
}

static void mqtt_send(const char *topic, int value)
{
  // Make sure we have wifi and if not try to get some wifi. If we do not have saved wifi settings create accespoint with esp_id and wifi_pw ( at first run login to ap and save wifi settings ).
  if (!mqttClient.connected()) {
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.connect(esp_id, MQTT_USER, MQTT_PASSWORD);
  }
  if (mqttClient.connected()) {
    char jsonMessage[256];
    DynamicJsonBuffer jsonBuffer(256);
    JsonObject& root  = jsonBuffer.createObject();
    root["chipid"] = esp_id;
    root["sensor"] = "button";
    root["millis"] = millis();

    JsonObject& data = root.createNestedObject("data");
    data["door1"] = bitRead(value, 0);
    data["door2"] = bitRead(value, 1);
    data["door3"] = bitRead(value, 2);

    //{"chipid":2057786,"":,"millis":964606330,"data":["door",25.21948,"_",0,"_",0]}
    //snprintf(jsonValue, sizeof(jsonValue), "{\"chipid\":%s,\"sensor\":\"button\",\"millis\":%d,\"data\":[\"door1\",%d,\"door2\",%d,\"door3\",%d]}", esp_id, millis(), bitRead(value, 0), bitRead(value, 1), bitRead(value, 2)  );
    Serial.print("Publishing ");
    root.prettyPrintTo(Serial);
    root.printTo(jsonMessage);
    //Serial.print(jsonValue);
    Serial.print(" to ");
    Serial.print(topic);
    Serial.print("...");
    int result = mqttClient.publish(topic, jsonMessage, true);
    Serial.println(result ? "OK" : "FAIL");
  }
}

void loop() {
  // Update the Bounce instances :
  debouncer1.update();
  debouncer2.update();
  debouncer3.update();
  
  if ( WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }
  else {
    uint8_t door_status_new = 0;
    if (debouncer3.read() == LOW) {
      digitalWrite(LED_PIN, HIGH);
    }
    else {
      digitalWrite(LED_PIN, LOW);
    }
    door_status_new = debouncer1.read();
    door_status_new += debouncer2.read() * 2;
    door_status_new += (!debouncer3.read()) * 4;
    if ( door_status != door_status_new ) {
      door_status = door_status_new;
      Serial.print("Ovien tila: " );
      Serial.println( door_status );
      mqtt_send(MQTT_TOPIC, door_status);
    }
  }
}
