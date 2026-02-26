#include "StateMachine/STATES/01_FEEDING.h"
#include "StateMachine/StateMachine_Common.h"
#include "Config/Pins_Definitions.h"

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚔️ FEEDING CONFIG ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
const float FEEDING_START_DELAY_MS = 300.0f;
const float FEEDING_DURATION_MS = 2300.0f;

//* ************************************************************************
//* *********************** FEEDING STATE HANDLER **************************
//* ************************************************************************
void handleFeedingState() {
    //! ************************************************************************
    //! STEP 1: WAIT FOR START DELAY AND RETRACT CYLINDER
    //! ************************************************************************
    if (currentStep == 1.0f) {
        log_state_step("State: FEEDING - Step 1: Waiting for start delay...");
        if (millis() - stateStartTime >= FEEDING_START_DELAY_MS) {
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
        if (millis() - stepStartTime >= FEEDING_DURATION_MS) {
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

