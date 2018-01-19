/*
 * Build18 S18
 * Bomb Lab 2.0 - On the Subject of Flashing Colors
 * 
 * The circuit:
 * - LCD VSS pin to ground
 * - LCD VDD pin to 5V
 * - LCD VO pin to a resistor and ground
 * - LCD RS pin to digital pin 12
 * - LCD R/W pin to ground
 * - LCD Enable pin to digital pin 11
 * - LCD D4 pin to digital pin 5
 * - LCD D5 pin to digital pin 4
 * - LCD D6 pin to digital pin 3
 * - LCD D7 pin to digital pin 2
 * - LCD A pin to 5V
 * - LCD R pin to digital pin 7
 * - LCD G pin to digital pin 8
 * - LCD B pin to digital pin 10
 * - 10K resistor: ends to +5V and ground
 * 
 * References:
 * http://www.arduino.cc/en/Tutorial/LiquidCrystalHelloWorld
 * http://howtomechatronics.com/tutorials/arduino/how-to-use-a-rgb-led-with-arduino/
 * https://www.baldengineer.com/tips/arduino-pinmode-on-analog-inputs
 * 
 */


// include the Liquid Crystal library code:
#include <LiquidCrystal.h>
#include <Bounce2.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 13, en = 12, d4 = 7, d5 = 4, d6 = 3, d7 = 2;
const int redPin = 9, greenPin = 10, bluePin = 11;
const int delayTime = 1000;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Speaker pin
const int speakerPin = 6;

// LED pin
const int LEDPin = 5;

// Button pins
const int yesButtonPin = 8; //?
const int noButtonPin = A1; //?
Bounce yesButton = Bounce(); //?
Bounce noButton = Bounce(); //?
bool yesState;
bool noState;

int defused = 0;
int wordNum = 0;

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  //lcd.print("HELLO WORLD!");
  // set up LCD's RGB pins
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  
  pinMode(LEDPin, OUTPUT);
  pinMode(speakerPin, OUTPUT);
  
  pinMode(yesButtonPin, INPUT_PULLUP);
  pinMode(noButtonPin, INPUT_PULLUP);
  yesButton.attach(yesButtonPin);
  noButton.attach(noButtonPin);
  
  setBlackColor();
  lcd.clear();

  Serial.begin(9600);
}

void loop() {
  yesButton.update();
  noButton.update();
  yesState = yesButton.fell();
  noState = noButton.fell();
  if (yesState || noState)
  {
    lastWordIsWhite(2);
    return;
  }
  else
  {
    
  }
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  // print the number of seconds since reset:
  //lcd.print(millis() / 1000);
  sequence0();
  return;
}

void sequence() {
  lcd.setCursor(6, 0);
  lcd.print("RED");
  setRedColor(); // Red Color
  delay(delayTime);
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("GREEN");
  setGreenColor(); // Green Color
  delay(delayTime);
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("YELLOW");
  setYellowColor(); // Yellow Color
  delay(delayTime);
  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print("BLUE");
  setBlueColor(); // Blue Color
  delay(delayTime);
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("WHITE");
  setWhiteColor(); // White Color
  delay(delayTime);
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("MAGENTA");
  setMagentaColor(); // Magenta Color //255
  delay(delayTime);
  lcd.clear();
}
//millis
void sequence0() {
  Serial.print(wordNum);
  switch(wordNum)
  {
    case 0 :
      lcd.setCursor(6, 0);
      lcd.print("BLUE");
      setYellowColor();
      delay(delayTime);
      lcd.clear();
      wordNum = wordNum + 1;
      break;
    case 1 :
      lcd.setCursor(6, 0);
      lcd.print("BLUE");
      setBlueColor();
      delay(delayTime);
      lcd.clear();
      wordNum = wordNum + 1;
      break;
    case 2 :
      lcd.setCursor(6, 0);
      lcd.print("BLUE");
      setRedColor();
      delay(delayTime);
      lcd.clear();
      wordNum = wordNum + 1;
      break;
    case 3 :
      lcd.setCursor(5, 0);
      lcd.print("WHITE");
      setGreenColor();
      delay(delayTime);
      lcd.clear();
      wordNum = wordNum + 1;
      break;
    case 4 :
      lcd.setCursor(5, 0);
      lcd.print("YELLOW");
      setYellowColor();
      delay(delayTime);
      lcd.clear();
      wordNum = wordNum + 1;
      break;
    case 5 :
      lcd.setCursor(5, 0);
      lcd.print("WHITE");
      setMagentaColor();
      delay(delayTime);
      lcd.clear();
      wordNum = wordNum + 1;
      break;
    case 6 :
      lcd.setCursor(4, 0);
      lcd.print("MAGENTA");
      setYellowColor();
      delay(delayTime);
      lcd.clear();
      wordNum = wordNum + 1;
      break;
    case 7 :
      lcd.setCursor(4, 0);
      lcd.print("MAGENTA");
      setWhiteColor();
      delay(delayTime);
      lcd.clear();
      wordNum = 0;
      break;
    default :
      setBlackColor();
      lcd.clear();
      break;

  }
  return;
}

int checkDisplayTime()
{
  
}
int checkButton(int buttonPin) 
{
  // read the state of the pushbutton value:
  int buttonState = digitalRead(buttonPin);
 
        Serial.print("i did not hit a butt\n");
  // check if the pushbutton is pressed.
  // if it is, the buttonState is LOW:
  if (buttonState == LOW) {
        Serial.print("i hit the no button\n");
    return 1;
  } 
  else {
    return 0;
  }
}
  
void lastWordIsWhite(int order)
{
    if (order == 0)
    {
      
    }
    else if (order == 1)
    {
      
    }
    else
    {
      if (noState == true)
      {
        wordNum = 8;
        lcd.noDisplay();
        setBlackColor();
        digitalWrite(LEDPin, HIGH);
        //while(1);
      }
    }
}
void setRedColor() 
{
  analogWrite(redPin, 0);
  analogWrite(greenPin, 255);
  analogWrite(bluePin, 255);
}

void setBlueColor() 
{
  analogWrite(redPin, 255);
  analogWrite(greenPin, 255);
  analogWrite(bluePin, 0);
}

void setGreenColor() 
{
  analogWrite(redPin, 255);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 255);
}

void setYellowColor() 
{
  analogWrite(redPin, 0);
  analogWrite(greenPin, 130);
  analogWrite(bluePin, 255);
}

void setMagentaColor() 
{
  analogWrite(redPin, 0);
  analogWrite(greenPin, 255);
  analogWrite(bluePin, 0);
}

void setWhiteColor() 
{
  analogWrite(redPin, 0);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 0);
}

void setBlackColor() 
{
  analogWrite(redPin, 255);
  analogWrite(greenPin, 255);
  analogWrite(bluePin, 255);
}

