/*
* This is the complete explained code for the Home Farmer V2.4
* (Home Farmer V2.4 is the one presented at the WRO 2024 season)
*
* Update: This code now also utilizes the Operate library
* The Operate library can be found here: https://github.com/vihaanvp/Operate-Lib
*/

// Defining E_OK (yes) and E_NOK (no) macros because coding habits
#define E_OK 0
#define E_NOK 1

// Including the Servo library and defining a Servo object for the seed picker
#include "Servo.h"
Servo pick;

// Including more needed libraries
#include <LiquidCrystal_I2C.h>
#include "Arduino.h"
#include "uRTCLib.h"
#include <dht.h>
#include <operate.h>

// Defining some more macros
#define outPin 0
#define watdel 6000
#define afterwatdel 3000

// Defining the RTC, DHT, and LCD objects
uRTCLib rtc(0x68);  // RTC at I2C address 0x68
dht DHT;
long mois = 0;

// Days of the week (I know the order of the days is wrong)
char daysOfTheWeek[7][12] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

// LCD object 20x4 at I2C address 0x27
LiquidCrystal_I2C lcd(0x27, 20, 4); 
int d, mo, y, h, m, s, AP;
int readData;
int hum, temp;

int counterScreen = 1;

// Defining the Required Variables
// Rotary encoder variables
int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir = "";
unsigned long lastButtonPress = 0;
int btnState;

int screen = 1;
int prevcounter = 0;
int diff = 0;
int flag = 0;
int config = 0;
long rtccount = 0;
int prevrtccount = 0;
int dotdiff = 0;

// Defining 3 stepper motor (operate) objects from the class
// Stepper motor objects
operate mot_x;
operate mot_y;
operate mot_z;

// Function for reading the Rotary Encoder (All the comments written inside this function are not written by me)
// Read rotary encoder and update global counter
void readrotary() {
  // Read the current state of CLK
  //currentStateCLK = digitalRead(CLK);
  currentStateCLK = digitalRead(A2);
  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK && currentStateCLK == 1) {
    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(A1) != currentStateCLK) {
      counter--;
      currentDir = "CCW";
    } else {
      // Encoder is rotating CW so increment
      counter++;
      currentDir = "CW";
    }
  }
  // Remember last CLK state
  lastStateCLK = currentStateCLK;
}

// Function to read the button state of the Rotary Encoder and switch the screen accordingly (dunno about the last part)
// (Comments written inside this function are not written by me)
// Read rotary button and set screen based on case
void readbutton(int casen) {
  btnState = digitalRead(A0);
  //If we detect LOW signal, button is pressed
  if (btnState == LOW) {
    //if 50ms have passed since last LOW pulse, it means that the
    //button has been pressed, released and pressed again
    if (millis() - lastButtonPress > 100) {
      switch (casen) {
        case 0:
          screen = 4;
          break;
        case 1:
          screen = 3;
          break;
        case 3:
          screen = 12;
          break;
        case 4:
          screen = 16;
          break;
        case 5:
          screen = 20;
          delay(300);
          lcd.clear();
          break;
        case 7:
          screen = 15;
          break;
        case 8:
          screen = 0;
          break;
        case 9:
          screen = 19;
          break;
        case 10:
          screen = 0;
          break;
      }
    }
    // Remember last button press event
    lastButtonPress = millis();
  }
}

// Function to update the RTC every time it is called
// Read RTC and update time/date variables
void readRTC() {
  rtc.refresh();
  d = rtc.day();
  mo = rtc.month();
  y = rtc.year();
  h = rtc.hour();
  m = rtc.minute();
  s = rtc.second();
  if (h > 12) {
    h = h - 12;
    AP = 1;
  } else {
    AP = 0;
  }
}

