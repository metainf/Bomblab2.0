#include <Tone.h>
#include <Wire.h>
#include <Bounce2.h>
#include <SoftwareSerial.h>

int display_number;
int choice1;
int choice2;
int choice3;
int choice4;

// display pins
const int display_a = 7;
const int display_b = 8;
const int display_c = 11;
const int display_d = 12;
const int display_e = 13;
const int display_f = 9;
const int display_g = 10;

// choice display
const int choice_display = 3;

// choice array
const int choice_array[] = {1234, 1243, 1324, 1342, 1423, 1432, 2134, 2143, 2314, 2341, 2413, 2431, 3124, 3142, 3214, 3241, 3412, 3421, 4123, 4132, 4213, 4231, 4312, 4321};

// button input pins
const int button1 = 14; // analog input pins are designated A0 - A3
const int button2 = 15;
const int button3 = 16;
const int button4 = 17;

// Bounce library for input buttons
Bounce read1 = Bounce();
Bounce read2 = Bounce();
Bounce read3 = Bounce();
Bounce read4 = Bounce();

// shifter pins
const int shifter_shift = 6;
const int shifter_input = 2;
const int shifter_show = 4;

// buzzer pins
const int buzzer_pin = 5;
Tone buzz;

// state
int stage = 1;
int strikes = 0;
bool solved = false;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

// software serial pin
const byte rx_pin = 0;
SoftwareSerial s7s(rx_pin, choice_display);

void display1() {
  digitalWrite(display_a, HIGH);
  digitalWrite(display_b, LOW);
  digitalWrite(display_c, LOW);
  digitalWrite(display_d, HIGH);
  digitalWrite(display_e, HIGH);
  digitalWrite(display_f, HIGH);
  digitalWrite(display_g, HIGH);
}

void display2() {
  digitalWrite(display_a, LOW);
  digitalWrite(display_b, LOW);
  digitalWrite(display_c, HIGH);
  digitalWrite(display_d, LOW);
  digitalWrite(display_e, LOW);
  digitalWrite(display_f, HIGH);
  digitalWrite(display_g, LOW);
}

void display3() {
  digitalWrite(display_a, LOW);
  digitalWrite(display_b, LOW);
  digitalWrite(display_c, LOW);
  digitalWrite(display_d, LOW);
  digitalWrite(display_e, HIGH);
  digitalWrite(display_f, HIGH);
  digitalWrite(display_g, LOW);
}
void display4() {
  digitalWrite(display_a, HIGH);
  digitalWrite(display_b, LOW);
  digitalWrite(display_c, LOW);
  digitalWrite(display_d, HIGH);
  digitalWrite(display_e, HIGH);
  digitalWrite(display_f, LOW);
  digitalWrite(display_g, LOW);
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
  int choice_pick = choice_array[random(0, 23)];
  int choice1 = choice_pick / 1000;
  int choice2 = choice_pick % 1000 / 100;
  int choice3 = choice_pick % 100 / 10;
  int choice4 = choice_pick % 10;
  String s_choice1 = String(choice1);
  String s_choice2 = String(choice2);
  String s_choice3 = String(choice3);
  String s_choice4 = String(choice4);
  String initial_choice = String(s_choice1 + s_choice2 + s_choice3 + s_choice4);
  s7s.print(initial_choice); // Print initial random values onto the display
}

void receiveEvent(int size) {
  char c = Wire.read();
  switch (c) {
    case '3':
    case 's':
    case 'r': {
      stage = 1;
      setup();
      break;
    }
  }
}

void requestEvent() {
  if (solved) {
    Wire.write('c');
  } else {
    char c_strikes = String(strikes).charAt(0);
    Wire.write(c_strikes);
  }
}

