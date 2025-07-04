#pragma once

// ************************************************************************
// ************************ IDLE STATE ************************************
// ************************************************************************

void handleIdleState() {
    // Check if start button pressed or sensor triggered
    if (digitalRead(START_SENSOR_PIN) || digitalRead(MANUAL_START_PIN)) {
        currentState = S_FEEDING;  // Go to FEEDING state
        stateStartTime = millis();
        currentStep = 1.0f;
    }
} 