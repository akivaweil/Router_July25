//* ************************************************************************
//* ************************ PROJECT CONFIGURATION *********************
//* ************************************************************************
//! Main configuration file for ESP32 Router Control System
//! Contains all system-wide configuration parameters

#ifndef CONFIG_H
#define CONFIG_H

//* ************************************************************************
//* ************************ SYSTEM SETTINGS ****************************
//* ************************************************************************

// System identification
inline const char* SYSTEM_NAME = "ESP32_Router_Control";
inline const char* VERSION = "1.0.0";

// Serial communication settings
inline const unsigned long SERIAL_BAUD_RATE = 115200;
inline const unsigned long SERIAL_TIMEOUT = 2000;

//* ************************************************************************
//* ************************ NETWORK CONFIGURATION **********************
//* ************************************************************************

// WiFi credentials
inline const char* WIFI_SSID = "Everwood";
inline const char* WIFI_PASSWORD = "Everwood-Staff";
inline const unsigned long WIFI_CONNECTION_TIMEOUT = 20000;  // 20 seconds
inline const unsigned long WIFI_RECONNECT_DELAY = 5000;      // 5 seconds

//* ************************************************************************
//* ************************ TIMING CONFIGURATION ***********************
//* ************************************************************************

// Main loop timing
inline const unsigned long MAIN_LOOP_DELAY = 100;            // 100ms main loop cycle
inline const unsigned long STATUS_UPDATE_INTERVAL = 5000;    // 5 seconds status updates
inline const unsigned long OTA_STATUS_INTERVAL = 30000;      // 30 seconds OTA status display

// State machine timing
inline const unsigned long STATE_TRANSITION_DELAY = 50;      // 50ms between state transitions
inline const unsigned long DEBOUNCE_DELAY = 50;              // 50ms for input debouncing

//* ************************************************************************
//* ************************ ROUTER CUTTING CYCLE TIMING ****************
//* ************************************************************************

// Feeding state timing
inline const unsigned long FEEDING_START_DELAY = 50;         // 50ms start delay before feeding
inline const unsigned long FEED_CYLINDER_EXTEND_TIME = 2000; // 2 seconds feed cylinder extended (pushing wood)
inline const unsigned long FEED_CYLINDER_RETRACT_TIME = 50;  // 50ms retract time to move out of way

// Flipping state timing
inline const unsigned long FLIP_SERVO_MOVE_DELAY = 200;      // 200ms wait for servo to reach position
inline const unsigned long FLIP_SERVO_RETURN_DELAY = 0;      // Immediate return to position zero

//* ************************************************************************
//* ************************ HARDWARE CONFIGURATION *********************
//* ************************************************************************

// Motor settings
inline const unsigned long MOTOR_STEP_DELAY = 1000;          // Microseconds between steps
inline const float MOTOR_ACCELERATION = 500.0;               // Steps per second squared
inline const float MOTOR_MAX_SPEED = 1000.0;                 // Steps per second

// Pneumatic timing
inline const unsigned long CYLINDER_EXTEND_DELAY = 500;      // 500ms for cylinder extension
inline const unsigned long CYLINDER_RETRACT_DELAY = 500;     // 500ms for cylinder retraction
inline const unsigned long SOLENOID_ACTIVATION_TIME = 100;   // 100ms solenoid activation time

// Feed cylinder specific settings (inverted logic)
inline const int FEED_CYLINDER_EXTEND_SIGNAL = LOW;   // LOW signal extends feed cylinder
inline const int FEED_CYLINDER_RETRACT_SIGNAL = HIGH; // HIGH signal retracts feed cylinder

// Servo settings
inline const int FLIP_SERVO_ZERO_POSITION = 0;     // 0 degrees servo position
inline const int FLIP_SERVO_PWM_FREQ = 50;         // 50Hz PWM frequency for servo

// Sensor settings
inline const unsigned long SENSOR_READ_INTERVAL = 10;        // 10ms sensor reading interval
inline const int SENSOR_STABLE_COUNT = 5;          // Number of stable readings required

//* ************************************************************************
//* ************************ STATE MACHINE CONFIGURATION ****************
//* ************************************************************************

// State machine settings
#define MAX_STATE_HISTORY 10           // Maximum state history to maintain (must be #define for array size)
inline const unsigned long STATE_TIMEOUT = 30000;            // 30 seconds state timeout
inline const int EMERGENCY_STOP_PRIORITY = 1;      // Highest priority for emergency stop
inline const unsigned long HEALTH_CHECK_INTERVAL = 5000;     // Health check every 5 seconds
inline const unsigned long SENSOR_CHECK_INTERVAL = 100;      // Sensor check every 100ms

//* ************************************************************************
//* ************************ SAFETY CONFIGURATION ***********************
//* ************************************************************************

// Safety timeouts
inline const unsigned long SAFETY_TIMEOUT = 60000;           // 60 seconds general safety timeout
inline const unsigned long MOTOR_TIMEOUT = 10000;            // 10 seconds motor operation timeout
inline const unsigned long PNEUMATIC_TIMEOUT = 5000;         // 5 seconds pneumatic operation timeout

// Safety limits
inline const int MAX_CONSECUTIVE_ERRORS = 5;       // Maximum consecutive errors before shutdown
inline const unsigned long WATCHDOG_TIMEOUT = 8000;          // 8 seconds watchdog timeout

#endif // CONFIG_H 