#pragma once

// ************************************************************************
// ********************* SECOND FEEDING STATE *****************************
// ************************************************************************
void log_state_step(const char* message);

void handleFeeding2State() {
    //! ************************************************************************
    //! STEP 1: START SECOND FEED
    //! ************************************************************************
    if (currentStep == 1.0f) {
        log_state_step("State: FEEDING2 - Step 1: Starting second feed.");
        // Retract cylinder to push wood
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
            // Extend cylinder to safe position
            digitalWrite(FEED_CYLINDER_PIN, LOW);
            currentState = S_IDLE;  // Go back to IDLE state
            currentStep = 1.0f;
        }
    }
} 