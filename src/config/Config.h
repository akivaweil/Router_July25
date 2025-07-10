#pragma once

// ************************************************************************
// ********************* CONFIGURATION VALUES *****************************
// ************************************************************************

//! ************************* TIMING SETTINGS ****************************
// All times are in milliseconds
const float FEEDING_START_DELAY = 600.0f;        // Wait before starting to feed
const float FEED_TIME = 2200.0f;                 // How long to push wood through router
const float SERVO_MOVE_DELAY = 500.0f;        // Time to wait after telling servo to move

//! *********************** SERVO SETTINGS *****************************
const float SERVO_HOME_ANGLE = 0.0f;        // Angle for the servo's home position
const float FLIP_ANGLE = 120.0f;            // Angle for the servo to flip the material
const float SERVO_TEST_START_ANGLE = 0.0f;  // Start angle for the startup test
const float SERVO_TEST_END_ANGLE = 90.0f;   // End angle for the startup test

//! *********************** STEPPER SETTINGS (OLD) *****************************
// const float STEPS_PER_REVOLUTION = 3200.0f; // Steps for one full revolution
// const float FLIP_DEGREES = 120.0f;           // Degrees to turn for the flip.
// const float STEPS_FOR_FLIP = (STEPS_PER_REVOLUTION / 360.0f) * FLIP_DEGREES;

// Stepper motor speed and acceleration settings
// const float HOMING_SPEED = 2000.0f;           // Homing speed in steps per second
// const float HOMING_DEGREES = -150.0f;         // Degrees to move for homing
// const float STEPPER_MAX_SPEED = 5000.0f;      // Maximum speed in steps per second
// const float STEPPER_ACCELERATION = 5000.0f;  // Acceleration in steps per second per second 