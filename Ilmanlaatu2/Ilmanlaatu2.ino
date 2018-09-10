/* 
 *  Code based for Somnath Bera code at 
 *  https://electronicsforu.com/electronics-projects/esp32-particulate-matter-monitoring/2
 */
   
#define RXD2 39
#define TXD2 36

HardwareSerial Serial2(2);
unsigned int Pm25 = 0;
unsigned int Pm10 = 0;

float humity;

void ReadAndPrintSDS011() {
  uint8_t mData = 0;
  uint8_t i = 0;
  uint8_t mPkt[10] = {0};
  uint8_t mCheck = 0;
  while (Serial2.available() > 0)  {
    // packet format: AA C0 PM25_Low PM25_High PM10_Low PM10_High 0 0 CRC AB
    mData = Serial2.read();
    delay(2);//wait until packet is received
    //    Serial.println(mData);
    //    Serial.println("*");
    if (mData == 0xAA) //head1 ok
    {
      mPkt[0] =  mData;
      mData = Serial2.read();
      if (mData == 0xc0) //head2 ok
      {
        mPkt[1] =  mData;
        mCheck = 0;
        for (i = 0; i < 6; i++) //data recv and crc calc
        {
          mPkt[i + 2] = Serial2.read();
          delay(2);
          mCheck += mPkt[i + 2];
        }
        mPkt[8] = Serial2.read();
        delay(1);
        mPkt[9] = Serial2.read();
        if (mCheck == mPkt[8]) //crc ok
        {
          Serial2.flush();
          Serial2.write(mPkt, 10);

          Pm25 = (uint16_t)mPkt[2] | (uint16_t)(mPkt[3] << 8);
          Pm10 = (uint16_t)mPkt[4] | (uint16_t)(mPkt[5] << 8);
          if (Pm25 > 9999)
            Pm25 = 9999;
          if (Pm10 > 9999)
            Pm10 = 9999;
          //get one good packet

          // Humity normalization function written by Zbyszek Kiliański, Piotr Paul
          // https://github.com/piotrkpaul/esp8266-sds011
          //Pm25 = Pm25 / (1.0 + 0.48756 * pow((humity / 100.0), 8.60068));
          //Pm10 = Pm10 / (1.0 + 0.81559 * pow((humity / 100.0), 5.83411));

          Serial.println();
          Serial.print("Pm2.5 ");
          Serial.print(float(Pm25) / 10.0);
          Serial.println();//  Display();
          Serial.print("Pm10 ");
          Serial.println(float(Pm10) / 10.0);
          Serial.println();
          Serial2.flush();
          //return;
        }
      }
    }
  }
}

static const byte SLEEPCMD[19] = {
  0xAA, // head
  0xB4, // command id
  0x06, // data byte 1
  0x01, // data byte 2 (set mode)
  0x00, // data byte 3 (sleep)
  0x00, // data byte 4
  0x00, // data byte 5
  0x00, // data byte 6
  0x00, // data byte 7
  0x00, // data byte 8
  0x00, // data byte 9
  0x00, // data byte 10
  0x00, // data byte 11
  0x00, // data byte 12
  0x00, // data byte 13
  0xFF, // data byte 14 (device id byte 1)
  0xFF, // data byte 15 (device id byte 2)
  0x05, // checksum
  0xAB // tail
};

void turnoff() {
  Serial2.flush();
  for (uint8_t j = 0; j < 19; j++) Serial2.write(SLEEPCMD[j]);
}

void turnon() {
  Serial2.write(0x01);
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  //  Serial.println("Serial Txd is on pin: "+String(TX));
  //  Serial.println("Serial Rxd is on pin: "+String(RX));
  Pm25 = 0;
  Pm10 = 0;
  turnon();
}

void loop() {
  delay(2000);
  ProcessSerialData();


  //turnoff();
}

