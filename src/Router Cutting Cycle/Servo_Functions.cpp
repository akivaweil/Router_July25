#include <Arduino.h>
#include <ESP32Servo.h>
#include "../../include/Config/Servo_Config.h"

//* ************************************************************************
//* ************************ SERVO FUNCTIONS ******************************
//* ************************************************************************
// Router servo control functions based on Stage 1 approach

// External servo object reference from main.cpp
extern Servo flipServo;

//* ************************************************************************
//* ************************ SERVO CONTROL FUNCTIONS ********************
//* ************************************************************************

void moveServoToHome() {
    // Move servo to home position using Stage 1 approach
    flipServo.write(ROUTER_SERVO_HOME_POSITION);
    Serial.printf("FORCED Servo command sent: %d degrees (home position)\n", ROUTER_SERVO_HOME_POSITION);
}

void moveServoToActive() {
    // Move servo to active position using Stage 1 approach  
    flipServo.write(ROUTER_SERVO_ACTIVE_POSITION);
    Serial.printf("FORCED Servo command sent: %d degrees (active position)\n", ROUTER_SERVO_ACTIVE_POSITION);
}

void moveServoToAngle(int angle) {
    // Move servo to specific angle with bounds checking
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    
    flipServo.write(angle);
    Serial.printf("FORCED Servo command sent: %d degrees (custom angle)\n", angle);
}

//* ************************************************************************
//* ************************ SERVO GETTER FUNCTION ***********************
//* ************************************************************************

Servo* getFlipServo() {
    return &flipServo;
} 