// Function for the Home Screen code
// Show home screen on LCD
void homeScreen() {
  // Print the Day
  lcd.clear();
  lcd.setCursor(0, 0);
  if (d > 9) {
    lcd.print(d);
  } else {
    lcd.print("0");
    lcd.print(d);
  }
  // Print the month
  lcd.print("/");
  if (mo > 9) {
    lcd.print(mo);
  } else {
    lcd.print("0");
    lcd.print(mo);
  }
  // Print the year
  lcd.print("/");
  lcd.print(y);
  lcd.print(" ");
  // Print the day
  lcd.print(daysOfTheWeek[rtc.dayOfWeek() - 1]);

  lcd.setCursor(0, 1);
  // Print the time (HH:MM) in 12-hour format
  if (h > 9) {
    lcd.print(h);
  } else {
    lcd.print("0");
    lcd.print(h);
  }
  lcd.print(":");
  if (m > 9) {
    lcd.print(m);
  } else {
    lcd.print("0");
    lcd.print(m);
  }
  lcd.print(" ");
  // Determine AM or PM using the AP boolean
  if (AP == 1)
    lcd.print("PM");
  else
    lcd.print("AM");

  lcd.setCursor(15, 0);
  // Print Humidity
  lcd.print("H:");
  lcd.print(hum);
  lcd.print("%");

  lcd.setCursor(15, 1);
  // Print temperature
  lcd.print("T:");
  lcd.print(temp);
  lcd.print("C");

  lcd.setCursor(5, 2);
  // Print the Title Text
  lcd.print("Home Farmer");

  // Print the Selectable Options
  lcd.setCursor(3, 3);
  lcd.print("Menu");

  lcd.setCursor(13, 3);
  lcd.print("Info");
}

// Function to read the DHT when called
// Read DHT sensor and update hum/temp
void readDHT() {
  // temperature
  readData = DHT.read11(outPin);
  temp = DHT.temperature;  // Read temperature
  hum = DHT.humidity;      // Read humidity
}

// Update the RTC and DHT every 8 seconds (to avoid lag)
// Update RTC and DHT periodically (every 8000 loops)
void RTCupdate() {
  rtccount++;
  if (rtccount > 8000) {
    readRTC();
    readDHT();
    screen = 0;
    rtccount = 0;
  }
}

// Shift the screen with the previous screen (encoder to left) and the next screen (encoder to right) parameters
// Shift between screens based on rotary encoder
void shiftscreen(int prev, int next) {
  diff = counter - prevcounter;
  if (diff > 0) {
    screen = prev;
  } else if (diff < 0) {
    screen = next;
  }
  prevcounter = counter;
}

// Function to pick the seed and plant it (Not sure tbh)
// Move to pick area for seed picking
void seedpickarea() {
  mot_z.setPosition(30000);
  delay(100);
  pick.write(100);
  delay(100);
  mot_y.setPosition(1000);
  delay(100);
  mot_x.setPosition(11250);
  delay(100);
  mot_z.setPosition(62000);
  delay(100);
  pick.write(40);
  delay(300);
  mot_z.setPosition(30000);
  delay(100);
}

// Drop the seed at specified coordinates
// Plant a seed at a given X, Y position
void seedplant(long x, long y) {
  mot_y.setPosition(y);
  delay(100);
  mot_x.setPosition(x);
  delay(100);
  mot_z.setPosition(55000);
  delay(100);
  pick.write(100);
  delay(500);
}

// Use the 2 functions above to place the seeds at set locations (for testing only)
// Plant all seeds (sequence)
void seeding() {
  seedpickarea();
  seedplant(500, 4000);

  seedpickarea();
  seedplant(500, 12000);

  seedpickarea();
  seedplant(8000, 4000);

  seedpickarea();
  seedplant(8000, 12000);
}

// Water the coordinates at which the seeds are placed
// Water all plants (sequence)
void watering() {
  seedplant(500, 4000);
  digitalWrite(12, HIGH);
  delay(watdel);
  digitalWrite(12, LOW);
  delay(afterwatdel);

  seedplant(500, 12000);
  digitalWrite(12, HIGH);
  delay(watdel);
  digitalWrite(12, LOW);
  delay(afterwatdel);

  seedplant(8000, 4000);
  digitalWrite(12, HIGH);
  delay(watdel);
  digitalWrite(12, LOW);
  delay(afterwatdel);

  seedplant(8000, 12000);
  digitalWrite(12, HIGH);
  delay(watdel);
  digitalWrite(12, LOW);
  delay(afterwatdel);
}

