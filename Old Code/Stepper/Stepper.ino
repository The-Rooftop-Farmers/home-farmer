// Include the AccelStepper Library
#include <AccelStepper.h>
#include <VarSpeedServo.h>

// Define pin connections
const int xstepPin = 2;
const int xdirPin = 5;

const int ystepPin = 3;
const int ydirPin = 6;

const int zstepPin = 4;
const int zdirPin = 7;

const int servopin = 12;
const int pumppin = 13;

// Define motor interface type
#define motorInterfaceType 1

// Creates an instance
VarSpeedServo picker;
AccelStepper xStepper(motorInterfaceType, xstepPin, xdirPin);
AccelStepper yStepper(motorInterfaceType, ystepPin, ydirPin);
AccelStepper zStepper(motorInterfaceType, zstepPin, zdirPin);
int pos = 0;
void setup() {
  picker.attach(servopin);

  // set the maximum speed, acceleration factor,
  // initial speed and the target position
  yStepper.setMaxSpeed(800);
  yStepper.setAcceleration(500);
  yStepper.setSpeed(700);

  xStepper.setMaxSpeed(800);
  xStepper.setAcceleration(500);
  xStepper.setSpeed(700);

  zStepper.setMaxSpeed(800);
  zStepper.setAcceleration(500);
  zStepper.setSpeed(700);

  Serial.begin(9600);
  picker.write(40, 30, true);
}

