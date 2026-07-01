#include "StateMachine/STATES/03_FEEDING2.h"
#include "StateMachine/StateMachine.h"
#include "Config/Pins_Definitions.h"

// FEEDING2 CONFIG
const float FEEDING2_START_DELAY_MS = 1.0f;
const float FEEDING2_DURATION_MS = 2300.0f;

// Second feeding state handler
void handleFeeding2State() {
    // Step 1: arm the pre-feed delay (non-blocking)
    if (currentStep == 1.0f) {
        stepStartTime = millis();
        currentStep = 1.5f;
    }

    // Step 1.5: wait out the pre-feed delay, then retract cylinder to push wood
    else if (currentStep == 1.5f) {
        if (millis() - stepStartTime >= (unsigned long)FEEDING2_START_DELAY_MS) {
            // Retract cylinder to push wood (HIGH = retracted/active)
            digitalWrite(FEED_CYLINDER_PIN, HIGH);
            stepStartTime = millis();
            currentStep = 2.0f;
        }
    }

    // Step 2: wait for feed time to elapse
    else if (currentStep == 2.0f) {
        if (millis() - stepStartTime >= FEEDING2_DURATION_MS) {
            // Extend cylinder to safe position (LOW = extended/safe)
            digitalWrite(FEED_CYLINDER_PIN, LOW);
            currentState = STATE_IDLE;  // Go back to IDLE state
            currentStep = 1.0f;
        }
    }
}
