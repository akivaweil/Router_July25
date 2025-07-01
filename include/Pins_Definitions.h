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
extern const int START_SENSOR_PIN;        // Start command sensor from stage 2 (Active HIGH)

// Feed cylinder control (inverted logic - LOW extends, HIGH retracts)
extern const int FEED_CYLINDER_PIN;      // Feed cylinder solenoid control

// Rotation servo control
extern const int FLIP_SERVO_PIN;         // Rotation servo PWM signal pin

//* ************************************************************************
//* ************************ STEPPER MOTOR PINS **************************
//* ************************************************************************

// Primary stepper motor (typically NEMA 23)
extern const int MOTOR_1_STEP_PIN;
extern const int MOTOR_1_DIR_PIN;
extern const int MOTOR_1_ENABLE_PIN;

// Secondary stepper motor (if needed)
extern const int MOTOR_2_STEP_PIN;
extern const int MOTOR_2_DIR_PIN;
extern const int MOTOR_2_ENABLE_PIN;

// Stepper motor common settings
extern const int MOTOR_STEPS_PER_REV;   // Standard NEMA 23 steps per revolution
extern const int MOTOR_MICROSTEPS;       // Microstepping setting

//* ************************************************************************
//* ************************ PNEUMATIC CONTROL PINS *********************
//* ************************************************************************

// Additional solenoid valves (5V relays controlling pneumatic solenoids)
extern const int SOLENOID_1_PIN;         // First pneumatic cylinder control
extern const int SOLENOID_2_PIN;         // Second pneumatic cylinder control
extern const int SOLENOID_3_PIN;         // Third pneumatic cylinder control
extern const int SOLENOID_4_PIN;         // Fourth pneumatic cylinder control

// Relay control pins
extern const int RELAY_1_PIN;            // General purpose relay 1
extern const int RELAY_2_PIN;            // General purpose relay 2
extern const int RELAY_3_PIN;            // General purpose relay 3
extern const int RELAY_4_PIN;            // General purpose relay 4

//* ************************************************************************
//* ************************ SENSOR INPUT PINS ***************************
//* ************************************************************************

// Limit switches and position sensors (Active HIGH with pulldown)
extern const int LIMIT_SWITCH_1_PIN;     // First limit switch
extern const int LIMIT_SWITCH_2_PIN;     // Second limit switch
extern const int LIMIT_SWITCH_3_PIN;     // Third limit switch
extern const int LIMIT_SWITCH_4_PIN;     // Fourth limit switch

// Proximity sensors (Active LOW with pullup)
extern const int PROXIMITY_SENSOR_1_PIN; // First proximity sensor
extern const int PROXIMITY_SENSOR_2_PIN; // Second proximity sensor (VP)
extern const int PROXIMITY_SENSOR_3_PIN; // Third proximity sensor (VN)
extern const int PROXIMITY_SENSOR_4_PIN; // Fourth proximity sensor

// Cylinder position sensors
extern const int CYLINDER_1_EXTENDED_PIN;    // Cylinder 1 extended position sensor
extern const int CYLINDER_1_RETRACTED_PIN;   // Cylinder 1 retracted position sensor
extern const int CYLINDER_2_EXTENDED_PIN;   // Cylinder 2 extended position sensor
extern const int CYLINDER_2_RETRACTED_PIN;  // Cylinder 2 retracted position sensor

//* ************************************************************************
//* ************************ CONTROL INPUT PINS **************************
//* ************************************************************************

// Manual control switches (Active HIGH with pulldown)
extern const int START_BUTTON_PIN;        // System start button
extern const int STOP_BUTTON_PIN;         // System stop button
extern const int EMERGENCY_STOP_PIN;     // Emergency stop button
extern const int RESET_BUTTON_PIN;        // System reset button

// Mode selection switches
extern const int AUTO_MODE_PIN;          // Automatic mode selection
extern const int MANUAL_MODE_PIN;         // Manual mode selection
extern const int SETUP_MODE_PIN;          // Setup mode selection

//* ************************************************************************
//* ************************ COMMUNICATION PINS **************************
//* ************************************************************************

// Serial communication (predefined ESP32 pins)
extern const int SERIAL_TX_PIN;           // UART TX pin
extern const int SERIAL_RX_PIN;           // UART RX pin

// I2C communication (if needed for sensors/displays)
extern const int I2C_SDA_PIN;            // I2C data line (shared with feed cylinder)
extern const int I2C_SCL_PIN;            // I2C clock line

// SPI communication (if needed for external devices)
extern const int SPI_MOSI_PIN;           // SPI Master Out Slave In
extern const int SPI_MISO_PIN;           // SPI Master In Slave Out
extern const int SPI_SCK_PIN;            // SPI Serial Clock
extern const int SPI_SS_PIN;              // SPI Slave Select

//* ************************************************************************
//* ************************ STATUS INDICATOR PINS **********************
//* ************************************************************************

// LED indicators
extern const int STATUS_LED_PIN;          // General status LED (shared with start sensor)
extern const int ERROR_LED_PIN;           // Error indicator LED
extern const int READY_LED_PIN;          // System ready LED
extern const int RUNNING_LED_PIN;        // System running LED

// Buzzer/Alarm
extern const int BUZZER_PIN;             // Audible alarm/notification

//* ************************************************************************
//* ************************ SPARE/EXPANSION PINS ***********************
//* ************************************************************************

// Reserved pins for future expansion
extern const int SPARE_DIGITAL_1_PIN;
extern const int SPARE_DIGITAL_2_PIN;
extern const int SPARE_ANALOG_1_PIN;     // Additional analog input
extern const int SPARE_ANALOG_2_PIN;     // Additional analog input

//* ************************************************************************
//* ************************ PIN CONFIGURATION FUNCTION PROTOTYPES ******
//* ************************************************************************

// Input configuration functions
void configureInputPullup(int pin);
void configureInputPulldown(int pin);
void configureOutput(int pin);

// Digital read/write functions
bool readPin(int pin);
void writePinHigh(int pin);
void writePinLow(int pin);
void togglePin(int pin);

// Router specific control functions
// Note: extendFeedCylinder() and retractFeedCylinder() are declared in FeedCylinder.h

#endif // PINS_DEFINITIONS_H 