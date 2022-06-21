// Pin Mapping
int NButtonPin[] = {A3, 3, 4, 5, 6, 7, 8, 9, 16, 14, 10, 15};
int NPinClutchL = A0;
int NPinClutchR = A1;

int NButtons = 12; // number of buttons defined in NButtonPin

// global for Clutch Paddle logic
int rClutchRemappedL = 0;
int rClutchRemappedR = 0;
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

// timer settings for button latch and trashold times
unsigned long tButtonThreshold[] = {100, 100, 500, 100, 250, 100, 100, 100, 100, 0, 100, 0};
unsigned long tButtonPressed[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long tButtonSet[] = {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 0, 32};

// buffer definition for serial read
const int BUFFER_SIZE = 4;
byte buf[BUFFER_SIZE];

// include library that turns the Pro Mirco into a gamecontroller
#include <Joystick.h>
#include <stdlib.h>
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, 
  JOYSTICK_TYPE_GAMEPAD, 40, 0,
  true, false, false, false, false, false,
  false, false, false, false, false);

void setup() {
  Joystick.begin(false);
  Serial.begin(9600);

  // define pins for buttons
  for (int i = 0; i < NButtons; i++) {
    pinMode(NButtonPin[i], INPUT_PULLUP);    
  }
}

void loop() {
  unsigned long tStartLoop = micros(); // for timer; 21.06.22: 670 µs

  // read bite point from serial
  if (Serial.available() == BUFFER_SIZE) {
    buf[0] = Serial.read();
    buf[1] = Serial.read();
    buf[2] = Serial.read();
    buf[3] = Serial.read();
    rBitePoint = *((float*)buf);

    byte * b = (byte *) &rBitePoint;
    Serial.write(b,4);
  }

  // call logic for each button
  for (int i = 0; i < NButtons; i++) {
    Button(i, digitalRead(NButtonPin[i])); 
  }

  // read raw clutch signals and pass them to the clutch logic
  int rClutch = Clutch(analogRead(NPinClutchL), analogRead(NPinClutchR), tStartLoop/1000);

  // set clutch output
  Joystick.setXAxis(rClutch);
  
  // send game controller state to PC
  Joystick.sendState();
  Serial.println(micros() - tStartLoop);
  
  delay(5); // allow for enough time to finish previous sending
  
}


void Button(int NButton, int ButtonState) {
  if (ButtonState == LOW) // button pressed
  {
    // threshold logic
    if (tButtonPressed[NButton] == 0) {tButtonPressed[NButton] = millis();}
    if (millis() - tButtonPressed[NButton] > tButtonThreshold[NButton]) {
      Joystick.pressButton(NButton);
    }
  }
  else // button released
  {
    // latching logic
    if (millis() - tButtonPressed[NButton] - tButtonThreshold[NButton] > tButtonSet[NButton]){
      Joystick.releaseButton(NButton);
      tButtonPressed[NButton] = 0;
    }
  }
}


int Clutch(int rClutchRawL, int rClutchRawR, unsigned long tNow) {  
  int rClutch = 0;
  
  // re-map clutch paddles to calibrated range
  int rClutchL = map(rClutchRawL, rClutchMinL, rClutchMaxL, 0, 1023);
  int rClutchR = map(rClutchRawR, rClutchMinR, rClutchMaxR, 0, 1023);

  // unsigned long tNow = millis();

  // start mode logic
  if (rClutchL >= 1000 && rClutchR >= 1000) {
    if (BBothPaddlesPressed == false) {
      // start timer when both paddles are pressed
      BBothPaddlesPressed = true;
      tBothPaddlesPressed = tNow;
    }
  }
  else {
    // paddle release
    BBothPaddlesPressed = false;
    if (rClutchL <= 10 && rClutchR <= 10) {
      if (BStartMode == true) {
        // reset start mode when both paddles released 
        BStartMode = false;
        BLeftIsBitePaddle = false;
        BRightIsBitePaddle = false;
        Serial.write(false);
      }
    }
  }

  // engage start mode when both paddles fully engaged for long enough
  if (BBothPaddlesPressed == true) {
    if ((tNow - tBothPaddlesPressed) >= tStartModeThreshold) {
      if (BStartMode == false) {
        BStartMode = true;
        Serial.write(true);
      }
    }
  }
  
  if (BStartMode == true) {
    // bite point mode logic
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
      // when in start mode and one paddle gets released, the other controls the bit point
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
    // when not in start model take max input
    rClutch = max(rClutchL, rClutchR);    
  }

  return rClutch;
}
