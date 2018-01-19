
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
#include <Wire.h>
#include <Bounce2.h>
#include <Tone.h>

#define numMod 5

// These are the Arduino pins required to create a software serial
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
const int buzzerPin = 3;

SoftwareSerial s7s(softwareRx, softwareTx);

Bounce start = Bounce();
Bounce diff = Bounce();
Tone buzz;

char strikes = '0';     // The total number of strikes
unsigned int difficulty = 0;  // The difficulty of the game, 0 through 3
char state = 'w';             // The state of the game
volatile unsigned int finishedMod = 0; // The number of finished modules

// i2c addr for the modules
// modAddr[0] is the module that requires the timer info
const int modAddr[] = {2, 4, 6, 8, 10};
/*
   w = waiting state, waiting for the user to start the game after selecting difficulty
   b = begin state, setup game to start.
   r = running state, game in progress
   c = completed, CT win, bomb has been defused
   f = failed, T win, bomb has exploded
*/

unsigned long maxTime = 15UL * 60UL * 1000UL; // The maximum value of time in ms
unsigned long startTime;
//volatile char tempString[10];  // Will be used with sprintf to create strings
String tempString;
String timeString;

void setup()
{

  // Open serial port for debugging
  Serial.begin(9600);

  // Start the random seed
  randomSeed(analogRead(0));

  // Setup the pins:
  pinMode(startButton, INPUT);
  pinMode(diffButton, INPUT);

  pinMode(strike0, OUTPUT);
  pinMode(strike1, OUTPUT);
  pinMode(strike2, OUTPUT);

  digitalWrite(strike0, LOW);
  digitalWrite(strike1, LOW);
  digitalWrite(strike2, LOW);

  // Start i2c and join as the master
  Wire.begin();

  // Attach the debounced buttons to the pins
  start.attach(startButton);
  diff.attach(diffButton);

  // Setup the debounce time in millis
  start.interval(25);
  diff.interval(25);

  // Setup the buzzer
  buzz.begin(buzzerPin);


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
        diff.update();

        // Increase the difficulty if there was a postive edge
        if ( diff.rose()) {
          difficulty++;
          difficulty = difficulty % 4;
          // print out the difficulty onto the display
          tempString = String("L" + String(difficulty, DEC) + "  ");
          s7s.print(tempString);
        }

        // Read the state of the start button
        start.update();

        // If the start button was pressed, begin the game
        if (start.rose()) {
          // TODO: SEND THE START SIGNAL TO ALL MODULES
          sendChar('s');
          state = 'b';
        }
      }
      break;
    case 'b':
      {
        strikes = '0';
        finishedMod = 0;
        /* Setup the maxTime depending on the difficulty.
           0 is 15 minutes
           1 is 10 minutes
           2 is 5 minutes
           3 is 5 minutes, start with 2 strikes
        */
        maxTime -= 5UL * 60UL * 1000UL * difficulty;
        if (difficulty == 3) {
          maxTime = 5UL * 60UL * 1000UL;
          strikes = '2';
        }

        delay(1000);

        // Setup the start time
        startTime = millis();
        state = 'r';
      }
      break;
    case 'r':
      {
        // Display the current time
        static unsigned long oldDisplayedTime = maxTime / 1000; // The previous time to display in seconds
        unsigned long elapsedTime = millis() - startTime; // The ammount of time elasped in ms
        unsigned long timeLeft = maxTime - elapsedTime;   // The ammount of time left in ms
        unsigned long displayedTime = timeLeft / 1000;    // The ammount of time left in sec
        if (displayedTime != oldDisplayedTime)
        {
          String min = String(displayedTime / 60);
          String sec = String(displayedTime % 60);
          if (sec.length() < 2)
          {
            sec = String("0" + sec);
          }
          if (min.length() < 2)
          {
            min = String("0" + min);
          }
          timeString = String(min + sec);
          s7s.print(timeString);
          setDecimals(0b00010000); // Set the colon on

          // Play a tone
          buzz.play(NOTE_C7, 100);
          delay(250);
          buzz.play(NOTE_A6, 50);
          oldDisplayedTime = displayedTime;
        }
        // Check if the player is out of time
        if (elapsedTime > maxTime) {
          state = 'f';
        }

        // Communicate with the other modules

        // Send the time to each module
        sendTime();

        // Send the number of strikes to each module
        sendChar(strikes);

        // Get the status of each module
        getStatus();

        // Check the number of strikes
        if (strikes == '3') {
          state = 'f';
        }

        // Check how many modules have finished
        if (finishedMod == 1) {
          state = 'c';
        }

        // Reset the oldDisplayedTime
        if (state != 'r') {
          oldDisplayedTime = maxTime / 1000;
        }
        Serial.print(strikes);
        Serial.println(finishedMod);

      }
      break;
    case 'c':
      {
        buzz.play(NOTE_C6, 150);
        delay(200);
        buzz.play(NOTE_C6, 150);
        delay(200);
        s7s.print("dfsd");
        state = 'd';
      }
      break;
    case 'f':
      {
        // Message all modules that player has failed.
        sendChar('3');
        clearDisplay();
        s7s.print(F("----"));
        buzz.play(NOTE_F1, 3000);
        delay(3000);
        clearDisplay();
        int randNumber = random(6);
        switch (randNumber) {
          case 0:
            s7s.print(F("n00b"));
            break;
          case 1:
            s7s.print(F("F---"));
            break;
          case 2:
            s7s.print(F("FAIL"));
            break;
          case 3:
            s7s.print(F("OOpS"));
            break;
          case 4:
            s7s.print(F("SH--"));
            break;
          case 5:
            s7s.print(F("NOOO"));
            break;
        }
        state = 'd';
      }
      break;
    case 'd':
      {
        // Read the state of the start button
        start.update();

        // If the start button was pressed, go back to the waiting state
        if (start.rose()) {
          clearDisplay();
          tempString = String("L" + String(difficulty, DEC) + "  ");
          s7s.print(tempString);
          sendChar('r');
          state = 'w';
        }

      }
      break;
  }
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

void sendChar(char c) {
  for (int x = 0; x < numMod; x++) {
    Wire.beginTransmission(modAddr[x]);
    Wire.write(c);
    Wire.endTransmission();
  }
}

void getStatus() {
  char totalStrikes = 0;
  unsigned int totalFinished = 0;
  for (int x = 0; x < numMod; x++) {
    Wire.requestFrom(modAddr[x], 1);
    char c = Wire.read();
    switch (c) {
      case '0':
        break;
      case '1':
        totalStrikes++;
        break;
      case '2':
        totalStrikes += 2;
        break;
      case '3':
        totalStrikes += 3;
        break;
      case 'c':
        totalFinished++;
        break;
    }
  }
  if (totalStrikes > 3) {
    totalStrikes = 3;
  }
  strikes = totalStrikes + '0';
  finishedMod = totalFinished;
}



void sendTime() {
  char buf[10];
  timeString.toCharArray(buf, 4);
  Wire.beginTransmission(modAddr[0]);
  Wire.write(buf);
  Wire.endTransmission();
}
