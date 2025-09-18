#pragma once

//* ************************************************************************
//* ************************* MACHINE PARAMETERS ***************************
//* ************************************************************************

//! ********************** TIMING PARAMETERS (milliseconds) ****************
const float FEEDING_START_DELAY_1 = 400.0f;  // Delay after start signal before first feeding begins
const float FEEDING_START_DELAY_2 = 200.0f;  // Delay after start signal before second feeding begins
const float FEED_TIME             = 2500.0f; // Duration the feed cylinder is active
const float SERVO_MOVE_DELAY      = 1000.0f; // Time allotted for the servo to complete its movement

//! ********************** SERVO ANGLES (degrees) ***************************
extern float SERVO_HOME_ANGLE;  // The safe/home position for the servo (modifiable via dashboard)
const float FLIP_ANGLE             = 0.0f;   // The target angle to flip the material
const float SERVO_TEST_START_ANGLE = 150.0f; // Start angle for the power-on test sequence
const float SERVO_TEST_END_ANGLE   = 100.0f; // End angle for the power-on test sequence
