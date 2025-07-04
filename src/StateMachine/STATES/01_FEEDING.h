#pragma once

// ************************************************************************
// *********************** FEEDING STATE **********************************
// ************************************************************************

void handleFeedingState() {
    //! ************************************************************************
    //! STEP 1: WAIT FOR START DELAY
    //! ************************************************************************
    if (currentStep == 1.0f) {
        if (millis() - stateStartTime >= FEEDING_START_DELAY) {
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
        if (millis() - stepStartTime >= FEED_TIME) {
            // Extend cylinder to safe position
            digitalWrite(FEED_CYLINDER_PIN, LOW);
            currentState = S_FLIPPING;  // Go to FLIPPING state
            stateStartTime = millis();
            currentStep = 1.0f;
        }
    }
} 