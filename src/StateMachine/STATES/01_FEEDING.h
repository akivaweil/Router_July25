#pragma once

#include "config/Config.h"

// ************************************************************************
// *********************** FEEDING STATE **********************************
// ************************************************************************

extern ServoControl flipServo;
void log_state_step(const char* message);

void handleFeedingState() {
    //! ************************************************************************
    //! STEP 1: INITIAL SERVO SEQUENCE - MOVE TO 120 DEGREES
    //! ************************************************************************
    if (currentStep == 1.0f) {
        log_state_step("State: FEEDING - Step 1: Initial servo sequence - moving to 120°...");
        if (flipServo.hasReachedTarget()) {
            Serial.println("                 - Servo reached 120°. Waiting 500ms...");
            stepStartTime = millis();
            currentStep = 1.5f;
        }
    }
    
    //! ************************************************************************
    //! STEP 1.5: WAIT 500MS AT 120 DEGREES
    //! ************************************************************************
    else if (currentStep == 1.5f) {
        log_state_step("State: FEEDING - Step 1.5: Waiting 500ms at 120°...");
        if (millis() - stepStartTime >= SERVO_INITIAL_WAIT) {
            Serial.println("                 - Wait complete. Moving servo back to 104°...");
            flipServo.write(SERVO_HOME_ANGLE);
            stepStartTime = millis();
            currentStep = 1.7f;
        }
    }
    
    //! ************************************************************************
    //! STEP 1.7: WAIT FOR SERVO TO RETURN TO HOME POSITION
    //! ************************************************************************
    else if (currentStep == 1.7f) {
        log_state_step("State: FEEDING - Step 1.7: Waiting for servo to return to 104°...");
        if (flipServo.hasReachedTarget()) {
            Serial.println("                 - Servo sequence complete. Starting feeding process...");
            stepStartTime = millis();
            currentStep = 2.0f;
        }
    }
    
    //! ************************************************************************
    //! STEP 2: WAIT FOR START DELAY
    //! ************************************************************************
    else if (currentStep == 2.0f) {
        log_state_step("State: FEEDING - Step 2: Waiting for start delay...");
        if (millis() - stepStartTime >= FEEDING_START_DELAY_1) {
            Serial.println("                 - Delay complete. Retracting cylinder to push wood.");
            // Retract cylinder to push wood
            digitalWrite(FEED_CYLINDER_PIN, HIGH);
            stepStartTime = millis();
            currentStep = 3.0f;
        }
    }
    
    //! ************************************************************************
    //! STEP 3: WAIT FOR FEED TIME TO ELAPSE
    //! ************************************************************************
    else if (currentStep == 3.0f) {
        log_state_step("State: FEEDING - Step 3: Waiting for feed time to elapse...");
        if (millis() - stepStartTime >= FEED_TIME) {
            Serial.println("                 - Feed time elapsed. Extending cylinder to safe position.");
            Serial.println("                 - Transitioning to FLIPPING state.");
            // Extend cylinder to safe position
            digitalWrite(FEED_CYLINDER_PIN, LOW);
            currentState = S_FLIPPING;  // Go to FLIPPING state
            stateStartTime = millis();
            currentStep = 1.0f;
        }
    }
} 