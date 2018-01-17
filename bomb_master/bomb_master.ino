/* Serial 7-Segment Display Example Code
    Serial Mode Stopwatch
   by: Jim Lindblom
     SparkFun Electronics
   date: November 27, 2012
   license: This code is public domain.

   This example code shows how you could use software serial
   Arduino library to interface with a Serial 7-Segment Display.

   There are example functions for setting the display's
   brightness, decimals and clearing the display.

   The print function is used with the SoftwareSerial library
   to send display data to the S7S.

   Circuit:
   Arduino -------------- Serial 7-Segment
     5V   --------------------  VCC
     GND  --------------------  GND
      8   --------------------  RX
*/
#include <SoftwareSerial.h>

// These are the Arduino pins required to create a software seiral
//  instance. We'll actually only use the TX pin.
const int softwareTx = 2;
const int softwareRx = 7;

// LED strike pins
const int strike0 = 9;
const int strike1 = 8;
const int strike2 = 7;

// Input button pins
const int startButton = 4;
const int diffButton = 5;

// Buzzer pin
const int buzzer = 3;

SoftwareSerial s7s(softwareRx, softwareTx);

unsigned int strikes = 0;     // The total number of strikes
unsigned int difficulty = 0;  // The difficulty of the game, 0 through 9
char state = 'w';             // The state of the game
/*
   w = waiting state, waiting for the user to start the game after selecting difficulty
   b = begin state, setup game to start.
   r = running state, game in progress
   c = completed, CT win, bomb has been defused
   f = failed, T win, bomb has exploded
*/
int startBttnState;
int startBttnStateLast = LOW;

int diffBttnState;
int diffBttnStateLast = LOW;

unsigned long startDebounceTimeLast = 0;  // the last time the output pin was toggled
unsigned long diffDebounceTimeLast = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

unsigned int counter = 0;     // This variable will count up to 65k
char tempString[10];  // Will be used with sprintf to create strings

void setup()
{

  // Setup the pins:
  pinMode(startButton, INPUT);
  pinMode(diffButton, INPUT);

  pinMode(strike0, OUTPUT);
  pinMode(strike1, OUTPUT);
  pinMode(strike2, OUTPUT);

  digitalWrite(strike0, LOW);
  digitalWrite(strike1, LOW);
  digitalWrite(strike2, LOW);

  // Must begin s7s software serial at the correct baud rate.
  //  The default of the s7s is 9600.
  s7s.begin(9600);

  // Clear the display, and then turn on all segments and decimals
  clearDisplay();  // Clears display, resets cursor
  s7s.print("----");  // Displays ---- on all digits

  setBrightness(255);  // High brightness
  delay(1500);

  // Clear the display before jumping into loop
  clearDisplay();

  s7s.print("L0  "); // Print lvl0 onto the display
}

void loop()
{

  switch (state)
  {
    case 'w':
      {

        // Read the state of the difficulty button
        int diffRead = digitalRead(diffButton);

        // check to see if you just pressed the button
        // (i.e. the input went from LOW to HIGH), and you've waited long enough
        // since the last press to ignore any noise:
        if (diffRead != diffBttnStateLast) {
          diffDebounceTimeLast = millis();
        }

        if (millis() - diffDebounceTimeLast > debounceDelay) {
          // If the button state has not changed, record the state

          // If the state has changed from the last recording
          if (diffRead != diffBttnState) {
            diffBttnState = diffRead;


            // Increase the difficulty if there was a postive edge
            if ( diffBttnState == HIGH) {
              difficulty++;
              difficulty = difficulty % 10;
              // print out the difficulty onto the display
              sprintf(tempString, "L%d  ", difficulty);
              s7s.print(tempString);
            }
          }
        }
        diffBttnStateLast = diffRead;

        // Read the state of the start button
        int startRead = digitalRead(startButton);

        // check to see if you just pressed the button
        // (i.e. the input went from LOW to HIGH), and you've waited long enough
        // since the last press to ignore any noise:
        if (startRead != startBttnStateLast) {
          startDebounceTimeLast = millis();
        }
        if (millis() - startDebounceTimeLast > debounceDelay) {
          // If the button state has not changed, record the state

          // If the state has changed from the last recording
          if (startRead != startBttnState) {
            startBttnState = startRead;
          }
        }
        // If the start button is high, begin the game
        if (startBttnState == HIGH) {
          state = 'b';
        }
        startBttnStateLast = startRead;
      }
      break;
    case 'b':
      {
        /* Setup the timer depending on the difficulty.
         * 0 - 5 is 10 minutes 
         * 6 - 9 is 5 minutes
        */
        
      }
      break
    case 'r':
      break;
    case 'c':
      break;
    case 'f':
      break;
  }
  //  // Magical sprintf creates a string for us to send to the s7s.
  //  //  The %4d option creates a 4-digit integer.
  //  sprintf(tempString, "%4d", counter);
  //
  //  // This will output the tempString to the S7S
  //  s7s.print(tempString);
  //  setDecimals(0b00000100);  // Sets digit 3 decimal on
  //
  //  counter++;  // Increment the counter
  //  delay(100);  // This will make the display update at 10Hz.
}

// Send the clear display command (0x76)
//  This will clear the display and reset the cursor
void clearDisplay()
{
  s7s.write(0x76);  // Clear display command
}

// Set the displays brightness. Should receive byte with the value
//  to set the brightness to
//  dimmest------------->brightest
//     0--------127--------255
void setBrightness(byte value)
{
  s7s.write(0x7A);  // Set brightness command byte
  s7s.write(value);  // brightness data byte
}

// Turn on any, none, or all of the decimals.
//  The six lowest bits in the decimals parameter sets a decimal
//  (or colon, or apostrophe) on or off. A 1 indicates on, 0 off.
//  [MSB] (X)(X)(Apos)(Colon)(Digit 4)(Digit 3)(Digit2)(Digit1)
void setDecimals(byte decimals)
{
  s7s.write(0x77);
  s7s.write(decimals);
}
