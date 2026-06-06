#include "StateMachine/STATES/00_IDLE.h"
#include "StateMachine/StateMachine.h"
#include "Config/Pins_Definitions.h"

// IDLE CONFIG
float IDLE_HOME_OFFSET = 5.0f;       // Idle rests this many degrees below dashboard home angle (runtime-tunable via config API)
float ESPNOW_KICKOFF_OFFSET = 15.0f; // Kickoff nudges servo this many degrees above dashboard home (runtime-tunable via config API)
const unsigned long KICKOFF_PRE_DELAY_MS = 50;   // Wait this long after start signal before nudging servo up
const unsigned long ESPNOW_KICKOFF_DELAY_MS = 300;

// Idle state handler
void handleIdleState() {
    log_state_step("State: IDLE - Waiting for start signal...");

    // Ensure servo is in idle position (dashboard home - offset)
    static bool servoHomed = false;
    static bool preKickoffPending = false;
    static bool kickoffPending = false;
    static unsigned long preKickoffStartTime = 0;
    static unsigned long kickoffStartTime = 0;
    if (!servoHomed) {
        flipServo.write(SERVO_HOME_ANGLE - IDLE_HOME_OFFSET);
        servoHomed = true;
    }

    // Pre-kickoff: wait 50ms before nudging servo up
    if (preKickoffPending) {
        // Drain any latched start edges so repeat pulses during kickoff don't queue another cycle
        startSensorDebouncer.rose();
        manualStartDebouncer.rose();
        espNowStartReceived = false;
        if (millis() - preKickoffStartTime >= KICKOFF_PRE_DELAY_MS) {
            Serial.println("Pre-kickoff delay elapsed. Nudging servo to kickoff position.");
            flipServo.write(SERVO_HOME_ANGLE + ESPNOW_KICKOFF_OFFSET);
            kickoffStartTime = millis();
            preKickoffPending = false;
            kickoffPending = true;
        }
        return;
    }

    // Kickoff: wait 300ms after nudge, then start cycle
    if (kickoffPending) {
        // Drain any latched start edges so repeat pulses during kickoff don't queue another cycle
        startSensorDebouncer.rose();
        manualStartDebouncer.rose();
        espNowStartReceived = false;
        if (millis() - kickoffStartTime >= ESPNOW_KICKOFF_DELAY_MS) {
            Serial.println("Kickoff complete. Transitioning to FEEDING state.");
            kickoffPending = false;
            currentState = STATE_FEEDING;
            stateStartTime = millis();
            currentStep = 1.0f;
            servoHomed = false;
        }
        return;
    }

    // Any start signal -> pre-kickoff + kickoff + full cycle (edge-triggered for sensor/manual)
    if (espNowStartReceived || startSensorDebouncer.rose() || manualStartDebouncer.rose()) {
        espNowStartReceived = false;
        Serial.println("Start signal received. Waiting pre-kickoff delay.");
        preKickoffStartTime = millis();
        preKickoffPending = true;
    }
}
