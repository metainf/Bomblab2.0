#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>
#include <Wire.h>
#include <Tone.h>


/*********************************************************************
This is a library for our Monochrome OLEDs based on SSD1325 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

These displays use SPI to communicate, 4 or 5 pins are required to  
interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen below must be included in any redistribution
*********************************************************************/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1325.h>

// If using software SPI, define CLK and MOSI
#define OLED_CLK 12
#define OLED_MOSI 11

// These are neede for both hardware & softare SPI
#define OLED_CS 10
#define OLED_RESET 9
#define OLED_DC 8

// this is software SPI, slower but any pins
Adafruit_SSD1325 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// this is for hardware SPI, fast! but fixed oubs
//Adafruit_SSD1325 display(OLED_DC, OLED_RESET, OLED_CS);

char state; //s - standby, i - initial, h - half, d - defused, r - reset, f - fail

//pins
const int buzzerPin = 2;
const int switchPin = 3;
const int ledPin = 4;

int switchVal = 0;

//array info
const int arraySize = 25;
int index;

//i2c variables
char* timer = "1234"; //format: mmss. received from master
char gameStatus = '0'; //status/strikes information received from master
char strikes = '0'; //local strikes counter, sent to master upon request

Tone buzz;

const char s0[] PROGMEM = "blank";
const char s1[] PROGMEM = "literally blank";
const char s2[] PROGMEM = "..";
const char s3[] PROGMEM = "period period";
const char s4[] PROGMEM = ".period";
const char s5[] PROGMEM = "period twice";
const char s6[] PROGMEM = "1 3 2 4";
const char s7[] PROGMEM = "one and then 3 to 4";
const char s8[] PROGMEM = "one three two four";
const char s9[] PROGMEM = "one three to for";
const char s10[] PROGMEM = "one 3 2 4";
const char s11[] PROGMEM = "1 3 2 for";
const char s12[] PROGMEM = "1 3 too 4";
const char s13[] PROGMEM = "1 3 to 4";
const char s14[] PROGMEM = "dot dot";
const char s15[] PROGMEM = "one in letters 3 2 4 in numbers";
const char s16[] PROGMEM = "left left right left right right";
const char s17[] PROGMEM = "it literally says the word one and then the numbers 3 2 4";
const char s18[] PROGMEM = "<-";
const char s19[] PROGMEM = "left arrow";
const char s20[] PROGMEM = "left arrow symbol";
const char s21[] PROGMEM = "an actual left arrow";
const char s22[] PROGMEM = "the word blank";
const char s23[] PROGMEM = "left";
const char s24[] PROGMEM = "we just blew up";

//array of text strings
const char* const text[] PROGMEM = {
  s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20, s21, s22, s23, s24
  };
char downAction[arraySize] = {'1', '1', '8', '8', '8', '8', '3', '4', '3', '3', '3', '1', '2', '3', '8', '3', '6', '4', '6', '6', '6', '6', '0', '6', '4'};
char upAction[arraySize] = {'3', '5', '5', '2', '1', '3', '2', '7', '7', '9', '4', '0', '0', '0', '6', '5', '7', '5', '3', '8', '4', '2', '1', '9', '2'};
  
void setup()   {
  Wire.begin(5);                // join i2c bus with address #5
  Wire.onReceive(receiveEvent); // register receive event
  Wire.onRequest(requestEvent); // register request event
  
  //start serial
  Serial.begin(9600);
  Serial.println("Crazy Talk Module");

  //setup pins
  pinMode(switchPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  //setup the buzzer
  buzz.begin(buzzerPin);

  state = 's';
}

void loop() {
  switchVal = digitalRead(switchPin);
  
  switch (state) { //1 = o down, 0 = | down. initial state = 0
    case 's': //standby - used to wait for game start signal
      break;
    case 'i': //initial - starting position before any inputs. switch should be up
      if(switchVal == 1) { //switch is down
        if(inTimer(downAction[index])) {
          //beep
          buzz.play(NOTE_C5, 200);
          delay(250);
          buzz.play(NOTE_E5, 200);

          Serial.println("state i to h");
          state = 'h';  
        }
        else {
          //add a strike
          addStrike();
          buzz.play(NOTE_DS2, 200);

          Serial.println("state i to r");
          state = 'r';
        }
      }
      break;
    
    case 'h': //half - state after getting the first input correct
       if(switchVal == 0) { //switch is up
         if(inTimer(upAction[index])) {
          //beep
          buzz.play(NOTE_C5, 200);
          delay(250);
          buzz.play(NOTE_E5, 200);

          Serial.println("state h to d");
          state = 'd';
          digitalWrite(4, HIGH); 
         }
         
         else {
         //add a strike
         addStrike();
         buzz.play(NOTE_DS2, 200);

         Serial.println("state h to r");
         state = 'r';
       }
       }
      break;
    
    case 'd': //defused!
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("defused!");
      display.display();
      break;
    
    case 'r': //reset - prompts user to reset switch position if needed, then generates random index before moving to initial
      if(switchVal == 0) { //switch is up
        randomSeed(analogRead(0));
        index = random(0, arraySize);
        Serial.println(index);
        Serial.println(downAction[index]);
        Serial.println(upAction[index]);

        char buf[60];
        display.clearDisplay();
        strcpy_P(buf, (char*)pgm_read_word(&(text[index]))); //prints randomly selected text
        Serial.println(buf);
        display.println(buf);
        display.display();

        Serial.println("state r to i");
        state = 'i';
      }
      if(switchVal == 1) { //switch is down
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("please flip switch up");
        display.display();
      }
      break;
    
    case 'f': //fail
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("you failed!");
        display.display();
      break;  

    delay(500);
  }
}

bool inTimer(char x) {
    for(int i = 0; i < 4; i++) {
      if(x == timer[i]) {
        return true;
      }
    }
    return false;
}

void receiveEvent(int howMany) {
  char* c;
  for(int i = 0; i < howMany; i++) {
      c[i] = Wire.read();
  }
  if(howMany == 1) {
    gameStatus = c;
    switch(gameStatus) {
      case '3':
        state = 'f';
        break;
      case 's': //game has started
        state = 'r'; //move from standby to reset to generate text
      case 'r': //game reset
        strikes = '0'; //reset strikes counter
        digitalWrite(4, LOW); //reset LED
        state = 's'; //move to standby
      default:  
        break;
    }
  }
  if(howMany == 4) {
    timer = c;
  }
}

void requestEvent() {
  Wire.write(strikes);
}

void addStrike() {
  if(strikes == '0') strikes = '1';
  else if(strikes == '1') strikes = '2';
  else strikes = '3';
}

