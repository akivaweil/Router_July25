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

// Feed cylinder control (inverted logic - LOW extends, HIGH retracts)
#define FEED_CYLINDER_PIN           19     // Feed cylinder solenoid control

// Flip servo control
#define FLIP_SERVO_PIN              18     // Flip servo PWM signal pin

// Start sensor/trigger
#define START_SENSOR_PIN            32     // Start command sensor (Active HIGH)

//* ************************************************************************
//* ************************ STEPPER MOTOR PINS **************************
//* ************************************************************************

// Primary stepper motor (typically NEMA 23)
#define MOTOR_1_STEP_PIN            2
#define MOTOR_1_DIR_PIN             4
#define MOTOR_1_ENABLE_PIN          16

// Secondary stepper motor (if needed)
#define MOTOR_2_STEP_PIN            17
#define MOTOR_2_DIR_PIN             5
#define MOTOR_2_ENABLE_PIN          15

// Stepper motor common settings
#define MOTOR_STEPS_PER_REV         200    // Standard NEMA 23 steps per revolution
#define MOTOR_MICROSTEPS            16     // Microstepping setting

//* ************************************************************************
//* ************************ PNEUMATIC CONTROL PINS *********************
//* ************************************************************************

// Additional solenoid valves (5V relays controlling pneumatic solenoids)
#define SOLENOID_1_PIN              21     // First pneumatic cylinder control
#define SOLENOID_2_PIN              22     // Second pneumatic cylinder control
#define SOLENOID_3_PIN              23     // Third pneumatic cylinder control
#define SOLENOID_4_PIN              25     // Fourth pneumatic cylinder control

// Relay control pins
#define RELAY_1_PIN                 26     // General purpose relay 1
#define RELAY_2_PIN                 27     // General purpose relay 2
#define RELAY_3_PIN                 14     // General purpose relay 3
#define RELAY_4_PIN                 12     // General purpose relay 4

//* ************************************************************************
//* ************************ SENSOR INPUT PINS ***************************
//* ************************************************************************

// Limit switches and position sensors (Active HIGH with pulldown)
#define LIMIT_SWITCH_1_PIN          33     // First limit switch
#define LIMIT_SWITCH_2_PIN          34     // Second limit switch
#define LIMIT_SWITCH_3_PIN          35     // Third limit switch
#define LIMIT_SWITCH_4_PIN          36     // Fourth limit switch

// Proximity sensors (Active LOW with pullup)
#define PROXIMITY_SENSOR_1_PIN      39     // First proximity sensor
#define PROXIMITY_SENSOR_2_PIN      VP     // Second proximity sensor (GPIO36)
#define PROXIMITY_SENSOR_3_PIN      VN     // Third proximity sensor (GPIO39)
#define PROXIMITY_SENSOR_4_PIN      A0     // Fourth proximity sensor

// Cylinder position sensors
#define CYLINDER_1_EXTENDED_PIN     13     // Cylinder 1 extended position sensor
#define CYLINDER_1_RETRACTED_PIN    0      // Cylinder 1 retracted position sensor
#define CYLINDER_2_EXTENDED_PIN     A3     // Cylinder 2 extended position sensor
#define CYLINDER_2_RETRACTED_PIN    A6     // Cylinder 2 retracted position sensor

//* ************************************************************************
//* ************************ CONTROL INPUT PINS **************************
//* ************************************************************************

// Manual control switches (Active HIGH with pulldown)
#define START_BUTTON_PIN            1      // System start button
#define STOP_BUTTON_PIN             3      // System stop button
#define EMERGENCY_STOP_PIN          10     // Emergency stop button
#define RESET_BUTTON_PIN            9      // System reset button

// Mode selection switches
#define AUTO_MODE_PIN               11     // Automatic mode selection
#define MANUAL_MODE_PIN             6      // Manual mode selection
#define SETUP_MODE_PIN              7      // Setup mode selection

//* ************************************************************************
//* ************************ COMMUNICATION PINS **************************
//* ************************************************************************

// Serial communication (predefined ESP32 pins)
#define SERIAL_TX_PIN               1      // UART TX pin
#define SERIAL_RX_PIN               3      // UART RX pin

// I2C communication (if needed for sensors/displays)
#define I2C_SDA_PIN                 21     // I2C data line
#define I2C_SCL_PIN                 22     // I2C clock line

// SPI communication (if needed for external devices)
#define SPI_MOSI_PIN                23     // SPI Master Out Slave In
#define SPI_MISO_PIN                19     // SPI Master In Slave Out
#define SPI_SCK_PIN                 18     // SPI Serial Clock
#define SPI_SS_PIN                  5      // SPI Slave Select

//* ************************************************************************
//* ************************ STATUS INDICATOR PINS **********************
//* ************************************************************************

// LED indicators
#define STATUS_LED_PIN              2      // General status LED
#define ERROR_LED_PIN               4      // Error indicator LED
#define READY_LED_PIN               16     // System ready LED
#define RUNNING_LED_PIN             17     // System running LED

// Buzzer/Alarm
#define BUZZER_PIN                  25     // Audible alarm/notification

//* ************************************************************************
//* ************************ SPARE/EXPANSION PINS ***********************
//* ************************************************************************

// Reserved pins for future expansion
#define SPARE_DIGITAL_1_PIN         26
#define SPARE_DIGITAL_2_PIN         27
#define SPARE_ANALOG_1_PIN          A4     // Additional analog input
#define SPARE_ANALOG_2_PIN          A7     // Additional analog input

//* ************************************************************************
//* ************************ PIN CONFIGURATION MACROS *******************
//* ************************************************************************

// Input configuration macros
#define CONFIGURE_INPUT_PULLUP(pin)     pinMode(pin, INPUT_PULLUP)
#define CONFIGURE_INPUT_PULLDOWN(pin)   pinMode(pin, INPUT_PULLDOWN)
#define CONFIGURE_OUTPUT(pin)           pinMode(pin, OUTPUT)

// Digital read/write macros
#define READ_PIN(pin)                   digitalRead(pin)
#define WRITE_PIN_HIGH(pin)             digitalWrite(pin, HIGH)
#define WRITE_PIN_LOW(pin)              digitalWrite(pin, LOW)
#define TOGGLE_PIN(pin)                 digitalWrite(pin, !digitalRead(pin))

// Router specific control macros
#define EXTEND_FEED_CYLINDER()          digitalWrite(FEED_CYLINDER_PIN, FEED_CYLINDER_EXTEND_SIGNAL)
#define RETRACT_FEED_CYLINDER()         digitalWrite(FEED_CYLINDER_PIN, FEED_CYLINDER_RETRACT_SIGNAL)

#endif // PINS_DEFINITIONS_H 