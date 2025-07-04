/*
 * ESP32 Router Control System
 *
 * This program controls a router machine that:
 * 1. Waits for a start signal (IDLE)
 * 2. Feeds wood through router (FEEDING)
 * 3. Flips the wood over with a stepper motor (FLIPPING)
 * 4. Feeds wood again (FEEDING2)
 * 5. Goes back to waiting (IDLE)
 */

#include <Arduino.h>
#include <AccelStepper.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

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

// Create stepper object
AccelStepper flipStepper(AccelStepper::DRIVER, FLIP_STEPPER_STEP_PIN, FLIP_STEPPER_DIR_PIN);

// Keep track of what the machine is doing
enum State { S_IDLE, S_FEEDING, S_FLIPPING, S_FEEDING2 };
State currentState = S_IDLE;

// Variables to remember when things started
unsigned long stateStartTime = 0;
unsigned long stepStartTime = 0;
float currentStep = 0.0f; // Using float as requested

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
    // --- DISABLE BROWNOUT DETECTOR ---
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    // Set up the pins
    pinMode(START_SENSOR_PIN, INPUT_PULLDOWN);
    pinMode(MANUAL_START_PIN, INPUT_PULLDOWN);
    pinMode(FEED_CYLINDER_PIN, OUTPUT);

    // Make sure cylinder starts in safe position (retracted)
    digitalWrite(FEED_CYLINDER_PIN, LOW); // LOW = extended = safe

    // Configure stepper motor
    flipStepper.setMaxSpeed(STEPPER_MAX_SPEED);
    flipStepper.setAcceleration(STEPPER_ACCELERATION);
    flipStepper.setCurrentPosition(0); // Assume it starts at 0

    // Initialize OTA functionality
    initOTA();
}

// ************************************************************************
// ***************************** LOOP *************************************
// ************************************************************************
void loop() {
    // Handle Over-The-Air updates
    handleOTA();

    // Run the state machine
    handleStateMachine();

    // The stepper motor must be run constantly
    flipStepper.run();
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