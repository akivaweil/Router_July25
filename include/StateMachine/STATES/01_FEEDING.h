#pragma once

#include "config/Config.h"

//* ************************************************************************
//* *********************** FEEDING STATE **********************************
//* ************************************************************************

//! ********************** EXTERNAL DECLARATIONS ***************************
extern ServoControl flipServo;
extern State currentState;
extern unsigned long stateStartTime;
extern unsigned long stepStartTime;
extern float currentStep;
void log_state_step(const char* message);

//* ************************************************************************
//* *********************** FEEDING STATE HANDLER **************************
//* ************************************************************************
void handleFeedingState() {
    //! ************************************************************************
    //! STEP 1: WAIT FOR START DELAY AND RETRACT CYLINDER
    //! ************************************************************************
    if (currentStep == 1.0f) {
        log_state_step("State: FEEDING - Step 1: Waiting for start delay...");
        if (millis() - stateStartTime >= FEEDING_START_DELAY_1) {
            Serial.println("                 - Start delay complete. Retracting cylinder to push wood.");
            // Retract cylinder to push wood (HIGH = retracted/active)
            digitalWrite(FEED_CYLINDER_PIN, HIGH);
            stepStartTime = millis();
            currentStep = 2.0f;
        }
    }
    
    //! ************************************************************************
    //! STEP 2: WAIT FOR FEED TIME TO ELAPSE
    //! ************************************************************************
    else if (currentStep == 2.0f) {
        log_state_step("State: FEEDING - Step 2: Waiting for feed time to elapse...");
        if (millis() - stepStartTime >= FEED_TIME) {
            Serial.println("                 - Feed time elapsed. Extending cylinder to safe position.");
            Serial.println("                 - Transitioning to FLIPPING state.");
            // Extend cylinder to safe position (LOW = extended/safe)
            digitalWrite(FEED_CYLINDER_PIN, LOW);
            currentState = S_FLIPPING;  // Go to FLIPPING state
            stateStartTime = millis();
            currentStep = 1.0f;
        }
    }
} 