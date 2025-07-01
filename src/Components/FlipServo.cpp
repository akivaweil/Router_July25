//* ************************************************************************
//* ************************ FLIP SERVO COMPONENT ***********************
//* ************************************************************************
//! Flip servo control component for ESP32 Router Control System
//! Handles all servo operations for wood flipping mechanism

#include <Arduino.h>
#include <ESP32Servo.h>
#include "../../include/Config.h"
#include "../../include/Pins_Definitions.h"

//* ************************************************************************
//* ************************ FLIP SERVO VARIABLES ***********************
//* ************************************************************************

static Servo flipServo;
static bool servoInitialized = false;
static bool servoAttached = false;
static int currentPosition = FLIP_SERVO_ZERO_POSITION;
static unsigned long lastMoveTime = 0;

//* ************************************************************************
//* ************************ FLIP SERVO FUNCTIONS ***********************
//* ************************************************************************

//! Initialize the flip servo component
void initFlipServo() {
    if (!servoInitialized) {
        // Attach servo to pin
        flipServo.attach(FLIP_SERVO_PIN);
        servoAttached = true;
        
        // Move to safe zero position
        flipServo.write(FLIP_SERVO_ZERO_POSITION);
        currentPosition = FLIP_SERVO_ZERO_POSITION;
        lastMoveTime = millis();
        
        Serial.println("Flip Servo initialized at zero position");
        servoInitialized = true;
        
        // Give servo time to reach position
        delay(500);
    }
}

//! Move servo to specified position
void moveFlipServoToPosition(int position) {
    if (!servoInitialized) {
        Serial.println("ERROR: Flip servo not initialized");
        return;
    }
    
    // Constrain position to valid servo range
    position = constrain(position, 0, 180);
    
    Serial.print("Flip Servo: Moving to ");
    Serial.print(position);
    Serial.println(" degrees");
    
    flipServo.write(position);
    currentPosition = position;
    lastMoveTime = millis();
}

//! Move servo to zero position
void moveFlipServoToZero() {
    moveFlipServoToPosition(FLIP_SERVO_ZERO_POSITION);
}

//! Perform flip sequence (move to position, wait, return to zero)
void performFlipSequence(int flipPosition, unsigned long waitTime) {
    if (!servoInitialized) {
        Serial.println("ERROR: Flip servo not initialized");
        return;
    }
    
    Serial.println("Starting flip sequence...");
    
    // Move to flip position
    moveFlipServoToPosition(flipPosition);
    
    // Wait for servo to reach position
    delay(waitTime);
    
    // Return to zero position
    moveFlipServoToZero();
    
    Serial.println("Flip sequence complete");
}

//! Perform standard flip sequence (uses config values)
void performStandardFlip() {
    performFlipSequence(FLIP_SERVO_ZERO_POSITION, FLIP_SERVO_MOVE_DELAY);
}

//! Get current servo position
int getFlipServoPosition() {
    return currentPosition;
}

//! Check if servo is at zero position
bool isFlipServoAtZero() {
    return (currentPosition == FLIP_SERVO_ZERO_POSITION);
}

//! Get time since last servo movement
unsigned long getTimeSinceLastMove() {
    return millis() - lastMoveTime;
}

//! Detach servo (for power saving or emergency)
void detachFlipServo() {
    if (servoAttached) {
        // Move to safe position first
        moveFlipServoToZero();
        delay(200);
        
        flipServo.detach();
        servoAttached = false;
        
        Serial.println("Flip Servo detached");
    }
}

//! Re-attach servo if detached
void reattachFlipServo() {
    if (!servoAttached && servoInitialized) {
        flipServo.attach(FLIP_SERVO_PIN);
        servoAttached = true;
        
        // Restore position
        flipServo.write(currentPosition);
        
        Serial.println("Flip Servo re-attached");
    }
}

//! Emergency stop servo (move to safe position and detach)
void emergencyStopFlipServo() {
    Serial.println("EMERGENCY: Stopping flip servo");
    
    if (servoAttached) {
        // Move to safe zero position
        flipServo.write(FLIP_SERVO_ZERO_POSITION);
        delay(100);
        flipServo.detach();
        servoAttached = false;
    }
    
    currentPosition = FLIP_SERVO_ZERO_POSITION;
}

//! Check servo component status
bool checkFlipServoStatus() {
    return servoInitialized && servoAttached;
}

//! Get servo state as string
const char* getFlipServoState() {
    if (!servoInitialized) {
        return "NOT_INITIALIZED";
    } else if (!servoAttached) {
        return "DETACHED";
    } else {
        return "ATTACHED";
    }
} 