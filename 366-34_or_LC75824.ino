/**********************************************************************/
/*
This code is not clean and far from perfect, that's just
a reference to extract ideas and adapt to your solution.
you can replace the BIN values with HEX... I leave it in BIN
because it is easier to relate the segment number with
the position of the bit in BIN.
Of course, a library can be created for this purpose! But I won't 
take the time to do that, I'll leave it up to you!
*/
/**********************************************************************/
/* This code show how is possible work with the driver LC75824 Sony */
/* This IC same times is marked as only the "366-34" from Sony */
/* Also pinout is equivalent to the PT6524, check the datasheet */

/*
* Note: DD ... Direction data
* •CCB address...............46H
* •D1 to D136.................Display data (At the LC75834JE, the display data D33 to D36, D69 to D72, D105 to D108, D133 to D136 must be set to 0.
* •P0 to P3......................Segment output port/general-purpose output port switching control data
* •DR..............................1/2-bias drive or 1/3-bias drive switching control data
* •SC...............................Segments on/off control data
* •BU..............................Normal mode/power-saving mode control dataNo. 5597-8/18LC75834E, 75834W, 75834JE
* 
* 0, 0, 0, P0, P1, P2, P3, DR, SC, BU, DD, DD;
*/


/*Begining of Auto generated code by Atmel studio */
#define VFD_in 8  // This is the pin number 8 on Arduino UNO  // You can use a resistor of 1Kohms to protect this line!
#define VFD_clk 9 // This is the pin number 9 on Arduino UNO  // You can use a resistor of 1Kohms to protect this line!
#define VFD_ce 10 // This is the pin number 10 on Arduino UNO // You can use a resistor of 1Kohms to protect this line!
#define addr 0x41 // This is the address of our IC

unsigned int shiftBit=0;
unsigned int nBitOnBlock=0; // Used to count number of bits and split to 8 bits... (number of byte)
unsigned int nByteOnBlock=0; 
unsigned int sequencyByte=0x00;
byte Aa,Ab,Ac,Ad,Ae,Af,Ag;
byte blockBit =0x00;


#define BUTTON_PIN 2 //Att check wich pins accept interrupts... Uno is 2 & 3, This is our trigger button
volatile byte buttonReleased = false;

// variables will change:
int buttonState = 0;  // variable for reading the pushbutton status

uint8_t LEDcolor = 0x00;
uint8_t LEDinfo = 0x00;

