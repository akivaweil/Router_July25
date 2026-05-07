#include "StateMachine/STATES/02_FLIPPING.h"
#include "StateMachine/StateMachine_Common.h"
#include "Config/Pins_Definitions.h"

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚔️ FLIPPING CONFIG ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
const float FLIP_ANGLE = 0.0f;
const float SERVO_PRE_HOME_ANGLE = 130.0f; // Servo angle before returning to home
const float CYLINDER_RETRACT_AFTER_PRE_HOME_DELAY_MS = 1000.0f; // Delay after servo BEGINS moving to 130 before cylinder retracts

//* ************************************************************************
//* *********************** FLIPPING STATE HANDLER **************************
//* ************************************************************************
void handleFlippingState() {
    //! ************************************************************************
    //! STEP 1: MOVE SERVO TO FLIP POSITION
    //! ************************************************************************
    if (currentStep == 1.0f) {
        log_state_step("State: FLIPPING - Step 1: Moving servo to flip position.");
        flipServo.write(FLIP_ANGLE);
        stepStartTime = millis();
        currentStep = 2.0f;
    }
    
    //! ************************************************************************
    //! STEP 2: WAIT FOR SERVO TO REACH FLIP POSITION
    //! ************************************************************************
    else if (currentStep == 2.0f) {
        log_state_step("State: FLIPPING - Step 2: Waiting for servo to reach flip position.");
        if (flipServo.hasReachedTarget()) {
            Serial.println("                 - Servo has reached flip position. Sending servo back to pre-home.");
            currentStep = 3.0f;
        }
    }

    //! ************************************************************************
    //! STEP 3: SEND SERVO BACK TO PRE-HOME (130°)
    //! ************************************************************************
    else if (currentStep == 3.0f) {
        log_state_step("State: FLIPPING - Step 3: Sending servo back to pre-home angle.");
        flipServo.write(SERVO_PRE_HOME_ANGLE);
        stepStartTime = millis();
        currentStep = 4.0f;
    }

    //! ************************************************************************
    //! STEP 4: AFTER DELAY, RETRACT CYLINDER AND TRANSITION TO FEEDING2
    //! ************************************************************************
    else if (currentStep == 4.0f) {
        log_state_step("State: FLIPPING - Step 4: Waiting before retracting cylinder.");
        if (millis() - stepStartTime >= CYLINDER_RETRACT_AFTER_PRE_HOME_DELAY_MS) {
            Serial.println("                 - Retracting cylinder and transitioning to FEEDING2 state.");
            // Retract cylinder to push wood (HIGH = retracted/active)
            digitalWrite(FEED_CYLINDER_PIN, HIGH);
            currentState = S_FEEDING2;
            stateStartTime = millis();
            stepStartTime = millis();
            currentStep = 2.0f;  // Skip FEEDING2 step 1 since cylinder is already retracted
        }
    }
}

