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

// Bounce library for input buttons
Bounce read1 = Bounce();
Bounce read2 = Bounce();
Bounce read3 = Bounce();
Bounce read4 = Bounce();

// shifter pins
const int shifter_clock = 2;
const int shifter_input = 32;

// completion pins
const int complete = 10;

// buzzer pins
const int buzzer_pin = 9;

// state
int stage = 1;


// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

// software serial pin
const byte tx_pin = shifter_input;
const byte rx_pin = 2;
SoftwareSerial mySerial (rx_pin, tx_pin);

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

  // setup buzzer
  buzz.begin(buzzer_pin);

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
  mySerial.begin(9600);
  mySerial.write(01000000) // fully on is 011111000

  // set button pins as input
  pinmode(button1, INPUT_PULLUP);
  pinmode(button2, INPUT_PULLUP);
  pinmode(button3, INPUT_PULLUP);
  pinmode(button4, INPUT_PULLUP);

  // set complete LED to off
  pinmode(complete, OUTPUT);
  digitalWrite(complete, LOW);

  // setup buttons
  read1.attach(button1);
  read2.attach(button2);
  read3.attach(button3);
  read4.attach(button4);
  read1.interval(25);
  read2.interval(25);
  read3.interval(25);
  read4.interval(25);
}

// call when an incorrect button is pushed
void fail() {
  // make failing tone
  buzz.play(NOTE_A4, 150);
  // send failure interrupt to master
  
}

// call when the correct button is pushed
void pass() {
  if (stage < 5) {
    buzz.play(NOTE_C6, 150);
    stage++;
    switch (stage) {
      case 1: {
        mySerial.write(01000000);
        break;
      }
      case 2: {
        mySerial.write(01100000);
        break;
      }
      case 3: {
        mySerial.write(01110000);
        break;
      }
      case 4: {
        mySerial.write(01111000);
        break;
      }
      case 5: {
        mySerial.write(01111100);
        break;
      }
    }
    
  } else {
    buzz.play(NOTE_C6, 300);
    digitalWrite(complete, HIGH);
    // return control to the master module
  }
}

