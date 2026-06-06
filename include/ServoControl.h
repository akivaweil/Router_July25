#pragma once

#include <Arduino.h>

// Servo control class
// Custom servo control class for ESP32 using LEDC (LED Control) PWM.
// Provides precise servo control with configurable parameters.
// Supports both angle-based and microsecond-based positioning.

class ServoControl {
private:
    // Private member variables
    int pin;                    // GPIO pin connected to servo
    int channel;                // LEDC channel for PWM generation
    int frequency;              // PWM frequency in Hz
    int resolution;             // PWM resolution in bits
    int minPulseWidth;          // Minimum pulse width in microseconds
    int maxPulseWidth;          // Maximum pulse width in microseconds
    int minAngle;               // Minimum servo angle in degrees
    int maxAngle;               // Maximum servo angle in degrees

    // Private methods
    int angleToDuty(float angle);  // Convert angle to PWM duty cycle

public:
    // Public member variables
    float targetAngle;          // Target angle for servo positioning
    float currentAngle;         // Last commanded angle (start of next move)
    unsigned long lastUpdateTime; // Timestamp of last servo update
    unsigned long moveDurationMs; // Computed duration for the current move

    // Constructor
    ServoControl();

    // Initialization
    void init(int servoPin, int pwmChannel = 7, int freq = 50, int res = 14);

    // Servo control methods
    void write(float angle);                    // Set servo to specific angle
    void writeMicroseconds(int microseconds);   // Set servo pulse width directly
    void detach();                              // Detach servo from pin

    // Configuration methods
    void setPulseWidthRange(int minUs, int maxUs);  // Set pulse width range
    void setAngleRange(int minDeg, int maxDeg);     // Set angle range

    // Status methods
    bool hasReachedTarget();                    // Check if servo reached target
};
