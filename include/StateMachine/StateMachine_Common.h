#pragma once

#include <Arduino.h>
#include <Bounce2.h>
#include "ServoControl.h"

//* ************************************************************************
//* ********************** STATE ENUMERATION *******************************
//* ************************************************************************
enum State {
    S_NONE,
    S_IDLE,
    S_FEEDING,
    S_FLIPPING,
    S_FEEDING2
};

//* ************************************************************************
//* ********************** GLOBAL VARIABLES ********************************
//* ************************************************************************

//! ********************** INPUT DEBOUNCERS ********************************
extern Bounce startSensorDebouncer;
extern Bounce manualStartDebouncer;

//! ********************** SERVO CONTROL ***********************************
extern ServoControl flipServo;
extern float SERVO_HOME_ANGLE;

//! ********************** STATE MACHINE VARIABLES *************************
extern State currentState;
extern unsigned long stateStartTime;
extern unsigned long stepStartTime;
extern float currentStep;

//! ********************** ESP-NOW ******************************************
extern volatile bool espNowStartReceived;

//! ********************** RUNTIME-TUNABLE STATE CONFIG ********************
extern float IDLE_HOME_OFFSET;       // 00_IDLE.cpp
extern float ESPNOW_KICKOFF_OFFSET;  // 00_IDLE.cpp
extern float FEEDING_DURATION_MS;    // 01_FEEDING.cpp

//* ************************************************************************
//* ********************** HELPER FUNCTIONS ********************************
//* ************************************************************************
void log_state_step(const char* message);

