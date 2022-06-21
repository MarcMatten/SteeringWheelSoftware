// Pin for shift register
int NPinIOSelect = 1;    // SR Pin 15.
int NPinClockPulse = 0;  //SR Pin 7. 
int NPinDataOut = 2;     //SR Pin 13.

// globals for thumb wheels
int NThumbWheelMapL[3] = {1, 2, 3};
int NThumbWheelMapR[3] = {7, 6, 5};
int NThumbWheelOldL = 0;
int NThumbWheelOldR = 0;
int NThumbWheelErrorL = 0;
int NThumbWheelErrorR = 0;
bool BThumbWheelInit = false;
bool BThumbWheelError = false;

// execution time calculation
unsigned long tExec = 0; // 20.06.2022: 145 µs

void setup() {
  pinMode(NPinIOSelect, OUTPUT);
  pinMode(NPinClockPulse, OUTPUT);
  pinMode(NPinDataOut, INPUT);  

  Serial.begin(9600);  //setting baud rate
}

void loop() {
  // tExec = micros();

   ThumbWheels();
  
  // Serial.println(micros() - tExec);
  delay(10); //short delay added for debugging
} 

void ThumbWheels() {
  // read in shift register states
  uint16_t  dataIn = 0; //Swap out byte for uint16_t or uint32_t
  int dataIn2[] = {0, 0, 0, 0, 0, 0, 0, 0};
  int value;
  int NState[8];
  
  digitalWrite(NPinIOSelect, 0);    // enables parallel inputs
  digitalWrite(NPinClockPulse, 0);  // start clock pin low
  digitalWrite(NPinClockPulse, 1);  // set clock pin high, data loaded into SR
  digitalWrite(NPinIOSelect, 1);    // disable parallel inputs and enable serial output 

  for(int j = 0; j < 8  ; j++) {         //sets integer to values 0 through 7 for all 8 bits
      value = digitalRead(NPinDataOut); //reads data from SR serial data out pin
      NState[j] = value;
      if (value) {
        int a = (1 << j);       // shifts bit to its proper place in sequence. 
                                /*for more information see Arduino "BitShift" */
        dataIn = dataIn | a;    //combines data from shifted bits to form a single 8-bit number
                                /*for more information see Arduino "Bitwise operators" */
        }
        digitalWrite(NPinClockPulse, LOW);  //after each bit is logged, 
        digitalWrite(NPinClockPulse, HIGH); //pulses clock to get next bit
      }
      
  // Interpret shift register states  
  int NStateCountL = 0;
  int NStateCountR = 0;
  int NThumbWheelL = 0;
  int NThumbWheelR = 0;
  
  for(int k=0; k <3; k++)
  {
    // how many pins are high
    NStateCountL += NState[NThumbWheelMapL[k]];
    NStateCountR += NState[NThumbWheelMapR[k]];

    // which pin is high
    if (NState[NThumbWheelMapL[k]] == 1){ NThumbWheelL = k; }    
    if (NState[NThumbWheelMapR[k]] == 1){ NThumbWheelR = k; }
  }

  // Detect Thumb Wheel Errors
  if (NStateCountL != 1 || NStateCountR != 1) {
    // error when not exactly one pin high per thumg wheel
    BThumbWheelError = true;
    // Left Thumb Wheel Error
    if (NStateCountL != 1) { 
      NThumbWheelErrorL += 1;
      if (NThumbWheelErrorL >= 10){ Serial.print("ERROR: Left Thumb Wheel reading "); Serial.print(NStateCountL); Serial.println(" states!");} }
    // Right Thumb Wheel Error 
    if (NStateCountR != 1) { 
      NThumbWheelErrorR += 1;
      if (NThumbWheelErrorR >= 10){ Serial.print("ERROR: Right Thumb Wheel reading "); Serial.print(NStateCountR); Serial.println(" states!");} } 
  }
  else{
    // No Error
    BThumbWheelError = false;
    NThumbWheelErrorL = 0;
    NThumbWheelErrorR = 0;
  }

  // Thumb Wheel Switch detection
  if (BThumbWheelError == false) {
    if (BThumbWheelInit == true){
      if (NThumbWheelOldL == 0){
        if (NThumbWheelL == 1) {Serial.println("L-");}
        else if (NThumbWheelL == 2) {Serial.println("L+");}
      }
      else if (NThumbWheelOldL == 1){
        if (NThumbWheelL == 2) {Serial.println("L-");}
        else if (NThumbWheelL == 0) {Serial.println("L+");}
      }
      else if (NThumbWheelOldL == 2){
        if (NThumbWheelL == 0) {Serial.println("L-");}
        else if (NThumbWheelL == 1) {Serial.println("L+");}
      }
      
      if (NThumbWheelOldR == 0){
        if (NThumbWheelR == 1) {Serial.println("R-");}
        else if (NThumbWheelR == 2) {Serial.println("R+");}
      }
      else if (NThumbWheelOldR == 1){
        if (NThumbWheelR == 2) {Serial.println("R-");}
        else if (NThumbWheelR == 0) {Serial.println("R+");}
      }
      else if (NThumbWheelOldR == 2){
        if (NThumbWheelR == 0) {Serial.println("R-");}
        else if (NThumbWheelR == 1) {Serial.println("R+");}
      }
        
      NThumbWheelOldL = NThumbWheelL;
      NThumbWheelOldR = NThumbWheelR;
    }
    else{ // Initialistion
      NThumbWheelOldL = NThumbWheelL;
      NThumbWheelOldR = NThumbWheelR;
      BThumbWheelInit = true;
    }
  }  
}
