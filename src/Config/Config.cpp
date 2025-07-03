//* ************************************************************************
//* ************************ ROUTER CONFIGURATION ***********************
//* ************************************************************************
//! Barebones ESP32 Router Control System Configuration
//! Contains only essential timing settings

#include "../../include/Config.h"
#include <Arduino.h>

//* ************************************************************************
//* ************************ WIFI CONFIGURATION ***************************
//* ************************************************************************

// WiFi network settings
const char* WIFI_SSID = "Everwood";
const char* WIFI_PASSWORD = "Everwood-Staff";

//* ************************************************************************
//* ************************ TIMING CONFIGURATION ***********************
//* ************************************************************************

// Feeding state timing
const unsigned long FEEDING_START_DELAY = 600;         // 400ms start delay before feeding
const unsigned long FEED_CYLINDER_EXTEND_TIME = 2200; // 2.2 seconds feed cylinder extended (pushing wood)
const unsigned long FEED_CYLINDER_RETRACT_TIME = 10;  // 10ms retract time to feed wood

// Flipping state timing
const unsigned long FLIP_SERVO_MOVE_DELAY = 1200;      // 1500ms wait for servo to reach position

// Manual start button debounce
const unsigned long MANUAL_START_DEBOUNCE = 30;       // 30ms debounce for manual start button

// Servo settings
const int FLIP_SERVO_HOME_POSITION = 0;               // 0 degrees servo position
const int FLIP_SERVO_FLIP_POSITION = 100;             // 100 degrees servo position for flipping 