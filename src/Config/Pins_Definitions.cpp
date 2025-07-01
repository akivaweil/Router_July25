//* ************************************************************************
//* ************************ PIN DEFINITIONS *****************************
//* ************************************************************************
//! Hardware pin assignment implementation for ESP32 Router Control System
//! Contains all pin definitions and configuration functions

#include "../../include/Pins_Definitions.h"
#include <Arduino.h>

//* ************************************************************************
//* ************************ ROUTER SPECIFIC PINS ************************
//* ************************************************************************

// Start signal from stage 2
const int START_SENSOR_PIN = 2;        // Start command sensor from stage 2 (Active HIGH)

// Feed cylinder control (inverted logic - LOW extends, HIGH retracts)
const int FEED_CYLINDER_PIN = 21;      // Feed cylinder solenoid control

// Rotation servo control
const int FLIP_SERVO_PIN = 14;         // Rotation servo PWM signal pin

//* ************************************************************************
//* ************************ STEPPER MOTOR PINS **************************
//* ************************************************************************

// Primary stepper motor (typically NEMA 23)
const int MOTOR_1_STEP_PIN = 4;
const int MOTOR_1_DIR_PIN = 5;
const int MOTOR_1_ENABLE_PIN = 16;

// Secondary stepper motor (if needed)
const int MOTOR_2_STEP_PIN = 17;
const int MOTOR_2_DIR_PIN = 18;
const int MOTOR_2_ENABLE_PIN = 15;

// Stepper motor common settings
const int MOTOR_STEPS_PER_REV = 200;   // Standard NEMA 23 steps per revolution
const int MOTOR_MICROSTEPS = 16;       // Microstepping setting

//* ************************************************************************
//* ************************ PNEUMATIC CONTROL PINS *********************
//* ************************************************************************

// Additional solenoid valves (5V relays controlling pneumatic solenoids)
const int SOLENOID_1_PIN = 22;         // First pneumatic cylinder control
const int SOLENOID_2_PIN = 23;         // Second pneumatic cylinder control
const int SOLENOID_3_PIN = 25;         // Third pneumatic cylinder control
const int SOLENOID_4_PIN = 26;         // Fourth pneumatic cylinder control

// Relay control pins
const int RELAY_1_PIN = 27;            // General purpose relay 1
const int RELAY_2_PIN = 12;            // General purpose relay 2
const int RELAY_3_PIN = 13;            // General purpose relay 3
const int RELAY_4_PIN = 32;            // General purpose relay 4

//* ************************************************************************
//* ************************ SENSOR INPUT PINS ***************************
//* ************************************************************************

// Limit switches and position sensors (Active HIGH with pulldown)
const int LIMIT_SWITCH_1_PIN = 33;     // First limit switch
const int LIMIT_SWITCH_2_PIN = 34;     // Second limit switch
const int LIMIT_SWITCH_3_PIN = 35;     // Third limit switch
const int LIMIT_SWITCH_4_PIN = 36;     // Fourth limit switch

// Proximity sensors (Active LOW with pullup)
const int PROXIMITY_SENSOR_1_PIN = 39; // First proximity sensor
const int PROXIMITY_SENSOR_2_PIN = 36; // Second proximity sensor (VP)
const int PROXIMITY_SENSOR_3_PIN = 39; // Third proximity sensor (VN)
const int PROXIMITY_SENSOR_4_PIN = A0; // Fourth proximity sensor

// Cylinder position sensors
const int CYLINDER_1_EXTENDED_PIN = 19;    // Cylinder 1 extended position sensor
const int CYLINDER_1_RETRACTED_PIN = 0;   // Cylinder 1 retracted position sensor
const int CYLINDER_2_EXTENDED_PIN = A3;   // Cylinder 2 extended position sensor
const int CYLINDER_2_RETRACTED_PIN = A6;  // Cylinder 2 retracted position sensor

//* ************************************************************************
//* ************************ CONTROL INPUT PINS **************************
//* ************************************************************************

// Manual control switches (Active HIGH with pulldown)
const int START_BUTTON_PIN = 1;        // System start button
const int STOP_BUTTON_PIN = 3;         // System stop button
const int EMERGENCY_STOP_PIN = 10;     // Emergency stop button
const int RESET_BUTTON_PIN = 9;        // System reset button

// Mode selection switches
const int AUTO_MODE_PIN = 11;          // Automatic mode selection
const int MANUAL_MODE_PIN = 6;         // Manual mode selection
const int SETUP_MODE_PIN = 7;          // Setup mode selection

//* ************************************************************************
//* ************************ COMMUNICATION PINS **************************
//* ************************************************************************

// Serial communication (predefined ESP32 pins)
const int SERIAL_TX_PIN = 1;           // UART TX pin
const int SERIAL_RX_PIN = 3;           // UART RX pin

// I2C communication (if needed for sensors/displays)
const int I2C_SDA_PIN = 21;            // I2C data line (shared with feed cylinder)
const int I2C_SCL_PIN = 22;            // I2C clock line

// SPI communication (if needed for external devices)
const int SPI_MOSI_PIN = 23;           // SPI Master Out Slave In
const int SPI_MISO_PIN = 19;           // SPI Master In Slave Out
const int SPI_SCK_PIN = 18;            // SPI Serial Clock
const int SPI_SS_PIN = 5;              // SPI Slave Select

//* ************************************************************************
//* ************************ STATUS INDICATOR PINS **********************
//* ************************************************************************

// LED indicators
const int STATUS_LED_PIN = 2;          // General status LED (shared with start sensor)
const int ERROR_LED_PIN = 4;           // Error indicator LED
const int READY_LED_PIN = 16;          // System ready LED
const int RUNNING_LED_PIN = 17;        // System running LED

// Buzzer/Alarm
const int BUZZER_PIN = 25;             // Audible alarm/notification

//* ************************************************************************
//* ************************ SPARE/EXPANSION PINS ***********************
//* ************************************************************************

// Reserved pins for future expansion
const int SPARE_DIGITAL_1_PIN = 26;
const int SPARE_DIGITAL_2_PIN = 27;
const int SPARE_ANALOG_1_PIN = A4;     // Additional analog input
const int SPARE_ANALOG_2_PIN = A7;     // Additional analog input

//* ************************************************************************
//* ************************ PIN CONFIGURATION FUNCTIONS ****************
//* ************************************************************************

// Input configuration functions
void configureInputPullup(int pin) {
    pinMode(pin, INPUT_PULLUP);
}

void configureInputPulldown(int pin) {
    pinMode(pin, INPUT_PULLDOWN);
}

void configureOutput(int pin) {
    pinMode(pin, OUTPUT);
}

// Digital read/write functions
bool readPin(int pin) {
    return digitalRead(pin);
}

void writePinHigh(int pin) {
    digitalWrite(pin, HIGH);
}

void writePinLow(int pin) {
    digitalWrite(pin, LOW);
}

void togglePin(int pin) {
    digitalWrite(pin, !digitalRead(pin));
} 