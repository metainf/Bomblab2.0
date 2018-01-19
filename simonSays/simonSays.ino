#include <Bounce2.h>
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
#define ledDuration 150
// the interval between displaying the sequence
#define displayInterval 3000
// the interval between firing the buttons
#define buttonInterval 500
// the I2C address
#define i2cAddr 8
#define red 0
#define green 1
#define blue 2
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
// the pin numbers for the colored COM-1044X buttons and their LEDs
const int button[] = {16, 14, 5, 6};  // RGBY respectively
const int led[] = {15, 13, 2, 4};     // RGBY respectively
// the notes for each button
const int tones[] = {NOTE_G5, NOTE_D5, NOTE_G4, NOTE_B4}; // RGBY respectively
// the pin number for the green LED and the piezo buzzer
const int clearLED = 12;
const int buzzerPin = 3;


/*
 * 42   defused
 * -1   not-started
 * 0    started
 * 1    received 1 correct input 
 * 12   received 2 correct inputs
 * 123  received 3 correct inputs
 */
volatile int state = 0;

/*
 * 2D array containing the button that needs to be pressed if a button
 * shows up in sequence (RGBY respectively in each array)
 * vowel/novowel
 */
volatile bool hasVowel = false;
const int v[][numButtons]   = {{blue, yellow, red, green},  // 0 strikes
                               {yellow, blue, green, red},  // 1 strike
                               {green, yellow, red, blue}}; // 2 strikes
const int nv[][numButtons]  = {{blue, green, yellow, red},  // 0 strikes
                               {red, yellow, blue, green},  // 1 strike
                               {yellow, blue, green, red}}; // 2 strikes

// all of the possible characters that can make up the serial number
// vowels are 'A', 'E', 'I', 'O', 'U' (indices 0, 4, 8, 14, and 20)
const char choices[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
                        'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
                        'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0',
                        '1', '2', '3', '4', '5', '6', '7', '8', '9'};

volatile int inputs[seqLength];
volatile int input;

Bounce buttons[numButtons];
Tone buzzer;
String serial;
volatile int sequence[seqLength];
volatile int externalStrikes  = 0;
volatile int internalStrikes = 0;
volatile int progress = 0;
volatile bool rcvdInput = false;

void setup() {
  // put your setup code here, to run once:
  // initialize the psuedo-random number generator
  Serial.begin(9600);
  Wire.begin(i2cAddr);          // join I2C bus with address i2cAddr
  Wire.onReceive(receiveEvent); // register receive event
  Wire.onRequest(requestEvent); // register request event
  randomSeed(analogRead(0));
  /*
   * initialize the buttons as inputs with internal pullup resistors and
   * initialize the buttons' LEDs as outputs
   * internal pullup resistors mean that the buttons will be active low
   */
  // initialize each of the COM-1044X buttons
  for (int x = 0; x < numButtons; x++) {
    pinMode(button[x], INPUT_PULLUP);
    buttons[x] = Bounce();
    buttons[x].attach(button[x]);
    pinMode(led[x], OUTPUT);
  }
  buttonLeds(LOW);
  // initialize the green LED as an output and make sure it's off
  pinMode(clearLED, OUTPUT);
  digitalWrite(clearLED, LOW);
  randomize();
  generateSerialNumber();
  // initialize the buzzer
  buzzer.begin(buzzerPin);
}

void loop() {
  /*
     * consider using Arduino Tone Library to work with the Piezo Buzzer
   * https://code.google.com/archive/p/rogue-code/wikis/ToneLibraryDocumentation.wiki
   */
  // put your main code here, to run repeatedly:
  if (externalStrikes == 3) {
    state = -1;
  }
  switch (state) {
    case -1: // not-started/failed
      break;
    case 42: // defused
      digitalWrite(clearLED, HIGH);
      buttonLeds(LOW);
      break;
    case 0: // started
      progressState(1);
      break;
    case 1: // received 1 correct input
      progressState(12);
      break;
    case 12:
      progressState(123);
      break;
    case 123:
      updateInputs();
      if (rcvdInput) {
        rcvdInput = false;
        if (checkInput()) {
          /*
           * THE BOMB HAS BEEN DEFUSED
           * COUNTER-TERRORISTS WIN
           */
          state = 42;
        } else {
          fail();
        }
      }
      break;
    default:
      break;
  }
}

