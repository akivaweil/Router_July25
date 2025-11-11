#include "StateMachine/STATES/02_FLIPPING.h"
#include "StateMachine/StateMachine_Common.h"
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"

const float SERVO_PRE_HOME_ANGLE = 180.0f; // Servo angle before returning to home

//* ************************************************************************
//* *********************** FLIPPING STATE HANDLER **************************
//* ************************************************************************
void handleFlippingState() {
    //! ************************************************************************
    //! CHECK IF BOARD END MODE IS ACTIVE - SKIP SERVO ROTATION
    //! ************************************************************************
    if (boardEndModeActive) {
        log_state_step("State: FLIPPING - Board end mode active, skipping servo rotation.");
        Serial.println("                 - Board end mode: Servo stays at current position.");
        Serial.println("                 - Transitioning to FEEDING2 state.");
        currentState = S_FEEDING2;  // Skip flipping, go directly to second feeding
        stateStartTime = millis();
        currentStep = 1.0f;
        return;
    }

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
    //! STEP 2: WAIT FOR SERVO TO FINISH MOVING
    //! ************************************************************************
    else if (currentStep == 2.0f) {
        log_state_step("State: FLIPPING - Step 2: Waiting for servo to finish moving.");
        // Wait for the servo to get to the flip position
        if (flipServo.hasReachedTarget()) {
            Serial.println("                 - Servo has reached flip position.");
            currentStep = 3.0f;
        }
    }
    
    //! ************************************************************************
    //! STEP 3: MOVE SERVO TO PRE-HOME POSITION (110 DEGREES)
    //! ************************************************************************
    else if (currentStep == 3.0f) {
        log_state_step("State: FLIPPING - Step 3: Moving servo to pre-home position (110 degrees).");
        flipServo.write(SERVO_PRE_HOME_ANGLE);
        stepStartTime = millis();
        currentStep = 4.0f;
    }

    //! ************************************************************************
    //! STEP 4: WAIT FOR SERVO TO REACH PRE-HOME POSITION THEN TRANSITION
    //! ************************************************************************
    else if (currentStep == 4.0f) {
        log_state_step("State: FLIPPING - Step 4: Waiting for servo to reach pre-home position.");
        if (flipServo.hasReachedTarget()) {
            Serial.println("                 - Servo has reached pre-home position. Transitioning to FEEDING2 state.");
            currentState = S_FEEDING2;  // Go to second feeding
            stateStartTime = millis();
            currentStep = 1.0f;
        }
    }
}

