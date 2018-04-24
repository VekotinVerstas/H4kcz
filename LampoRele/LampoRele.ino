/***************************************************************************
  This is a library for the BME280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2650

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface. The device's I2C address is either 0x76 or 0x77.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme1; // I2C
Adafruit_BME280 bme2; // I2C

unsigned long delayTime = 5000;

void setup() {
    Serial.begin(9600);
    Serial.println(F("BME280 test"));
    //SPI: SDA/SCL default to pins 4 & 5 but any two pins can be assigned as SDA/SCL using Wire.begin(SDA,SCL)
    bool status1, status2;
    pinMode(D1, OUTPUT);

    Wire.begin(D4,D5);
    // default settings
    // (you can also pass in a Wire library object like &Wire2)
    status1 = bme1.begin(0x77); // 76  
    status2 = bme2.begin(0x76); 
    if (!status1) {
        Serial.println("Sisä lämpöanturia ei löydy. Tarksita kytkentä.");
        //while (1);
    }
    if (!status2) {
        Serial.println("Ulko lämpöanturia ei löydy. Tarksita kytkentä.");
        //while (1);
    }
    
}

void loop() { 
    printValues();
    delay(delayTime);
}

void printValues() {
    float sisalampo, ulkolampo;
    Serial.print("Sisä lämpö = ");
    Serial.print(sisalampo=bme1.readTemperature());
    Serial.print(" *C");

    Serial.print("  Sisä paine = ");

    Serial.print(bme1.readPressure() / 100.0F);
    Serial.print(" hPa");

    /*Serial.print("Approx. Altitude = ");
    Serial.print(bme1.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");
		*/
		
    Serial.print("  Sisä kosteus = ");
    Serial.print(bme1.readHumidity());
    Serial.println(" %");

    Serial.print("Ulko lämpö = ");
    Serial.print(ulkolampo=bme2.readTemperature());
    Serial.print(" *C");

    Serial.print("  Ulko paine = ");

    Serial.print(bme2.readPressure() / 100.0F);
    Serial.print(" hPa");

    /*Serial.print("Approx. Altitude = ");
    Serial.print(bme1.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");
    */
    Serial.print("  Ulko kosteus = ");
    Serial.print(bme2.readHumidity());
    Serial.println(" %");

    if( (sisalampo < 20) && (sisalampo < (ulkolampo+5)) ) {
        Serial.println("Lämmitys päällä");
        digitalWrite(D1, HIGH);
    }
    else {
        Serial.println("Lämmitys pois");        
        digitalWrite(D1, LOW);
    }
}
