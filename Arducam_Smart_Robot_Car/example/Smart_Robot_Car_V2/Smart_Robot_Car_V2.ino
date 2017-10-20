
// ArduCAM Smart_Robot_Car demo (C)2017 Lee
// This demo support smart mode .
// This demo support bluetooth control and IR remote control.
// The default mode is bluetooth control you can change it to
// the IR control mode by touching the IRENABLE key.
// We optimize the IR control code for more sensitive control.
// When there are some obstacles ahead, the car will stop.
//video link: https://youtu.be/0FB7J-Qzcag
/***********************[NOTICE]*********************************
  We can't guarantee that the motor load
  is exactly the same, so it increases the compensation
  factor. You should adjust them to suit for your motor
****************************************************************/
#define leftFactor 10
#define rightFactor 5
#define speedSet  150

#define TURN_DIST 40
#include <AFMotor.h>
#include <Servo.h>
#include "ArducamNEC.h"



ArducamNEC myIR(2);
AF_DCMotor leftMotor(3, MOTOR34_64KHZ);
AF_DCMotor rightMotor(4, MOTOR34_64KHZ);
Servo neckControllerServoMotor;

int trig = A2;
int echo = A3;
unsigned int S;
unsigned int Sleft;
unsigned int Sright;
int RECV_PIN = 2;
bool smartEnable = false;  //This flag is used for enable smart mode
bool IR_Enable = false;    //This flag is used for enable ir remote control mode
bool detected_flag = false; //This flag is used for enable detected obstacles ahead
void setup() {
  // put your setup code here, to run once:
  uint8_t temp;
  Serial.begin(9600);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  neckControllerServoMotor.attach(10);
  neckControllerServoMotor.write(90);
  delay(2000);
}
void loop() {
  // put your main code here, to run repeatedly:
  uint32_t temp = 0xff;
  int detecteVal = 0;
  if (!IR_Enable)
  {
    if (Serial.available())
    {
      temp = Serial.read();
      switch (temp)
      {
        case 0x01:
          Serial.println(F( "Move forward"));
          temp = 0;
          smartEnable = false;
          detected_flag = true;
          range();
          if (S <= 30) {
            moveStop();
          }
          else
            moveForward();
          break;
        case 0x02:
          temp = 0;
          Serial.println(F(" Move backward"));
          smartEnable = false;
          detected_flag = false;
          moveBackward();
          break;
        case 0x03:
          temp = 0;
          Serial.println(F(" Turn left"));
          smartEnable = false;
          detected_flag = false;
          turnLeft();
          break;
        case 0x04:
          temp = 0;
          Serial.println(F(" Turn right"));
          smartEnable = false;
          detected_flag = false;
          turnRight();
          break;
        case 0x05:
          temp = 0;
          Serial.println(F(" Stop"));
          smartEnable = false;
          detected_flag = false;
          neckControllerServoMotor.write(90);
          delay(100);
          neckControllerServoMotor.detach();
          moveStop();
          break;
        case 0x06:
          temp = 0;
          neckControllerServoMotor.attach(10);
          neckControllerServoMotor.write(90);
          smartEnable = true;
          IR_Enable = false;
          detected_flag = false;
          Serial.println(F("this is smart mode and you don't do anything"));
          moveStop();
          delay(1000);
          detected_flag = false;
          moveForward();
          break;
        case 0x07:
          temp = 0;
          delay(100);
          neckControllerServoMotor.detach();
          delay(100);
          myIR.begin();
          IR_Enable = true;
          detected_flag = false;
          smartEnable = false;
          Serial.println(F("this is IR control mode, Please use your IR controller"));
          moveStop();
          break;
        default:
          break;
      }
    }
  }
  if (IR_Enable) {
    while (myIR.available())
    {
      temp =  myIR.read();
    }
    if (temp == 0xFF46B9)  //up
    {
      temp = 0;
      Serial.println(F(" Move forward"));
      detected_flag = true;
      range();
      if (S <= 30) {
        moveStop();
      }
      else
        moveForward();
      moveForward();
    } else if (temp == 0xFF15EA) { //down
      detected_flag = false;
      Serial.println(F(" Move backward"));
      temp = 0;
      moveBackward();
    } else if (temp == 0xFF44BB) { // left
      Serial.println(F(" Turn left"));
      temp = 0;
      detected_flag = true;
      turnLeft();
    } else if (temp == 0xFF43BC) { //right
      temp = 0;
      detected_flag = false;
      Serial.println(F(" Turn right"));
      turnRight();
    } else if (temp == 0xFF40BF) {
      Serial.println(F(" Stop"));
      temp = 0;
      smartEnable = false;
      detected_flag = false;
      neckControllerServoMotor.write(90);
      moveStop();
    } else if (temp == 0xFF42BD) { //change to bluetooth mode
      IR_Enable = false;
      smartEnable = false;
      detected_flag = false;
      neckControllerServoMotor.write(90);
      Serial.println(F(" This is bluetooth control mode, Please use your bluetooth controller"));
      moveStop();
    }
  }
  if (detected_flag) {
    range();
    if (S <= 30) {
      moveStop();
    }
  }
  if (smartEnable) {
    neckControllerServoMotor.write(90);
    range();
    if (S <= TURN_DIST ) {
      turn();
    } else if (S > TURN_DIST) {
      moveForward();
    }
  }
}

void turn() {
  moveStop();
  neckControllerServoMotor.write(150);
  delay(500);
  range();
  Sleft = S;
  neckControllerServoMotor.write(90);
  delay(500);
  neckControllerServoMotor.write(30);
  delay(500);
  range();
  Sright = S;
  neckControllerServoMotor.write(90);
  delay(500);
  if (Sleft <= TURN_DIST && Sright <= TURN_DIST) {
    moveBackward();
    delay(500);
    int x = random(1);
    if (x = 0) {
      turnRight();
    }
    else {
      turnLeft();
    }
  } else {
    if (Sleft >= Sright) {
      turnLeft();
    } else {
      turnRight();
    }
  }
}

void range() {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(20);
  digitalWrite(trig, LOW);
  int distance = pulseIn(echo, HIGH);
  distance = distance / 58;
  S = distance;
  if (S < TURN_DIST) {
    delay(50);
  }
}
void moveForward() {
  leftMotor.run(FORWARD);
  rightMotor.run(FORWARD);
  leftMotor.setSpeed(speedSet + leftFactor);
  rightMotor.setSpeed(speedSet + rightFactor);
}
void turnLeft() {
  leftMotor.run(BACKWARD);
  rightMotor.run(FORWARD);
  leftMotor.setSpeed(speedSet + leftFactor);
  rightMotor.setSpeed(speedSet + rightFactor);
  delay(400);
  moveStop();
}
void turnRight() {
  leftMotor.run(FORWARD);
  rightMotor.run(BACKWARD);
  leftMotor.setSpeed(speedSet + leftFactor);
  rightMotor.setSpeed(speedSet + rightFactor);
  delay(400);
  moveStop();
}
void moveBackward() {
  leftMotor.run(BACKWARD);
  rightMotor.run(BACKWARD);
  leftMotor.setSpeed(speedSet + leftFactor);
  rightMotor.setSpeed(speedSet + rightFactor);
}
void moveStop() {
  leftMotor.run(RELEASE); rightMotor.run(RELEASE);
}

