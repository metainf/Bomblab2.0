#include <SoftwareSerial.h>
#include <Wire.h>
#include <Bounce2.h>
#include <Tone.h>

int display_number;
int choice1;
int choice2;
int choice3;
int choice4;

// display pins
const int display_a = 11;
const int display_b = 12;
const int display_c = 15;
const int display_d = 16;
const int display_e = 17;
const int display_f = 13;
const int display_g = 14;

// button input pins
const int button1 = 23; // analog input pins are designated A0 - A3
const int button2 = 24;
const int button3 = 25;
const int button4 = 26;

// shifter pins
const int shifter_clock = 2;
const int shifter_input = 32;

// completion pins
const int complete = 10;

// state
int stage = 1; 

void display1() {
  digitalWrite(display_a, LOW);
  digitalWrite(display_b, HIGH);
  digitalWrite(display_c, HIGH);
  digitalWrite(display_d, LOW);
  digitalWrite(display_e, LOW);
  digitalWrite(display_f, LOW);
  digitalWrite(display_g, LOW);
}

void display2() {
  digitalWrite(display_a, HIGH);
  digitalWrite(display_b, HIGH);
  digitalWrite(display_c, LOW);
  digitalWrite(display_d, HIGH);
  digitalWrite(display_e, HIGH);
  digitalWrite(display_f, LOW);
  digitalWrite(display_g, HIGH);
}

void display3() {
  digitalWrite(display_a, HIGH);
  digitalWrite(display_b, HIGH);
  digitalWrite(display_c, HIGH);
  digitalWrite(display_d, HIGH);
  digitalWrite(display_e, LOW);
  digitalWrite(display_f, LOW);
  digitalWrite(display_g, HIGH);
}
void display4() {
  digitalWrite(display_a, LOW);
  digitalWrite(display_b, HIGH);
  digitalWrite(display_c, HIGH);
  digitalWrite(display_d, LOW);
  digitalWrite(display_e, LOW);
  digitalWrite(display_f, HIGH);
  digitalWrite(display_g, HIGH);
}

void setDisplay(int display_number) {
  if (display_number == 1) {
    display1();
  } else if (display_number == 2) {
    display2();
  } else if (display_number == 3) {
    display3();
  } else if (display_number == 4) {
    display4();
  }
}

void setChoice() {
  choice1 = random(1, 5);
  choice2 = random(1, 5);
  choice3 = random(1, 5);
  choice4 = random(1, 5);
  String s_choice1 = String(choice1);
  String s_choice2 = String(choice2);
  String s_choice3 = String(choice3);
  String s_choice4 = String(choice4);
  String initial_choice = String(s_choice1 + s_choice2 + s_choice3 + s_choice4);
  s7s.print(initial_choice); // Print initial random values onto the display
}

void setup() {
  // put your setup code here, to run once:
  // construct initial values
  randomSeed(analogRead(0));
  display_number = random(1, 5);
  
  // set all display pins as output
  pinmode(display_a, OUTPUT);
  pinmode(display_b, OUTPUT);
  pinmode(display_c, OUTPUT);
  pinmode(display_d, OUTPUT);
  pinmode(display_e, OUTPUT);
  pinmode(display_f, OUTPUT);
  pinmode(display_g, OUTPUT);

  // manual set of display
  setDisplay(display_number);

  // Must begin s7s software serial at the correct baud rate.
  // The default of the s7s is 9600.
  s7s.begin(9600);

  // Clear the display, and then turn on all segments and decimals
  clearDisplay();  // Clears display, resets cursor
  s7s.print("----");  // Displays ---- on all digits

  setBrightness(255);  // High brightness
  delay(1500);

  // Clear the display before jumping into loop
  clearDisplay();
  // set choice display
  setChoice();
  
  // set clock
  pinmode(shifter_clock, OUTPUT);
  tone(shifter_clock, 100);

  // set progress LED to 1 via shifter
  pinmode(shifter_input, OUTPUT);
  Serial.write(01000000) // fully on is 011111000

  // set button pins as input
  pinmode(button1, INPUT);
  pinmode(button2, INPUT);
  pinmode(button3, INPUT);
  pinmode(button4, INPUT);

  // set complete LED to off
  pinmode(complete, OUTPUT);
  digitalWrite(complete, LOW);
}

// call when an incorrect button is pushed
void wrong() {
  
}

// call when the correct button is pushed
void pass() {
  
}

void loop() {
  
}
