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
#include <avr/wdt.h>

// Defining some more macros
#define outPin 0
#define watdel 6000
#define afterwatdel 3000

// Defining the RTC, DHT, and LCD objects
uRTCLib rtc(0x68);  // RTC at I2C address 0x68
dht DHT;
long mois = 0;

// Custom Position when Seeding
bool customloc = false;
int x_cust_val = 0;
int y_cust_val = 0;

// Boolean to figure out if command is from Alexa
bool alexabool = false;
volatile bool reset_required = false;

// Auto-watering variables
unsigned long lastWateringTime = 0;
const unsigned long WATERING_INTERVAL = 180000;  // 3 minutes in milliseconds

static unsigned long lastSend = 0;

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
  checkSerial();  // Check serial before encoder read
  // Read the current state of CLK
  //currentStateCLK = digitalRead(CLK);
  currentStateCLK = digitalRead(A2);
  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK && currentStateCLK == 1) {
    checkSerial();  // Check serial during encoder processing
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
  checkSerial();  // Check serial after encoder read
}

// Function to reset the arduino in code
void softwareReset() {
  wdt_enable(WDTO_15MS);  // reset after 15 ms
  while (1) {}
}

// Function to read the button state of the Rotary Encoder and switch the screen accordingly (dunno about the last part)
// (Comments written inside this function are not written by me)
// Read rotary button and set screen based on case
void readbutton(int casen) {
  checkSerial();  // Check serial before button read
  btnState = digitalRead(A0);
  //If we detect LOW signal, button is pressed
  if (btnState == LOW) {
    checkSerial();  // Check serial when button is pressed
    //if 50ms have passed since last LOW pulse, it means that the
    //button has been pressed, released and pressed again
    if (millis() - lastButtonPress > 100) {
      checkSerial();  // Check serial before switch
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
      checkSerial();  // Check serial after switch
    }
    // Remember last button press event
    lastButtonPress = millis();
  }
  checkSerial();  // Check serial after button read
}

// Function to update the RTC every time it is called
// Read RTC and update time/date variables
void readRTC() {
  checkSerial();  // Check serial before RTC read
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
  checkSerial();  // Check serial after RTC read
}

// Function for the Home Screen code
// Show home screen on LCD
void homeScreen() {
  checkSerial();  // Check serial at start of homeScreen
  // Print the Day
  lcd.clear();
  lcd.setCursor(0, 0);
  if (d > 9) {
    lcd.print(d);
  } else {
    lcd.print("0");
    lcd.print(d);
  }
  checkSerial();  // Check serial during LCD operations
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

  checkSerial();  // Check serial between LCD operations
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

  checkSerial();  // Check serial between LCD operations
  checkSerial();
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
  checkSerial();  // Check serial at end of homeScreen
}

// Function to read the DHT when called
void readDHT(String cmd) {
  checkSerial();  // Check serial at start of readDHT
  /*readData = DHT.read11(outPin);
  temp = DHT.temperature;
  hum = DHT.humidity;*/
  int commaIdx = cmd.indexOf(',');
  if (commaIdx > 4) {
    temp = cmd.substring(4, commaIdx).toInt();
    hum = cmd.substring(commaIdx + 1).toInt();
  }
  checkSerial();  // Check serial at end of readDHT
}

// Update the RTC and DHT every 8 seconds (to avoid lag)
void RTCupdate() {
  checkSerial();  // Check serial at start of RTCupdate
  rtccount++;
  if (rtccount > 8000) {
    readRTC();
    screen = 0;
    rtccount = 0;
  }
  checkSerial();  // Check serial at end of RTCupdate
}

void shiftscreen(int prev, int next) {
  checkSerial();  // Check serial at start of shiftscreen
  diff = counter - prevcounter;
  if (diff > 0) {
    screen = prev;
  } else if (diff < 0) {
    screen = next;
  }
  prevcounter = counter;
  checkSerial();  // Check serial at end of shiftscreen
}

void seedpickarea() {
  checkSerial();
  mot_z.setPosition(30000);
  delay(100);
  checkSerial();
  pick.write(100);
  delay(100);
  checkSerial();
  mot_y.setPosition(1000);
  delay(100);
  checkSerial();
  mot_x.setPosition(11250);
  delay(100);
  checkSerial();
  mot_z.setPosition(62000);
  delay(100);
  checkSerial();
  pick.write(40);
  delay(300);
  checkSerial();
  mot_z.setPosition(30000);
  delay(100);
  checkSerial();
}

