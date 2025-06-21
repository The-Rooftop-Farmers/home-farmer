#define E_OK 0
#define E_NOK 1

#include"Servo.h"
Servo pick;

#include <LiquidCrystal_I2C.h>
#include "Arduino.h"
#include "uRTCLib.h"
#include <dht.h>
#define outPin 0
#define watdel 6000
#define afterwatdel 3000
// uRTCLib rtc;
uRTCLib rtc(0x68);
dht DHT;
long mois =0;

char daysOfTheWeek[7][12] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

LiquidCrystal_I2C lcd(0x27, 20, 4);  // set the LCD address to 0x3F for a 16 chars and 2 line display
int d, mo, y, h, m, s, AP;
int readData;
int hum, temp;


void setup() {
  pick.attach(13);
  // put your setup code here, to run once:
 // pinMode(0,OUTPUT);
  pinMode(2,OUTPUT); // pin
  pinMode(3,OUTPUT); // pin
  pinMode(4,OUTPUT);
  pinMode(9,INPUT_PULLUP); // limit switch
  pinMode(10,INPUT_PULLUP); 
  pinMode(11,INPUT_PULLUP);
  pinMode(5,OUTPUT); // direction
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
  pinMode(12,OUTPUT);
  pinMode(8,OUTPUT); 
  digitalWrite(8, LOW);
  digitalWrite(12,LOW);
  // Define all the other components for LCD and Rotary , DHT, RTC
    URTCLIB_WIRE.begin();
  //rtc.set(0, 32, 11, 7, 24, 8, 24);
  // rtc.set(second, minute, hour, dayOfWeek, dayOfMonth, month, year)
  // set day of week (1=Sunday, 7=Saturday)
  //LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();  // Make sure backlight is on
                    // Read the initial state of CLK
  lastStateCLK = analogRead(A2);
  if (lastStateCLK > 512)
    lastStateCLK = 1;
  else
    lastStateCLK = 0;


  readRTC();
  readDHT();
  

  //Define motors
  mot_x.setPin(2,5); // x
  mot_y.setPin(3,6); // y 
  mot_z.setPin(4,7); // z
  mot_x.setSpeed(160);
  mot_y.setSpeed(160);
  mot_z.setSpeed(35);
  mot_x.setHoming(E_OK,160, 10000,9);
  mot_y.setHoming(E_OK,160, 10000,10); // disable homing for extrusion motor
  mot_z.setHoming(E_OK,35, 10000,11);
  mot_x.setPositionLimits(12000,0);
  mot_y.setPositionLimits(14000,0);
  mot_z.setPositionLimits(65000,0);
  lcd.clear();
  lcd.setCursor(5,0);
  lcd.print("Home Farmer");
  lcd.setCursor(7,1);
  lcd.print("Homing...");
  lcd.setCursor(0,2);
  lcd.print("Please Wait...");
  lcd.setCursor(0,3);
  lcd.print("[-");
  lcd.setCursor(16,3);
  lcd.print("]");
  lcd.setCursor(17,3);
  lcd.print(" 1%");
  delay(3000);
  
  mot_z.home();
  lcd.setCursor(17,3);
  lcd.print("33%");
  lcd.setCursor(2,3);
  lcd.print("----");
  mot_x.home();
  lcd.setCursor(17,3);
  lcd.print("66%");
  lcd.setCursor(6,3);
  lcd.print("----");
  mot_y.home();
  lcd.setCursor(17,3);
  lcd.print("99%");
  lcd.setCursor(10,3);
  lcd.print("----");
  delay(1000);
  lcd.setCursor(0,2);
  lcd.print("    Homing Done!!!");
  lcd.setCursor(0,1);
  lcd.print("                    ");
   lcd.setCursor(0,3);
  lcd.print("                    ");
  pick.write(100);
  delay(2000);
  homeScreen();
}

