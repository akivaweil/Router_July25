#include "StateMachine/STATES/02_FLIPPING.h"
#include "StateMachine/StateMachine.h"
#include "Config/Pins_Definitions.h"

// FLIPPING CONFIG
const float FLIP_ANGLE = 0.0f;
const float SERVO_PRE_HOME_ANGLE = 130.0f; // Servo angle before returning to home
const float CYLINDER_RETRACT_AFTER_PRE_HOME_DELAY_MS = 1000.0f; // Delay after servo BEGINS moving to 130 before cylinder retracts

// Flipping state handler
void handleFlippingState() {
    // Step 1: move servo to flip position
    if (currentStep == 1.0f) {
        log_state_step("State: FLIPPING - Step 1: Moving servo to flip position.");
        flipServo.write(FLIP_ANGLE);
        stepStartTime = millis();
        currentStep = 2.0f;
    }

    // Step 2: wait for servo to reach flip position
    else if (currentStep == 2.0f) {
        log_state_step("State: FLIPPING - Step 2: Waiting for servo to reach flip position.");
        if (flipServo.hasReachedTarget()) {
            Serial.println("                 - Servo has reached flip position. Sending servo back to pre-home.");
            currentStep = 3.0f;
        }
    }

    // Step 3: send servo back to pre-home (130 deg)
    else if (currentStep == 3.0f) {
        log_state_step("State: FLIPPING - Step 3: Sending servo back to pre-home angle.");
        flipServo.write(SERVO_PRE_HOME_ANGLE);
        stepStartTime = millis();
        currentStep = 4.0f;
    }

    // Step 4: after delay, retract cylinder and transition to FEEDING2
    else if (currentStep == 4.0f) {
        log_state_step("State: FLIPPING - Step 4: Waiting before retracting cylinder.");
        if (millis() - stepStartTime >= CYLINDER_RETRACT_AFTER_PRE_HOME_DELAY_MS) {
            Serial.println("                 - Retracting cylinder and transitioning to FEEDING2 state.");
            // Retract cylinder to push wood (HIGH = retracted/active)
            digitalWrite(FEED_CYLINDER_PIN, HIGH);
            currentState = STATE_FEEDING2;
            stateStartTime = millis();
            stepStartTime = millis();
            currentStep = 2.0f;  // Skip FEEDING2 step 1 since cylinder is already retracted
        }
    }
}