void seedplant(long x, long y) {
  checkSerial();
  mot_y.setPosition(y);
  delay(100);
  checkSerial();
  mot_x.setPosition(x);
  delay(100);
  checkSerial();
  mot_z.setPosition(55000);
  delay(100);
  checkSerial();
  pick.write(100);
  delay(500);
  checkSerial();
}

void seeding() {
  checkSerial();
  if (customloc) {
    mot_y.setPosition(UnitConversion('Y', y_cust_val, "steps"));
    checkSerial();
    delay(100);
    checkSerial();
    mot_x.setPosition(UnitConversion('X', x_cust_val, "steps"));
    checkSerial();
    delay(1500);
    checkSerial();
  } else {
    seedpickarea();
    checkSerial();
    seedplant(500, 4000);
    checkSerial();

    seedpickarea();
    checkSerial();
    seedplant(500, 12000);
    checkSerial();

    seedpickarea();
    checkSerial();
    seedplant(8000, 4000);
    checkSerial();

    seedpickarea();
    checkSerial();
    seedplant(8000, 12000);
    checkSerial();
  }
}

// ---- NEW LOGIC IMPLEMENTATION BELOW ----
void watering() {
  checkSerial();

  if (millis() - lastSend > 1000) {
    sendMoistureToPi();
    lastSend = millis();
  }

  // Plant positions
  long x_coords[4] = { 500, 500, 8000, 8000 };
  long y_coords[4] = { 4000, 12000, 4000, 12000 };

  for (int i = 0; i < 4; i++) {
    if (millis() - lastSend > 1000) {
      sendMoistureToPi();
      lastSend = millis();
    }
    checkSerial();
    // Move to plant location and put sensor in soil
    seedplant(x_coords[i], y_coords[i]);
    checkSerial();
    delay(2000);
    checkSerial();

    // Read initial moisture
    mois = analogRead(A3);
    checkSerial();
    mois = map(mois, 230, 1023, 0, 100);
    checkSerial();
    mois = 100 - mois;
    checkSerial();

    if (mois < 40) {
      checkSerial();
      // Sensor out before watering
      mot_z.setPosition(75000);
      delay(500);
      checkSerial();

      int wateringAttempts = 0;
      const int maxAttempts = 2;

      // Water and check until >= 70% or max attempts reached
      while (wateringAttempts < maxAttempts) {
        checkSerial();

        digitalWrite(12, HIGH);
        checkSerial();
        delay(4000);  // Water for 4 seconds
        checkSerial();
        digitalWrite(12, LOW);
        delay(1000);  // Allow water to seep
        checkSerial();

        // Lower sensor for new measurement
        mot_z.setPosition(75000);  // Insert sensor (same as seedplant Z)
        delay(2000);               // Wait for sensor to stabilize
        checkSerial();

        mois = analogRead(A3);
        mois = map(mois, 230, 1023, 0, 100);
        mois = 100 - mois;
        checkSerial();

        // Take sensor out again
        mot_z.setPosition(35000);
        delay(500);
        checkSerial();

        wateringAttempts++;

        if (mois >= 70) {
          break;
        }
        checkSerial();
        if (millis() - lastSend > 1000) {
          sendMoistureToPi();
          lastSend = millis();
        }
      }
    }
    checkSerial();
    if (millis() - lastSend > 1000) {
      sendMoistureToPi();
      lastSend = millis();
    }
    // If initial mois >= 40, skip watering (do nothing for this plant)
  }
  checkSerial();
  if (millis() - lastSend > 1000) {
    sendMoistureToPi();
    lastSend = millis();
  }
}
// ---- END: watering() ----

