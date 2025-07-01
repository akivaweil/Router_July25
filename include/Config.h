//* ************************************************************************
//* ************************ ROUTER CONFIGURATION ***********************
//* ************************************************************************
//! Barebones ESP32 Router Control System Configuration
//! Contains only essential timing settings

#ifndef CONFIG_H
#define CONFIG_H

//* ************************************************************************
//* ************************ TIMING CONFIGURATION ***********************
//* ************************************************************************

// Feeding state timing
extern const unsigned long FEEDING_START_DELAY;         // 50ms start delay before feeding
extern const unsigned long FEED_CYLINDER_EXTEND_TIME;   // 2 seconds feed cylinder extended (pushing wood)
extern const unsigned long FEED_CYLINDER_RETRACT_TIME;  // 50ms retract time to feed wood

// Flipping state timing
extern const unsigned long FLIP_SERVO_MOVE_DELAY;       // 200ms wait for servo to reach position

// Manual start button debounce
extern const unsigned long MANUAL_START_DEBOUNCE;       // 30ms debounce for manual start button

//* ************************************************************************
//* ************************ SERVO CONFIGURATION ***********************
//* ************************************************************************

// Servo settings
extern const int FLIP_SERVO_ZERO_POSITION;              // 0 degrees servo position
extern const int FLIP_SERVO_FLIP_POSITION;              // 100 degrees servo position for flipping

#endif // CONFIG_H 