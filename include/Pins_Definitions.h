//* ************************************************************************
//* ************************ PIN DEFINITIONS *****************************
//* ************************************************************************
//! Hardware pin assignments for ESP32 Router Control System
//! Organized by functional groups for easy maintenance

#ifndef PINS_DEFINITIONS_H
#define PINS_DEFINITIONS_H

//* ************************************************************************
//* ************************ ROUTER SPECIFIC PINS ************************
//* ************************************************************************

// Start signal from stage 2
inline const int START_SENSOR_PIN = 2;        // Start command sensor from stage 2 (Active HIGH)

// Feed cylinder control (inverted logic - LOW extends, HIGH retracts)
inline const int FEED_CYLINDER_PIN = 21;      // Feed cylinder solenoid control

// Rotation servo control
inline const int FLIP_SERVO_PIN = 14;         // Rotation servo PWM signal pin

//* ************************************************************************
//* ************************ STEPPER MOTOR PINS **************************
//* ************************************************************************

// Primary stepper motor (typically NEMA 23)
inline const int MOTOR_1_STEP_PIN = 4;
inline const int MOTOR_1_DIR_PIN = 5;
inline const int MOTOR_1_ENABLE_PIN = 16;

// Secondary stepper motor (if needed)
inline const int MOTOR_2_STEP_PIN = 17;
inline const int MOTOR_2_DIR_PIN = 18;
inline const int MOTOR_2_ENABLE_PIN = 15;

// Stepper motor common settings
inline const int MOTOR_STEPS_PER_REV = 200;   // Standard NEMA 23 steps per revolution
inline const int MOTOR_MICROSTEPS = 16;       // Microstepping setting

//* ************************************************************************
//* ************************ PNEUMATIC CONTROL PINS *********************
//* ************************************************************************

// Additional solenoid valves (5V relays controlling pneumatic solenoids)
inline const int SOLENOID_1_PIN = 22;         // First pneumatic cylinder control
inline const int SOLENOID_2_PIN = 23;         // Second pneumatic cylinder control
inline const int SOLENOID_3_PIN = 25;         // Third pneumatic cylinder control
inline const int SOLENOID_4_PIN = 26;         // Fourth pneumatic cylinder control

// Relay control pins
inline const int RELAY_1_PIN = 27;            // General purpose relay 1
inline const int RELAY_2_PIN = 12;            // General purpose relay 2
inline const int RELAY_3_PIN = 13;            // General purpose relay 3
inline const int RELAY_4_PIN = 32;            // General purpose relay 4

//* ************************************************************************
//* ************************ SENSOR INPUT PINS ***************************
//* ************************************************************************

// Limit switches and position sensors (Active HIGH with pulldown)
inline const int LIMIT_SWITCH_1_PIN = 33;     // First limit switch
inline const int LIMIT_SWITCH_2_PIN = 34;     // Second limit switch
inline const int LIMIT_SWITCH_3_PIN = 35;     // Third limit switch
inline const int LIMIT_SWITCH_4_PIN = 36;     // Fourth limit switch

// Proximity sensors (Active LOW with pullup)
inline const int PROXIMITY_SENSOR_1_PIN = 39; // First proximity sensor
inline const int PROXIMITY_SENSOR_2_PIN = 36; // Second proximity sensor (VP)
inline const int PROXIMITY_SENSOR_3_PIN = 39; // Third proximity sensor (VN)
inline const int PROXIMITY_SENSOR_4_PIN = A0; // Fourth proximity sensor

// Cylinder position sensors
inline const int CYLINDER_1_EXTENDED_PIN = 19;    // Cylinder 1 extended position sensor
inline const int CYLINDER_1_RETRACTED_PIN = 0;   // Cylinder 1 retracted position sensor
inline const int CYLINDER_2_EXTENDED_PIN = A3;   // Cylinder 2 extended position sensor
inline const int CYLINDER_2_RETRACTED_PIN = A6;  // Cylinder 2 retracted position sensor

//* ************************************************************************
//* ************************ CONTROL INPUT PINS **************************
//* ************************************************************************

// Manual control switches (Active HIGH with pulldown)
inline const int START_BUTTON_PIN = 1;        // System start button
inline const int STOP_BUTTON_PIN = 3;         // System stop button
inline const int EMERGENCY_STOP_PIN = 10;     // Emergency stop button
inline const int RESET_BUTTON_PIN = 9;        // System reset button

// Mode selection switches
inline const int AUTO_MODE_PIN = 11;          // Automatic mode selection
inline const int MANUAL_MODE_PIN = 6;         // Manual mode selection
inline const int SETUP_MODE_PIN = 7;          // Setup mode selection

//* ************************************************************************
//* ************************ COMMUNICATION PINS **************************
//* ************************************************************************

// Serial communication (predefined ESP32 pins)
inline const int SERIAL_TX_PIN = 1;           // UART TX pin
inline const int SERIAL_RX_PIN = 3;           // UART RX pin

// I2C communication (if needed for sensors/displays)
inline const int I2C_SDA_PIN = 21;            // I2C data line (shared with feed cylinder)
inline const int I2C_SCL_PIN = 22;            // I2C clock line

// SPI communication (if needed for external devices)
inline const int SPI_MOSI_PIN = 23;           // SPI Master Out Slave In
inline const int SPI_MISO_PIN = 19;           // SPI Master In Slave Out
inline const int SPI_SCK_PIN = 18;            // SPI Serial Clock
inline const int SPI_SS_PIN = 5;              // SPI Slave Select

//* ************************************************************************
//* ************************ STATUS INDICATOR PINS **********************
//* ************************************************************************

// LED indicators
inline const int STATUS_LED_PIN = 2;          // General status LED (shared with start sensor)
inline const int ERROR_LED_PIN = 4;           // Error indicator LED
inline const int READY_LED_PIN = 16;          // System ready LED
inline const int RUNNING_LED_PIN = 17;        // System running LED

// Buzzer/Alarm
inline const int BUZZER_PIN = 25;             // Audible alarm/notification

//* ************************************************************************
//* ************************ SPARE/EXPANSION PINS ***********************
//* ************************************************************************

// Reserved pins for future expansion
inline const int SPARE_DIGITAL_1_PIN = 26;
inline const int SPARE_DIGITAL_2_PIN = 27;
inline const int SPARE_ANALOG_1_PIN = A4;     // Additional analog input
inline const int SPARE_ANALOG_2_PIN = A7;     // Additional analog input

//* ************************************************************************
//* ************************ PIN CONFIGURATION FUNCTION PROTOTYPES ******
//* ************************************************************************

// Input configuration functions
inline void configureInputPullup(int pin) {
    pinMode(pin, INPUT_PULLUP);
}

inline void configureInputPulldown(int pin) {
    pinMode(pin, INPUT_PULLDOWN);
}

inline void configureOutput(int pin) {
    pinMode(pin, OUTPUT);
}

// Digital read/write functions
inline bool readPin(int pin) {
    return digitalRead(pin);
}

inline void writePinHigh(int pin) {
    digitalWrite(pin, HIGH);
}

inline void writePinLow(int pin) {
    digitalWrite(pin, LOW);
}

inline void togglePin(int pin) {
    digitalWrite(pin, !digitalRead(pin));
}

// Router specific control functions
// Note: extendFeedCylinder() and retractFeedCylinder() are declared in FeedCylinder.h

#endif // PINS_DEFINITIONS_H 