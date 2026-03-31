#include "StateMachine/STATES/00_IDLE.h"
#include "StateMachine/StateMachine_Common.h"
#include "Config/Pins_Definitions.h"

//! *** TEMP: cylinder pulse duration on ESP-NOW signal ***
static const unsigned long ESPNOW_CYLINDER_PULSE_MS = 100;

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
    //! TEMP: ESP-NOW FROM STAGE 2 → PULSE CYLINDER FOR 100ms, NO FULL CYCLE
    //! ************************************************************************
    static bool cylinderPulsing = false;
    static unsigned long cylinderPulseStart = 0;

    if (cylinderPulsing) {
        if (millis() - cylinderPulseStart >= ESPNOW_CYLINDER_PULSE_MS) {
            digitalWrite(FEED_CYLINDER_PIN, LOW); // retract back to safe
            cylinderPulsing = false;
        }
        return;
    }

    if (espNowStartReceived) {
        espNowStartReceived = false;
        digitalWrite(FEED_CYLINDER_PIN, HIGH); // extend/push
        cylinderPulseStart = millis();
        cylinderPulsing = true;
        return;
    }

    //! ************************************************************************
    //! CHECK FOR START SIGNAL (sensor or manual → full cycle)
    //! ************************************************************************
    if (startSensorDebouncer.read() || manualStartDebouncer.read()) {
        Serial.println("Start signal received! Transitioning to FEEDING state.");
        currentState = S_FEEDING;
        stateStartTime = millis();
        currentStep = 1.0f;
        servoHomed = false;
    }
}

