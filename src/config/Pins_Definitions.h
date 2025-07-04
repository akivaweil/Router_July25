#pragma once

// ************************************************************************
// *********************** PIN DEFINITIONS ********************************
// ************************************************************************

//! *************************** INPUT PINS *******************************
const int START_SENSOR_PIN = 48;     // Button or sensor to start the cycle
const int MANUAL_START_PIN = 19;     // Manual start button

//! *************************** OUTPUT PINS ******************************
const int FEED_CYLINDER_PIN = 41;    // Controls the feeding cylinder

//! ************************** STEPPER PINS ******************************
const int FLIP_STEPPER_STEP_PIN = 17; // Step pin for the flipping stepper
const int FLIP_STEPPER_DIR_PIN = 16;  // Direction pin for the flipping stepper 