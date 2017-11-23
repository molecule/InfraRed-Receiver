/* Trinket/Gemma compatible IR read sketch
This sketch/program uses an Adafruit Trinket or Gemma
ATTiny85 based mini microcontroller and a PNA4602 or TSOP38238 to
read an IR code and perform a function.  
Based on Adafruit tutorial http://learn.adafruit.com/ir-sensor/using-an-ir-sensor
and http://learn.adafruit.com/trinket-gemma-mini-theramin-music-maker
*/
 
// We need to use the 'raw' pin reading methods because timing is very important here 
// and the digitalRead() procedure is slower!
#define IRrx_PIN_VECTOR PIND
#define IRrx 5   // IR sensor - TSOP38238

#define MAXPULSE    5000  // the maximum pulse we'll listen for - 5 milliseconds 
#define NUMPULSES    50  // max IR pulse pairs to sample
#define RESOLUTION     2  // // time between IR measurements
 
// we will store up to 50 pulse pairs (this is -a lot-, reduce if needed)
uint16_t pulses[50][2];   // pair is high and low pulse
uint16_t currentpulse = 0; // index for pulses we're storing
uint32_t irCode = 0;

//******** Neopixel ***********//
#include <Adafruit_NeoPixel.h>

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)
#define NEOPIXEL_PIN 4
#define NUM_LEDS 10
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

uint32_t blue          = strip.Color(0, 0, 255);
uint32_t leaf_green    = strip.Color(30, 44, 0);
uint32_t hot_pink      = strip.Color(48, 0, 24);
uint32_t yellowOrange  = strip.Color(44, 30, 0);

int delayVal = 50;

// Convenient 2D point structure
struct Point {
  float x;
  float y;
};

float phase = 0.0;
float phaseIncrement = 0.03;  // Controls the speed of the moving points. Higher == faster. I like 0.08 .
float colorStretch = 0.11;    // Higher numbers will produce tighter color bands. I like 0.11 .

// ******** IR ******** //
const uint32_t IR_REMOTE_POWER     = 0x8322A15E;
const uint32_t IR_REMOTE_SELECT    = 0x8322A659;
const uint32_t IR_REMOTE_MUTE      = 0x8322AE51;
const uint32_t IR_REMOTE_V_UP      = 0x8322A25D;
const uint32_t IR_REMOTE_V_DOWN    = 0x8322A35C;
const uint32_t IR_REMOTE_MODE      = 0x8322B24D;
const uint32_t IR_REMOTE_REWIND    = 0x8322A55A;
const uint32_t IR_REMOTE_PLAY      = 0x8322B04F;
const uint32_t IR_REMOTE_FORWARD   = 0x8322A45B;
const uint32_t HEADER              = 0x83220000;

const uint32_t MASK = 0xFFFE0000;
 
void setup() {
  Serial.begin(9600);
  Serial.println("raw_ir_receive");

  // Neopixel setup
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}
 
void loop() {
  
  irCode=listenForIR(); // Wait for an IR Code
  
  // Process the pulses to get our code
  for (int i = 0; i < 34; i++) {
    irCode=irCode<<1;
    if((pulses[i][0] * RESOLUTION)>0&&(pulses[i][0] * RESOLUTION)<500) {
      irCode|=0; 
    } else {
      irCode|=1;
    }
  }
  Serial.println("irCode");
  Serial.println(irCode, HEX);

  if (irCode == IR_REMOTE_SELECT) {
    chase(hot_pink, false);
    chase(blue, true);
  }
  else if (irCode == IR_REMOTE_PLAY) {
    chase(leaf_green, false);
    chase(blue, true);
  } else {
    chase(yellowOrange, false);
    chase(blue, true);
  }

  // Clear the pulse buffer so random IR signals don't trigger the action
  memset(pulses, 0, sizeof(pulses));
} // end loop
 

uint16_t listenForIR() {  // IR receive code
  currentpulse = 0;
  while (1) {
   unsigned int highpulse, lowpulse;  // temporary storage timing
   highpulse = lowpulse = 0; // start out with no pulse length 
  
   while (IRrx_PIN_VECTOR & _BV(IRrx)) { // got a high pulse
      highpulse++; 
      delayMicroseconds(RESOLUTION);
      if (((highpulse >= MAXPULSE) && (currentpulse != 0)) || currentpulse == NUMPULSES) {
        return currentpulse; 
      }
   }
   pulses[currentpulse][0] = highpulse;

   while (! (IRrx_PIN_VECTOR & _BV(IRrx))) { // got a low pulse
      lowpulse++; 
      delayMicroseconds(RESOLUTION);
      if (((lowpulse >= MAXPULSE) && (currentpulse != 0))|| currentpulse == NUMPULSES) {
        return currentpulse; 
      }
   }
   pulses[currentpulse][1] = lowpulse;
   currentpulse++;
  }
}

static void chase(uint32_t color, boolean turnOff) {
  for(uint16_t i=0; i<strip.numPixels()+4; i++) {
      strip.setPixelColor(i  , color); // Draw new pixel
      if (turnOff) {
        strip.setPixelColor(i-1, 0); // Erase pixel a few steps back
      }
      strip.show();
      delay(delayVal);
  }
}
