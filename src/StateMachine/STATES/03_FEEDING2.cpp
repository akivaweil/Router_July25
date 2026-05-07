#include "StateMachine/STATES/03_FEEDING2.h"
#include "StateMachine/StateMachine_Common.h"
#include "Config/Pins_Definitions.h"

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚔️ FEEDING2 CONFIG ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
const float FEEDING2_START_DELAY_MS = 1.0f;
const float FEEDING2_DURATION_MS = 2500.0f;

//* ************************************************************************
//* ********************* SECOND FEEDING STATE HANDLER **********************
//* ************************************************************************
void handleFeeding2State() {
    //! ************************************************************************
    //! STEP 1: START SECOND FEED
    //! ************************************************************************
    if (currentStep == 1.0f) {
        log_state_step("State: FEEDING2 - Step 1: Starting second feed.");
        // Retract cylinder to push wood (HIGH = retracted/active)
        delay(FEEDING2_START_DELAY_MS);
        digitalWrite(FEED_CYLINDER_PIN, HIGH);
        stepStartTime = millis();
        currentStep = 2.0f;
    }
    
    //! ************************************************************************
    //! STEP 2: WAIT FOR FEED TIME TO ELAPSE
    //! ************************************************************************
    else if (currentStep == 2.0f) {
        log_state_step("State: FEEDING2 - Step 2: Waiting for feed time to elapse.");
        if (millis() - stepStartTime >= FEEDING2_DURATION_MS) {
            Serial.println("                 - Feed time elapsed. Extending cylinder to safe position.");
            Serial.println("                 - Machine cycle complete. Returning to IDLE state.");
            // Extend cylinder to safe position (LOW = extended/safe)
            digitalWrite(FEED_CYLINDER_PIN, LOW);
            currentState = S_IDLE;  // Go back to IDLE state
            currentStep = 1.0f;
        }
    }
}

