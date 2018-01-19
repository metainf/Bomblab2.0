#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include "bitmaps.h"
#include <Bounce2.h>

// Display Pins

// Chip select pins
#define displaycs1 7
#define displaycs2 8
#define displaycs3 9
#define displaycs4 10

#define dcPin 5
#define rstPin 6
#define dummyrstPin 20

Adafruit_PCD8544 display1 = Adafruit_PCD8544(13, 11, dcPin, displaycs1, rstPin);
Adafruit_PCD8544 display2 = Adafruit_PCD8544(13, 11, dcPin, displaycs2, dummyrstPin);
Adafruit_PCD8544 display3 = Adafruit_PCD8544(13, 11, dcPin, displaycs3, dummyrstPin);
Adafruit_PCD8544 display4 = Adafruit_PCD8544(13, 11, dcPin, displaycs4, dummyrstPin);

Adafruit_PCD8544* displayArray[] = {&display1, &display2, &display3, &display4};

static const unsigned char PROGMEM contrast[] = {50, 30, 30, 50};

// Counters
int i;

// Bitmap columns
static const unsigned char  column0[] = {24, 11, 25, 10, 6, 8, 20};
static const unsigned char  column1[] = {14, 24, 20, 22, 2, 8, 17};
static const unsigned char  column2[] = {0, 7, 22, 4, 13, 25, 2};
static const unsigned char  column3[] = {9, 18, 26, 6, 4, 17, 3};
static const unsigned char  column4[] = {21, 3, 26, 19, 18, 16, 1};
static const unsigned char  column5[] = {9, 14, 23, 12, 21, 15, 5};

const unsigned char* columnTable[] = {column0, column1, column2, column3, column4, column5};

long randColumn;
static unsigned char randArray[] = {0, 1, 2, 3, 4, 5, 6};

// Game Logic Vars
volatile char myStrikes = '0';     // The total number of myStrikes
volatile char totalStrikes = '0'; // The total strikes the player gotten in the game
unsigned int difficulty = 0;  // The difficulty of the game, 0 through 3
char state = 'w';             // The state of the game
/*
   w = waiting state, waiting for the user to start the game after selecting difficulty
   b = begin state, setup game to start.
   r = running state, game in progress
   c = completed, CT win, bomb has been defused
   f = failed, T win, bomb has exploded
*/


// Button pins
#define b0Pin 17
#define b1Pin 16
#define b2Pin 15
#define b3Pin 14

// Buttons
Bounce b0 = Bounce();
Bounce b1 = Bounce();
Bounce b2 = Bounce();
Bounce b3 = Bounce();

// LED pins
#define led0Pin 2
#define led1Pin 3
#define led2Pin 4
#define led3Pin 12

int ledArray[] = {led0Pin, led1Pin, led2Pin, led3Pin};


