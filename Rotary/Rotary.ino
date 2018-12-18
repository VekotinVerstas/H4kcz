/**************************************************************************************
   Minimal code to demonstrate rotary knob usage.
   Copyright 2018 Aki Salminen / Vekotinverstas / Forum Virium Helsinki / MIT license

   ToDo:

   Rotary pinA and PinB could benefit from debounce too. Rotary code is bad on fast rotation.
***************************************************************************************/

#define pinA D1  //CLK
#define pinB D2  // DT
#define buttonPin D3  //SW
// +   -> 3.3V
// GND -> GND

volatile int16_t position = 0;
volatile uint16_t bouncedelay = 0;
volatile int8_t change = 0;

void setup() {
  pinMode (pinA, INPUT);
  pinMode (pinB, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(pinA), doRotate, CHANGE);
  attachInterrupt(digitalPinToInterrupt(buttonPin), doButton, FALLING);
  Serial.begin (115200);
}

void loop() {
  // If Rotary interrupt has happened
  if ( change ) {
    change = 0;
    if (digitalRead(pinB) != digitalRead(pinA)) {
      position ++;
    } else {
      position --;
    }
    // Do your rotary actions in here
    Serial.println(position);
  }
  deBounce(); // Button actions in deBounce
}

// Rotary interrupt
void doRotate() {
  change = 1;
}

// Button interrupt
void doButton() {
  if ( !bouncedelay ) {
    bouncedelay = 6000; // 6000 is tricker value for main loop to start count
  }
}

// Button debounce and actions in main loop as millis and others behave strange in interrupt vector
void deBounce() {
  if ( bouncedelay ) {
    if ( bouncedelay == 6000 ) { // 6000 is just random tricker value to start counting. This is also better place to execute code than interrupt vector.
      bouncedelay = millis();
      // Add your button activity here
      Serial.println("Button pressed.");
    }
    else if ( (bouncedelay + 200) < millis() or millis() < bouncedelay ) { //millis can loop back to 0
      bouncedelay = 0;
    }
  }
}
