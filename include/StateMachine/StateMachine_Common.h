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

//! ********************** STATE MACHINE VARIABLES *************************
extern State currentState;
extern unsigned long stateStartTime;
extern unsigned long stepStartTime;
extern float currentStep;

//* ************************************************************************
//* ********************** HELPER FUNCTIONS ********************************
//* ************************************************************************
void log_state_step(const char* message);