void loop() {
  // Change direction once the motor reaches target position

  switch (pos) {
    case 0:

      zStepper.moveTo(-2300);

      if (zStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 0;
        delay(2000);
        // yStepper.moveTo(0);
      }
      // Move the motor one step
      zStepper.run();

      break;
    case 1:
      zStepper.moveTo(2300);
      zStepper.run();

      if (zStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 2;
        picker.write(40, 30, true);
        delay(1000);
        picker.write(0, 30, true);
        delay(500);
      }
      break;
    case 2:
      zStepper.moveTo(0);
      zStepper.run();

      //yStepper.moveTo(-17500);

      if (zStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 3;
      }
      // Move the motor one step
      //yStepper.run();

      break;

    case 3:

      yStepper.moveTo(-16500);

      if (yStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 4;
        delay(2000);
        yStepper.moveTo(0);
      }
      // Move the motor one step
      yStepper.run();

      break;

    case 4:
      zStepper.moveTo(5000);
      zStepper.run();

      //yStepper.moveTo(-17500);

      if (zStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 5;
        picker.write(40, 30, true);
        delay(500);
      }

      break;


    case 5:
      zStepper.moveTo(0);
      zStepper.run();

      //yStepper.moveTo(-17500);

      if (zStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 6;
      }

      break;

    case 6:

      yStepper.moveTo(0);

      if (yStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 7;
      }
      // Move the motor one step
      yStepper.run();

      break;

    case 7:
      zStepper.moveTo(2300);
      zStepper.run();

      //yStepper.moveTo(-17500);

      if (zStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 8;
        picker.write(0, 30, true);
        delay(500);
      }
      break;

    case 8:
      zStepper.moveTo(0);
      zStepper.run();

      //yStepper.moveTo(-17500);

      if (zStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 9;
      }

      break;

    case 9:

      yStepper.moveTo(-33000);

      if (yStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 10;
      }
      // Move the motor one step
      yStepper.run();

      break;

    case 10:
      zStepper.moveTo(5000);
      zStepper.run();

      //yStepper.moveTo(-17500);

      if (zStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 11;
        picker.write(40, 30, true);
        delay(500);
      }
      // Move the motor one step


      break;
    case 11:
      zStepper.moveTo(0);


      //yStepper.moveTo(-17500);

      if (zStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 12;
      }
      // Move the motor one step
      zStepper.run();

      break;
    case 12:

      yStepper.moveTo(0);

      if (yStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 13;
      }
      // Move the motor one step
      yStepper.run();

      break;
    case 13:
      zStepper.moveTo(2300);
      zStepper.run();

      //yStepper.moveTo(-17500);

      if (zStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 14;
        picker.write(0, 30, true);
        delay(500);
      }
      // Move the motor one step


      break;
    case 14:
      zStepper.moveTo(0);
      zStepper.run();

      //yStepper.moveTo(-17500);

      if (zStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 15;
      }
      // Move the motor one step


      break;
    case 15:

      xStepper.moveTo(-15250);

      if (xStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 16;
      }
      // Move the motor one step
      xStepper.run();

      break;
    case 16:
      yStepper.moveTo(-3000);
      yStepper.run();

      //yStepper.moveTo(-17500);

      if (yStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 17;
      }
      // Move the motor one step


      break;
    case 17:
      zStepper.moveTo(5000);
      zStepper.run();

      //yStepper.moveTo(-17500);

      if (zStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 18;
        picker.write(40, 30, true);
        delay(500);
      }
      // Move the motor one step


      break;
    case 18:
      zStepper.moveTo(0);
      zStepper.run();

      //yStepper.moveTo(-17500);

      if (zStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 19;
      }
      // Move the motor one step


      break;
    case 19:
      xStepper.moveTo(0);
      xStepper.run();
      yStepper.moveTo(0);
      yStepper.run();

      //yStepper.moveTo(-17500);

      if (xStepper.distanceToGo() == 0 && yStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 20;
      }
      // Move the motor one step


      break;
    case 20:
      zStepper.moveTo(2300);
      zStepper.run();

      //yStepper.moveTo(-17500);

      if (zStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 21;
        picker.write(0, 30, true);
        delay(500);
      }
      // Move the motor one step


      break;
    case 21:
      zStepper.moveTo(0);
      zStepper.run();

      //yStepper.moveTo(-17500);

      if (zStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 22;
      }
      // Move the motor one step


      break;
    case 23:

      xStepper.moveTo(-15250);

      if (xStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 24;
      }
      // Move the motor one step
      xStepper.run();

      break;
    case 24:
      yStepper.moveTo(-16500);
      yStepper.run();

      //yStepper.moveTo(-17500);

      if (yStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 25;
      }
      // Move the motor one step


      break;
    case 25:
      zStepper.moveTo(5000);
      zStepper.run();

      //yStepper.moveTo(-17500);

      if (zStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 26;
        picker.write(40, 30, true);
        delay(500);
      }
      // Move the motor one step


      break;
    case 26:
      zStepper.moveTo(0);
      zStepper.run();

      //yStepper.moveTo(-17500);

      if (zStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 27;
      }
      // Move the motor one step


      break;
    case 27:
      xStepper.moveTo(0);
      xStepper.run();
      yStepper.moveTo(0);
      yStepper.run();

      //yStepper.moveTo(-17500);

      if (xStepper.distanceToGo() == 0 && yStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 28;
      }
      // Move the motor one step


      break;
    case 28:
      zStepper.moveTo(2300);
      zStepper.run();

      //yStepper.moveTo(-17500);

      if (zStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 29;
        picker.write(0, 30, true);
        delay(500);
      }
      // Move the motor one step


      break;
    case 29:
      zStepper.moveTo(0);
      zStepper.run();

      //yStepper.moveTo(-17500);

      if (zStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 30;
      }
      // Move the motor one step


      break;
    case 30:

      xStepper.moveTo(-15250);

      if (xStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 31;
      }
      // Move the motor one step
      xStepper.run();

      break;
    case 31:
      yStepper.moveTo(-33000);
      yStepper.run();

      //yStepper.moveTo(-17500);

      if (yStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 32;
      }
      // Move the motor one step


      break;
    case 32:
      zStepper.moveTo(5000);
      zStepper.run();

      //yStepper.moveTo(-17500);

      if (zStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 33;
        picker.write(40, 30, true);
        delay(500);
      }
      // Move the motor one step


      break;
    case 33:
      zStepper.moveTo(0);
      zStepper.run();

      //yStepper.moveTo(-17500);

      if (zStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 34;
      }
      // Move the motor one step


      break;
    case 34:
      xStepper.moveTo(0);
      xStepper.run();
      yStepper.moveTo(0);
      yStepper.run();

      //yStepper.moveTo(-17500);

      if (xStepper.distanceToGo() == 0 && yStepper.distanceToGo() == 0) {
        //yStepper.moveTo(yStepper.currentPosition());
        pos = 35;
      }
      // Move the motor one step


      break;
  }
}