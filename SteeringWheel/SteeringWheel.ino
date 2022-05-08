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

  rClutchL = map(NClutchL, 188, 633, 0, 1023);
  rClutchR = map(NClutchR, 156, 643, 0, 1023);

  Joystick.setXAxis(max(rClutchL, rClutchR));

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
