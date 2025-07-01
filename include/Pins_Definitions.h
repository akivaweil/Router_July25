//* ************************************************************************
//* ************************ PIN DEFINITIONS *****************************
//* ************************************************************************
//! Barebones ESP32 Router Control System Pin Definitions
//! Contains only essential pins for router operation

#ifndef PINS_DEFINITIONS_H
#define PINS_DEFINITIONS_H

//* ************************************************************************
//* ************************ ESSENTIAL PINS *******************************
//* ************************************************************************

// Start signals
extern const int START_SENSOR_PIN;          // Start signal from stage 2 machine (Active HIGH)
extern const int MANUAL_START_PIN;          // Manual start button (Active HIGH)

// Feed cylinder control (LOW extends = safe default, HIGH retracts = cutting cycle)
extern const int FEED_CYLINDER_PIN;         // Feed cylinder solenoid control

// Servo control
extern const int FLIP_SERVO_PIN;            // Flip servo PWM signal pin

//* ************************************************************************
//* ************************ PIN CONFIGURATION FUNCTION PROTOTYPES ******
//* ************************************************************************

// Pin configuration functions
void configureInputPulldown(int pin);
void configureOutput(int pin);

// Feed cylinder control functions
void retractFeedCylinder();
void extendFeedCylinder();

#endif // PINS_DEFINITIONS_H 