#include "StateMachine/STATES/00_IDLE.h"
#include "StateMachine/StateMachine_Common.h"
#include "Config/Pins_Definitions.h"

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
    //! CHECK FOR START SIGNAL (ESP-NOW from Stage 2, sensor, or manual → full cycle)
    //! ************************************************************************
    if (espNowStartReceived || startSensorDebouncer.read() || manualStartDebouncer.read()) {
        espNowStartReceived = false;
        Serial.println("Start signal received! Transitioning to FEEDING state.");
        currentState = S_FEEDING;
        stateStartTime = millis();
        currentStep = 1.0f;
        servoHomed = false;
    }
}

