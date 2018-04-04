// MIT License
// https://github.com/gonzalocasas/arduino-uno-dragino-lorawan/blob/master/LICENSE
// Based on examples from https://github.com/matthijskooijman/arduino-lmic
// Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman

#include <Arduino.h>
#include <lmic.h> /* Kirjaston oletuksena lorabase.h:ssa MAX_LEN_FRAME = 64 muutettava 128:si */
#include <hal/hal.h>
#include <SPI.h>
#include <SSD1306Wire.h>
#include "soc/efuse_reg.h"

#include "mhz19.h"

#define LEDPIN 2

#define OLED_I2C_ADDR 0x3C
#define OLED_RESET 16
#define OLED_SDA 4
#define OLED_SCL 15

static char esp_id[16];

unsigned int counter = 0;

SSD1306Wire display (OLED_I2C_ADDR, OLED_SDA, OLED_SCL);

HardwareSerial sensor(1); //  rxPin = 9; txPin = 10;

static bool exchange_command(uint8_t cmd, uint8_t data[], int timeout)
{
    // create command buffer
    uint8_t buf[9];
    int len = prepare_tx(cmd, data, buf, sizeof(buf));

    // send the command
    sensor.write(buf, len);

    // wait for response
    long start = millis();
    while ((millis() - start) < timeout) {
        if (sensor.available() > 0) {
            uint8_t b = sensor.read();
            if (process_rx(b, cmd, data)) {
                return true;
            }
        }
    }

    return false;
}

static bool read_temp_co2(int *co2, int *temp)
{
    uint8_t data[] = {0, 0, 0, 0, 0, 0};
    bool result = exchange_command(0x86, data, 3000);
    if (result) {
        *co2 = (data[0] << 8) + data[1];
        *temp = data[2] - 40;
#if 1
        char raw[32];
        sprintf(raw, "RAW: %02X %02X %02X %02X %02X %02X", data[0], data[1], data[2], data[3], data[4], data[5]);
        Serial.println(raw);
#endif
    }
    return result;
}

/*************************************
 * TODO: Change the following keys
 * NwkSKey: network session key, AppSKey: application session key, and DevAddr: end-device address
 *************************************/
static u1_t NWKSKEY[16] = { 0x05, 0x23, 0x24, 0xc3, 0x46, 0xed, 0xd5, 0xe2, 0xfb, 0x97, 0xdb, 0x56, 0x71, 0x8f, 0xdd, 0x86 };  // Paste here the key in MSB format

static u1_t APPSKEY[16] = { 0x60, 0x02, 0x3c, 0x5f, 0xd1, 0x09, 0x8b, 0xce, 0xf7, 0x6b, 0x81, 0xd2, 0x6a, 0xce, 0x4d, 0x85 };  // Paste here the key in MSB format

static u1_t DEVEUI[16] = { 0x60, 0x45, 0xb1, 0x6b, 0x4d, 0x99, 0xac, 0x85 };

static u4_t DEVADDR = 0xe064c38e;   // Put here the device id in hexadecimal form.

void os_getDevEui(u1_t* buf) { memcpy_P(buf, DEVEUI, 8); }
void os_getArtEui(u1_t* buf) { }
//void os_getDevEui(u1_t* buf) { }
void os_getDevKey(u1_t* buf) { }

static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 60;
char TTN_response[30];

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 14,
    .dio = {26, 33, 32}  // Pins for the Heltec ESP32 Lora board/ TTGO Lora32 with 3D metal antenna
};