void setup() {
  Serial.begin(9600);
  Wire.begin(1);
  Wire.onRequest(requestEvent); // register event
  Wire.onReceive(receiveEvent); // register event



  randomSeed(analogRead(6));

  // Setup the IO pins

  // button input pins
  pinMode(b0Pin, INPUT_PULLUP);
  pinMode(b1Pin, INPUT_PULLUP);
  pinMode(b2Pin, INPUT_PULLUP);
  pinMode(b3Pin, INPUT_PULLUP);


  // LED output pins
  pinMode(led0Pin, OUTPUT);
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(led3Pin, OUTPUT);

  // Set LED output pins to turn LEDs off
  digitalWrite(led0Pin, LOW);
  digitalWrite(led1Pin, LOW);
  digitalWrite(led2Pin, LOW);
  digitalWrite(led3Pin, LOW);

  // Attach the button pins to the debouncing objects
  b0.attach(b0Pin);
  b1.attach(b1Pin);
  b2.attach(b2Pin);
  b3.attach(b3Pin);

  // Setup the debounce time in millis
  b0.interval(25);
  b1.interval(25);
  b2.interval(25);
  b3.interval(25);

  // Init the displays
  for (i = 0; i < 4; i++) {
    displayArray[i]->begin();
  }

  for (i = 0; i < 4; i++) {
    displayArray[i]->setRotation(2);
    displayArray[i]->setContrast(pgm_read_byte_near(contrast + i));
    displayArray[i]->clearDisplay();
    //    displayArray[i]->drawBitmap(display1.width() / 2 - 48 / 2, display1.height() / 2 - 48 / 2,
    //                                (bitmap_table[0]), 48, 48, BLACK);
    displayArray[i]->display();
  }

  // Choose a column to display from, and randomize the display array
  scrambleArray(randArray, 7);
  randColumn = random(6);

  for (i = 0; i < 7; i++)
  {
    Serial.print(randArray[i]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.println(randColumn);



}

void loop() {

  //  for (unsigned char j = 0; j < 27; j++)
  //  {
  //    for (int i = 0; i < 4; i++) {
  //    displayArray[i]->setRotation(2);
  //    displayArray[i]->clearDisplay();
  //    displayArray[i]->drawBitmap(display1.width() / 2 - 48 / 2, display1.height() / 2 - 48 / 2,
  //                                (bitmap_table[(j + i)%27]), 48, 48, BLACK);
  //    displayArray[i]->display();
  //    }
  //    delay(500);
  //  }

  switch (state)
  {
    case 'w':
      {
        // Wait for the start of the game
        if (totalStrikes == 's') {
          state = 'b';
        }
      }
      break;
    case 'b':
      {
        // Display the chosen output
        for (i = 0; i < 4; i++) {
          // Get the right bitmap index
          int displayIndex = columnTable[randColumn][randArray[i]];
          Serial.println(displayIndex);
          displayArray[i]->clearDisplay();
          displayArray[i]->drawBitmap(display1.width() / 2 - 48 / 2, display1.height() / 2 - 48 / 2,
                                      (bitmap_table[displayIndex]), 48, 48, BLACK);
          displayArray[i]->display();
          state = 'r';
        }
      }
      break;
    case 'r':
      {
        static int curr_stage = 0;
        int pressed_button = -1;
        // Read the state of the buttons
        b0.update();
        b1.update();
        b2.update();
        b3.update();

        if (b0.rose()) {
          pressed_button = 0;
        }
        else if (b1.rose()) {
          pressed_button = 1;
        }
        else if (b2.rose()) {
          pressed_button = 2;
        }
        else if (b3.rose()) {
          pressed_button = 3;
        }

        // If a button has been pressed, check if it was the right one
        if (pressed_button > -1) {
          Serial.print("Pressed Button");
          Serial.println(pressed_button);
          if (lessThan(randArray, pressed_button) == curr_stage) {
            curr_stage++;
            digitalWrite(ledArray[pressed_button], HIGH);
          }
          else
          {
            myStrikes++;
          }
        }

        if (curr_stage == 4) {
          state = 'c';
        }

        if (myStrikes == '3') {
          state = 'f';
        }

        if (totalStrikes == '3') {
          state = 'f';
        }
      }
      break;
    case 'c':
      {
        state = 'd';
      }
      break;
    case 'f':
      {
        state = 'd';
      }
      break;
    case 'd':
      {
        state = 'w';
      }
      break;
  }

}

void scrambleArray(unsigned char * array, int size)
{
  unsigned char last = 0;
  unsigned char temp = array[last];
  for (int i = 0; i < size; i++) {
    int index = random(size);
    array[last] = array[index];
    last = index;
  }
  array[last] = temp;
}

int lessThan(unsigned char * array, int current)
{
  int min = 0;
  for (int i = 0; i < 4; i++) {
    if (array[i] < array[current]) {
      min++;
    }
  }
  return min;
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  Wire.write(myStrikes); // respond with message of 1 bytes
  // as expected by master
}

void receiveEvent(int howMany) {
  while (0 < Wire.available()) {
    totalStrikes = Wire.read(); // receive byte as a character
    Serial.print(totalStrikes); // print the character
  }
}