bool forward = false;
bool backward = false;
bool isRequest = true;
bool allOn=false;
bool cycle=false;
/*
//ATT: On the Uno and other ATMEGA based boards, unsigned ints (unsigned integers) are the same as ints in that they store a 2 byte value.
//Long variables are extended size variables for number storage, and store 32 bits (4 bytes), from -2,147,483,648 to 2,147,483,647.
/*************************************************/
void setup() {
  pinMode(VFD_clk, OUTPUT);
  pinMode(VFD_in, OUTPUT);
  pinMode(VFD_ce, OUTPUT);
  //initialize the LED pin as an output:
  pinMode(13, OUTPUT);
  //pinMode(ledPin, OUTPUT);
  //initialize the pushbutton pin as an input:
  //pinMode(buttonPin, INPUT);  //Next line is the attach of interruption to pin 2
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  //
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN),
                buttonReleasedInterrupt,
                FALLING);
  //
  Serial.begin(115200); // only to debug  
  Serial.println("You reach a Reset Hardware!");               
}
void send_char(unsigned char a){
  //
  unsigned char transmit = 15; //define our transmit pin
  unsigned char data = 170; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask
  data=a;
  // the validation of data happen when clk go from LOW to HIGH.
  // This lines is because the clk have one advance in data, see datasheet of sn74HC595
  // case don't have this signal instead of "." will se "g"
  digitalWrite(VFD_ce, LOW); // When strobe is low, all output is enable. If high, all output will be set to low.
  delayMicroseconds(5);
  digitalWrite(VFD_clk,LOW);// need invert the signal to allow 8 bits is is low only send 7 bits
  delayMicroseconds(5);
  for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
  digitalWrite(VFD_clk,LOW);// need invert the signal to allow 8 bits is is low only send 7 bits
  delayMicroseconds(5);
  if (data & mask){ // if bitwise AND resolves to true
    digitalWrite(VFD_in, HIGH);
    //Serial.print(1);
  }
  else{ //if bitwise and resolves to false
    digitalWrite(VFD_in, LOW);
    //Serial.print(0);
  }
  digitalWrite(VFD_clk,HIGH);// need invert the signal to allow 8 bits is is low only send 7 bits
  delayMicroseconds(5);
  //
  digitalWrite(VFD_ce, HIGH); // When strobe is low, all output is enable. If high, all output will be set to low.
  delayMicroseconds(5);
  }
}
/*******************************************************************/
// I h've created 3 functions to send bit's, one with strobe, other without strobe and one with first byte with strobe followed by remaing bits.
void send_char_without(unsigned char a){
  unsigned char transmit = 15; //define our transmit pin
  unsigned char data = 170; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask
  data=a;
  for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
  digitalWrite(VFD_clk, LOW);
  delayMicroseconds(5);
  if (data & mask){ // if bitwise AND resolves to true
    digitalWrite(VFD_in, HIGH);
    //Serial.print(1);
  }
  else{ //if bitwise and resolves to false
    digitalWrite(VFD_in, LOW);
    //Serial.print(0);
  }
  digitalWrite(VFD_clk,HIGH);// need invert the signal to allow 8 bits is is low only send 7 bits
  delayMicroseconds(5);
  }
}
/*******************************************************************/
void send_char_8bit_stb(unsigned char a){
    unsigned char transmit = 15; //define our transmit pin
    unsigned char data = 170; //value to transmit, binary 10101010
    unsigned char mask = 1; //our bitmask
    int i = -1;
    data=a;
    // This lines is because the clk have one advance in data, see datasheet of sn74HC595
    // case don't have this signal instead of "." will se "g"
    for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
    i++;
    digitalWrite(VFD_clk, LOW);
    delayMicroseconds(5);
    if (data & mask){ // if bitwise AND resolves to true
      digitalWrite(VFD_in, HIGH);
      //Serial.print(1);
    }
    else{ //if bitwise and resolves to false
      digitalWrite(VFD_in, LOW);
      //Serial.print(0);
    }
    digitalWrite(VFD_clk,HIGH);// 
    delayMicroseconds(1);
    if (i==7){
    //Serial.println(i);
    digitalWrite(VFD_ce, HIGH);
    delayMicroseconds(1);
    }  
    }
}
/*******************************************************************/
void allON(){
  //send total of 64 bits, the 4 last bits belongs to set of DR, SC, BU, etc;
  //The p0, p1, p2 & p3 at 0, means all pins from s1 to s12 will belongs to segments, other's settings tell will px is a port general purpose!
  digitalWrite(VFD_ce, LOW); //
  delayMicroseconds(1);
  send_char_8bit_stb(addr); //(0x41) firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
  //
  // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
  send_char_without(0B11110001);  send_char_without(0B11111111); //  8:1   -16:9
  send_char_without(0B11111111);  send_char_without(0B11111111); // 24:17  -32:25
  send_char_without(0B11111111);  send_char_without(0B11111111); // 40:33  -48:41
  send_char_without(0B00001111);  send_char_without(0B00000110); // 53:49   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
  // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
  delayMicroseconds(1);
  digitalWrite(VFD_ce, LOW); // 
  send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
  //
  // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
  send_char_without(0B11111111);  send_char_without(0B11111111); // 60:53  -68:61
  send_char_without(0B11111111);  send_char_without(0B11111111); // 76:69  -84:77
  send_char_without(0B11111111);  send_char_without(0B11111111); // 92:85 -100:93
  send_char_without(0B00001111);  send_char_without(0B01000000); //104:101   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
  // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
  delayMicroseconds(1);
  digitalWrite(VFD_ce, LOW); // 
  send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
  //
  // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
  send_char_without(0B11111111);  send_char_without(0B11111111); // 112:105  -120:113
  send_char_without(0B11111111);  send_char_without(0B11111111); // 128:121  -136:129
  send_char_without(0B11111111);  send_char_without(0B11111111); // 144:137  -152:145
  send_char_without(0B00001111);  send_char_without(0B10000000); // 156:153   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
  // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
  delayMicroseconds(1);
  //send total of 156 bits, the 4 last bits belongs to DR, SC, BU, X;
  digitalWrite(VFD_ce, LOW); //
  delayMicroseconds(1);
  send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
  //
  // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
  send_char_without(0B11111111);  send_char_without(0B11111111); // 164:157  -172:165
  send_char_without(0B11111111);  send_char_without(0B11111111); // 180:173  -188:181
  send_char_without(0B11111111);  send_char_without(0B11111111); // 196:189  -204:197
  send_char_without(0B00000000);  send_char_without(0B11000000); // empty & cmd // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
  // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
  delayMicroseconds(1);
  digitalWrite(VFD_ce, LOW); // 
  delayMicroseconds(1);
}
void allOFF(){
  //send total of 64 bits, the 4 last bits belongs to set of DR, SC, BU, etc;
  //The p0, p1, p2 & p3 at 0, means all pins from s1 to s12 will belongs to segments, other's settings tell will px is a port general purpose!
  digitalWrite(VFD_ce, LOW); //
  delayMicroseconds(1);
  send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
  //
  // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
  send_char_without(0B00000000);  send_char_without(0B00000001); //  8:1   -16:9
  send_char_without(0B00000000);  send_char_without(0B00000000); // 24:17  -32:25
  send_char_without(0B00000000);  send_char_without(0B00000000); // 40:33  -48:41
  send_char_without(0B00000000);  send_char_without(0B00000110); // 53:49   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
  // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
  delayMicroseconds(1);
  digitalWrite(VFD_ce, LOW); // 
  delayMicroseconds(1);
  //
  send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
  //
  // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
  send_char_without(0B00000000);  send_char_without(0B00000000); // 60:53  -68:61
  send_char_without(0B00000000);  send_char_without(0B00000000); // 76:69  -84:77
  send_char_without(0B00000000);  send_char_without(0B00000000); // 92:85 -100:93
  send_char_without(0B00000000);  send_char_without(0B01000000); //104:101   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
  // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
  delayMicroseconds(1);
  digitalWrite(VFD_ce, LOW); // 
  delayMicroseconds(1);
  //
  send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
  //
  // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
  send_char_without(0B00000000);  send_char_without(0B00000000); // 112:105  -120:113
  send_char_without(0B00000000);  send_char_without(0B00000000); // 128:121  -136:129
  send_char_without(0B00000000);  send_char_without(0B00000000); // 144:137  -152:145
  send_char_without(0B00000000);  send_char_without(0B10000000); // 156:153   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
  // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
  delayMicroseconds(1);
  digitalWrite(VFD_ce, LOW); // 
  delayMicroseconds(1);
  //
  send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
  //
  // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
  send_char_without(0B00000000);  send_char_without(0B00000000); // 164:157  -172:165
  send_char_without(0B00000000);  send_char_without(0B00000000); // 180:173  -188:181
  send_char_without(0B00000000);  send_char_without(0B00000000); // 196:189  -204:197// Third 14 segments display(Note:Skip 40 & 44 the 41,42 & 43) Belongs first 7seg display
  send_char_without(0B00000000);  send_char_without(0B11000000); // empty & cmd // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
  // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
  delayMicroseconds(1);
  digitalWrite(VFD_ce, LOW); // 
  delayMicroseconds(1);
}
void testLSW906(void){
  //The bit number 1 is the base of transistor to active ambar color LED, bit 9 is one button specific, port marked as SHIFT.
  //The bit number 5 is the base of transistor to active green color LED, bit 9 is one button specific, port marked as SHIFT.
  //There are a million ways to do this!
  for (int i = 0; i < 6; i++){
    LEDcolor = 0B00010000; //This is bit 5 (firts byte)
    LEDinfo = !LEDinfo; //This is bit 1 (second byte, means bit 9)
    LSW906(LEDcolor, LEDinfo);
    delay(500);
  }
  for (int i = 0; i < 6; i++){
    LEDcolor = 0B00000001; //This is bit 1 (firts byte)
    LEDinfo = !LEDinfo; //This is bit 1 (second byte, means bit 9)
    LSW906(LEDcolor, LEDinfo);
    delay(500);
  }
}
void green(void){
  //The bit number 5 is the base of transistor to active ambar color LED, bit 9 is one button specific.
  //send total of 64 bits, the 4 last bits belongs to set of DR, SC, BU, etc;
  //The p0, p1, p2 & p3 at 0, means all pins from s1 to s12 will belongs to segments, other's settings tell will px is a port general purpose!
  digitalWrite(VFD_ce, LOW); //
  delayMicroseconds(1);
  send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
  //
  // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
  send_char_without(0B00010000);  send_char_without(0B00000001); //  8:1   -16:9
  send_char_without(0B00000000);  send_char_without(0B00000000); // 24:17  -32:25
  send_char_without(0B00000000);  send_char_without(0B00000000); // 40:33  -48:41
  send_char_without(0B00000000);  send_char_without(0B00000110); // 53:49   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
  // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
  delayMicroseconds(1);
  digitalWrite(VFD_ce, LOW); // 
  delayMicroseconds(1);
  //
  send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
  //
  // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
  send_char_without(0B00000000);  send_char_without(0B00000000); // 60:53  -68:61
  send_char_without(0B00000000);  send_char_without(0B00000000); // 76:69  -84:77
  send_char_without(0B00000000);  send_char_without(0B00000000); // 92:85 -100:93
  send_char_without(0B00000000);  send_char_without(0B01000000); //104:101   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
  // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
  delayMicroseconds(1);
  digitalWrite(VFD_ce, LOW); // 
  delayMicroseconds(1);
  //
  send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
  //
  // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
  send_char_without(0B00000000);  send_char_without(0B00000000); // 112:105  -120:113
  send_char_without(0B00000000);  send_char_without(0B00000000); // 128:121  -136:129
  send_char_without(0B00000000);  send_char_without(0B00000000); // 144:137  -152:145
  send_char_without(0B00000000);  send_char_without(0B10000000); // 156:153   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
  // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
  delayMicroseconds(1);
  digitalWrite(VFD_ce, LOW); // 
  delayMicroseconds(1);
  //
  send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
  //
  // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
  send_char_without(0B00000000);  send_char_without(0B00000000); // 164:157  -172:165
  send_char_without(0B00000000);  send_char_without(0B00000000); // 180:173  -188:181
  send_char_without(0B00000000);  send_char_without(0B00000000); // 196:189  -204:197// Third 14 segments display(Note:Skip 40 & 44 the 41,42 & 43) Belongs first 7seg display
  send_char_without(0B00000000);  send_char_without(0B11000000); // empty & cmd // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
  // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
  delayMicroseconds(1);
  digitalWrite(VFD_ce, LOW); // 
  delayMicroseconds(1);
}
void amber(void){
  //The bit number 1 is the base of transistor to active green color LED, bit 9 is one button specific.
  //send total of 64 bits, the 4 last bits belongs to set of DR, SC, BU, etc;
  //The p0, p1, p2 & p3 at 0, means all pins from s1 to s12 will belongs to segments, other's settings tell will px is a port general purpose!
  digitalWrite(VFD_ce, LOW); //
  delayMicroseconds(1);
  send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
  //
  // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
  send_char_without(0B00000001);  send_char_without(0B00000001); //  8:1   -16:9
  send_char_without(0B00000000);  send_char_without(0B00000000); // 24:17  -32:25
  send_char_without(0B00000000);  send_char_without(0B00000000); // 40:33  -48:41
  send_char_without(0B00000000);  send_char_without(0B00000110); // 53:49   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
  // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
  delayMicroseconds(1);
  digitalWrite(VFD_ce, LOW); // 
  delayMicroseconds(1);
  //
  send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
  //
  // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
  send_char_without(0B00000000);  send_char_without(0B00000000); // 60:53  -68:61
  send_char_without(0B00000000);  send_char_without(0B00000000); // 76:69  -84:77
  send_char_without(0B00000000);  send_char_without(0B00000000); // 92:85 -100:93
  send_char_without(0B00000000);  send_char_without(0B01000000); //104:101   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
  // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
  delayMicroseconds(1);
  digitalWrite(VFD_ce, LOW); // 
  delayMicroseconds(1);
  //
  send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
  //
  // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
  send_char_without(0B00000000);  send_char_without(0B00000000); // 112:105  -120:113
  send_char_without(0B00000000);  send_char_without(0B00000000); // 128:121  -136:129
  send_char_without(0B00000000);  send_char_without(0B00000000); // 144:137  -152:145
  send_char_without(0B00000000);  send_char_without(0B10000000); // 156:153   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
  // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
  delayMicroseconds(1);
  digitalWrite(VFD_ce, LOW); // 
  delayMicroseconds(1);
  //
  send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
  //
  // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
  send_char_without(0B00000000);  send_char_without(0B00000000); // 164:157  -172:165
  send_char_without(0B00000000);  send_char_without(0B00000000); // 180:173  -188:181
  send_char_without(0B00000000);  send_char_without(0B00000000); // 196:189  -204:197// Third 14 segments display(Note:Skip 40 & 44 the 41,42 & 43) Belongs first 7seg display
  send_char_without(0B00000000);  send_char_without(0B11000000); // empty & cmd // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
  // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
  delayMicroseconds(1);
  digitalWrite(VFD_ce, LOW); // 
  delayMicroseconds(1);
}
void LSW906(uint8_t LEDcolor, uint8_t LEDinfo){
  //This is a button marked as LSW906, and comming from transistor connected to the P2. Note to control this pino you must have P1 & P2 as 1
  // to allow the control as GPIO's pin until Seg4.
  // Note: The backlight is permanent connected to the B+ (this case 12VDC of battery of car)
  //send total of 64 bits, the 4 last bits belongs to set of DR, SC, BU, etc;
  //The p0, p1, p2 & p3 at 0, means all pins from s1 to s12 will belongs to segments, other's settings tell will px is a port general purpose!
  digitalWrite(VFD_ce, LOW); //
  delayMicroseconds(1);
  send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
  //
  // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
  send_char_without(0B00000000 | LEDcolor);  send_char_without(0B00000000 | LEDinfo); //  8:1   -16:9
  send_char_without(0B00000000);  send_char_without(0B00000000); // 24:17  -32:25
  send_char_without(0B00000000);  send_char_without(0B00000000); // 40:33  -48:41
  send_char_without(0B00000000);  send_char_without(0B00000110); // 53:49   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
  // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
  delayMicroseconds(1);
  digitalWrite(VFD_ce, LOW); // 
  delayMicroseconds(1);
  //
  send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
  //
  // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
  send_char_without(0B00000000);  send_char_without(0B00000000); // 60:53  -68:61
  send_char_without(0B00000000);  send_char_without(0B00000000); // 76:69  -84:77
  send_char_without(0B00000000);  send_char_without(0B00000000); // 92:85 -100:93
  send_char_without(0B00000000);  send_char_without(0B01000000); //104:101   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
  // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
  delayMicroseconds(1);
  digitalWrite(VFD_ce, LOW); // 
  delayMicroseconds(1);
  //
  send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
  //
  // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
  send_char_without(0B00000000);  send_char_without(0B00000000); // 112:105  -120:113
  send_char_without(0B00000000);  send_char_without(0B00000000); // 128:121  -136:129
  send_char_without(0B00000000);  send_char_without(0B00000000); // 144:137  -152:145
  send_char_without(0B00000000);  send_char_without(0B10000000); // 156:153   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
  // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
  delayMicroseconds(1);
  digitalWrite(VFD_ce, LOW); // 
  delayMicroseconds(1);
  //
  send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
  //
  // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
  send_char_without(0B00000000);  send_char_without(0B00000000); // 164:157  -172:165
  send_char_without(0B00000000);  send_char_without(0B00000000); // 180:173  -188:181
  send_char_without(0B00000000);  send_char_without(0B00000000); // 196:189  -204:197// Third 14 segments display(Note:Skip 40 & 44 the 41,42 & 43) Belongs first 7seg display
  send_char_without(0B00000000);  send_char_without(0B11000000); // empty & cmd // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
  // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
  delayMicroseconds(1);
  digitalWrite(VFD_ce, LOW); // 
  delayMicroseconds(1);
}
void msgGreen(){
  //send total of 64 bits, the 4 last bits belongs to set of DR, SC, BU, etc;
    //The p0, p1, p2 & p3 at 0, means all pins from s1 to s12 will belongs to segments, other's settings tell will px is a port general purpose!
    digitalWrite(VFD_ce, LOW); //
    delayMicroseconds(1);
    send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
    //
    // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
    send_char_without(0B00010000);  send_char_without(0B00000001); //  8:1   -16:9//  
    send_char_without(0B00000000);  send_char_without(0B00000000); // 24:17  -32:25// 
    send_char_without(0B00000000);  send_char_without(0B00000000); // 40:33  -48:41// 
    send_char_without(0B00000000);  send_char_without(0B00000110); // 52:49   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
    // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
    delayMicroseconds(1);
    digitalWrite(VFD_ce, LOW); // 
    delayMicroseconds(1);
    //
    send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
    //
    // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
    send_char_without(0B00000000);  send_char_without(0B00010000); // 64:57  -72:65// 
    send_char_without(0B11100001);  send_char_without(0B10110101); // 80:73  -88:81//  
    send_char_without(0B10110000);  send_char_without(0B00010101); // 96:89 -104:97//   
    send_char_without(0B00001010);  send_char_without(0B10000000); //112:105   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
    // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
    delayMicroseconds(1);
    digitalWrite(VFD_ce, LOW); // 
    delayMicroseconds(1);
    //
    send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
    //
    // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
    send_char_without(0B00010101);  send_char_without(0B11001010); // 120:113  -128:121//  
    send_char_without(0B00110010);  send_char_without(0B01011000); // 136:129  -144:137//  
    send_char_without(0B00000000);  send_char_without(0B00000000); // 152:145  -160:153//  
    send_char_without(0B00000000);  send_char_without(0B01000000); // 168:161   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
    // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
    delayMicroseconds(1);
    digitalWrite(VFD_ce, LOW); // 
    delayMicroseconds(1);
    //
    send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
    //
    // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
    send_char_without(0B00000000);  send_char_without(0B00000000); // 176:169  -184:177// 
    send_char_without(0B00000000);  send_char_without(0B00000000); // 192:185  -200:193//  
    send_char_without(0B00000000);  send_char_without(0B00000000); // 208:201  -216:209// 
    send_char_without(0B00000000);  send_char_without(0B11000000); // empty & cmd // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
    // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
    delayMicroseconds(1);
    digitalWrite(VFD_ce, LOW); // 
    delayMicroseconds(1);
}
void msgAmber(){
  //send total of 64 bits, the 4 last bits belongs to set of DR, SC, BU, etc;
    //The p0, p1, p2 & p3 at 0, means all pins from s1 to s12 will belongs to segments, other's settings tell will px is a port general purpose!
    digitalWrite(VFD_ce, LOW); //
    delayMicroseconds(1);
    send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
    //
    // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
    send_char_without(0B00000001);  send_char_without(0B00000001); //  8:1   -16:9//  
    send_char_without(0B00000000);  send_char_without(0B00000000); // 24:17  -32:25// 
    send_char_without(0B00000000);  send_char_without(0B00000000); // 40:33  -48:41// 
    send_char_without(0B00000000);  send_char_without(0B00000110); // 52:49   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
    // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
    delayMicroseconds(1);
    digitalWrite(VFD_ce, LOW); // 
    delayMicroseconds(1);
    //
    send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
    //
    // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
    send_char_without(0B00000001);  send_char_without(0B01010000); // 64:57  -72:65// 
    send_char_without(0B01110001);  send_char_without(0B00100011); // 80:73  -88:81//  
    send_char_without(0B10100101);  send_char_without(0B01010000); // 96:89 -104:97//   
    send_char_without(0B00011111);  send_char_without(0B10000000); //112:105   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
    // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
    delayMicroseconds(1);
    digitalWrite(VFD_ce, LOW); // 
    delayMicroseconds(1);
    //
    send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
    //
    // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
    send_char_without(0B00010101);  send_char_without(0B01001010); // 120:113  -128:121//  
    send_char_without(0B01010010);  send_char_without(0B00001011); // 136:129  -144:137//  
    send_char_without(0B00000000);  send_char_without(0B00000000); // 152:145  -160:153//  
    send_char_without(0B00000000);  send_char_without(0B01000000); // 168:161   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
    // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
    delayMicroseconds(1);
    digitalWrite(VFD_ce, LOW); // 
    delayMicroseconds(1);
    //
    send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
    //
    // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
    send_char_without(0B00000000);  send_char_without(0B00000000); // 176:169  -184:177// 
    send_char_without(0B00000000);  send_char_without(0B00000000); // 192:185  -200:193//  
    send_char_without(0B00000000);  send_char_without(0B00000000); // 208:201  -216:209// 
    send_char_without(0B00000000);  send_char_without(0B11000000); // empty & cmd // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
    // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
    delayMicroseconds(1);
    digitalWrite(VFD_ce, LOW); // 
    delayMicroseconds(1);
}
void HiFolks(){
    //send total of 64 bits, the 4 last bits belongs to set of DR, SC, BU, etc;
    //The p0, p1, p2 & p3 at 0, means all pins from s1 to s12 will belongs to segments, other's settings tell will px is a port general purpose!
    digitalWrite(VFD_ce, LOW); //
    delayMicroseconds(1);
    send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
    //
    // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
    send_char_without(0B00010001);  send_char_without(0B00000001); //  8:1   -16:9//  
    send_char_without(0B00000000);  send_char_without(0B00000000); // 24:17  -32:25// 
    send_char_without(0B00000000);  send_char_without(0B00000000); // 40:33  -48:41// 
    send_char_without(0B00000000);  send_char_without(0B00000110); // 52:49   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
    // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
    delayMicroseconds(1);
    digitalWrite(VFD_ce, LOW); // 
    delayMicroseconds(1);
    //
    send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
    //
    // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
    send_char_without(0B00000000);  send_char_without(0B01010000); // 64:57  -72:65// 
    send_char_without(0B01110000);  send_char_without(0B01010000); // 80:73  -88:81//  
    send_char_without(0B10001000);  send_char_without(0B00000000); // 96:89 -104:97//   
    send_char_without(0B00000000);  send_char_without(0B10000000); //112:105   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
    // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
    delayMicroseconds(1);
    digitalWrite(VFD_ce, LOW); // 
    delayMicroseconds(1);
    //
    send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
    //
    // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
    send_char_without(0B00010101);  send_char_without(0B01000010); // 120:113  -128:121//  
    send_char_without(0B00010110);  send_char_without(0B11010001); // 136:129  -144:137//  
    send_char_without(0B00000001);  send_char_without(0B01011000); // 152:145  -160:153//  
    send_char_without(0B00001010);  send_char_without(0B01000000); // 168:161   // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
    // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
    delayMicroseconds(1);
    digitalWrite(VFD_ce, LOW); // 
    delayMicroseconds(1);
    //
    send_char_8bit_stb(addr); // firts 8 bits is address, every fixed as (0B010000001), see if clk finish LOW or HIGH Very important!
    //
    // On the 75824 of Sony the message have first 8 bits more 64. Max 4 times to performe 256 bits( correspond send 4 times 8+64 bits
    send_char_without(0B01010000);  send_char_without(0B11100001); // 176:169  -184:177// 
    send_char_without(0B00001000);  send_char_without(0B00000000); // 192:185  -200:193//  
    send_char_without(0B00000000);  send_char_without(0B00000000); // 208:201  -216:209// 
    send_char_without(0B00000000);  send_char_without(0B11000000); // empty & cmd // 0,0,CU,p0,p1,p2,p3,DR,SC,BU, 0, 0; Last 2 bits is "DD" data direction, and is used
    // to mark the 4 groups of 64 bits, 00, 01, 10, 11.
    delayMicroseconds(1);
    digitalWrite(VFD_ce, LOW); // 
    delayMicroseconds(1);
}

