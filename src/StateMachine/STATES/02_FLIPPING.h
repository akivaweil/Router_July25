#pragma once

// ************************************************************************
// *********************** FLIPPING STATE *********************************
// ************************************************************************

void handleFlippingState() {
    //! ************************************************************************
    //! STEP 1: MOVE STEPPER TO FLIP POSITION
    //! ************************************************************************
    if (currentStep == 1.0f) {
        flipStepper.moveTo(STEPS_FOR_FLIP);
        stepStartTime = millis();
        currentStep = 2.0f;
    }
    
    //! ************************************************************************
    //! STEP 2: WAIT FOR STEPPER TO FINISH MOVING
    //! ************************************************************************
    else if (currentStep == 2.0f) {
        // Wait until the stepper has reached its target
        if (flipStepper.distanceToGo() == 0) {
            // Add a small delay for stability if needed, but for now we proceed
            currentStep = 3.0f;
            stepStartTime = millis();
        }
    }
    
    //! ************************************************************************
    //! STEP 3: MOVE STEPPER BACK TO HOME POSITION
    //! ************************************************************************
    else if (currentStep == 3.0f) {
        flipStepper.moveTo(0);
        stepStartTime = millis();
        currentStep = 4.0f;
    }

    //! ************************************************************************
    //! STEP 4: WAIT FOR STEPPER TO RETURN HOME
    //! ************************************************************************
    else if (currentStep == 4.0f) {
        if (flipStepper.distanceToGo() == 0) {
            currentState = S_FEEDING2;  // Go to second feeding
            stateStartTime = millis();
            currentStep = 1.0f;
        }
    }
} 