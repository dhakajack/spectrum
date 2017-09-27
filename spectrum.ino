/*

7 band AF spectrum display using MSGEQ7 chip

*/

#define MSG_RESET 18
#define MSG_STROBE 17
#define MSG_SAMPLE 5 // analog 5
#define NOISE_LVL 1  // analog 1

const int LED_COL[7] = {0,1,2,3,4,5,6};
const int LED_ROW[8] = {14,13,12,11,10,9,8,7};
const int WARNING = 1000;

const int SCALE[4][6] = {
  {128,256,384,512,640,768}, // linear
  {128,181,256,362,512,724}, // 3dB increments (21dB range)
  {16,32,64,128,256,512}, // 6dB (42 dB range)
  {0,1,4,16,64,256} //12dB (84 dB range)
};

int noiseThreshold = 0;

byte mode = 0; 

/* set by 2-position dip switch

RANGE_LO = PB6
RANGE_HI = PB7

RANGE_HI RANGE_LO   mode    SCALE
0        0          0       linear  
0        1          1       3dB
1        0          2       6dB
1        1          3       12dB
*/


// Set noise threshold via trimmer pot
int setThreshold () {
  int adcTotal = 0;
  
  for (int x = 0; x < 10; x++) {
    adcTotal += analogRead(NOISE_LVL);
    delayMicroseconds(35);
  }
  return(adcTotal / 10);
}

void setup() {
  for (int x=0; x < 7; x++) {
    pinMode(LED_COL[x],OUTPUT);
    digitalWrite(LED_COL[x],LOW);
  }
  for (int x=0; x < 8; x++) {
    pinMode(LED_ROW[x],OUTPUT);
    digitalWrite(LED_ROW[x],HIGH);
  }
  pinMode(MSG_RESET,OUTPUT);
  pinMode(MSG_STROBE,OUTPUT);
  
  // initialize MSGEQ7
  digitalWrite(MSG_RESET, LOW);
  digitalWrite(MSG_STROBE, HIGH);
  
  /* set meter scale; presumably, done once at power up
  
  built for ATmega328 using internal 8Mhz RC clock
  so using the pins normally assigned to loading caps
  for the crystal oscillator. Unfortunately, they are not
  assigned digital pins in Arduino IDE, so doing this 
  directly via port registers.
  
  This amounts to: 
  mode = digitalRead(RANGE_HI)*2 + digitalRead(RANGE_LO);
  
  */

  DDRB &= B00111111;  // set port B for input on PB6 and 7
  PORTB |= B11000000; // set port B interal pull-ups on PB6 and 7
  
  if (PINB & B10000000) { // PB6, RANGE_HI
    mode = 2;
  }
  if (PINB & B01000000) { // PB7, RANGE_LO
    mode += 1;
  }
  
  noiseThreshold = setThreshold(); 
}

void loop() {
  int spectrumRead = 0;
  byte topBar = 0;
  
  digitalWrite(MSG_RESET, HIGH);
  digitalWrite(MSG_RESET, LOW);
  for (int band = 0; band < 7; band++) {
     digitalWrite(MSG_STROBE, LOW);
     delayMicroseconds(35); // allow value to settle
     spectrumRead = analogRead(MSG_SAMPLE);
     
     if (spectrumRead > noiseThreshold) {  // if nothing registers above, nothing to display
       // figure out how many LEDs to light up in a given band
       if (spectrumRead > WARNING) { 
         topBar = 7;
       } else {
         topBar = 0;
         for (int y = 5; y > -1; y--) {
           if (spectrumRead >= SCALE[mode][y] + noiseThreshold ) {
             topBar = y + 1;
             break;
           }
         } 
       }

      // turn LEDs on in a band
       digitalWrite(LED_COL[band],HIGH);
       for (int y = 0; y < topBar + 1; y++) {
         digitalWrite(LED_ROW[y], LOW);
       }
       
       // persistance of vision delay; adjust to taste
       delay(2); // above 2 ms, flickery; for finer adjust, use delayMicroseconds()
       
       // turn LEDs off in a band
       digitalWrite(LED_COL[band],LOW);
       for (int y = 0; y < topBar + 1; y++) {
         digitalWrite(LED_ROW[y],HIGH);
       }
     }
     digitalWrite(MSG_STROBE, HIGH);
  }
}
