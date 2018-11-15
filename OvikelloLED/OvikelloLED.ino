#include <ArduinoJson.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include "settings.h"

#define PIXELPIN D6
#define NUMPIXELS 44

int ovikello = 0;
int viive = 0;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIXELPIN, NEO_RGB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);
  strip.begin();
  strip.clear();
  strip.show();
}

void setup_wifi() {
  delay (10);
  Serial.println();
  Serial.print("Connecting");
  Serial.print(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP adress:");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  DynamicJsonBuffer  jsonBuffer(128);
  char message[128];
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  Serial.println(message);
  JsonObject& root = jsonBuffer.parseObject(message);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  if ( root["data"]["door3"] ) {
    ovikello = 1;
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe(MQTT_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
 
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (ovikello == 1) {
    int Ledit = 0;
    {
      uint32_t low = strip.Color(0, 0, 0);
      uint32_t high = strip.Color(255, 255, 255);
      uint32_t Red = strip.Color(0, 255, 0);
      uint32_t Blue = strip.Color(255, 0, 0);
      uint32_t Green = strip.Color(0, 0, 255);

      if ( viive == 0 ) viive = millis();

      if ( viive < millis() - 5000 ) {
        for ( int i = 0; i < NUMPIXELS; i++) {
          strip.setPixelColor(i, low);
          strip.show();
          ovikello = 0;
          viive = 0;
        }
      }

      else if ( viive < millis() - 4000 ) {
        for ( int i = 0; i < NUMPIXELS; i++) {
          strip.setPixelColor(i, Green);
          strip.show();
        }
      }

      else if ( viive < millis() - 3000 ) {
        for ( int i = 0; i < NUMPIXELS; i++) {
          strip.setPixelColor(i, Blue);
          strip.show();
        }
      }

      else if ( viive < millis() - 2000 ) {
        for ( int i = 0; i < NUMPIXELS; i++) {
          strip.setPixelColor(i, Red);
          strip.show();
        }
      }

      else if ( viive < millis() - 1000 ) {
        for ( int i = 0; i < NUMPIXELS; i++) {
          strip.setPixelColor(i, high);
          strip.show();
        }
      }      
    }
  }
}

