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
extern WebDashboard dashboard;
void log_state_step(const char* message);

//* ************************************************************************
//* ************************ IDLE STATE HANDLER ****************************
//* ************************************************************************
void handleIdleState() {
    log_state_step("State: IDLE - Waiting for start signal...");

    //! ************************************************************************
    //! ENSURE SERVO IS IN HOME POSITION (ONLY WHEN ENTERING IDLE STATE)
    //! ************************************************************************
    static bool servoHomed = false;
    if (!servoHomed) {
        flipServo.write(SERVO_HOME_ANGLE);
        servoHomed = true;
    }

    //! ************************************************************************
    //! CHECK FOR START SIGNAL
    //! ************************************************************************
    if (startSensorDebouncer.read() || manualStartDebouncer.read()) {
        Serial.println("Start signal received! Transitioning to FEEDING state.");
        
        //! ************************************************************************
        //! RECORD TRIGGER FOR STATISTICS
        //! ************************************************************************
        dashboard.recordTrigger();
        
        currentState = S_FEEDING;  // Go to FEEDING state
        stateStartTime = millis();
        currentStep = 1.0f;
        servoHomed = false;  // Reset flag for next time we return to IDLE
    }
} 