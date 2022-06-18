int ioSelect = 1; //2;     // SR Pin 15.
int clockPulse = 0; //3;   //SR Pin 7. 
int dataOut = 2;  //4;      //SR Pin 13.

int j;               //used in a for loop to declare which bit is set to 1
int value;           //stores the digital read value of the data pin 
                     //(0 if no button is pressed, 1 if a button is pressed)

byte switchVar = 0;  //stores a byte array to show which button was pressed

void setup() {
  pinMode(ioSelect, OUTPUT);
  pinMode(clockPulse, OUTPUT);
  pinMode(dataOut, INPUT);  

Serial.begin(9600);  //setting baud rate
}

void loop() {
  uint16_t  dataIn = 0;       //Swap out byte for uint16_t or uint32_t
  int dataIn2[] = {0, 0, 0, 0, 0, 0, 0, 0};
  digitalWrite(ioSelect, 0);    // enables parallel inputs
  digitalWrite(clockPulse, 0);  // start clock pin low
  digitalWrite(clockPulse, 1);  // set clock pin high, data loaded into SR
  digitalWrite(ioSelect, 1);    // disable parallel inputs and enable serial output 
  Serial.println("###############");
  Serial.print("1 2 3 4 5 6 7 8");
  Serial.println("");

for(j = 0; j < 8  ; j++) {         //sets integer to values 0 through 7 for all 8 bits
    value = digitalRead(dataOut); //reads data from SR serial data out pin
//    Serial.print("Button Position: "); // checks bit position 0-7
//    Serial.println(j);
//    Serial.print("Button Value: ");  //checks whether button was pressed (high vs low)
//    Serial.println(value);   
    Serial.print(value);
    Serial.print(" ");
    if (value) {
      int a = (1 << j);       // shifts bit to its proper place in sequence. 
                              /*for more information see Arduino "BitShift" */
      dataIn = dataIn | a;    //combines data from shifted bits to form a single 8-bit number
                              /*for more information see Arduino "Bitwise operators" */
      }
      digitalWrite(clockPulse, LOW);  //after each bit is logged, 
      digitalWrite(clockPulse, HIGH); //pulses clock to get next bit
    }
//    Serial.print("dataIn BIN: "); //Will show readout of data in binary
//    Serial.print(dataIn, BIN);
//    Serial.print("dataIn: "); //Will show readout of data in binary
//    Serial.print(dataIn);
    Serial.println();
//       if (switchVar != dataIn)
//    {
//      switchVar = dataIn;
//      Serial.print("dataIn DEC: "); // Will show readout of data in decimal
//      Serial.print(dataIn, DEC);
//      Serial.println();
//      Serial.print("dataIn BIN: "); //Will show readout of data in binary
//      Serial.print(dataIn, BIN);
//      Serial.println();
//    } 
 delay(50); //short delay added for debugging
} 