void setup() {
  // put your setup code here, to run once:
  // i2c integration
  Wire.begin(2);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  
  Serial.begin(9600);
  // construct initial values
  randomSeed(analogRead(0));
  
  display_number = random(1, 5);
  // manual set of display
  setDisplay(display_number);
  
  // set all display pins as output
  pinMode(display_a, OUTPUT);
  pinMode(display_b, OUTPUT);
  pinMode(display_c, OUTPUT);
  pinMode(display_d, OUTPUT);
  pinMode(display_e, OUTPUT);
  pinMode(display_f, OUTPUT);
  pinMode(display_g, OUTPUT);

  // setup buzzer
  buzz.begin(buzzer_pin);

  // Must begin s7s software serial at the correct baud rate.
  // The default of the s7s is 9600.
  s7s.begin(9600);

  // Clear the display, and then turn on all segments and decimals
  s7s.write(0x76);  // Clears display, resets cursor
  s7s.print("----");  // Displays ---- on all digits

  s7s.write(0x7A);  // Set brightness command byte
  s7s.write(255); // brightness data byte

  // set choice display
  setChoice();
  
  // shifter setup
  pinMode(shifter_input, OUTPUT);
  pinMode(shifter_shift, OUTPUT);
  pinMode(shifter_show, OUTPUT);
  
  // clear stage LED
  digitalWrite(shifter_input, LOW);
  for (int i = 0; i < 8; i++) {
    digitalWrite(shifter_shift, HIGH);
    delay(10);
    digitalWrite(shifter_shift, LOW);
    delay(10);
  }
  // light stage 1 LED
  digitalWrite(shifter_input, HIGH);
  for (int j = 0; j < 4; j++) {
    digitalWrite(shifter_shift, HIGH);
    delay(10);
    digitalWrite(shifter_shift, LOW);
    delay(10);
  }
  digitalWrite(shifter_show, HIGH);
  delay(10);
  digitalWrite(shifter_show, LOW);
  delay(10);

  // set button pins as input
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);

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
  strikes++;
  display_number = random(1, 5);
  // manual set of display
  setDisplay(display_number);
  setChoice();
  Serial.println(stage);
  // send failure interrupt to master
}

// call when the correct button is pushed
void pass() {
  if (stage < 5) {
    buzz.play(NOTE_C6, 150);
    stage++;
    switch (stage) {
      case 1: {
        break;
      }
      case 2: {
        break;
      }
      case 3: {
        break;
      }
      case 4: {
        break;
      }
      case 5: {
        break;
      }
    }
    display_number = random(1, 5);
    // manual set of display
    setDisplay(display_number);
    setChoice();
    Serial.println(stage);
  } else {
    buzz.play(NOTE_C6, 300);
    solved = true;
    // return control to the master module
  }
}

