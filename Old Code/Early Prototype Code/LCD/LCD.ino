#include <LiquidCrystal_I2C.h>
#include "uRTCLib.h"

//Define Variables
int x = 0, y = 0, hour;
bool pm;

char daysOfTheWeek[7][12] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

// Define the Main objects
LiquidCrystal_I2C lcd(0x27, 20, 4);  // set the LCD address to 0x27 for a 20x4 display
uRTCLib rtc(0x68);                   // set RTC adress to 0x68 for DS 1307 Module

void setup() {
  URTCLIB_WIRE.begin();
  lcd.init();  // Removed the parameters

  lcd.clear();
  lcd.backlight();
  pinMode(2, INPUT);

  // Comment out below line once you set the date & time.
  // Following line sets the RTC with an explicit date & time
  // for example to set January 13 2022 at 12:56 you would call:
  rtc.set(0, 16, 17, 1, 8, 4, 24);
  // rtc.set(second, minute, hour, dayOfWeek, dayOfMonth, month, year)
  // set day of week (1=Monday, 7=Sunday)
  lcd.setCursor(6, 1);
  lcd.print("HOME FARMER");
  lcd.clear();
  delay(1000);
}

void loop() {

  x = digitalRead(2);
  // Turn on the backlight

  // Timer to Print messages on the LCD (testing purposes ONLY)
  if (x == 1) {
    y = y + 1;
    delay(1000);
  }

  // RTC
  rtc.refresh();
  lcd.setCursor(0, 0);
  // lcd.clear();
  lcd.print(rtc.day());
  lcd.print('/');
  lcd.print(rtc.month());
  lcd.print('/');
  lcd.print(rtc.year());


  lcd.setCursor(7, 0);
  lcd.print(" (");
  lcd.print(daysOfTheWeek[rtc.dayOfWeek() - 1]);
  lcd.print(") ");

  lcd.setCursor(16, 0);
  if (rtc.hour() > 12) {
    hour = rtc.hour() - 12;
    pm = true;
  } else {
    hour = rtc.hour();
    pm = false;
  }
  lcd.print("0");
  lcd.print(hour);
  lcd.print(":");
  if (rtc.minute() >= 10) {
    lcd.print(rtc.minute());
  } else {
    lcd.print("0");
    lcd.print(rtc.minute());
  }
  lcd.print(":");
  if (rtc.second() >= 10) {
    lcd.print(rtc.second());
  } else {
    lcd.print("0");
    lcd.print(rtc.second());
  }
  lcd.setCursor(14, 1);
  if (pm == true) {
    lcd.print("PM");
  } else {
    lcd.print("AM");
    delay(1000);
  }
  lcd.setCursor(18, 0);
  lcd.print(" ");
  lcd.setCursor(0, 1);
  lcd.print("seed status:");

  if (y == 1) {
    lcd.setCursor(13, 1);
    lcd.print("start");
  }

  if (y == 2) {
    lcd.setCursor(13, 1);
    lcd.print("stop ");
  }

  if (rtc.hour() == 8 && rtc.minute() == 5) {

    lcd.setCursor(0, 2);
    lcd.print("watering ");
    //stepper code;
    //water pump;
  }



  if (rtc.hour() == 8 && rtc.minute() == 6) {

    lcd.setCursor(0, 2);
    lcd.print(" stopped ");
    //exactly the same as the last one
  }

  if (y > 2) {
    y = 0;
  }
}
//farmbot