#include <Bounce2.h>
#include <Tone.h>
#include <Wire.h>

// the number of physical buttons
#define numButtons 4
// the length of the Simon Says sequence
#define seqLength 4
// the debounce time in milliseconds
#define debounceTime 5
// the tone duration in milliseconds
#define toneDuration 
#define red 0
#define blue 1
#define green 2
#define yellow 3

/*
    The buttons for Simon Says are as follows:
    Red     = COM-10442
    Green   = COM-10440
    Blue    = COM-10443
    Yellow  = COM-10441 (orange IRL)
    https://www.sparkfun.com/products/10441

    The Piezo Buzzer for the tones is:
    https://www.adafruit.com/product/160
*/
// TODO: PICK PINS FOR THE BUTTON, LEDS, AND THE BUZZER
// the pin numbers for the colored COM-1044X buttons and their LEDs
const int button = {}; // RGBY respectively
const int led = {}; // RGBY respectively
// the notes for each button
const int tones = {}; // RGBY respectively TODO: FIGURE OUT WHAT TONES TO USE
// the pin number for the green LED
const int clearLED = ;
// the pin number for the Piezo Buzzer
const int buzzerPin = ;

// the button that needs to be pressed if a button shows up in sequence
// vowel/no vowel
const int v   = {{blue, red, yellow, green},  // 0 strikes
                 {yellow, green, blue, red},  // 1 strike
                 {green, red, yellow, blue}}; // 2 strikes
const int nv  = {{blue, yellow, green, red},  // 0 strikes
                 {red, blue, yellow, green},  // 1 strike
                 {yellow, green, blue, red}}; // 2 strikes

Bounce bouncer[numButtons];
Tone buzzer;
int sequence[seqLength];
int strikes;
int progress = 0;

void setup() {
  // put your setup code here, to run once:
  // initialize the psuedo-random number generator
  randomSeed(analogRead(0)); // TODO: PICK AN UNUSED PIN
  /*
   * initialize the buttons as inputs with internal pullup resistors and
   * initialize the buttons' LEDs as outputs
   * internal pullup resistors mean that the buttons will be active low
   */
  // initialize each of the COM-1044X buttons
  for (int x = 0; x < numButtons; x++) {
    pinMode(button[x], INPUT_PULLUP);
    pinMode(led[x], OUTPUT);
    // create and setup the Bounce object
    bouncer[x] = Bounce();
    bouncer[x].attach(button[x]);
    bouncer[x].interval(debounceTime);
  }
  buttonLeds(LOW);
  // initialize the green LED as an output and make sure it's off
  pinMode(clearLED, OUTPUT);
  digitalWrite(clearLED, LOW);
  randomize();
  // 
  buzzer.begin(buzzerPin);
}

void loop() {
  /*
     * consider using Arduino Tone Library to work with the Piezo Buzzer
   * https://code.google.com/archive/p/rogue-code/wikis/ToneLibraryDocumentation.wiki
   */
  // put your main code here, to run repeatedly:

}

void buttonISR() {
  
}

void buttonLeds(byte state) {
  for (int x = 0; x < numButtons; x++) {
    digitalWrite(led[x], state);
  }
}

void fail() {
  progress = 0;
  buzzer.play(, toneDuration); // TODO: FIGURE OUT THE FAILURE TONE
  randomize();
}

void randomize() {
  // generate a new sequence for Simon Says
  for (int x = 0; x < seqLength; x++) {
    sequence[x] = random(0, numButtons);
  }
}