long UnitConversion(char axis, int value, String outputUnit) {
  checkSerial();  // Check serial at start of UnitConversion
  const float xStepsPerCm = 12000.0 / 29.5;
  const float yStepsPerCm = 14500.0 / 34.6;
  const float xOffset = 11.7;
  const float halfSquareWidth = 3.5;
  if (outputUnit == "steps") {
    if (axis == 'X') {
      float steps = (value - xOffset) * xStepsPerCm + 12;
      checkSerial();
      return lround(steps);
    } else if (axis == 'Y') {
      float steps = (value + halfSquareWidth) * yStepsPerCm - 5.4;
      checkSerial();
      return lround(steps);
    }
  } else if (outputUnit == "cm") {
    if (axis == 'X') {
      float cm = (value / xStepsPerCm) + xOffset;
      checkSerial();
      return lround(cm);
    } else if (axis == 'Y') {
      float cm = (value / yStepsPerCm) - halfSquareWidth;
      checkSerial();
      return lround(cm);
    }
  }
  checkSerial();
  return -1;
}

// Sends current soil moisture to Pi
void sendMoistureToPi() {
  checkSerial();  // Check serial at start
  Serial.print("MOISTURE:");
  int raw = analogRead(A3);
  mois = map(raw, 180, 1023, 0, 100);
  mois = 100 - mois;

  Serial.println(mois);  // mois is already in percent
  checkSerial();         // Check serial at end
}

// Receives DHT data from Pi and updates temp/hum
/*void receiveDHTFromPi() {
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.startsWith("DHT:")) {
      int commaIdx = line.indexOf(',');
      if (commaIdx > 4) {
        temp = line.substring(4, commaIdx).toInt();
        hum = line.substring(commaIdx + 1).toInt();
      }
    }
  }
}
*/

void homeAllMotors() {
  checkSerial();
  mot_z.home();
  checkSerial();
  mot_x.home();
  checkSerial();
  mot_y.home();
  delay(5);
  checkSerial();
  mot_z.home();
  checkSerial();
  mot_x.home();
  checkSerial();
  mot_y.home();
  checkSerial();
}

void checkSerial() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "RESET") {
      softwareReset();
    } else if (cmd == "SEED") {
      homeAllMotors();
      alexabool = true;
      screen = 15;
    } else if (cmd == "WATER") {
      homeAllMotors();
      screen = 19;
    } else if (cmd.startsWith("DHT:")) {
      readDHT(cmd);
    }
  }
}

// Function to check if auto-watering should trigger
void checkAutoWatering() {
  checkSerial();  // Check serial before auto-watering check

  // Check if 3 minutes have passed and we're not already on screen 19
  if (millis() - lastWateringTime >= WATERING_INTERVAL && screen != 19) {
    lastWateringTime = millis();  // Update the last watering time
    screen = 19;                  // Go to watering screen
  }

  checkSerial();  // Check serial after auto-watering check
}

void setup() {
  Serial.begin(9600);
  pick.attach(13);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);
  digitalWrite(12, LOW);

  checkSerial();  // Check serial during setup

  URTCLIB_WIRE.begin();
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lastStateCLK = analogRead(A2);
  if (lastStateCLK > 512)
    lastStateCLK = 1;
  else
    lastStateCLK = 0;
  readRTC();

  checkSerial();  // Check serial during setup

  mot_x.setPin(2, 5);
  mot_y.setPin(3, 6);
  mot_z.setPin(4, 7);
  mot_x.setSpeed(160);
  mot_y.setSpeed(160);
  mot_z.setSpeed(35);
  mot_x.setHoming(E_OK, 160, 10000, 9);
  mot_y.setHoming(E_OK, 160, 10000, 10);
  mot_z.setHoming(E_OK, 35, 10000, 11);
  mot_x.setPositionLimits(12000, 0);
  mot_y.setPositionLimits(14000, 0);
  mot_z.setPositionLimits(80000, 0);

  checkSerial();  // Check serial during setup

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
  checkSerial();
  mot_z.home();
  lcd.setCursor(17, 3);
  lcd.print("33%");
  lcd.setCursor(2, 3);
  lcd.print("----");
  checkSerial();
  mot_x.home();
  lcd.setCursor(17, 3);
  lcd.print("66%");
  lcd.setCursor(6, 3);
  lcd.print("----");
  checkSerial();
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

  // Initialize the auto-watering timer
  lastWateringTime = millis();

  checkSerial();  // Check serial at end of setup
}

