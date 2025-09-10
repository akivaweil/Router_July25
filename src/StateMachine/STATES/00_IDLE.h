#pragma once

#include "config/Config.h"

// ************************************************************************
// ************************ IDLE STATE ************************************
// ************************************************************************

extern Bounce startSensorDebouncer;
extern Bounce manualStartDebouncer;
extern ServoControl flipServo;
void log_state_step(const char* message);

void handleIdleState() {
    log_state_step("State: IDLE - Waiting for start signal...");

    //! ************************************************************************
    //! ENSURE SERVO IS IN HOME POSITION
    //! ************************************************************************
    flipServo.write(SERVO_HOME_ANGLE);

    // Check if start button pressed or sensor triggered
    if (startSensorDebouncer.read() || manualStartDebouncer.read()) {
        Serial.println("Start signal received! Starting parallel servo sequence and feeding process.");
        currentState = S_FEEDING;  // Go to FEEDING state to handle both processes
        stateStartTime = millis();
        currentStep = 1.0f;
    }
} 