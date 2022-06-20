int ioSelect = 1;    // SR Pin 15.
int clockPulse = 0;  //SR Pin 7. 
int dataOut = 2;     //SR Pin 13.

int j;               //used in a for loop to declare which bit is set to 1
int value;           //stores the digital read value of the data pin 
                     //(0 if no button is pressed, 1 if a button is pressed)
int NState[8];

int NThumbWheelMapL[3] = {1, 2, 3};
int NThumbWheelMapR[3] = {7, 6, 5};
int NThumbWheelOldL = 0;
int NThumbWheelOldR = 0;

int NThumbWheelErrorL = 0;
int NThumbWheelErrorR = 0;

bool BThumbWheelInit = false;
bool BThumbWheelError = false;

byte switchVar = 0;  //stores a byte array to show which button was pressed

unsigned long tExec = 0; // 20.06.2022: 145 µs

void setup() {
  pinMode(ioSelect, OUTPUT);
  pinMode(clockPulse, OUTPUT);
  pinMode(dataOut, INPUT);  

  Serial.begin(9600);  //setting baud rate
}

void loop() {
  // tExec = micros();
  uint16_t  dataIn = 0;       //Swap out byte for uint16_t or uint32_t
  int dataIn2[] = {0, 0, 0, 0, 0, 0, 0, 0};
  digitalWrite(ioSelect, 0);    // enables parallel inputs
  digitalWrite(clockPulse, 0);  // start clock pin low
  digitalWrite(clockPulse, 1);  // set clock pin high, data loaded into SR
  digitalWrite(ioSelect, 1);    // disable parallel inputs and enable serial output 

  for(j = 0; j < 8  ; j++) {         //sets integer to values 0 through 7 for all 8 bits
      value = digitalRead(dataOut); //reads data from SR serial data out pin
      NState[j] = value;
      if (value) {
        int a = (1 << j);       // shifts bit to its proper place in sequence. 
                                /*for more information see Arduino "BitShift" */
        dataIn = dataIn | a;    //combines data from shifted bits to form a single 8-bit number
                                /*for more information see Arduino "Bitwise operators" */
        }
        digitalWrite(clockPulse, LOW);  //after each bit is logged, 
        digitalWrite(clockPulse, HIGH); //pulses clock to get next bit
      }
      
  // check for error
  int NStateCountL = 0;
  int NStateCountR = 0;
  int NThumbWheelL = 0;
  int NThumbWheelR = 0;
  
  for(int k=0; k <3; k++)
  {
    NStateCountL += NState[NThumbWheelMapL[k]];
    NStateCountR += NState[NThumbWheelMapR[k]];
  
    if (NState[NThumbWheelMapL[k]] == 1){
      NThumbWheelL = k;
    }
    
    if (NState[NThumbWheelMapR[k]] == 1){
      NThumbWheelR = k;
    }
  }
  
  if (NStateCountL != 1 || NStateCountR != 1) {
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
    BThumbWheelError = false;
    NThumbWheelErrorL = 0;
    NThumbWheelErrorR = 0;
  }
  
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
    else{ // initialise
      NThumbWheelOldL = NThumbWheelL;
      NThumbWheelOldR = NThumbWheelR;
      BThumbWheelInit = true;
    }
  }   
  
  // Serial.println(micros() - tExec);
  delay(10); //short delay added for debugging
} 