void loop() {
  read1.update();
  read2.update();
  read3.update();
  read4.update();

  input1 = read1.fell();
  input2 = read2.fell();
  input3 = read3.fell();
  input4 = read4.fell();
  
  int stage1_pass;
  int stage2_pass;
  int stage3_pass;
  int stage4_pass;
  int stage5_pass;
  
  switch (stage) {
    case 1: {
      switch (display_number) {
        case 1: {
          if (input2) {
            stage1_pass = 2;
            pass();
          } else {fail();}
          break;
        }
        case 2: {
          if (input2) {
            stage1_pass = 2;
            pass();
          } else {fail();}
          break;
        }
        case 3: {
          if (input3) {
            stage1_pass = 3;
            pass();
          } else {fail();}
          break;
        }
        case 4: {
          if (input4) {
            stage1_pass = 4;
            pass();
          } else {fail():}
        }
      }
      break;
    }
    case 2: {
      switch (display_number) {
        case1: {
          if (input1 && choice1 = 4) {
            stage2_pass = 1;
            pass();
          } else if (input2 && choice2 = 4) {
            stage2_pass = 2;
            pass();
          } else if (input3 && choice3 = 4) {
            stage2_pass = 3;
            pass();
          } else if (input4 && choice4 = 4) {
            stage2_pass = 4;
            pass();
          } else {fail();}
          break;
        }
        case2: {
          if (input1 && stage1_pass = 1) {
            stage2_pass = 1;
            pass();
          } else if (input2 && stage1_pass = 2) {
            stage2_pass = 2;
            pass();
          } else if (input3 && stage1_pass = 3) {
            stage2_pass = 3;
            pass();
          } else if (input4 && stage1_pass = 4) {
            stage2_pass = 4;
            pass();
          } else {fail();}
          break;
        }
        case3: {
          if (input1) {
            stage2_pass = 1;
            pass();
          } else {fail();}
          break;
        }
        case4: {
          if (input1 && stage1_pass = 1) {
            stage2_pass = 1;
            pass();
          } else if (input2 && stage1_pass = 2) {
            stage2_pass = 2;
            pass();
          } else if (input3 && stage1_pass = 3) {
            stage2_pass = 3;
            pass();
          } else if (input4 && stage1_pass = 4) {
            stage2_pass = 4;
            pass();
          } else {fail();}
          break;
        }
      }
      break;
    }
    case 3: {
      switch (display_number) {
        case1: {
          if (input1 && stage2_pass = 1) {
            stage3_pass = 1;
            pass();
          } else if (input2 && stage2_pass = 2) {
            stage3_pass = 2;
            pass();
          } else if (input3 && stage2_pass = 3) {
            stage3_pass = 3;
            pass();
          } else if (input4 && stage2_pass = 4) {
            stage3_pass = 4;
            pass();
          } else {fail();}
          break;
        }
        case2: {
          if (input1 && stage1_pass = 1) {
            stage3_pass = 1;
            pass();
          } else if (input2 && stage1_pass = 2) {
            stage3_pass = 2;
            pass();
          } else if (input3 && stage1_pass = 3) {
            stage3_pass = 3;
            pass();
          } else if (input4 && stage1_pass = 4) {
            stage3_pass = 4;
            pass();
          } else {fail();}
          break;
        }
        case3: {
          if (input3) {
            stage3_pass = 3;
            pass();
          } else {fail();}
          break;
        }
        case4: {
          if (input1 && choice1 = 4) {
            stage3_pass = 1;
            pass();
          } else if (input2 && choice2 = 4) {
            stage3_pass = 2;
            pass();
          } else if (input3 && choice3 = 4) {
            stage3_pass = 3;
            pass();
          } else if (input4 && choice4 = 4) {
            stage3_pass = 4;
            pass();
          } else {fail();}
          break;
        }
      }
      break;
    }
    case 4: {
      switch (display_number) {
        case1: {
          if (input1 && stage1_pass = 1) {
            stage4_pass = 1;
            pass();
          } else if (input2 && stage1_pass = 2) {
            stage4_pass = 2;
            pass();
          } else if (input3 && stage1_pass = 3) {
            stage4_pass = 3;
            pass();
          } else if (input4 && stage1_pass = 4) {
            stage4_pass = 4;
            pass();
          } else {fail();}
          break;
        }
        case2: {
          if (input1) {
            stage4_pass = 1;
            pass();
          } else {fail();}
          break;
        }
        case3: {
          if (input1 && stage2_pass = 1) {
            stage4_pass = 1;
            pass();
          } else if (input2 && stage2_pass = 2) {
            stage4_pass = 2;
            pass();
          } else if (input3 && stage2_pass = 3) {
            stage4_pass = 3;
            pass();
          } else if (input4 && stage2_pass = 4) {
            stage4_pass = 4;
            pass();
          } else {fail();}
          break;
        }
        case4: {
          if (input1 && stage2_pass = 1) {
            stage4_pass = 1;
            pass();
          } else if (input2 && stage2_pass = 2) {
            stage4_pass = 2;
            pass();
          } else if (input3 && stage2_pass = 3) {
            stage4_pass = 3;
            pass();
          } else if (input4 && stage2_pass = 4) {
            stage4_pass = 4;
            pass();
          } else {fail();}
          break;
        }
      }
      break;
    }
    case 5: {
      switch (display_number) {
        case1: {
          if (input1 && stage1_pass = 1) {
            stage5_pass = 1;
            pass();
          } else if (input2 && stage1_pass = 2) {
            stage5_pass = 2;
            pass();
          } else if (input3 && stage1_pass = 3) {
            stage5_pass = 3;
            pass();
          } else if (input4 && stage1_pass = 4) {
            stage5_pass = 4;
            pass();
          } else {fail();}
          break;
        }
        case2: {
          if (input1 && stage2_pass = 1) {
            stage5_pass = 1;
            pass();
          } else if (input2 && stage2_pass = 2) {
            stage5_pass = 2;
            pass();
          } else if (input3 && stage2_pass = 3) {
            stage5_pass = 3;
            pass();
          } else if (input4 && stage2_pass = 4) {
            stage5_pass = 4;
            pass();
          } else {fail();}
          break;
        }
        case3: {
          if (input1 && stage4_pass = 1) {
            stage5_pass = 1;
            pass();
          } else if (input2 && stage4_pass = 2) {
            stage5_pass = 2;
            pass();
          } else if (input3 && stage4_pass = 3) {
            stage5_pass = 3;
            pass();
          } else if (input4 && stage4_pass = 4) {
            stage5_pass = 4;
            pass();
          } else {fail();}
          break;
        }
        case4: {
          if (input1 && stage3_pass = 1) {
            stage5_pass = 1;
            pass();
          } else if (input2 && stage3_pass = 2) {
            stage5_pass = 2;
            pass();
          } else if (input3 && stage3_pass = 3) {
            stage5_pass = 3;
            pass();
          } else if (input4 && stage3_pass = 4) {
            stage5_pass = 4;
            pass();
          } else {fail();}
          break;
        }
      }
      break;
    }
  }
}
