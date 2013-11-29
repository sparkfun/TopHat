/*
 This is the original code written by Dave Clausen http://dclausen.net/ for the wedding top hat.
 
 https://learn.sparkfun.com/tutorials/das-blinken-top-hat
*/


#include <avr/pgmspace.h>
#include <math.h>

const int sensorPinX = A3;    // X axis of ADXL335 accelerometer
const int sensorPinY = A2;    // Y axis
const int sensorPinZ = A1;    // Z axis
const int sensorBias = 512;
const int sensorScale = 4;
const int onboardLedPin = 13; // LED on the arduino mini
const int firstLedPin = 2;    // first MOSFET-controlled output pin, which controls a strip of LEDs
const int ledPinCount = 8;    // number of MOSFET-controlled output pins
float angle = 0; // position of virtual mass
float velocity = 0; // velocity of virtual mass
const float friction = 0.995;

void writeLed(byte pin, boolean val) {
  digitalWrite((pin & 7) + firstLedPin, val);
}

int readAccel(int pin) {
  return analogRead(pin) - sensorBias;
}


void toggleOnboardLed() {
  static bool on = false;
  digitalWrite(onboardLedPin, on);  
  on = !on;
}

int angleToLed(float angle) {
  int led = angle * (8 / 3.14159265359);
  return led & 7;
}

void blink_out(int count) {
  for (int j = 0; j < count; j++) {
    for (int i = 0; i < ledPinCount; i++) {
      writeLed(i, HIGH);
    }
    delay(random(10, 500));
    for (int i = 0; i < ledPinCount; i++) {
      writeLed(i, LOW);
    }
    delay(random(100, 500));
  }
}


void setup() {
  pinMode(onboardLedPin, OUTPUT);  
  digitalWrite(onboardLedPin, LOW);   
  
  for (int i = 0; i < ledPinCount; i++) {
    int pin = firstLedPin + i;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);   
  }

  randomSeed((analogRead(A0) * 13) + (analogRead(A1) * 27) + (analogRead(A2 * 157)) + (analogRead(A3 * 49)));

/*
  Serial.begin(9600);

  for (int j = 0; j < 10; j++) {
    for (int i = 0; i < ledPinCount; i++) {
      writeLed(i, HIGH);
      delay(10 * j);          
      writeLed(i, LOW);
    }
  }
*/
}

float old_fx = 0;
float old_fy = 0;
float old_fz = 0;
bool inverted = false;
bool reverse = false;
bool dbl = true;

void loop() {
  toggleOnboardLed();
  long r = random(10000);
  switch (r) {
    case 0:
      inverted = true;
      break;
    case 1:
      reverse = !reverse;
      break;
    case 2:
      velocity = 1.0;
      break;
    case 3:
      blink_out(random(2,10));
      break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
      inverted = false;
      break;
    case 11:
      dbl = true;
      break;
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
      dbl = false;
      break;
  }
  
  float fx = readAccel(sensorPinX);
  float fy = readAccel(sensorPinY);
  float fz = readAccel(sensorPinZ);
  
  float dfx = abs(fx - old_fx);
  float dfy = abs(fy - old_fy);
  float dfz = abs(fz - old_fz);
  
  old_fx = fx;
  old_fy = fy;
  old_fz = fz;
  
  float df = sqrt((dfx * dfx) + (dfy * dfy) + (dfz * dfz));
  
  float scale = 10000.0;
  if (reverse) {
    velocity = velocity - (df / scale);
  } else {
    velocity = velocity + (df / scale);
  }
  velocity = velocity * friction;
  velocity = min(velocity, 0.5);
  
  int old_led = angleToLed(angle);

  angle = angle + velocity;
  while (angle > (2 * 3.14159265359)) {
    angle = angle - (2 * 3.14159265359);
  }
  while (angle < 0.0) {
    angle = angle + (2 * 3.14159265359);
  }
  int new_led = angleToLed(angle);
  if (new_led != old_led) {
    writeLed(old_led, inverted ? HIGH : LOW);
    writeLed(new_led, inverted ? LOW : HIGH);
    if (dbl) {
      writeLed(old_led + 4, LOW);
      writeLed(new_led + 4, HIGH);
    } else if (inverted) {
      if (reverse) {
        writeLed(new_led - 1, LOW);
      } else {
        writeLed(new_led + 1, LOW);
      }
    }
  }
  delay(10);
}
