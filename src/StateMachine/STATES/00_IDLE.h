#pragma once

// ************************************************************************
// ************************ IDLE STATE ************************************
// ************************************************************************

extern Bounce startSensorDebouncer;
extern Bounce manualStartDebouncer;

void handleIdleState() {
    // Check if start button pressed or sensor triggered
    if (startSensorDebouncer.read() || manualStartDebouncer.read()) {
        currentState = S_FEEDING;  // Go to FEEDING state
        stateStartTime = millis();
        currentStep = 1.0f;
    }
} 