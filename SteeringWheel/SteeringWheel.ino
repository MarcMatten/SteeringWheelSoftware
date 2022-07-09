// Pin Mapping for buttons and clutch
int NButtonPin[] = {A3, 3, 4, 5, 6, 7, 8, 9, 16, 14, 10, 15};
int NPinClutchL = A0;
int NPinClutchR = A1;

int NButtons = 12; // number of buttons defined in NButtonPin

// Pins for shift register
const int NShiftRegister = 1;
int NPinIOSelect = 1;    // SR Pin 15.
int NPinClockPulse = 0;  //SR Pin 7. 
int NPinDataOut = 2;     //SR Pin 13.
int NStateShiftRegister[8*NShiftRegister];

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

// timer settings for button latch and threshold times
unsigned long tButtonThreshold[] = {100, 100, 500, 100, 250, 100, 100, 100, 100, 0, 100, 0};
unsigned long tButtonPressed[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long tButtonSet[] = {33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33};

// globals for thumb wheels
int NThumbWheelMapL[3] = {1, 2, 3};
int NThumbWheelMapR[3] = {7, 6, 5};
int NThumbWheelOldL = 0;
int NThumbWheelOldR = 0;
int NThumbWheelErrorL = 0;
int NThumbWheelErrorR = 0;
bool BThumbWheelInit = false;
bool BThumbWheelError = false;
unsigned long tThumbWheelChange[] = {0, 0};
unsigned long tThumbWheelLatched = 80;
unsigned long tThumbWheelNoError = 0;
int NThubWheelErrorTotal = 0;
int NThubWheelErrorAllowed = 5;
bool BThumbWheelDeactivated = false;
//                         L+  L-  R+  R-
int NButtonThumbWheel[] = {12, 13, 14, 15};
int NThumbWheelLBuffer[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int NThumbWheelRBuffer[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// buffer definition for serial read
const int BUFFER_SIZE = 4;
byte buf[BUFFER_SIZE];

// global timer
unsigned long tStartLoop = 0;

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

  // define pins for shift register
  pinMode(NPinIOSelect, OUTPUT);
  pinMode(NPinClockPulse, OUTPUT);
  pinMode(NPinDataOut, INPUT);   
  }
}

void loop() {
  tStartLoop = micros(); // for timer; 21.06.22: 670 µs

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
  Clutch(analogRead(NPinClutchL), analogRead(NPinClutchR));

  ReadShiftRegister();

  ThumbWheels();
  
  // send game controller state to PC
  Joystick.sendState();

  // for timing
  //Serial.println(micros() - tStartLoop);
  
  delay(10); // allow for enough time to finish previous sending
  
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


void Clutch(int rClutchRawL, int rClutchRawR) {  
  int rClutch = 0;
  
  // re-map clutch paddles to calibrated range
  int rClutchL = map(rClutchRawL, rClutchMinL, rClutchMaxL, 0, 1023);
  int rClutchR = map(rClutchRawR, rClutchMinR, rClutchMaxR, 0, 1023);

  // start mode logic
  if (rClutchL >= 1000 && rClutchR >= 1000) {
    if (BBothPaddlesPressed == false) {
      // start timer when both paddles are pressed
      BBothPaddlesPressed = true;
      tBothPaddlesPressed = tStartLoop/1000;
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
    if ((tStartLoop/1000 - tBothPaddlesPressed) >= tStartModeThreshold) {
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

  Joystick.setXAxis(rClutch);
}


void ReadShiftRegister() {
  // read in shift register states  
  digitalWrite(NPinIOSelect, 0);    // enables parallel inputs
  digitalWrite(NPinClockPulse, 0);  // start clock pin low
  digitalWrite(NPinClockPulse, 1);  // set clock pin high, data loaded into SR
  digitalWrite(NPinIOSelect, 1);    // disable parallel inputs and enable serial output 

  for(int j = 0; j < 8*NShiftRegister ; j++) {   //sets integer to values 0 through 7 for all 8 bits
    NStateShiftRegister[j] = digitalRead(NPinDataOut);
    digitalWrite(NPinClockPulse, LOW);  //after each bit is logged, 
    digitalWrite(NPinClockPulse, HIGH); //pulses clock to get next bit
  }
}


void ThumbWheels() {
  if (BThumbWheelDeactivated != true) {
    // Interpret shift register states  
    int NStateCountL = 0;
    int NStateCountR = 0;
    int NThumbWheelTempL = 0;
    int NThumbWheelTempR = 0;
    
    for(int k=0; k <3; k++)
    {
      // how many pins are high
      NStateCountL += NStateShiftRegister[NThumbWheelMapL[k]];
      NStateCountR += NStateShiftRegister[NThumbWheelMapR[k]];
  
      // which pin is high
      if (NStateShiftRegister[NThumbWheelMapL[k]] == 1){ NThumbWheelTempL = k; }    
      if (NStateShiftRegister[NThumbWheelMapR[k]] == 1){ NThumbWheelTempR = k; }
    }

    int NThumbWheelL = ThumbWheelFilterL(NThumbWheelTempL);
    int NThumbWheelR = ThumbWheelFilterR(NThumbWheelTempR);
  
    // Detect Thumb Wheel Errors
    if (NStateCountL != 1 || NStateCountR != 1) {
      // error when not exactly one pin high per thumb wheel
      // BThumbWheelError = true;
      // tThumbWheelNoError = 0;
      // Left Thumb Wheel Error
      if (NStateCountL != 1) { 
        NThumbWheelErrorL += 1;
        // if (NThumbWheelErrorL > 10){ Serial.print("TWEL"); Serial.println(NStateCountL);} }
        if ((700 + NThumbWheelErrorL) % 1000 == 0){
          BThumbWheelError = true;
          tThumbWheelNoError = 0;
          Serial.print("TWEL"); 
          Serial.println(NStateCountL);
          NThubWheelErrorTotal = NThubWheelErrorTotal + 1;
        } 
      }
      // Right Thumb Wheel Error 
      if (NStateCountR != 1) { 
        NThumbWheelErrorR += 1;
        // if (NThumbWheelErrorR > 10){ Serial.print("TWER"); Serial.println(NStateCountR);} } 
        if ((700 + NThumbWheelErrorR) % 1000 == 0){
          BThumbWheelError = true;
          tThumbWheelNoError = 0;
          Serial.print("TWER");
          Serial.println(NStateCountR);
          NThubWheelErrorTotal = NThubWheelErrorTotal + 1;
        }
      } 
    }
    else{
      if (BThumbWheelError == true) {
        if (tThumbWheelNoError == 0) {
          tThumbWheelNoError = tStartLoop/1000;
        }
        else if (tStartLoop/1000 - tThumbWheelNoError > 10000) {
          // recover if no error for 10 s
          Serial.print("TWOK");
          NThumbWheelErrorL = 0;
          NThumbWheelErrorR = 0;
          BThumbWheelError = false;
        }
        if (NThubWheelErrorTotal > NThubWheelErrorAllowed) {
          // deactivate Thumbwheels when too many errors
          BThumbWheelDeactivated = true;
          Serial.print("TWOFF");
          }
      }
      // No Error
      //BThumbWheelError = false;
      // NThumbWheelErrorL = 0;
      // NThumbWheelErrorR = 0;
    }
  
    // Thumb Wheel Switch detection
    // Button allocation:
    // 0   1   2   3
    // L+  L-  R+  R-
      
    if (BThumbWheelError == false) {
      if (BThumbWheelInit == true){
        if (NThumbWheelOldL == 0){
          if (NThumbWheelL == 1) { ThumbWheelChange(1); }
          else if (NThumbWheelL == 2) { ThumbWheelChange(0); }
        }
        else if (NThumbWheelOldL == 1){
          if (NThumbWheelL == 2) { ThumbWheelChange(1); }
          else if (NThumbWheelL == 0) { ThumbWheelChange(0); }
        }
        else if (NThumbWheelOldL == 2){
          if (NThumbWheelL == 0) { ThumbWheelChange(1); }
          else if (NThumbWheelL == 1) { ThumbWheelChange(0); }
        }
        
        if (NThumbWheelOldR == 0){
          if (NThumbWheelR == 1) { ThumbWheelChange(3); }
          else if (NThumbWheelR == 2) { ThumbWheelChange(2); }
        }
        else if (NThumbWheelOldR == 1){
          if (NThumbWheelR == 2) { ThumbWheelChange(3); }
          else if (NThumbWheelR == 0) { ThumbWheelChange(2); }
        }
        else if (NThumbWheelOldR == 2){
          if (NThumbWheelR == 0) { ThumbWheelChange(3); }
          else if (NThumbWheelR == 1) { ThumbWheelChange(2); }
        }
        
        NThumbWheelOldL = NThumbWheelL;
        NThumbWheelOldR = NThumbWheelR;
        
        // reset button press
        //    Button allocation:
        //    0   1   2   3
        //    L+  L-  R+  R-
        for (int m = 0; m < 2; m++) {
          if (tStartLoop/1000 - tThumbWheelChange[m] > tThumbWheelLatched){
            Joystick.releaseButton(NButtonThumbWheel[2*m]);
            Joystick.releaseButton(NButtonThumbWheel[2*m+1]);
            tThumbWheelChange[m] = 0;
          }
        }
      }
      else{ // Initialistion
        NThumbWheelOldL = NThumbWheelL;
        NThumbWheelOldR = NThumbWheelR;
        BThumbWheelInit = true;
      }
    }
  //  else {
  //    // in case of thumb wheel error    
  //    for (int m = 0; m < 4; m++) {
  //      Joystick.releaseButton(NButtonThumbWheel[2*m+1]);
  //    }
  //    tThumbWheelChange[0] = tStartLoop/1000;
  //    tThumbWheelChange[1] = tStartLoop/1000;
  //  }
  }
}

void ThumbWheelChange(int NAction) {
  // Button allocation:
  // 0   1   2   3
  // L+  L-  R+  R-
  
  if (NAction < 2) {
    if (tThumbWheelChange[0] == 0) { 
      Joystick.pressButton(NButtonThumbWheel[NAction]);
    }
    tThumbWheelChange[0] = tStartLoop/1000;
  }
  else {
    if (tThumbWheelChange[1] == 0) { 
      Joystick.pressButton(NButtonThumbWheel[NAction]);
    }
    tThumbWheelChange[1] = tStartLoop/1000;
  }
}

int ThumbWheelFilterL(int N) {
  int* temp = NThumbWheelLBuffer;
  int mean = N;
  NThumbWheelLBuffer[0] = N;
  for(int i=0; i < 19; i++) {
    NThumbWheelLBuffer[i+1] = temp[i];
    mean += temp[i];
  }
  return mean/20;
}

int ThumbWheelFilterR(int N) {
  int* temp = NThumbWheelRBuffer;
  int mean = N;
  NThumbWheelRBuffer[0] = N;
  for(int i=0; i < 19; i++) {
    NThumbWheelRBuffer[i+1] = temp[i];
    mean += temp[i];
  }
  return mean/20;
}
