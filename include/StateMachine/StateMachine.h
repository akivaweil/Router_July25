#pragma once

#include <Arduino.h>
#include <Bounce2.h>
#include "ServoControl.h"

// State enumeration
enum SystemState {
    STATE_NONE,
    STATE_IDLE,
    STATE_FEEDING,
    STATE_FLIPPING,
    STATE_FEEDING2
};

// Global variables

// Input debouncers
extern Bounce startSensorDebouncer;
extern Bounce manualStartDebouncer;

// Servo control
extern ServoControl flipServo;
extern float SERVO_HOME_ANGLE;

// State machine variables
extern SystemState currentState;
extern unsigned long stateStartTime;
extern unsigned long stepStartTime;
extern float currentStep;

// ESP-NOW
extern volatile bool espNowStartReceived;

// Runtime-tunable state config
extern float IDLE_HOME_OFFSET;       // 00_IDLE.cpp
extern float ESPNOW_KICKOFF_OFFSET;  // 00_IDLE.cpp
extern float FEEDING_DURATION_MS;    // 01_FEEDING.cpp

// Helper functions
void log_state_step(const char* message);
