#pragma once

#include "config/Config.h"

//* ************************************************************************
//* ************************ IDLE STATE ************************************
//* ************************************************************************

//! ********************** EXTERNAL DECLARATIONS ***************************
extern Bounce startSensorDebouncer;
extern Bounce manualStartDebouncer;
extern ServoControl flipServo;
extern State currentState;
extern unsigned long stateStartTime;
extern float currentStep;
void log_state_step(const char* message);

//* ************************************************************************
//* ************************ IDLE STATE HANDLER ****************************
//* ************************************************************************
void handleIdleState() {
    log_state_step("State: IDLE - Waiting for start signal...");

    //! ************************************************************************
    //! ENSURE SERVO IS IN HOME POSITION
    //! ************************************************************************
    flipServo.write(SERVO_HOME_ANGLE);

    //! ************************************************************************
    //! CHECK FOR START SIGNAL
    //! ************************************************************************
    if (startSensorDebouncer.read() || manualStartDebouncer.read()) {
        Serial.println("Start signal received! Transitioning to FEEDING state.");
        currentState = S_FEEDING;  // Go to FEEDING state
        stateStartTime = millis();
        currentStep = 1.0f;
    }
} 