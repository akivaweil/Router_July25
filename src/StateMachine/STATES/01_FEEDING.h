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
        
        // Start servo sequence on first call
        if (millis() - stateStartTime < 100) { // Only on first call (within 100ms of state start)
            Serial.println("                 - Moving servo to 120°...");
            flipServo.write(SERVO_INITIAL_ANGLE);
        }
        
        // Check if servo reached 120° and start wait
        static unsigned long servoWaitStart = 0;
        static bool servoWaitStarted = false;
        if (!servoWaitStarted && flipServo.hasReachedTarget()) {
            Serial.println("                 - Servo reached 120°. Waiting 500ms...");
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
        
        // Check if both servo sequence and feeding delay are complete
        bool feedingDelayComplete = (millis() - stateStartTime >= FEEDING_START_DELAY_1);
        bool servoBackToHome = servoMovedBack && flipServo.hasReachedTarget();
        
        if (feedingDelayComplete && servoBackToHome) {
            Serial.println("                 - Both servo sequence and feeding delay complete. Starting feed...");
            // Retract cylinder to push wood
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
            // Extend cylinder to safe position
            digitalWrite(FEED_CYLINDER_PIN, LOW);
            currentState = S_FLIPPING;  // Go to FLIPPING state
            stateStartTime = millis();
            currentStep = 1.0f;
        }
    }
} 