#include "StateMachine/STATES/03_FEEDING2.h"
#include "StateMachine/StateMachine.h"
#include "Config/Pins_Definitions.h"

// FEEDING2 CONFIG
const float FEEDING2_START_DELAY_MS = 1.0f;
const float FEEDING2_DURATION_MS = 2300.0f;

// Second feeding state handler
void handleFeeding2State() {
    // Step 1: start second feed
    if (currentStep == 1.0f) {
        log_state_step("State: FEEDING2 - Step 1: Starting second feed.");
        // Retract cylinder to push wood (HIGH = retracted/active)
        delay(FEEDING2_START_DELAY_MS);
        digitalWrite(FEED_CYLINDER_PIN, HIGH);
        stepStartTime = millis();
        currentStep = 2.0f;
    }

    // Step 2: wait for feed time to elapse
    else if (currentStep == 2.0f) {
        log_state_step("State: FEEDING2 - Step 2: Waiting for feed time to elapse.");
        if (millis() - stepStartTime >= FEEDING2_DURATION_MS) {
            Serial.println("                 - Feed time elapsed. Extending cylinder to safe position.");
            Serial.println("                 - Machine cycle complete. Returning to IDLE state.");
            // Extend cylinder to safe position (LOW = extended/safe)
            digitalWrite(FEED_CYLINDER_PIN, LOW);
            currentState = STATE_IDLE;  // Go back to IDLE state
            currentStep = 1.0f;
        }
    }
}
