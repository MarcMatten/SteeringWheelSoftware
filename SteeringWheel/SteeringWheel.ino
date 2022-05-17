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
int rClutchMinL = 200;
int rClutchMaxL = 650;
int rClutchMinR = 170;
int rClutchMaxR = 660;
int rBitePoint = 50;
bool BBothPaddlesPressed = false;
bool BStartMode = false;
unsigned long tClutchStartMode = 0;
unsigned long tBothPaddlesPressed = 0;
unsigned long tStartModeThreshold = 1000;
unsigned long tNow = 0;

// include library that turns the Pro Mirco into a gamecontroller
#include <Joystick.h>
Joystick_ Joystick;

void setup() {
  Joystick.begin();
  // Serial.begin(9600);

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
//    Joystick.pressButton(24);
    if (BBothPaddlesPressed == false) {
      BBothPaddlesPressed = true;
      tBothPaddlesPressed = tNow; 
//      Joystick.pressButton(20);
    }
  }
  else {
    BBothPaddlesPressed = false;    
//    Joystick.releaseButton(24);
    if (rClutchL <= 10 && rClutchR <= 10) {
      if (BStartMode == true) {
        BStartMode = false;
        // Serial.println("================================");
        // Serial.print("<<< START MODE OFF>>>");
        Joystick.releaseButton(23);
      }
    }
  }

  if (BBothPaddlesPressed == true) {
//    Joystick.pressButton(25);
    if ((tNow - tBothPaddlesPressed) >= tStartModeThreshold) {
      if (BStartMode == false) {
        BStartMode = true;
        // Serial.println("================================");
        // Serial.print("<<< START MODE >>>");
        Joystick.pressButton(23);
      }
    }
  }
  else {    
//    Joystick.releaseButton(23);
//    Joystick.releaseButton(25);
  }

  Joystick.setXAxis(max(rClutchL, rClutchR));

  // send game controller state to PC
  Joystick.sendState();

//  Serial.println("================================");
//  Serial.print(tNow);
//  Serial.print(" | ");
//  Serial.print(tBothPaddlesPressed);
//  Serial.print(" | ");
//  Serial.print(tStartModeThreshold);
//  Serial.print(" | ");
//  Serial.println(BBothPaddlesPressed);
  
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
