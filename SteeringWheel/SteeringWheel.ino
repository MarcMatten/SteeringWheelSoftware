// Pin Mapping
int NButtonPin[] = {A3, 3, 4, 5, 6, 7, 8, 9, 16, 14, 10, 15};
int NPinClutchL = A0;
int NPinClutchR = A1;

int NButtons = 12; // number of buttons defined in NButtonPin

// variable to store Clutch Paddle reading
int NClutchL = 0;
int NClutchR = 0;
int rClutchL = 0;
int rClutchR = 0;
int rClutchRemappedL = 0;
int rClutchRemappedR = 0;
int rClutch = 0;
int rClutchMinL = 200;
int rClutchMaxL = 650;
int rClutchMinR = 170;
int rClutchMaxR = 660;
float rBitePoint = 0.5;
bool BBothPaddlesPressed = false;
bool BStartMode = false;
bool BLeftIsBitePaddle = false;
bool BRightIsBitePaddle = false;
unsigned long tClutchStartMode = 0;
unsigned long tBothPaddlesPressed = 0;
unsigned long tStartModeThreshold = 1000;
unsigned long tNow = 0;

// include library that turns the Pro Mirco into a gamecontroller
#include <Joystick.h>
Joystick_ Joystick;

void setup() {
  Joystick.begin();

  for (int i = 0; i < NButtons; i++) {
    pinMode(NButtonPin[i], INPUT_PULLUP);    
  }
}

void loop() {
  for (int i = 0; i < NButtons; i++) {
    Button(i, digitalRead(NButtonPin[i])); 
  }

  NClutchL = analogRead(NPinClutchL);
  NClutchR = analogRead(NPinClutchR);

  rClutchL = map(NClutchL, rClutchMinL, rClutchMaxL, 0, 1023);
  rClutchR = map(NClutchR, rClutchMinR, rClutchMaxR, 0, 1023);

  tNow = millis();

  if (rClutchL >= 1000 && rClutchR >= 1000) {
    if (BBothPaddlesPressed == false) {
      BBothPaddlesPressed = true;
      tBothPaddlesPressed = tNow;
    }
  }
  else {
    BBothPaddlesPressed = false;
    if (rClutchL <= 10 && rClutchR <= 10) {
      if (BStartMode == true) {
        BStartMode = false;
        BLeftIsBitePaddle = false;
        BRightIsBitePaddle = false;
      }
    }
  }

  if (BBothPaddlesPressed == true) {
    if ((tNow - tBothPaddlesPressed) >= tStartModeThreshold) {
      if (BStartMode == false) {
        BStartMode = true;
      }
    }
  }

  if (BStartMode == true) {
    if (BLeftIsBitePaddle == true || BRightIsBitePaddle == true) {
      if (BLeftIsBitePaddle == true) {
          rClutchRemappedL = map(rClutchL, 0, 1023, 0, 1023*rBitePoint);
          rClutchRemappedR = map(rClutchR, 0, 1023, 0, 1023);
      }
      else if (BRightIsBitePaddle == true) {
          rClutchRemappedL = map(rClutchL, 0, 1023, 0, 1023);
          rClutchRemappedR = map(rClutchR, 0, 1023, 0, 1023*rBitePoint);
      }
      rClutch = max(rClutchRemappedL, rClutchRemappedR);      
    }
    else {
      if (rClutchL + 50 < rClutchR) {
        BRightIsBitePaddle = true;
      }
      else if (rClutchR + 50 < rClutchL) {
        BLeftIsBitePaddle = true;
      }
      rClutch = max(rClutchL, rClutchR);
    }
  }
  else {
    rClutch = max(rClutchL, rClutchR);    
  }

  Joystick.setXAxis(rClutch);

  // send game controller state to PC
  Joystick.sendState();
  
  delay(10);
}

void Button(int NButton, int ButtonState) {
  if (ButtonState == LOW)
  {
    Joystick.pressButton(NButton);
  }
  else
  {
    Joystick.releaseButton(NButton);
  }
}
