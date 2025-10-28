#pragma once

#include "config/Config.h"

//* ************************************************************************
//* ********************* SECOND FEEDING STATE *****************************
//* ************************************************************************

//! ********************** EXTERNAL DECLARATIONS ***************************
extern State currentState;
extern unsigned long stepStartTime;
extern float currentStep;
void log_state_step(const char* message);

//* ************************************************************************
//* ********************* SECOND FEEDING STATE HANDLER **********************
//* ************************************************************************
void handleFeeding2State() {
    //! ************************************************************************
    //! STEP 1: START SECOND FEED
    //! ************************************************************************
    if (currentStep == 1.0f) {
        log_state_step("State: FEEDING2 - Step 1: Starting second feed.");
        // Retract cylinder to push wood (HIGH = retracted/active)
        digitalWrite(FEED_CYLINDER_PIN, HIGH);
        stepStartTime = millis();
        currentStep = 2.0f;
    }
    
    //! ************************************************************************
    //! STEP 2: WAIT FOR FEED TIME TO ELAPSE
    //! ************************************************************************
    else if (currentStep == 2.0f) {
        log_state_step("State: FEEDING2 - Step 2: Waiting for feed time to elapse.");
        if (millis() - stepStartTime >= FEED_TIME) {
            Serial.println("                 - Feed time elapsed. Extending cylinder to safe position.");
            Serial.println("                 - Machine cycle complete. Returning to IDLE state.");
            // Extend cylinder to safe position (LOW = extended/safe)
            digitalWrite(FEED_CYLINDER_PIN, LOW);
            currentState = S_IDLE;  // Go back to IDLE state
            currentStep = 1.0f;
        }
    }
} 