/*

7 band AF spectrum display using MSGEQ7 chip

*/

#define MSG_RESET 18
#define MSG_STROBE 17
#define MSG_SAMPLE 0
#define RANGE_HI 16
#define RANGE_LO 15

const int LED_COL[7] = {0,1,2,3,4,5,6};
const int LED_ROW[8] = {19,13,12,11,10,9,8,7};
const int NOISE_THRESHOLD = 50;
const int WARNING = 1000;
const int SCALE[4][6] = {
  {128,256,384,512,640,768}, // linear
  {128,181,256,362,512,724}, // 3dB increments (21dB range)
  {16,32,64,128,256,512}, // 6dB (42 dB range)
  {0,1,4,16,64,256} //12dB (84 dB range)
};

byte mode = 0; 
/* set by 2-position dip switch
RANGE_HI RANGE_LO   mode    SCALE
0        0          0       linear  
0        1          1       3dB
1        0          2       6dB
1        1          3       12dB
*/


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
  pinMode(RANGE_HI,INPUT_PULLUP);
  pinMode(RANGE_LO,INPUT_PULLUP);
  
  // initialize MSGEQ7
  digitalWrite(MSG_RESET, LOW);
  digitalWrite(MSG_STROBE, HIGH);
  
  // set meter scale; presumably, done once at power up
  mode |= (digitalRead(RANGE_HI) << 2);
  mode |= digitalRead(RANGE_LO);
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
     if (spectrumRead > NOISE_THRESHOLD) {
       topBar = spectrumRead / 128;
       digitalWrite(LED_COL[band],HIGH);
       for (int y = 0; y < topBar + 1; y++) {
         digitalWrite(LED_ROW[y], LOW);
       }
       delay(1); // persistance of vision delay; adjust to taste
       digitalWrite(LED_COL[band],LOW);
       for (int y = 0; y < topBar + 1; y++) {
         digitalWrite(LED_ROW[y],HIGH);
       }
     }
     digitalWrite(MSG_STROBE, HIGH);
  }
}