void progressState(int nextState) {
  updateInputs();
  if (rcvdInput) {
    rcvdInput = false;
    // if you have received correct input
    if (checkInput()) {
      if (progress > state) {
        // you can move on to the next state
        progress = max(nextState, progress);
        state = nextState;
      } else {
        progress = state + 1;
        state = 0;
      }
    } else {
      fail();
    }
  } else {
    if (not state) {
      displaySequence();
    }
  }
}

void updateInputs() {
  rcvdInput = false;
  for (int x = 0; x < numButtons; x++) {
    buttons[x].update();
    if (buttons[x].fell()) {
      rcvdInput = true;
      input = x;
      fireButton(x);
      break;
    }
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
  static unsigned long lastDisplayTime = 0;
  unsigned long curTime = millis();
  if ((curTime - lastDisplayTime) < displayInterval) {
    return;
  } else {
    lastDisplayTime = curTime;
  }
  int x;
  switch (progress) {
    case 0:
      x = 1;
      break;
    case 1:
      x = 2;
      break;
    case 2:
      x = 3;
      break;
    case 12:
      x = 3;
      break;
    case 13:
      x = 4;
      break;
    case 123:
      x = 4;
      break;
    default:
      x = 0;
      break;
  }
  for (int a = 0; a < x; a++) {
    fireButton(sequence[a]);
    delay(buttonInterval);
  }
}

void fireButton(int color) {
  // will not do anything if 
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

bool checkInput() {
  int index;
  // figure out what part of the sequence to check against
  switch (state) {
    case 0: // started
      // expect answer to first button in sequence
      index = 0;
      break;
    case 1: // received 1 correct input
      // expect answer to second button in sequence
      index = 1;
      break;
    case 12: // received 2 correct inputs
      // expect answer to third button in sequence
      index = 2;
      break;
    case 123: // received 3 correct inputs
      // expect answer to fourth button in sequence
      index = 3;
      break;
  }
  /*
   * sequence[index] is the button in the sequence
   * ^ COLORS HAVE BEEN DEFINED AS INTEGERS ^
   * v/nv[strikes] determines the array to index into
   * in order to determine the expected input color response
   */
  if (hasVowel) {
    return input == v[externalStrikes][sequence[index]];
  } else {
    return input == nv[externalStrikes][sequence[index]];
  }
}

void buttonLeds(byte val) {
  for (int x = 0; x < numButtons; x++) {
    digitalWrite(led[x], val);
  }
}

void fail() {
  Serial.println("gg");
  progress = 0;
  internalStrikes++;
  buzzer.play(NOTE_DS2, toneDuration);
  randomize();
  state = 0;
}

void randomize() {
  // generate a new sequence for Simon Says
  for (int x = 0; x < seqLength; x++) {
    sequence[x] = random(0, numButtons);
    /*
    switch (sequence[x]) {
      case 0:
        Serial.println("red");
        break;
      case 1:
        Serial.println("green");
        break;
      case 2:
        Serial.println("blue");
        break;
      case 3:
        Serial.println("yellow");
        break;
    }
    */
  }
}

// TODO: UPDATE TO CHANGE STATE TO THE START STATE (but only if you get a start signal or if it's already defused/not-started)
// TODO: UPDATE TO HANDLE STRIKE FORMATTINGS
// TODO: UPDATE TO HANDLE BEING TOLD TO STOP / RESTART
// ^ COMPLETE !? ^
void receiveEvent(int howMany) {
  char c;
  while (Wire.available()) {
    c = Wire.read();
    switch (c) {
      case 's':
        if (state == -1) {
          state = 0;
        }
        break;
      case 'r':
        state = 0;
        progress = 0;
        internalStrikes = 0;
        break;
      case '0':
        externalStrikes = 0;
        break;
      case '1':
        externalStrikes = 1;
        break;
      case '2':
        externalStrikes = 2;
        break;
      case '3':
        externalStrikes = 3;
        break;
    }
  }
}

void requestEvent() {
  switch (internalStrikes) {
    case 0:
      Wire.write('0');
      break;
    case 1:
      Wire.write('1');
      break;
    case 2:
      Wire.write('2');
      break;
    case 3:
      Wire.write('3');
      break;
  }
}