void loop() {
  read1.update();
  read2.update();
  read3.update();
  read4.update();

  int input1 = read1.fell();
  int input2 = read2.fell();
  int input3 = read3.fell();
  int input4 = read4.fell();

  bool response = input1 || input2 || input3 || input4;
  if (!response) {return;}
  
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
          } else {fail();}
        }
      }
      break;
    }
    case 2: {
      switch (display_number) {
        case 1: {
          if (input1 && choice1 == 4) {
            stage2_pass = 1;
            pass();
          } else if (input2 && choice2 == 4) {
            stage2_pass = 2;
            pass();
          } else if (input3 && choice3 == 4) {
            stage2_pass = 3;
            pass();
          } else if (input4 && choice4 == 4) {
            stage2_pass = 4;
            pass();
          } else {fail();}
          break;
        }
        case 2: {
          if (input1 && stage1_pass == 1) {
            stage2_pass = 1;
            pass();
          } else if (input2 && stage1_pass == 2) {
            stage2_pass = 2;
            pass();
          } else if (input3 && stage1_pass == 3) {
            stage2_pass = 3;
            pass();
          } else if (input4 && stage1_pass == 4) {
            stage2_pass = 4;
            pass();
          } else {fail();}
          break;
        }
        case 3: {
          if (input1) {
            stage2_pass = 1;
            pass();
          } else {fail();}
          break;
        }
        case 4: {
          if (input1 && stage1_pass == 1) {
            stage2_pass = 1;
            pass();
          } else if (input2 && stage1_pass == 2) {
            stage2_pass = 2;
            pass();
          } else if (input3 && stage1_pass == 3) {
            stage2_pass = 3;
            pass();
          } else if (input4 && stage1_pass == 4) {
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
        case 1: {
          if (input1 && stage2_pass == 1) {
            stage3_pass = 1;
            pass();
          } else if (input2 && stage2_pass == 2) {
            stage3_pass = 2;
            pass();
          } else if (input3 && stage2_pass == 3) {
            stage3_pass = 3;
            pass();
          } else if (input4 && stage2_pass == 4) {
            stage3_pass = 4;
            pass();
          } else {fail();}
          break;
        }
        case 2: {
          if (input1 && stage1_pass == 1) {
            stage3_pass = 1;
            pass();
          } else if (input2 && stage1_pass == 2) {
            stage3_pass = 2;
            pass();
          } else if (input3 && stage1_pass == 3) {
            stage3_pass = 3;
            pass();
          } else if (input4 && stage1_pass == 4) {
            stage3_pass = 4;
            pass();
          } else {fail();}
          break;
        }
        case 3: {
          if (input3) {
            stage3_pass = 3;
            pass();
          } else {fail();}
          break;
        }
        case 4: {
          if (input1 && choice1 == 4) {
            stage3_pass = 1;
            pass();
          } else if (input2 && choice2 == 4) {
            stage3_pass = 2;
            pass();
          } else if (input3 && choice3 == 4) {
            stage3_pass = 3;
            pass();
          } else if (input4 && choice4 == 4) {
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
        case 1: {
          if (input1 && stage1_pass == 1) {
            stage4_pass = 1;
            pass();
          } else if (input2 && stage1_pass == 2) {
            stage4_pass = 2;
            pass();
          } else if (input3 && stage1_pass == 3) {
            stage4_pass = 3;
            pass();
          } else if (input4 && stage1_pass == 4) {
            stage4_pass = 4;
            pass();
          } else {fail();}
          break;
        }
        case 2: {
          if (input1) {
            stage4_pass = 1;
            pass();
          } else {fail();}
          break;
        }
        case 3: {
          if (input1 && stage2_pass == 1) {
            stage4_pass = 1;
            pass();
          } else if (input2 && stage2_pass == 2) {
            stage4_pass = 2;
            pass();
          } else if (input3 && stage2_pass == 3) {
            stage4_pass = 3;
            pass();
          } else if (input4 && stage2_pass == 4) {
            stage4_pass = 4;
            pass();
          } else {fail();}
          break;
        }
        case 4: {
          if (input1 && stage2_pass == 1) {
            stage4_pass = 1;
            pass();
          } else if (input2 && stage2_pass == 2) {
            stage4_pass = 2;
            pass();
          } else if (input3 && stage2_pass == 3) {
            stage4_pass = 3;
            pass();
          } else if (input4 && stage2_pass == 4) {
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
        case 1: {
          if (input1 && stage1_pass == 1) {
            stage5_pass = 1;
            pass();
          } else if (input2 && stage1_pass == 2) {
            stage5_pass = 2;
            pass();
          } else if (input3 && stage1_pass == 3) {
            stage5_pass = 3;
            pass();
          } else if (input4 && stage1_pass == 4) {
            stage5_pass = 4;
            pass();
          } else {fail();}
          break;
        }
        case 2: {
          if (input1 && stage2_pass == 1) {
            stage5_pass = 1;
            pass();
          } else if (input2 && stage2_pass == 2) {
            stage5_pass = 2;
            pass();
          } else if (input3 && stage2_pass == 3) {
            stage5_pass = 3;
            pass();
          } else if (input4 && stage2_pass == 4) {
            stage5_pass = 4;
            pass();
          } else {fail();}
          break;
        }
        case 3: {
          if (input1 && stage4_pass == 1) {
            stage5_pass = 1;
            pass();
          } else if (input2 && stage4_pass == 2) {
            stage5_pass = 2;
            pass();
          } else if (input3 && stage4_pass == 3) {
            stage5_pass = 3;
            pass();
          } else if (input4 && stage4_pass == 4) {
            stage5_pass = 4;
            pass();
          } else {fail();}
          break;
        }
        case 4: {
          if (input1 && stage3_pass == 1) {
            stage5_pass = 1;
            pass();
          } else if (input2 && stage3_pass == 2) {
            stage5_pass = 2;
            pass();
          } else if (input3 && stage3_pass == 3) {
            stage5_pass = 3;
            pass();
          } else if (input4 && stage3_pass == 4) {
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
