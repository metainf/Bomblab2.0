#include <Tone.h>
#include <Wire.h>

// the number of physical buttons
#define numButtons 4
// the length of the Simon Says sequence
#define seqLength 4
// the length of the serial number
#define serialLength 6
// the number of characters for the serial number
#define numChars 36
// the tone duration in milliseconds
#define toneDuration 150
// the LED duration in milliseconds
#define ledDuration 200
// the I2C address
#define i2cAddr 8
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
const int button[] = {}; // RGBY respectively
const int led[] = {}; // RGBY respectively
// the notes for each button
const int tones[] = {NOTE_G5, NOTE_D5, NOTE_G4, NOTE_B4}; // RGBY respectively
// the pin number for the green LED
const int clearLED = ;
// the pin number for the Piezo Buzzer
const int buzzerPin = ;

/*
 * 2D array containing the button that needs to be pressed if a button
 * shows up in sequence (RGBY respectively in each array)
 * vowel/novowel
 */
volatile bool hasVowel = false;
const int v[]   = {{blue, yellow, red, green},  // 0 strikes
                   {yellow, blue, green, red},  // 1 strike
                   {green, yellow, red, blue}}; // 2 strikes
const int nv[]  = {{blue, green, yellow, red},  // 0 strikes
                   {red, yellow, blue, green},  // 1 strike
                   {yellow, blue, green, red}}; // 2 strikes

// all of the possible characters that can make up the serial number
// vowels are 'A', 'E', 'I', 'O', 'U' (indices 0, 4, 8, 14, and 20)
const char choices[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
                        'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
                        'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0',
                        '1', '2', '3', '4', '5', '6', '7', '8', '9'};

volatile int inputs[seqLength];

Tone buzzer;
String serial;
volatile int sequence[seqLength];
volatile int strikes  = 0;
volatile int progress = 0;
volatile int numInput = 0;
volatile bool rcvdInput = false;
volatile bool start = false;
volatile bool done = false;

void setup() {
  // put your setup code here, to run once:
  // initialize the psuedo-random number generator
  Serial.begin(9600);
  Wire.begin(i2cAddr);          // join I2C bus with address i2cAddr
  Wire.onReceive(receiveEvent); // register receive event
  Wire.onRequest(requestEvent); // register request event
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
  }
  attachInterrupt(digitalPinToInterrupt(button[0]), buttonISR0, FALLING);
  attachInterrupt(digitalPinToInterrupt(button[1]), buttonISR1, FALLING);
  attachInterrupt(digitalPinToInterrupt(button[2]), buttonISR2, FALLING);
  attachInterrupt(digitalPinToInterrupt(button[3]), buttonISR3, FALLING);
  buttonLeds(LOW);
  // initialize the green LED as an output and make sure it's off
  pinMode(clearLED, OUTPUT);
  digitalWrite(clearLED, LOW);
  randomize();
  // initialize the buzzer
  buzzer.begin(buzzerPin);
}

void loop() {
  /*
     * consider using Arduino Tone Library to work with the Piezo Buzzer
   * https://code.google.com/archive/p/rogue-code/wikis/ToneLibraryDocumentation.wiki
   */
  // put your main code here, to run repeatedly:
  if (!start) {
    // return and restart the loop
    return;
  }
  if (done) {
    noInterrupts();
    digitalWrite(clearLED, HIGH);
    buttonLeds(LOW);
  } else if (numInput) {
    if (rcvdInput) {
      checkInput();
    }
  } else {
    displaySequence();
    delay(1000);
  }
}

void generateSerialNumber() {
  char temp[serialLength];
  int tempIndex;
  for (int x = 0; x < serialLength; x++) {
    tempIndex = random(0, numChars);
    temp[x] = choices[tempIndex];
    switch (tempIndex) {
      case 0:   // 'A', fallthrough
      case 4:   // 'E', fallthrough
      case 8:   // 'I', fallthrough
      case 14:  // 'O', fallthrough
      case 20:  // 'U'
        hasVowel = true;
      default:  // not a vowel
        break;
    }
  }
  serial = String(temp);
  Serial.println(serial);
  Serial.println(hasVowel);
}

void displaySequence() {
  for (int x = 0; x <= progress; x++) {
    fireButton(sequence[x]);
  }
}

// TODO: FIGURE OUT TONES FOR EACH COLOR
void fireButton(int color) {
  switch (color) {
    case red:     // fallthrough
    case green:   // fallthrough
    case blue:    // fallthrough
    case yellow:  // fallthrough
      digitalWrite(led[color], HIGH);
      buzzer.play(tones[color], toneDuration);
      delay(ledDuration);
      digitalWrite(led[color], LOW);
      break;
    default:  // should not reach here
      break;
  }
}

void checkInput() {
  noInterrupts();
  rcvdInput = false;
  int cap = min(numInput, progress);
  // sequence[x] is the color that the module flashes
  // input[x] is the player's input in response
  // index with [strikes][sequence[x]] to find the correct color
  if (hasVowel) {
    for (int x = 0; x <= cap; x++) {
      if (inputs[x] != v[strikes][sequence[x]]) {
        fail();
      }
    }
  } else {
    for (int x = 0; x <= cap; x++) {
      if (inputs[x] != nv[strikes][sequence[x]]) {
        fail();
      }
    }
  }
  progress++;
  numInput = 0;
  if (progress == seq_length) {
    done = true;
  }
  interrupts();
}

// red
void buttonISR0() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  if ((interruptTime - lastInterruptTime) > 200) {
    inputs[numInput] = red;
    rcvdInput = true;
    numInput++;
    lastInterruptTime = interruptTime;
  }
}

// green
void buttonISR1() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  if ((interruptTime - lastInterruptTime) > 200) {
    inputs[numInput] = green;
    rcvdInput = true;
    numInput++;
    lastInterruptTime = interruptTime;
  }
}

// blue
void buttonISR2() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  if ((interruptTime - lastInterruptTime) > 200) {
    inputs[numInput] = blue;
    rcvdInput = true;
    numInput++;
    lastInterruptTime = interruptTime;
  }
}

// yellow
void buttonISR3() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  if ((interruptTime - lastInterruptTime) > 200) {
    inputs[numInput] = yellow;
    rcvdInput = true;
    numInput++;
    lastInterruptTime = interruptTime;
  }
}

void buttonLeds(byte state) {
  for (int x = 0; x < numButtons; x++) {
    digitalWrite(led[x], state);
  }
}

void fail() {
  noInterrupts();
  numInput = 0;
  progress = 0;
  strikes++;
  buzzer.play(NOTE_DS2, toneDuration);
  randomize();
  interrupts();
}

void randomize() {
  // generate a new sequence for Simon Says
  for (int x = 0; x < seqLength; x++) {
    sequence[x] = random(0, numButtons);
  }
}

void receiveEvent(int howMany) {
  int x = Wire.read();
  switch (x) {
    case -1: // failed
      start = false;
      break;
    case 0: // fallthrough
    case 1: // fallthrough
    case 2:
      start = true;
      strikes = max(x, strikes);
      break;
    default:
      break;
  }
}

void requestEvent() {
  Wire.write(strikes);
}

