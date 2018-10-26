#define pinA D1
#define pinB D2
#define buttonPin D3

int position = 0;

void setup() {
  pinMode (pinA, INPUT);
  pinMode (pinB, INPUT);

  attachInterrupt(pinA, doRotate, CHANGE);
  attachInterrupt(buttonPin, doButton, CHANGE);
  Serial.begin (115200);
}

void loop() {
}

void doRotate() {
  if (digitalRead(pinB) != digitalRead(pinA)) {
    position ++;
  } else {
    position --;
  }
  Serial.println(position);
}

void doButton() {
  Serial.println("Button");
}
