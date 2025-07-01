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
#define SYSTEM_NAME "ESP32_Router_Control"
#define VERSION "1.0.0"

// Serial communication settings
#define SERIAL_BAUD_RATE 115200
#define SERIAL_TIMEOUT 2000

//* ************************************************************************
//* ************************ NETWORK CONFIGURATION **********************
//* ************************************************************************

// WiFi credentials
#define WIFI_SSID "Everwood"
#define WIFI_PASSWORD "Everwood-Staff"
#define WIFI_CONNECTION_TIMEOUT 20000  // 20 seconds
#define WIFI_RECONNECT_DELAY 5000      // 5 seconds

//* ************************************************************************
//* ************************ TIMING CONFIGURATION ***********************
//* ************************************************************************

// Main loop timing
#define MAIN_LOOP_DELAY 100            // 100ms main loop cycle
#define STATUS_UPDATE_INTERVAL 5000    // 5 seconds status updates
#define OTA_STATUS_INTERVAL 30000      // 30 seconds OTA status display

// State machine timing
#define STATE_TRANSITION_DELAY 50      // 50ms between state transitions
#define DEBOUNCE_DELAY 50              // 50ms for input debouncing

//* ************************************************************************
//* ************************ ROUTER CUTTING CYCLE TIMING ****************
//* ************************************************************************

// Feeding state timing
#define FEEDING_START_DELAY 50         // 50ms start delay before feeding
#define FEED_CYLINDER_EXTEND_TIME 2000 // 2 seconds feed cylinder extended (pushing wood)
#define FEED_CYLINDER_RETRACT_TIME 50  // 50ms retract time to move out of way

// Flipping state timing
#define FLIP_SERVO_MOVE_DELAY 200      // 200ms wait for servo to reach position
#define FLIP_SERVO_RETURN_DELAY 0      // Immediate return to position zero

//* ************************************************************************
//* ************************ HARDWARE CONFIGURATION *********************
//* ************************************************************************

// Motor settings
#define MOTOR_STEP_DELAY 1000          // Microseconds between steps
#define MOTOR_ACCELERATION 500.0       // Steps per second squared
#define MOTOR_MAX_SPEED 1000.0         // Steps per second

// Pneumatic timing
#define CYLINDER_EXTEND_DELAY 500      // 500ms for cylinder extension
#define CYLINDER_RETRACT_DELAY 500     // 500ms for cylinder retraction
#define SOLENOID_ACTIVATION_TIME 100   // 100ms solenoid activation time

// Feed cylinder specific settings (inverted logic)
#define FEED_CYLINDER_EXTEND_SIGNAL LOW   // LOW signal extends feed cylinder
#define FEED_CYLINDER_RETRACT_SIGNAL HIGH // HIGH signal retracts feed cylinder

// Servo settings
#define FLIP_SERVO_ZERO_POSITION 0     // 0 degrees servo position
#define FLIP_SERVO_PWM_FREQ 50         // 50Hz PWM frequency for servo

// Sensor settings
#define SENSOR_READ_INTERVAL 10        // 10ms sensor reading interval
#define SENSOR_STABLE_COUNT 5          // Number of stable readings required

//* ************************************************************************
//* ************************ STATE MACHINE CONFIGURATION ****************
//* ************************************************************************

// State machine settings
#define MAX_STATE_HISTORY 10           // Maximum state history to maintain
#define STATE_TIMEOUT 30000            // 30 seconds state timeout
#define EMERGENCY_STOP_PRIORITY 1      // Highest priority for emergency stop

//* ************************************************************************
//* ************************ SAFETY CONFIGURATION ***********************
//* ************************************************************************

// Safety timeouts
#define SAFETY_TIMEOUT 60000           // 60 seconds general safety timeout
#define MOTOR_TIMEOUT 10000            // 10 seconds motor operation timeout
#define PNEUMATIC_TIMEOUT 5000         // 5 seconds pneumatic operation timeout

// Safety limits
#define MAX_CONSECUTIVE_ERRORS 5       // Maximum consecutive errors before shutdown
#define WATCHDOG_TIMEOUT 8000          // 8 seconds watchdog timeout

#endif // CONFIG_H 