#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include "bitmaps.h"

// Display Pins

// Chip select pins
#define displaycs1 7
#define displaycs2 8
#define displaycs3 9
#define displaycs4 10

#define dcPin 5
#define rstPin 6
#define dummyrstPin 20

Adafruit_PCD8544 display1 = Adafruit_PCD8544(dcPin, displaycs1, rstPin);
Adafruit_PCD8544 display2 = Adafruit_PCD8544(dcPin, displaycs2, dummyrstPin);
Adafruit_PCD8544 display3 = Adafruit_PCD8544(dcPin, displaycs3, dummyrstPin);
Adafruit_PCD8544 display4 = Adafruit_PCD8544(dcPin, displaycs4, dummyrstPin);

Adafruit_PCD8544* displayArray[] = {&display1, &display2, &display3, &display4};

static const unsigned char PROGMEM contrast[] = {50, 30, 30, 50};

// Counters
int i;

// Bitmap columns
static const unsigned char PROGMEM column[] = {24,11,25,10,6,8,20};
static const unsigned char PROGMEM column2[] = {14,24,20,22,2,8,17};
static const unsigned char PROGMEM column3[] = {0,7,22,4,13,25,2};
static const unsigned char PROGMEM column4[] = {9,18,26,6,4,17,3};
static const unsigned char PROGMEM column5[] = {21,3,26,19,18,16,1};
static const unsigned char PROGMEM column6[] = {9,14,23,12,21,15,5};

static unsigned char randArray[] = {0,1,2,3,4,5,6};

void setup() {
  Serial.begin(9600);

  randomSeed(analogRead(6));
  
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
  scrambleArray(randArray,7);

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
}

void scrambleArray(unsigned char * array, int size)
{
  unsigned char last = 0;
  unsigned char temp = array[last];
  for (int i=0; i<size; i++)
  {
    int index = random(size);
    array[last] = array[index];
    last = index;
  }
  array[last] = temp;
}

