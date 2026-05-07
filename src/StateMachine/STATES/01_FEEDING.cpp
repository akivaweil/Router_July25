#include "StateMachine/STATES/01_FEEDING.h"
#include "StateMachine/StateMachine_Common.h"
#include "Config/Pins_Definitions.h"

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚔️ FEEDING CONFIG ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
const float FEEDING_START_DELAY_MS = 300.0f;
const float FEEDING_DURATION_MS = 2500.0f;
const float FEEDING_SERVO_PREP_DELAY_MS = 1000.0f; // Time after retraction begins before servo prep move
const float FEEDING_SERVO_PREP_ANGLE = 70.0f;      // Servo angle in prep for the flip to 0

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
    //! STEP 2: AFTER 1000ms OF RETRACTION, MOVE SERVO TO 70° IN PREP FOR FLIP
    //! ************************************************************************
    else if (currentStep == 2.0f) {
        log_state_step("State: FEEDING - Step 2: Waiting to move servo to prep angle...");
        if (millis() - stepStartTime >= FEEDING_SERVO_PREP_DELAY_MS) {
            Serial.println("                 - Moving servo to prep angle (70 deg) in prep for flip to 0.");
            flipServo.write(FEEDING_SERVO_PREP_ANGLE);
            currentStep = 3.0f;
        }
    }

    //! ************************************************************************
    //! STEP 3: WAIT FOR FEED TIME TO ELAPSE
    //! ************************************************************************
    else if (currentStep == 3.0f) {
        log_state_step("State: FEEDING - Step 3: Waiting for feed time to elapse...");
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

