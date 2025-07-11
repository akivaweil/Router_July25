#pragma once

// ************************************************************************
// ************************* MACHINE PARAMETERS ***************************
// ************************************************************************


// --- TIMING (milliseconds) ---
// Purpose: Defines wait times and delays for machine operations.
// ------------------------------------------------------------------------
const float FEEDING_START_DELAY = 600.0f;     // Delay after start signal before feeding begins.
const float FEED_TIME             = 2200.0f;    // Duration the feed cylinder is active.
const float SERVO_MOVE_DELAY      = 1000.0f;    // Time allotted for the servo to complete its movement.


// --- SERVO ANGLES (degrees) ---
// Purpose: Defines the angular positions for the flip servo.
// ------------------------------------------------------------------------
const float SERVO_HOME_ANGLE       = 123.0f;   // The safe/home position for the servo.
const float FLIP_ANGLE             = 20.0f;    // The target angle to flip the material.
const float SERVO_TEST_START_ANGLE = 150.0f;   // Start angle for the power-on test sequence.
const float SERVO_TEST_END_ANGLE   = 100.0f;     // End angle for the power-on test sequence. 