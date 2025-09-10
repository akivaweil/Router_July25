#pragma once

#include "config/Config.h"

// ************************************************************************
// *********************** FEEDING STATE **********************************
// ************************************************************************
void log_state_step(const char* message);

extern ServoControl flipServo;

void handleFeedingState() {
    //! ************************************************************************
    //! STEP 1: MOVE SERVO TO START ANGLE AND WAIT
    //! ************************************************************************
    if (currentStep == 1.0f) {
        log_state_step("State: FEEDING - Step 1: Moving servo to start angle...");
        flipServo.write(SERVO_START_ANGLE);
        stepStartTime = millis();
        currentStep = 1.5f;
    }
    
    //! ************************************************************************
    //! STEP 1.5: WAIT AT START ANGLE
    //! ************************************************************************
    else if (currentStep == 1.5f) {
        log_state_step("State: FEEDING - Step 1.5: Waiting at start angle...");
        if (millis() - stepStartTime >= SERVO_START_WAIT) {
            Serial.println("                 - Start angle wait complete. Moving servo to home position.");
            flipServo.write(SERVO_HOME_ANGLE);
            stepStartTime = millis();
            currentStep = 2.0f;
        }
    }
    
    //! ************************************************************************
    //! STEP 2: WAIT FOR SERVO TO RETURN HOME AND START DELAY
    //! ************************************************************************
    else if (currentStep == 2.0f) {
        log_state_step("State: FEEDING - Step 2: Waiting for servo to return home and start delay...");
        if (millis() - stepStartTime >= SERVO_MOVE_DELAY && millis() - stateStartTime >= FEEDING_START_DELAY_1) {
            Serial.println("                 - Servo home and delay complete. Retracting cylinder to push wood.");
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