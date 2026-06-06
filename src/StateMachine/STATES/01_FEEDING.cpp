#include "StateMachine/STATES/01_FEEDING.h"
#include "StateMachine/StateMachine.h"
#include "Config/Pins_Definitions.h"

// FEEDING CONFIG
const float FEEDING_START_DELAY_MS = 300.0f;
float FEEDING_DURATION_MS = 2300.0f; // Runtime-tunable via config API
const float FEEDING_SERVO_PREP_DELAY_MS = 1000.0f; // Time after retraction begins before servo prep move
const float FEEDING_SERVO_PREP_ANGLE = 70.0f;      // Servo angle in prep for the flip to 0

// Feeding state handler
void handleFeedingState() {
    // Step 1: wait for start delay and retract cylinder
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

    // Step 2: after 1000ms of retraction, move servo to 70 deg in prep for flip
    else if (currentStep == 2.0f) {
        log_state_step("State: FEEDING - Step 2: Waiting to move servo to prep angle...");
        if (millis() - stepStartTime >= FEEDING_SERVO_PREP_DELAY_MS) {
            Serial.println("                 - Moving servo to prep angle (70 deg) in prep for flip to 0.");
            flipServo.write(FEEDING_SERVO_PREP_ANGLE);
            currentStep = 3.0f;
        }
    }

    // Step 3: wait for feed time to elapse
    else if (currentStep == 3.0f) {
        log_state_step("State: FEEDING - Step 3: Waiting for feed time to elapse...");
        if (millis() - stepStartTime >= FEEDING_DURATION_MS) {
            Serial.println("                 - Feed time elapsed. Extending cylinder to safe position.");
            Serial.println("                 - Transitioning to FLIPPING state.");
            // Extend cylinder to safe position (LOW = extended/safe)
            digitalWrite(FEED_CYLINDER_PIN, LOW);
            currentState = STATE_FLIPPING;  // Go to FLIPPING state
            stateStartTime = millis();
            currentStep = 1.0f;
        }
    }
}
