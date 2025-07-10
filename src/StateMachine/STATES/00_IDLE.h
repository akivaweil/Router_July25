#pragma once

// ************************************************************************
// ************************ IDLE STATE ************************************
// ************************************************************************

extern Bounce startSensorDebouncer;
extern Bounce manualStartDebouncer;
void log_state_step(const char* message);

void handleIdleState() {
    log_state_step("State: IDLE - Waiting for start signal...");

    // Check if start button pressed or sensor triggered
    if (startSensorDebouncer.read() || manualStartDebouncer.read()) {
        Serial.println("Start signal received! Transitioning to FEEDING state.");
        currentState = S_FEEDING;  // Go to FEEDING state
        stateStartTime = millis();
        currentStep = 1.0f;
    }
} 