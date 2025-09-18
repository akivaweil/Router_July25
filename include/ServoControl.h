#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <Arduino.h>

//* ************************************************************************
//* ********************** SERVO CONTROL CLASS *****************************
//* ************************************************************************
//! Custom servo control class for ESP32 using LEDC (LED Control) PWM
//! Provides precise servo control with configurable parameters
//! Supports both angle-based and microsecond-based positioning

class ServoControl {
private:
    //! ********************** PRIVATE MEMBER VARIABLES *********************
    int pin;                    // GPIO pin connected to servo
    int channel;                // LEDC channel for PWM generation
    int frequency;              // PWM frequency in Hz
    int resolution;             // PWM resolution in bits
    int minPulseWidth;          // Minimum pulse width in microseconds
    int maxPulseWidth;          // Maximum pulse width in microseconds
    int minAngle;               // Minimum servo angle in degrees
    int maxAngle;               // Maximum servo angle in degrees
    
    //! ********************** PRIVATE METHODS ******************************
    int angleToDuty(float angle);  // Convert angle to PWM duty cycle
    
public:
    //! ********************** PUBLIC MEMBER VARIABLES *********************
    float targetAngle;          // Target angle for servo positioning
    unsigned long lastUpdateTime; // Timestamp of last servo update

    //! ********************** CONSTRUCTOR **********************************
    ServoControl();
    
    //! ********************** INITIALIZATION *******************************
    void init(int servoPin, int pwmChannel = 7, int freq = 50, int res = 14);
    
    //! ********************** SERVO CONTROL METHODS ************************
    void write(float angle);                    // Set servo to specific angle
    void writeMicroseconds(int microseconds);   // Set servo pulse width directly
    void detach();                              // Detach servo from pin
    
    //! ********************** CONFIGURATION METHODS ************************
    void setPulseWidthRange(int minUs, int maxUs);  // Set pulse width range
    void setAngleRange(int minDeg, int maxDeg);     // Set angle range
    
    //! ********************** STATUS METHODS *******************************
    bool hasReachedTarget();                    // Check if servo reached target
};

#endif // SERVO_CONTROL_H 