void loop() {
  // put your main code here, to run repeatedly:
switch (screen) {
    case 0:
      homeScreen();
      rtccount =0;
      screen = 1;
      break;
    case 1:
      readrotary();
      readbutton(0);
      RTCupdate();
      lcd.setCursor(2, 3);
      lcd.print(">");
      lcd.setCursor(12, 3);
      lcd.print(" ");
      shiftscreen(2, 1);
      break;
    case 2:
      readrotary();
      readbutton(1);
      RTCupdate();
      lcd.setCursor(2, 3);
      lcd.print(" ");
      lcd.setCursor(12, 3);
      lcd.print(">");
      shiftscreen(1, 2);
      break;
    case 3:
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("Home Farmer V2.4");
      lcd.setCursor(4, 1);
      lcd.print("A Project By");
      lcd.setCursor(1, 2);
      lcd.print("Vihaan & Yogeshwar");
      lcd.setCursor(2, 3);
      lcd.print("Mentor: Malhar A");
      rtccount =0;
      delay(7000);
      screen = 0;
      break;
    case 4:
    lcd.clear();
    lcd.setCursor(15,0);
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
    lcd.setCursor(9,0);
    lcd.print(daysOfTheWeek[rtc.dayOfWeek() - 1]);

    lcd.setCursor(0,3);
    lcd.print(" ");
    lcd.setCursor(19,3);
    lcd.print(">");

    lcd.setCursor(7,1);
    lcd.print("[Menu]");

    lcd.setCursor(2,2);
    lcd.print("-| Plant Seed |-");
    rtccount =0;
    screen =5;
    break;
    case 5: 
    readrotary();
    readbutton(3);
    RTCupdate();
    shiftscreen(5,6);
    break;
    case 6:
    lcd.setCursor(2,2);
    lcd.print("-|Water Plants|-");
    lcd.setCursor(0,3);
    lcd.print("<");
    rtccount =0;
    screen = 7;
    break;
    case 7:
    readrotary();
    readbutton(4);
    RTCupdate();
    shiftscreen(4,8);
    break;
    case 8:
    lcd.setCursor(1,2);
    lcd.print("                  ");
    lcd.setCursor(2,2);
    lcd.print("-| Parameters |-");
    lcd.setCursor(0,3);
    lcd.print("<");
    lcd.setCursor(19,3);
    lcd.print(">");
    rtccount =0;
    screen = 9;
    break;
    case 9:
    readrotary();
    readbutton(5);
    RTCupdate();
    shiftscreen(6,10);
    break;
    case 10:
    lcd.setCursor(1,2);
    lcd.print("                  ");
    lcd.setCursor(6,2);
    lcd.print("-| Exit |-");
    lcd.setCursor(0,3);
    lcd.print("<");
    lcd.setCursor(19,3);
    lcd.print(" ");
    rtccount =0;
    screen = 11;
    break;
    case 11:
    readrotary();
    readbutton(6);
    RTCupdate();
    shiftscreen(8,10);
    break;
    case 12:
    lcd.clear();
    lcd.setCursor(15,0);
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
    lcd.setCursor(9,0);
    lcd.print(daysOfTheWeek[rtc.dayOfWeek() - 1]);

    lcd.setCursor(2,1);
    lcd.print("-| Plant Seed |-");

    lcd.setCursor(2,2);
    lcd.print("Plant seeds Now?");

    lcd.setCursor(2,3);
    lcd.print("Yes");

    lcd.setCursor(16,3);
    lcd.print("No");

    lcd.setCursor(1,3);
    lcd.print(">");

    lcd.setCursor(15,3);
    lcd.print(" ");
    screen =13;
    rtccount =0;
    break;
    case 13:
    readrotary();
    readbutton(7);
    RTCupdate();
    shiftscreen(12,14);
    break;
    case 14:
    lcd.setCursor(1,3);
    lcd.print(" ");

    lcd.setCursor(15,3);
    lcd.print(">");
    readrotary();
    readbutton(8);
    RTCupdate();
    shiftscreen(14,12);
    break;
    case 15:
    lcd.clear();
    lcd.setCursor(15,0);
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
    lcd.setCursor(9,0);
    lcd.print(daysOfTheWeek[rtc.dayOfWeek() - 1]);

    lcd.setCursor(2,2);
    lcd.print("Planting Seeds...");
    lcd.setCursor(2,3);
    lcd.print("Please wait...");
    seeding();
    lcd.clear();
      lcd.setCursor(1,0);
  lcd.print("Seeding Successful!");
  lcd.setCursor(7,1);
  lcd.print("Homing...");
  lcd.setCursor(0,2);
  lcd.print("Please Wait...");
  lcd.setCursor(0,3);
  lcd.print("[-");
  lcd.setCursor(16,3);
  lcd.print("]");
  lcd.setCursor(17,3);
  lcd.print(" 1%");
      mot_z.home();
  lcd.setCursor(17,3);
  lcd.print("33%");
  lcd.setCursor(2,3);
  lcd.print("----");
  mot_x.home();
  lcd.setCursor(17,3);
  lcd.print("66%");
  lcd.setCursor(6,3);
  lcd.print("----");
  mot_y.home();
  lcd.setCursor(17,3);
  lcd.print("99%");
  lcd.setCursor(10,3);
  lcd.print("----");
  delay(1000);
  lcd.setCursor(0,2);
  lcd.print("    Homing Done!!!");
  lcd.setCursor(0,1);
  lcd.print("                    ");
   lcd.setCursor(0,3);
  lcd.print("                    ");
  pick.write(100);
  delay(2000);
    screen =0;
    break;
  case 16:
  delay(300);
  lcd.clear();
    lcd.setCursor(15,0);
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
    lcd.setCursor(9,0);
    lcd.print(daysOfTheWeek[rtc.dayOfWeek() - 1]);

    lcd.setCursor(2,1);
    lcd.print("-| Water Plants |-");

    lcd.setCursor(2,2);
    lcd.print("Water Plants Now?");

    lcd.setCursor(2,3);
    lcd.print("Yes");

    lcd.setCursor(16,3);
    lcd.print("No");
    screen =17;
    rtccount =0;
    break;
    case 17:
    lcd.setCursor(1,3);
    lcd.print(">");

    lcd.setCursor(15,3);
    lcd.print(" ");
    readrotary();
    readbutton(9);
    RTCupdate();
    shiftscreen(17,18);
    break;
    case 18:
    lcd.setCursor(1,3);
    lcd.print(" ");

    lcd.setCursor(15,3);
    lcd.print(">");
    readrotary();
    readbutton(8);
    RTCupdate();
    shiftscreen(18,17);
    break;
     case 19:
    lcd.clear();
    lcd.setCursor(15,0);
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
    lcd.setCursor(9,0);
    lcd.print(daysOfTheWeek[rtc.dayOfWeek() - 1]);

    lcd.setCursor(2,2);
    lcd.print("Water Planting...");
    lcd.setCursor(2,3);
    lcd.print("Please wait...");
    watering();
    lcd.clear();
    lcd.setCursor(0,0);
  lcd.print("Watering Successful!");
  lcd.setCursor(7,1);
  lcd.print("Homing...");
  lcd.setCursor(0,2);
  lcd.print("Please Wait...");
  lcd.setCursor(0,3);
  lcd.print("[-");
  lcd.setCursor(16,3);
  lcd.print("]");
  lcd.setCursor(17,3);
  lcd.print(" 1%");
      mot_z.home();
  lcd.setCursor(17,3);
  lcd.print("33%");
  lcd.setCursor(2,3);
  lcd.print("----");
  mot_x.home();
  lcd.setCursor(17,3);
  lcd.print("66%");
  lcd.setCursor(6,3);
  lcd.print("----");
  mot_y.home();
  lcd.setCursor(17,3);
  lcd.print("99%");
  lcd.setCursor(10,3);
  lcd.print("----");
  delay(1000);
  lcd.setCursor(0,2);
  lcd.print("    Homing Done!!!");
  lcd.setCursor(0,1);
  lcd.print("                    ");
   lcd.setCursor(0,3);
  lcd.print("                    ");
  pick.write(100);
  delay(2000);
    screen =0;
    break;
    case 20:
    
      readDHT();
      readRTC();
    lcd.setCursor(3,0);
    lcd.print("* Parameters *");
    lcd.setCursor(0,1);
    lcd.print("Temperature:");
    lcd.print(temp);
    lcd.print("C");
    lcd.setCursor(0,2);
    lcd.print("Humidity:");
    lcd.print(hum);
    lcd.print("%");
    lcd.setCursor(0,3);
    mois = analogRead(A3);
    mois = map(mois, 230, 1023, 0, 100);
    mois = 100 - mois;
    lcd.print("Moisture:");
    lcd.print(mois);
    lcd.print("%   ");
    for(int i=0;i<1000;i++)
    {
    readrotary();
    readbutton(10);
    RTCupdate();
    delay(1);
    }
    rtccount =0;
    break;
  }
    delay(1);
}