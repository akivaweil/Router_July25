#pragma once

#include "config/Config.h"

// ************************************************************************
// *********************** FEEDING STATE **********************************
// ************************************************************************

extern ServoControl flipServo;
void log_state_step(const char* message);

void handleFeedingState() {
    //! ************************************************************************
    //! STEP 1: PARALLEL SERVO SEQUENCE AND FEEDING START DELAY
    //! ************************************************************************
    if (currentStep == 1.0f) {
        log_state_step("State: FEEDING - Step 1: Starting servo sequence and feeding delay simultaneously...");
        
        // Start servo sequence after 1 second delay
        static bool servoStarted = false;
        if (!servoStarted && millis() - stateStartTime >= SERVO_START_DELAY) {
            Serial.println("                 - Servo delay complete. Moving servo to 130°...");
            flipServo.write(SERVO_INITIAL_ANGLE);
            servoStarted = true;
        }
        
        // Check if servo reached 130° and start wait
        static unsigned long servoWaitStart = 0;
        static bool servoWaitStarted = false;
        if (servoStarted && !servoWaitStarted && flipServo.hasReachedTarget()) {
            Serial.println("                 - Servo reached 130°. Waiting 500ms...");
            servoWaitStart = millis();
            servoWaitStarted = true;
        }
        
        // Check if servo wait is complete and move back to 104°
        static bool servoMovedBack = false;
        if (servoWaitStarted && !servoMovedBack && millis() - servoWaitStart >= SERVO_INITIAL_WAIT) {
            Serial.println("                 - Servo wait complete. Moving back to 104°...");
            flipServo.write(SERVO_HOME_ANGLE);
            servoMovedBack = true;
        }
        
        // Check if feeding delay is complete (independent of servo)
        static bool cylinderRetracted = false;
        if (!cylinderRetracted && millis() - stateStartTime >= FEEDING_START_DELAY_1) {
            Serial.println("                 - Feeding delay complete. Retracting cylinder to push wood...");
            // Retract cylinder to push wood
            digitalWrite(FEED_CYLINDER_PIN, HIGH);
            cylinderRetracted = true;
            stepStartTime = millis();
        }
        
        // Move to next step when cylinder is retracted
        if (cylinderRetracted) {
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
            // Extend cylinder to safe position
            digitalWrite(FEED_CYLINDER_PIN, LOW);
            currentState = S_FLIPPING;  // Go to FLIPPING state
            stateStartTime = millis();
            currentStep = 1.0f;
        }
    }
} 