void loop() {
  // Alexa Integration
  checkSerial();

  // Check for auto-watering every loop iteration
  checkAutoWatering();

  // Main UI state machine
  // Define the screens and the role of the Rotary Encoder
  switch (screen) {
    case 0:  // Home Screen (is without the selector arrow)
      checkSerial();
      homeScreen();
      rtccount = 0;
      screen = 1;  // Directly shifts to the Home Screen 2
      checkSerial();
      break;
    case 1:  // Home Screen with Selector on "Menu"
      checkSerial();
      readrotary();
      checkSerial();
      readbutton(0);
      checkSerial();
      RTCupdate();
      checkSerial();
      lcd.setCursor(2, 3);
      lcd.print(">");
      lcd.setCursor(12, 3);
      lcd.print(" ");
      shiftscreen(2, 1);
      checkSerial();
      checkSerial();
      lcd.setCursor(15, 0);
      lcd.print("H:");
      lcd.print(hum);
      lcd.print("%");

      lcd.setCursor(15, 1);
      // Print temperature
      lcd.print("T:");
      lcd.print(temp);
      lcd.print("C");
      break;
    case 2:  // Home Screen with Selector on "Info"
      checkSerial();
      readrotary();
      checkSerial();
      readbutton(1);
      checkSerial();
      RTCupdate();
      checkSerial();
      lcd.setCursor(2, 3);
      lcd.print(" ");
      lcd.setCursor(12, 3);
      lcd.print(">");
      shiftscreen(1, 2);
      checkSerial();
      break;
    case 3:  // Info Screen (When user selects "Info" on the Home screen)
      checkSerial();
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
      checkSerial();
      delay(7000);
      checkSerial();
      screen = 0;
      break;
    case 4:  // Plant seed screen under menu (When user clicks on "Menu" on the Home screen or when the user turns rotary to the left from "Water Plants" screen)
      checkSerial();
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
      checkSerial();
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
      checkSerial();
      break;
    case 5:  // Read the Rotary
      checkSerial();
      readrotary();
      checkSerial();
      readbutton(3);
      checkSerial();
      RTCupdate();
      checkSerial();
      shiftscreen(5, 6);
      checkSerial();
      break;
    case 6:  // Water Plants screen under Menu (When user moves the encoder to the right when on "Plant Seed" screen or to the left when on "Parameters" screen)
      checkSerial();
      lcd.setCursor(2, 2);
      lcd.print("-|Water Plants|-");
      lcd.setCursor(0, 3);
      lcd.print("<");
      rtccount = 0;
      screen = 7;
      checkSerial();
      break;
    case 7:  // Read the Rotary
      checkSerial();
      readrotary();
      checkSerial();
      readbutton(4);
      checkSerial();
      RTCupdate();
      checkSerial();
      shiftscreen(4, 8);
      checkSerial();
      break;
    case 8:  // Parameters screen under Menu (When user moves the encoder to the right when on "Water Plants" screen or left from the "Exit" screen)
      checkSerial();
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
      checkSerial();
      break;
    case 9:  // Read the rotary
      checkSerial();
      readrotary();
      checkSerial();
      readbutton(5);
      checkSerial();
      RTCupdate();
      checkSerial();
      shiftscreen(6, 10);
      checkSerial();
      break;
    case 10:  // Exit screen under Menu (When user moves the encoder to the right when on "Parameters screen")
      checkSerial();
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
      checkSerial();
      break;
    case 11:  // Read the Rotary
      checkSerial();
      readrotary();
      checkSerial();
      readbutton(6);
      checkSerial();
      RTCupdate();
      checkSerial();
      shiftscreen(8, 10);
      checkSerial();
      break;
    case 12:  // Plant Seeds Now screen when arrow is on Yes (Shown when the user clicks the rotary button when on the Plant Seeds screen)
      checkSerial();
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
      checkSerial();
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
      checkSerial();
      break;
    case 13:  // Read the rotary
      checkSerial();
      readrotary();
      checkSerial();
      readbutton(7);
      checkSerial();
      RTCupdate();
      checkSerial();
      shiftscreen(12, 14);
      checkSerial();
      break;
    case 14:  // Plant Seeds Now screen when the arrow is on No (Triggered when the user turns encoder to the right)
      checkSerial();
      lcd.setCursor(1, 3);
      lcd.print(" ");
      lcd.setCursor(15, 3);
      lcd.print(">");
      readrotary();
      checkSerial();
      readbutton(8);
      checkSerial();
      RTCupdate();
      checkSerial();
      shiftscreen(14, 12);
      checkSerial();
      break;
    case 15:  // Seed Planting Progress screen (Triggered when user clicks on Yes in the Plant Seeds Now screen)
      checkSerial();
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
      checkSerial();
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
      checkSerial();
      homeAllMotors();
      checkSerial();
      seeding();
      lcd.clear();
      checkSerial();
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
      checkSerial();
      mot_z.home();
      lcd.setCursor(17, 3);
      lcd.print("33%");
      lcd.setCursor(2, 3);
      lcd.print("----");
      checkSerial();
      mot_x.home();
      lcd.setCursor(17, 3);
      lcd.print("66%");
      lcd.setCursor(6, 3);
      lcd.print("----");
      checkSerial();
      mot_y.home();
      lcd.setCursor(17, 3);
      lcd.print("99%");
      lcd.setCursor(10, 3);
      lcd.print("----");
      checkSerial();
      homeAllMotors();
      delay(1000);
      lcd.setCursor(0, 2);
      lcd.print("    Homing Done!!!");
      lcd.setCursor(0, 1);
      lcd.print("                    ");
      lcd.setCursor(0, 3);
      lcd.print("                    ");
      pick.write(100);
      delay(2000);
      checkSerial();
      if (alexabool) {
        checkSerial();
        alexabool = false;
        screen = 19;
      } else {
        checkSerial();
        screen = 0;
      }
      break;
    case 16:  // Water Plants Now screen without arrows (Triggered when the user clicks the encoder button when on the Water Plants screen)
      checkSerial();
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
      checkSerial();
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
      checkSerial();
      break;
    case 17:  // Water Plants now screen with the arrow on Yes (Triggered right after the Water Plants Now screen)
      checkSerial();
      lcd.setCursor(1, 3);
      lcd.print(">");

      lcd.setCursor(15, 3);
      lcd.print(" ");
      readrotary();
      checkSerial();
      readbutton(9);
      checkSerial();
      RTCupdate();
      checkSerial();
      shiftscreen(17, 18);
      checkSerial();
      break;
    case 18:  // Water Plants now screen with the arrow on No (Triggered after moving encoder right after the Water Plants Now screen while the arrow is on Yes)
      checkSerial();
      lcd.setCursor(1, 3);
      lcd.print(" ");

      lcd.setCursor(15, 3);
      lcd.print(">");
      readrotary();
      checkSerial();
      readbutton(8);
      checkSerial();
      RTCupdate();
      checkSerial();
      shiftscreen(18, 17);
      checkSerial();
      break;
    case 19:  // Watering progress screen (Triggered after clicking the encoder button while the arrow is on Yes on the Water Plants now screen)
      checkSerial();
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
      checkSerial();
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
      checkSerial();
      homeAllMotors();
      checkSerial();
      watering();
      lcd.clear();
      checkSerial();
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
      checkSerial();
      mot_z.home();
      lcd.setCursor(17, 3);
      lcd.print("33%");
      lcd.setCursor(2, 3);
      lcd.print("----");
      checkSerial();
      mot_x.home();
      lcd.setCursor(17, 3);
      lcd.print("66%");
      lcd.setCursor(6, 3);
      lcd.print("----");
      checkSerial();
      mot_y.home();
      lcd.setCursor(17, 3);
      lcd.print("99%");
      lcd.setCursor(10, 3);
      lcd.print("----");
      checkSerial();
      homeAllMotors();
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
      checkSerial();
      break;
    case 20:  // Parameters screen (Triggered when encoder button is clicked on Parameters screen under Menu)
      checkSerial();
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
      checkSerial();
      for (int i = 0; i < 1000; i++) {
        readrotary();
        checkSerial();
        readbutton(10);
        checkSerial();
        RTCupdate();
        checkSerial();
        delay(1);
      }
      rtccount = 0;
      checkSerial();
      break;
  }

  checkSerial();  // Check serial before moisture sending

  if (millis() - lastSend > 1000) {
    sendMoistureToPi();
    lastSend = millis();
  }

  checkSerial();  // Check serial after moisture sending

  // Always listen for Pi DHT data
  // delay to avoid hardware errors
  delay(1);

  checkSerial();  // Check serial at end of loop
}