void searchOfSegments(){
  int group = 0x00;
  byte nBit =0x00;
  byte nMask = 0b00000001;
  unsigned int block =0;
  unsigned int nSeg=0x00;  
  Serial.println();
  Serial.println("We start the test of segments!");
  for(block=0; block<4; block++){  //This is the last 2 bit's marked as DD, group: 0x00, 0x01, 0x10, 0x11;
  for( group=0; group<7; group++){   // Do until n bits 15*8 bits
      //for(int nBit=0; nBit<8; nBit++){
        for (nMask = 0b00000001; nMask>0; nMask <<= 1){
          Aa=0x00; Ab=0x00; Ac=0x00; Ad=0x00; Ae=0x00; Af=0x00; Ag=0x00; 
                switch (group){
                  case 0: Aa=nMask; break;
                  case 1: Ab=nMask; break;
                  case 2: Ac=nMask; break;
                  case 3: Ad=nMask; break;
                  case 4: Ae=nMask; break;
                  case 5: Af=nMask; break;
                  case 6: Ag=nMask; break;
                }
          
        nSeg++;
        if((nSeg >=0) && (nSeg<56)){
          blockBit=0;
          }
          if((nSeg >=57) && (nSeg<112)){
          blockBit=1;
          }
          if((nSeg >=112) && (nSeg<168)){
          blockBit=2;
          }
          if((nSeg >=169) && (nSeg<224)){
          blockBit=3;
          }
        
          if (nSeg >=224){
            nSeg=0;
            group=0;
            block=0;
            break;
          }
        
    //This start the control of button to allow continue teste! 
                    while(1){
                          if(!buttonReleased){
                            delay(200);
                          }
                          else{
                            delay(15);
                            buttonReleased = false;
                            break;
                            }
                      }
            delay(50);
            segments();
          Serial.print(nSeg, DEC); Serial.print(", group: "); Serial.print(group, DEC);Serial.print(", BlockBit: "); Serial.print(blockBit, HEX);Serial.print(", nMask: "); Serial.print(nMask, BIN);Serial.print("   \t");
          Serial.print(Ag, HEX);Serial.print(", ");Serial.print(Af, HEX);Serial.print(", ");Serial.print(Ae, HEX);Serial.print(", ");Serial.print(Ad, HEX);Serial.print(", ");
          Serial.print(Ac, HEX);Serial.print(", ");Serial.print(Ab, HEX);Serial.print(", ");Serial.print(Aa, HEX); Serial.print("; ");
          
          Serial.println();
          delay (250);  
              }         
        }        
    }
}

