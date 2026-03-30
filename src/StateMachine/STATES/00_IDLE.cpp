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
    //! CHECK FOR START SIGNAL (sensor, manual, or ESP-NOW from Stage 2)
    //! ************************************************************************
    if (startSensorDebouncer.read() || manualStartDebouncer.read() || espNowStartReceived) {
        Serial.println("Start signal received! Transitioning to FEEDING state.");
        espNowStartReceived = false;  // Clear ESP-NOW flag
        currentState = S_FEEDING;
        stateStartTime = millis();
        currentStep = 1.0f;
        servoHomed = false;
    }
}