void do_send(osjob_t* j){
    // Payload to send (uplink)
    int co2, temp;
    if (read_temp_co2(&co2, &temp)) {
        Serial.print("CO2:");
        Serial.println(co2, DEC);
        Serial.print("TEMP:");
        Serial.println(temp, DEC);
    }
    char message[110];
    snprintf(message, strlen(message), "{\"chipid\":%s,\"sensor\":\"BKS\",\"millis\":%d,\"data\":[\"%s\\
",%d,\"_\",0,\"_\",0]}", esp_id, millis(), "co2", co2 );

    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, (uint8_t *)message, sizeof(message)-1, 0);
        Serial.println(F("Sending uplink packet..."));
        digitalWrite(LEDPIN, HIGH);
        display.clear();
        display.drawString (0, 0, "Sending uplink packet...");
        display.drawString (0, 50, String (++counter));
        display.display ();
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void onEvent (ev_t ev) {
    if (ev == EV_TXCOMPLETE) {
        display.clear();
        display.drawString (0, 0, "EV_TXCOMPLETE event!");


        Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
        if (LMIC.txrxFlags & TXRX_ACK) {
          Serial.println(F("Received ack"));
          display.drawString (0, 20, "Received ACK.");
        }

        if (LMIC.dataLen) {
          int i = 0;
          // data received in rx slot after tx
          Serial.print(F("Data Received: "));
          Serial.write(LMIC.frame+LMIC.dataBeg, LMIC.dataLen);
          Serial.println();

          display.drawString (0, 20, "Received DATA.");
          for ( i = 0 ; i < LMIC.dataLen ; i++ )
            TTN_response[i] = LMIC.frame[LMIC.dataBeg+i];
          TTN_response[i] = 0;
          display.drawString (0, 32, String(TTN_response));
        }

        // Schedule next transmission
        os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
        digitalWrite(LEDPIN, LOW);
        display.drawString (0, 50, String (counter));
        display.display ();
    }
}

int getChipRevision()
{
  return (REG_READ(EFUSE_BLK0_RDATA3_REG) >> (EFUSE_RD_CHIP_VER_RESERVE_S)&&EFUSE_RD_CHIP_VER_RESERVE_V) ;
}

void printESPRevision() {
  Serial.print("REG_READ(EFUSE_BLK0_RDATA3_REG) ");
  Serial.println(REG_READ(EFUSE_BLK0_RDATA3_REG), BIN);

  Serial.print("EFUSE_RD_CHIP_VER_RESERVE_S ");
  Serial.println(EFUSE_RD_CHIP_VER_RESERVE_S, BIN);

  Serial.print("EFUSE_RD_CHIP_VER_RESERVE_V ");
  Serial.println(EFUSE_RD_CHIP_VER_RESERVE_V, BIN);

  Serial.println();

  Serial.print("Chip Revision (official version): ");
  Serial.println(getChipRevision());

  Serial.print("Chip Revision from shift Operation ");
  Serial.println(REG_READ(EFUSE_BLK0_RDATA3_REG) >> 15, BIN);

}

void setup() {
    Serial.begin(115200);
    sensor.begin(9600, SERIAL_8N1, 14, 12);
    delay(1500);   // Give time for the seral monitor to start up
    Serial.println(F("Starting..."));

    printESPRevision();

    uint64_t chipid;  
    chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
    Serial.printf("ESP32 Chip ID = %04X",(uint16_t)(chipid>>32));//print High 2 bytes
    Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.
    sprintf(esp_id, "%08X", (uint32_t)chipid);

    // Use the Blue pin to signal transmission.
    pinMode(LEDPIN,OUTPUT);

   // reset the OLED
   pinMode(OLED_RESET,OUTPUT);
   digitalWrite(OLED_RESET, LOW);
   delay(50);
   digitalWrite(OLED_RESET, HIGH);

   display.init ();
   display.flipScreenVertically ();
   display.setFont (ArialMT_Plain_10);

   display.setTextAlignment (TEXT_ALIGN_LEFT);

   display.drawString (0, 0, "Init!");
   display.display ();

    // LMIC init
    os_init();

    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Set up the channels used by the Things Network, which corresponds
    // to the defaults of most gateways. Without this, only three base
    // channels from the LoRaWAN specification are used, which certainly
    // works, so it is good for debugging, but can overload those
    // frequencies, so be sure to configure the full frequency range of
    // your network here (unless your network autoconfigures them).
    // Setting up channels should happen after LMIC_setSession, as that
    // configures the minimal channel set.
    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
    LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
    // TTN defines an additional channel at 869.525Mhz using SF9 for class B
    // devices' ping slots. LMIC does not have an easy way to define set this
    // frequency and support for class B is spotty and untested, so this
    // frequency is not configured here.

    // Set static session parameters.
    LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);

    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
    //LMIC_setDrTxpow(DR_SF11,14);
    LMIC_setDrTxpow(DR_SF9,14);

    // Start job
    do_send(&sendjob);
}

void loop() {
    os_runloop_once();
}

