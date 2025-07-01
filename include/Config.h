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
extern const char* SYSTEM_NAME;
extern const char* VERSION;

// Serial communication settings
extern const unsigned long SERIAL_BAUD_RATE;
extern const unsigned long SERIAL_TIMEOUT;

//* ************************************************************************
//* ************************ NETWORK CONFIGURATION **********************
//* ************************************************************************

// WiFi credentials
extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;
extern const unsigned long WIFI_CONNECTION_TIMEOUT;  // 20 seconds
extern const unsigned long WIFI_RECONNECT_DELAY;      // 5 seconds

//* ************************************************************************
//* ************************ TIMING CONFIGURATION ***********************
//* ************************************************************************

// Main loop timing
extern const unsigned long MAIN_LOOP_DELAY;            // 100ms main loop cycle
extern const unsigned long STATUS_UPDATE_INTERVAL;    // 5 seconds status updates
extern const unsigned long OTA_STATUS_INTERVAL;      // 30 seconds OTA status display

// State machine timing
extern const unsigned long STATE_TRANSITION_DELAY;      // 50ms between state transitions
extern const unsigned long DEBOUNCE_DELAY;              // 50ms for input debouncing

//* ************************************************************************
//* ************************ ROUTER CUTTING CYCLE TIMING ****************
//* ************************************************************************

// Feeding state timing
extern const unsigned long FEEDING_START_DELAY;         // 50ms start delay before feeding
extern const unsigned long FEED_CYLINDER_EXTEND_TIME; // 2 seconds feed cylinder extended (pushing wood)
extern const unsigned long FEED_CYLINDER_RETRACT_TIME;  // 50ms retract time to move out of way

// Flipping state timing
extern const unsigned long FLIP_SERVO_MOVE_DELAY;      // 200ms wait for servo to reach position
extern const unsigned long FLIP_SERVO_RETURN_DELAY;      // Immediate return to position zero

//* ************************************************************************
//* ************************ HARDWARE CONFIGURATION *********************
//* ************************************************************************

// Motor settings
extern const unsigned long MOTOR_STEP_DELAY;          // Microseconds between steps
extern const float MOTOR_ACCELERATION;               // Steps per second squared
extern const float MOTOR_MAX_SPEED;                 // Steps per second

// Pneumatic timing
extern const unsigned long CYLINDER_EXTEND_DELAY;      // 500ms for cylinder extension
extern const unsigned long CYLINDER_RETRACT_DELAY;     // 500ms for cylinder retraction
extern const unsigned long SOLENOID_ACTIVATION_TIME;   // 100ms solenoid activation time

// Feed cylinder specific settings (inverted logic)
extern const int FEED_CYLINDER_EXTEND_SIGNAL;   // LOW signal extends feed cylinder
extern const int FEED_CYLINDER_RETRACT_SIGNAL; // HIGH signal retracts feed cylinder

// Servo settings
extern const int FLIP_SERVO_ZERO_POSITION;     // 0 degrees servo position
extern const int FLIP_SERVO_PWM_FREQ;         // 50Hz PWM frequency for servo

// Sensor settings
extern const unsigned long SENSOR_READ_INTERVAL;        // 10ms sensor reading interval
extern const int SENSOR_STABLE_COUNT;          // Number of stable readings required

//* ************************************************************************
//* ************************ STATE MACHINE CONFIGURATION ****************
//* ************************************************************************

// State machine settings
#define MAX_STATE_HISTORY 10           // Maximum state history to maintain (must be #define for array size)
extern const unsigned long STATE_TIMEOUT;            // 30 seconds state timeout
extern const int EMERGENCY_STOP_PRIORITY;      // Highest priority for emergency stop
extern const unsigned long HEALTH_CHECK_INTERVAL;     // Health check every 5 seconds
extern const unsigned long SENSOR_CHECK_INTERVAL;      // Sensor check every 100ms

//* ************************************************************************
//* ************************ SAFETY CONFIGURATION ***********************
//* ************************************************************************

// Safety timeouts
extern const unsigned long SAFETY_TIMEOUT;           // 60 seconds general safety timeout
extern const unsigned long MOTOR_TIMEOUT;            // 10 seconds motor operation timeout
extern const unsigned long PNEUMATIC_TIMEOUT;         // 5 seconds pneumatic operation timeout

// Safety limits
extern const int MAX_CONSECUTIVE_ERRORS;       // Maximum consecutive errors before shutdown
extern const unsigned long WATCHDOG_TIMEOUT;          // 8 seconds watchdog timeout

#endif // CONFIG_H 