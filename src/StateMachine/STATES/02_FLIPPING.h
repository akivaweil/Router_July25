#pragma once

// ************************************************************************
// *********************** FLIPPING STATE *********************************
// ************************************************************************
void log_state_step(const char* message);

void handleFlippingState() {
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
        if (millis() - stepStartTime >= SERVO_MOVE_DELAY) {
            Serial.println("                 - Servo has reached flip position.");
            currentStep = 3.0f;
            stepStartTime = millis();
        }
    }
    
    //! ************************************************************************
    //! STEP 3: MOVE SERVO BACK TO HOME POSITION
    //! ************************************************************************
    else if (currentStep == 3.0f) {
        log_state_step("State: FLIPPING - Step 3: Moving servo back to home position.");
        flipServo.write(SERVO_HOME_ANGLE);
        stepStartTime = millis();
        currentStep = 4.0f;
    }

    //! ************************************************************************
    //! STEP 4: WAIT FOR SERVO TO RETURN HOME
    //! ************************************************************************
    else if (currentStep == 4.0f) {
        log_state_step("State: FLIPPING - Step 4: Waiting for servo to return home.");
        if (millis() - stepStartTime >= SERVO_MOVE_DELAY) {
            Serial.println("                 - Servo has returned home. Transitioning to FEEDING2 state.");
            currentState = S_FEEDING2;  // Go to second feeding
            stateStartTime = millis();
            currentStep = 1.0f;
        }
    }
} 