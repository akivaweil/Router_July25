#include "StateMachine/STATES/04_BOARD_END.h"
#include "StateMachine/StateMachine_Common.h"
#include "Config/Pins_Definitions.h"

//* ************************************************************************
//* ********************** BOARD END STATE HANDLER *************************
//* ************************************************************************
void handleBoardEndState() {
    log_state_step("State: BOARD END - Servo positioned for board end processing...");

    //! ************************************************************************
    //! SET SERVO TO BOARD END POSITION (HOME ANGLE + 30 DEGREES)
    //! ************************************************************************
    static bool servoPositioned = false;
    if (!servoPositioned) {
        float boardEndAngle = SERVO_HOME_ANGLE + 30.0f;
        // Ensure angle stays within valid range (0-180 degrees)
        if (boardEndAngle > 180.0f) {
            boardEndAngle = 180.0f;
        }
        flipServo.write(boardEndAngle);
        servoPositioned = true;
        Serial.print("Board end mode: Setting servo to ");
        Serial.print(boardEndAngle);
        Serial.println(" degrees");
    }

    //! ************************************************************************
    //! STAY IN BOARD END MODE UNTIL DEACTIVATED
    //! ************************************************************************
    // The servo stays at the board end position and does not rotate during cycles
    // This state is exited when boardEndModeActive is set to false from dashboard
    if (!boardEndModeActive) {
        Serial.println("Board end mode deactivated. Returning to IDLE state.");
        currentState = S_IDLE;
        stateStartTime = millis();
        currentStep = 1.0f;
        servoPositioned = false;  // Reset flag for next activation
    }
}
