#pragma once

// ************************************************************************
// ********************* CONFIGURATION VALUES *****************************
// ************************************************************************

//! ************************* TIMING SETTINGS ****************************
// All times are in milliseconds
const float FEEDING_START_DELAY = 600.0f;        // Wait before starting to feed
const float FEED_TIME = 2200.0f;                 // How long to push wood through router
const float STEPPER_MOVE_DELAY = 1000.0f;        // Time to wait after telling stepper to move

//! *********************** STEPPER SETTINGS *****************************
const float STEPS_PER_REVOLUTION = 3200.0f; // Steps for one full revolution
const float FLIP_DEGREES = 90.0f;           // Degrees to turn for the flip. Was 88, 90 is cleaner.
const float STEPS_FOR_FLIP = (STEPS_PER_REVOLUTION / 360.0f) * FLIP_DEGREES;

// Stepper motor speed and acceleration settings
const float STEPPER_MAX_SPEED = 1000.0f;      // Maximum speed in steps per second
const float STEPPER_ACCELERATION = 500.0f;  // Acceleration in steps per second per second 