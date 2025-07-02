#include "../include/ServoMotor.h"
#include <ESP32Servo.h>

//* ************************************************************************
//* **************************** SERVO MOTOR *******************************
//* ************************************************************************

ServoMotor::ServoMotor(int pin) : servoPin(pin), currentAngle(0.0f) {
    // FORCE SERVO ATTACHMENT - IGNORE ALL RESULTS
    Serial.printf("FORCING servo attachment on pin %d - IGNORING ALL CHECKS\n", servoPin);
    
    // Try all methods but ignore results completely
    servo.attach(servoPin);
    servo.attach(servoPin, 500, 2500);
    servo.attach(servoPin, 1000, 2000);
    servo.attach(servoPin, 544, 2400);  // Standard servo range
    
    // Final forced attach - we don't care about the result
    servo.attach(servoPin);
    
    Serial.printf("✓ SERVO FORCED ACTIVE - Commands will be sent regardless of attach status\n");
}

void ServoMotor::init(float initialAngle) {
    // FORCE servo operation - no checks whatsoever
    setAngle(initialAngle);
    Serial.println("Servo FORCED Initialized at: " + String(initialAngle) + " degrees");
}

void ServoMotor::setAngle(float angle) {
    if (angle < 0.0f) angle = 0.0f;
    if (angle > 180.0f) angle = 180.0f;
    
    // FORCE WRITE - No attach checks, no verification, just send the command
    servo.write((int)angle);
    currentAngle = angle;
    Serial.printf("FORCED Servo command sent: %d degrees (attach status ignored)\n", (int)angle);
}

float ServoMotor::getCurrentAngle() {
    return currentAngle;
}

bool ServoMotor::isAttached() {
    return attached;
}

void ServoMotor::detach() {
    servo.detach();
    attached = false;
}

void ServoMotor::reattach() {
    servo.attach(servoPin);
    attached = true;
}

bool ServoMotor::isValidAngle(float angle) {
    return (angle >= 0.0f && angle <= 180.0f);
}

float ServoMotor::constrainAngle(float angle) {
    if (angle < 0.0f) return 0.0f;
    if (angle > 180.0f) return 180.0f;
    return angle;
} 