// FINALLY we have the void setup
// Some of the comments written inside are mine
void setup() {
  pick.attach(13);
  // Define pins to the different components (idk which pin is which component)
  // Setup pin modes
  pinMode(2, OUTPUT); // X step
  pinMode(3, OUTPUT); // Y step
  pinMode(4, OUTPUT); // Z step
  pinMode(9, INPUT_PULLUP);  // X limit switch
  pinMode(10, INPUT_PULLUP); // Y limit switch
  pinMode(11, INPUT_PULLUP); // Z limit switch
  pinMode(5, OUTPUT); // X dir
  pinMode(6, OUTPUT); // Y dir
  pinMode(7, OUTPUT); // Z dir
  pinMode(12, OUTPUT); // Water
  pinMode(8, OUTPUT);  // Not used here
  digitalWrite(8, LOW);
  digitalWrite(12, LOW);

  // Define all the other components for LCD and Rotary , DHT, RTC
  // RTC
  URTCLIB_WIRE.begin();
  //rtc.set(0, 32, 11, 7, 24, 8, 24);
  //  rtc.set(second, minute, hour, dayOfWeek, dayOfMonth, month, year)
  // set day of week (1=Monday, 7=Sunday)

  // LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();  // Make sure backlight is on

  // Read the initial state of CLK
  lastStateCLK = analogRead(A2);
  if (lastStateCLK > 512)
    lastStateCLK = 1;
  else
    lastStateCLK = 0;

  // Invoke the functions to update both the modules
  readRTC();
  readDHT();

  // Define motors
  mot_x.setPin(2, 5); // x
  mot_y.setPin(3, 6); // y
  mot_z.setPin(4, 7); // z
  // Set Motor speeds
  mot_x.setSpeed(160);
  mot_y.setSpeed(160);
  mot_z.setSpeed(35);
  // Set Homing for All the motors
  mot_x.setHoming(E_OK, 160, 10000, 9);
  mot_y.setHoming(E_OK, 160, 10000, 10);
  mot_z.setHoming(E_OK, 35, 10000, 11);
  // Set position limits for all the motors
  mot_x.setPositionLimits(12000, 0);
  mot_y.setPositionLimits(14000, 0);
  mot_z.setPositionLimits(65000, 0); // Yes the 65000 is not a typo, its a motor attached to a lead screw

  // Print the Initializing Screen on the LCD
  // Homing sequence with progress display
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("Home Farmer");
  lcd.setCursor(7, 1);
  lcd.print("Homing...");
  lcd.setCursor(0, 2);
  lcd.print("Please Wait...");
  lcd.setCursor(0, 3);
  lcd.print("[-");
  lcd.setCursor(16, 3);
  lcd.print("]");
  lcd.setCursor(17, 3);
  lcd.print(" 1%");
  delay(3000);

  // Perform on-start homing
  mot_z.home();
  lcd.setCursor(17, 3);
  lcd.print("33%");
  lcd.setCursor(2, 3);
  lcd.print("----");
  mot_x.home();
  lcd.setCursor(17, 3);
  lcd.print("66%");
  lcd.setCursor(6, 3);
  lcd.print("----");
  mot_y.home();
  lcd.setCursor(17, 3);
  lcd.print("99%");
  lcd.setCursor(10, 3);
  lcd.print("----");
  delay(1000);
  lcd.setCursor(0, 2);
  lcd.print("    Homing Done!!!");
  lcd.setCursor(0, 1);
  lcd.print("                    ");
  lcd.setCursor(0, 3);
  lcd.print("                    ");
  pick.write(100);
  delay(2000);
  homeScreen();
}

