//* ************************************************************************
//* ************************ PIN DEFINITIONS *****************************
//* ************************************************************************
//! Barebones ESP32 Router Control System Pin Definitions
//! Contains only essential pins for router operation

#include "../../include/Pins_Definitions.h"
#include <Arduino.h>

//* ************************************************************************
//* ************************ ESSENTIAL PINS *******************************
//* ************************************************************************

// Start signals
const int START_SENSOR_PIN = 2;          // Start signal from stage 2 machine (Active HIGH)
const int MANUAL_START_PIN = 25;         // Manual start button (Active HIGH)

// Feed cylinder control (LOW extends = safe default, HIGH retracts = cutting cycle)
const int FEED_CYLINDER_PIN = 21;        // Feed cylinder solenoid control

// Servo control
const int FLIP_SERVO_PIN = 33;           // Flip servo PWM signal pin

//* ************************************************************************
//* ************************ PIN CONFIGURATION FUNCTIONS ****************
//* ************************************************************************

// Pin configuration functions
void configureInputPulldown(int pin) {
    pinMode(pin, INPUT_PULLDOWN);
}

void configureOutput(int pin) {
    pinMode(pin, OUTPUT);
}

// Feed cylinder control functions
void retractFeedCylinder() {
    digitalWrite(FEED_CYLINDER_PIN, HIGH);  // HIGH retracts the cylinder (cutting cycle position)
}

void extendFeedCylinder() {
    digitalWrite(FEED_CYLINDER_PIN, LOW);   // LOW extends the cylinder (safe default position)
} 