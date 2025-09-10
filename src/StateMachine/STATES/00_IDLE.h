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
        Serial.println("Start signal received! Beginning initial servo sequence.");
        
        //! ************************************************************************
        //! INITIAL SERVO SEQUENCE: 120° → wait 500ms → 104°
        //! ************************************************************************
        Serial.println("                 - Moving servo to 120 degrees...");
        flipServo.write(SERVO_INITIAL_ANGLE);
        stepStartTime = millis();
        currentStep = 1.0f;
        currentState = S_FEEDING;  // Go to FEEDING state to handle the sequence
        stateStartTime = millis();
    }
} 