void loop() {
  // Main UI state machine
  // Define the screens and the role of the Rotary Encoder
  switch (screen) {
    case 0: // Home Screen (is without the selector arrow)
      homeScreen();
      rtccount = 0;
      screen = 1; // Directly shifts to the Home Screen 2
      break;
    case 1: // Home Screen with Selector on "Menu"
      readrotary();
      readbutton(0);
      RTCupdate();
      lcd.setCursor(2, 3);
      lcd.print(">");
      lcd.setCursor(12, 3);
      lcd.print(" ");
      shiftscreen(2, 1);
      break;
    case 2: // Home Screen with Selector on "Info"
      readrotary();
      readbutton(1);
      RTCupdate();
      lcd.setCursor(2, 3);
      lcd.print(" ");
      lcd.setCursor(12, 3);
      lcd.print(">");
      shiftscreen(1, 2);
      break;
    case 3: // Info Screen (When user selects "Info" on the Home screen)
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("Home Farmer V2.4");
      lcd.setCursor(4, 1);
      lcd.print("A Project By");
      lcd.setCursor(1, 2);
      lcd.print("Vihaan & Yogeshwar");
      lcd.setCursor(2, 3);
      lcd.print("Mentor: Malhar A");
      rtccount = 0;
      delay(7000);
      screen = 0;
      break;
    case 4: // Plant seed screen under menu (When user clicks on "Menu" on the Home screen or when the user turns rotary to the left from "Water Plants" screen)
      lcd.clear();
      lcd.setCursor(15, 0);
      if (h > 9) {
        lcd.print(h);
      } else {
        lcd.print("0");
        lcd.print(h);
      }
      lcd.print(":");
      if (m > 9) {
        lcd.print(m);
      } else {
        lcd.print("0");
        lcd.print(m);
      }
      lcd.setCursor(0, 0);
      if (d > 9) {
        lcd.print(d);
      } else {
        lcd.print("0");
        lcd.print(d);
      }
      lcd.print("/");
      if (mo > 9) {
        lcd.print(mo);
      } else {
        lcd.print("0");
        lcd.print(mo);
      }
      lcd.setCursor(9, 0);
      lcd.print(daysOfTheWeek[rtc.dayOfWeek() - 1]);

      lcd.setCursor(0, 3);
      lcd.print(" ");
      lcd.setCursor(19, 3);
      lcd.print(">");

      lcd.setCursor(7, 1);
      lcd.print("[Menu]");

      lcd.setCursor(2, 2);
      lcd.print("-| Plant Seed |-");
      rtccount = 0;
      screen = 5;
      break;
    case 5: // Read the Rotary
      readrotary();
      readbutton(3);
      RTCupdate();
      shiftscreen(5, 6);
      break;
    case 6: // Water Plants screen under Menu (When user moves the encoder to the right when on "Plant Seed" screen or to the left when on "Parameters" screen)
      lcd.setCursor(2, 2);
      lcd.print("-|Water Plants|-");
      lcd.setCursor(0, 3);
      lcd.print("<");
      rtccount = 0;
      screen = 7;
      break;
    case 7: // Read the Rotary
      readrotary();
      readbutton(4);
      RTCupdate();
      shiftscreen(4, 8);
      break;
    case 8: // Parameters screen under Menu (When user moves the encoder to the right when on "Water Plants" screen or left from the "Exit" screen)
      lcd.setCursor(1, 2);
      lcd.print("                  ");
      lcd.setCursor(2, 2);
      lcd.print("-| Parameters |-");
      lcd.setCursor(0, 3);
      lcd.print("<");
      lcd.setCursor(19, 3);
      lcd.print(">");
      rtccount = 0;
      screen = 9;
      break;
    case 9: // Read the rotary
      readrotary();
      readbutton(5);
      RTCupdate();
      shiftscreen(6, 10);
      break;
    case 10: // Exit screen under Menu (When user moves the encoder to the right when on "Parameters screen")
      lcd.setCursor(1, 2);
      lcd.print("                  ");
      lcd.setCursor(6, 2);
      lcd.print("-| Exit |-");
      lcd.setCursor(0, 3);
      lcd.print("<");
      lcd.setCursor(19, 3);
      lcd.print(" ");
      rtccount = 0;
      screen = 11;
      break;
    case 11: // Read the Rotary
      readrotary();
      readbutton(6);
      RTCupdate();
      shiftscreen(8, 10);
      break;
    case 12: // Plant Seeds Now screen when arrow is on Yes (Shown when the user clicks the rotary button when on the Plant Seeds screen)
      lcd.clear();
      lcd.setCursor(15, 0);
      if (h > 9) {
        lcd.print(h);
      } else {
        lcd.print("0");
        lcd.print(h);
      }
      lcd.print(":");
      if (m > 9) {
        lcd.print(m);
      } else {
        lcd.print("0");
        lcd.print(m);
      }
      lcd.setCursor(0, 0);
      if (d > 9) {
        lcd.print(d);
      } else {
        lcd.print("0");
        lcd.print(d);
      }
      lcd.print("/");
      if (mo > 9) {
        lcd.print(mo);
      } else {
        lcd.print("0");
        lcd.print(mo);
      }
      lcd.setCursor(9, 0);
      lcd.print(daysOfTheWeek[rtc.dayOfWeek() - 1]);

      lcd.setCursor(2, 1);
      lcd.print("-| Plant Seed |-");

      lcd.setCursor(2, 2);
      lcd.print("Plant seeds Now?");

      lcd.setCursor(2, 3);
      lcd.print("Yes");

      lcd.setCursor(16, 3);
      lcd.print("No");

      lcd.setCursor(1, 3);
      lcd.print(">");

      lcd.setCursor(15, 3);
      lcd.print(" ");
      screen = 13;
      rtccount = 0;
      break;
    case 13: // Read the rotary
      readrotary();
      readbutton(7);
      RTCupdate();
      shiftscreen(12, 14);
      break;
    case 14: // Plant Seeds Now screen when the arrow is on No (Triggered when the user turns encoder to the right)
      lcd.setCursor(1, 3);
      lcd.print(" ");
      lcd.setCursor(15, 3);
      lcd.print(">");
      readrotary();
      readbutton(8);
      RTCupdate();
      shiftscreen(14, 12);
      break;
    case 15: // Seed Planting Progress screen (Triggered when user clicks on Yes in the Plant Seeds Now screen)
      lcd.clear();
      lcd.setCursor(15, 0);
      if (h > 9) {
        lcd.print(h);
      } else {
        lcd.print("0");
        lcd.print(h);
      }
      lcd.print(":");
      if (m > 9) {
        lcd.print(m);
      } else {
        lcd.print("0");
        lcd.print(m);
      }
      lcd.setCursor(0, 0);
      if (d > 9) {
        lcd.print(d);
      } else {
        lcd.print("0");
        lcd.print(d);
      }
      lcd.print("/");
      if (mo > 9) {
        lcd.print(mo);
      } else {
        lcd.print("0");
        lcd.print(mo);
      }
      lcd.setCursor(9, 0);
      lcd.print(daysOfTheWeek[rtc.dayOfWeek() - 1]);

      lcd.setCursor(2, 2);
      lcd.print("Planting Seeds...");
      lcd.setCursor(2, 3);
      lcd.print("Please wait...");
      seeding();
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Seeding Successful!");
      lcd.setCursor(7, 1);
      lcd.print("Homing...");
      lcd.setCursor(0, 2);
      lcd.print("Please Wait...");
      lcd.setCursor(0, 3);
      lcd.print("[-");
      lcd.setCursor(16, 3);
      lcd.print("]");
      lcd.setCursor(17, 3);
      lcd.print(" 1%");
      mot_z.home();
      lcd.setCursor(17, 3);
      lcd.print("33%");
      lcd.setCursor(2, 3);
      lcd.print("----");
      mot_x.home();
      lcd.setCursor(17, 3);
      lcd.print("66%");
      lcd.setCursor(6, 3);
      lcd.print("----");
      mot_y.home();
      lcd.setCursor(17, 3);
      lcd.print("99%");
      lcd.setCursor(10, 3);
      lcd.print("----");
      delay(1000);
      lcd.setCursor(0, 2);
      lcd.print("    Homing Done!!!");
      lcd.setCursor(0, 1);
      lcd.print("                    ");
      lcd.setCursor(0, 3);
      lcd.print("                    ");
      pick.write(100);
      delay(2000);
      screen = 0;
      break;
    case 16: // Water Plants Now screen without arrows (Triggered when the user clicks the encoder button when on the Water Plants screen)
      delay(300);
      lcd.clear();
      lcd.setCursor(15, 0);
      if (h > 9) {
        lcd.print(h);
      } else {
        lcd.print("0");
        lcd.print(h);
      }
      lcd.print(":");
      if (m > 9) {
        lcd.print(m);
      } else {
        lcd.print("0");
        lcd.print(m);
      }
      lcd.setCursor(0, 0);
      if (d > 9) {
        lcd.print(d);
      } else {
        lcd.print("0");
        lcd.print(d);
      }
      lcd.print("/");
      if (mo > 9) {
        lcd.print(mo);
      } else {
        lcd.print("0");
        lcd.print(mo);
      }
      lcd.setCursor(9, 0);
      lcd.print(daysOfTheWeek[rtc.dayOfWeek() - 1]);

      lcd.setCursor(2, 1);
      lcd.print("-| Water Plants |-");

      lcd.setCursor(2, 2);
      lcd.print("Water Plants Now?");

      lcd.setCursor(2, 3);
      lcd.print("Yes");

      lcd.setCursor(16, 3);
      lcd.print("No");
      screen = 17;
      rtccount = 0;
      break;
    case 17: // Water Plants now screen with the arrow on Yes (Triggered right after the Water Plants Now screen)
      lcd.setCursor(1, 3);
      lcd.print(">");

      lcd.setCursor(15, 3);
      lcd.print(" ");
      readrotary();
      readbutton(9);
      RTCupdate();
      shiftscreen(17, 18);
      break;
    case 18: // Water Plants now screen with the arrow on No (Triggered after moving encoder right after the Water Plants Now screen while the arrow is on Yes)
      lcd.setCursor(1, 3);
      lcd.print(" ");

      lcd.setCursor(15, 3);
      lcd.print(">");
      readrotary();
      readbutton(8);
      RTCupdate();
      shiftscreen(18, 17);
      break;
    case 19: // Watering progress screen (Triggered after clicking the encoder button while the arrow is on Yes on the Water Plants now screen)
      lcd.clear();
      lcd.setCursor(15, 0);
      if (h > 9) {
        lcd.print(h);
      } else {
        lcd.print("0");
        lcd.print(h);
      }
      lcd.print(":");
      if (m > 9) {
        lcd.print(m);
      } else {
        lcd.print("0");
        lcd.print(m);
      }
      lcd.setCursor(0, 0);
      if (d > 9) {
        lcd.print(d);
      } else {
        lcd.print("0");
        lcd.print(d);
      }
      lcd.print("/");
      if (mo > 9) {
        lcd.print(mo);
      } else {
        lcd.print("0");
        lcd.print(mo);
      }
      lcd.setCursor(9, 0);
      lcd.print(daysOfTheWeek[rtc.dayOfWeek() - 1]);

      lcd.setCursor(2, 2);
      lcd.print("Watering Plants..");
      lcd.setCursor(2, 3);
      lcd.print("Please wait...");
      watering();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Watering Successful!");
      lcd.setCursor(7, 1);
      lcd.print("Homing...");
      lcd.setCursor(0, 2);
      lcd.print("Please Wait...");
      lcd.setCursor(0, 3);
      lcd.print("[-");
      lcd.setCursor(16, 3);
      lcd.print("]");
      lcd.setCursor(17, 3);
      lcd.print(" 1%");
      mot_z.home();
      lcd.setCursor(17, 3);
      lcd.print("33%");
      lcd.setCursor(2, 3);
      lcd.print("----");
      mot_x.home();
      lcd.setCursor(17, 3);
      lcd.print("66%");
      lcd.setCursor(6, 3);
      lcd.print("----");
      mot_y.home();
      lcd.setCursor(17, 3);
      lcd.print("99%");
      lcd.setCursor(10, 3);
      lcd.print("----");
      delay(1000);
      lcd.setCursor(0, 2);
      lcd.print("    Homing Done!!!");
      lcd.setCursor(0, 1);
      lcd.print("                    ");
      lcd.setCursor(0, 3);
      lcd.print("                    ");
      pick.write(100);
      delay(2000);
      screen = 0;
      break;
    case 20: // Parameters screen (Triggered when encoder button is clicked on Parameters screen under Menu)
      readDHT();
      readRTC();
      lcd.setCursor(3, 0);
      lcd.print("* Parameters *");
      lcd.setCursor(0, 1);
      lcd.print("Temperature:");
      lcd.print(temp);
      lcd.print("C");
      lcd.setCursor(0, 2);
      lcd.print("Humidity:");
      lcd.print(hum);
      lcd.print("%");
      lcd.setCursor(0, 3);
      mois = analogRead(A3);
      mois = map(mois, 230, 1023, 0, 100);
      mois = 100 - mois;
      lcd.print("Moisture:");
      lcd.print(mois);
      lcd.print("%   ");
      for (int i = 0; i < 1000; i++) {
        readrotary();
        readbutton(10);
        RTCupdate();
        delay(1);
      }
      rtccount = 0;
      break;
  }
  // delay to avoid hardware errors
  delay(1);
}