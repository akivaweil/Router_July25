#pragma once

// ************************************************************************
// ********************* SECOND FEEDING STATE *****************************
// ************************************************************************

void handleFeeding2State() {
    //! ************************************************************************
    //! STEP 1: START SECOND FEED
    //! ************************************************************************
    if (currentStep == 1.0f) {
        // Retract cylinder to push wood
        digitalWrite(FEED_CYLINDER_PIN, HIGH);
        stepStartTime = millis();
        currentStep = 2.0f;
    }
    
    //! ************************************************************************
    //! STEP 2: WAIT FOR FEED TIME TO ELAPSE
    //! ************************************************************************
    else if (currentStep == 2.0f) {
        if (millis() - stepStartTime >= FEED_TIME) {
            // Extend cylinder to safe position
            digitalWrite(FEED_CYLINDER_PIN, LOW);
            currentState = S_IDLE;  // Go back to IDLE state
            currentStep = 1.0f;
        }
    }
} 