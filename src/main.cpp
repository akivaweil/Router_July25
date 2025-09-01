/*
 * ESP32 Router Control System
 *
 * This program controls a router machine that:
 * 1. Waits for a start signal (IDLE)
 * 2. Feeds wood through router (FEEDING)
 * 3. Flips the wood over with a servo motor (FLIPPING)
 * 4. Feeds wood again (FEEDING2)
 * 5. Goes back to waiting (IDLE)
 */

#include <Arduino.h>
#include <Bounce2.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "ServoControl.h" // Include the custom servo library

// ************************************************************************
// ********************** PROJECT FILES ***********************************
// ************************************************************************
#include "config/Pins_Definitions.h"
#include "config/Config.h"

// ************************************************************************
// ********************** FORWARD DECLARATIONS ****************************
// ************************************************************************
void initOTA();
void handleOTA();
void handleStateMachine();

// ************************************************************************
// ********************** GLOBAL VARIABLES ********************************
// ************************************************************************

// Create Bounce objects for debouncing
Bounce startSensorDebouncer = Bounce();
Bounce manualStartDebouncer = Bounce();

// Create servo object
ServoControl flipServo;

// Keep track of what the machine is doing
enum State { S_NONE, S_IDLE, S_FEEDING, S_FLIPPING, S_FEEDING2 };
State currentState = S_IDLE;

// To prevent log spam, keep track of the last logged state and step
State lastLoggedState = S_NONE;
float lastLoggedStep = 0.0f;

// Variables to remember when things started
unsigned long stateStartTime = 0;
unsigned long stepStartTime = 0;
float currentStep = 0.0f; // Using float as requested

// ************************************************************************
// ********************** HELPER FUNCTIONS ********************************
// ************************************************************************
void log_state_step(const char* message) {
    if (currentState != lastLoggedState || currentStep != lastLoggedStep) {
        Serial.println(message);
        lastLoggedState = currentState;
        lastLoggedStep = currentStep;
    }
}

// ************************************************************************
// ********************** STATE MACHINE FILES *****************************
// ************************************************************************
#include "StateMachine/STATES/00_IDLE.h"
#include "StateMachine/STATES/01_FEEDING.h"
#include "StateMachine/STATES/02_FLIPPING.h"
#include "StateMachine/STATES/03_FEEDING2.h"

// ************************************************************************
// **************************** SETUP *************************************
// ************************************************************************
void setup() {
    // --- START SERIAL ---
    Serial.begin(115200);
    
    // --- DISABLE BROWNOUT DETECTOR ---
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    // Set up the pins
    pinMode(START_SENSOR_PIN, INPUT_PULLDOWN);
    pinMode(MANUAL_START_PIN, INPUT_PULLDOWN);
    pinMode(FEED_CYLINDER_PIN, OUTPUT);

    // Setup the debouncers
    startSensorDebouncer.attach(START_SENSOR_PIN);
    startSensorDebouncer.interval(5); // 30ms debounce
    manualStartDebouncer.attach(MANUAL_START_PIN);
    manualStartDebouncer.interval(30); // 30ms debounce

    // Make sure cylinder starts in safe position (retracted)
    digitalWrite(FEED_CYLINDER_PIN, LOW); // LOW = extended = safe

    // Configure servo motor and perform test sequence
    flipServo.init(FLIP_SERVO_PIN);

    //! ************************************************************************
    //! SERVO TEST SEQUENCE
    //! ************************************************************************
    flipServo.write(SERVO_TEST_START_ANGLE);
    delay(1000);
    flipServo.write(SERVO_TEST_END_ANGLE);
    delay(1000);
    flipServo.write(SERVO_HOME_ANGLE);
    delay(5300); // Give it time to get home before starting

    // Initialize OTA functionality
    initOTA();
}

// ************************************************************************
// ***************************** LOOP *************************************
// ************************************************************************
void loop() {
    // Update the debouncers
    startSensorDebouncer.update();
    manualStartDebouncer.update();

    // Handle Over-The-Air updates
    handleOTA();

    // Run the state machine
    handleStateMachine();
}

// ************************************************************************
// ********************** STATE MACHINE HANDLER ***************************
// ************************************************************************
void handleStateMachine() {
    switch (currentState) {
        case S_IDLE:
            handleIdleState();
            break;
        case S_FEEDING:
            handleFeedingState();
            break;
        case S_FLIPPING:
            handleFlippingState();
            break;
        case S_FEEDING2:
            handleFeeding2State();
            break;
    }
}