void segments(){
  //Bit function: 
  digitalWrite(VFD_ce, LOW); //
  delayMicroseconds(1);
  send_char_8bit_stb(0B01000001); //(0x41) firts 8 bits is address, every fixed as (0B01000001), see if clk finish LOW or HIGH Very important!
  delayMicroseconds(1);
  // 
      send_char_without(Aa);  send_char_without(Ab);  //   8:1     -16:9// 
      send_char_without(Ac);  send_char_without(Ad);  //  24:17    -32:25//
      send_char_without(Ae);  send_char_without(Af);  //  40:33    -48:41// 
      send_char_without(Ag);  //  52:49   //Cmd Bit's belongs to this byte is "0", "0", "CU", "P0".
      
      //The next switch finalize the burst of bits -41:48//  
          switch (blockBit){ //Last 2 bits is "DD" data direction, and is used to mark the 4 groups of 36 bits, 00, 01, 10, 11.                                 
            case 0: send_char_without(0B00000000); break; //Block 00
            case 1: send_char_without(0B10000000); break; //Block 01
            case 2: send_char_without(0B01000000); break; //Block 10
            case 3: send_char_without(0B11000000); break; //Block 11
          }
  delayMicroseconds(1);
  digitalWrite(VFD_ce, LOW); //                   
}

void loop() {
    HiFolks();
    delay(1500);
    allON(); // All on
    delay(1500);
    allOFF();
    delay(500);

    testLSW906(); //To put the LED of LWW906 blinking

    for(int i=0; i<4; i++){
     green();  //LED's green On on the keys
     delay(800);
     amber();  //LED's ambar On on the keys
     delay(800);
    }
    
    while(1){
      HiFolks();
       delay(1500);
       msgGreen();
       delay(1500);
       msgAmber();
       delay(1500);
       allOFF();
    //searchOfSegments(); //Uncomment this line if you want run the find segments. This need a button to GND (with a 2k) at pin 2 of Arduino already implemented on code!
    }  
}
void buttonReleasedInterrupt() {
buttonReleased = true; // This is the line of interrupt button to advance one step on the search of segments!
}
