//* ************************************************************************
//* ************************ 01_IDLE STATE *******************************
//* ************************************************************************
//! IDLE State: Waiting for start command from sensor or manual button
//! Also handles OTA uploads during idle time

#include <Arduino.h>
#include "../include/Config.h"
#include "../include/Pins_Definitions.h"
#include <Bounce2.h>

//* ************************************************************************
//* ************************ IDLE STATE VARIABLES ************************
//* ************************************************************************

static bool idleInitialized = false;

// Bounce2 debouncer for manual start button
static Bounce manualStartButton = Bounce();

//* ************************************************************************
//* ************************ FORWARD DECLARATIONS ***********************
//* ************************************************************************

void resetIdleState();

//* ************************************************************************
//* ************************ IDLE STATE FUNCTIONS ************************
//* ************************************************************************

//! Initialize IDLE state
void initIdleState() {
    if (!idleInitialized) {
        Serial.println("=== ENTERING IDLE STATE ===");
        Serial.println("Waiting for start command from stage 2 machine or manual button...");
        
        // Configure input pins
        configureInputPulldown(START_SENSOR_PIN);
        configureInputPulldown(MANUAL_START_PIN);
        
        // Configure output pins
        configureOutput(FEED_CYLINDER_PIN);
        
        // Ensure feed cylinder is retracted (safe position)
        writePinHigh(FEED_CYLINDER_PIN);
        
        // Setup manual start button with Bounce2 debouncer
        manualStartButton.attach(MANUAL_START_PIN);
        manualStartButton.interval(MANUAL_START_DEBOUNCE);
        
        Serial.println("IDLE state initialized - System ready");
        idleInitialized = true;
    }
}

//! Execute IDLE state logic
void executeIdleState() {
    // Update debouncer
    manualStartButton.update();
    
    // Check for start signal from stage 2 machine
    if (readPin(START_SENSOR_PIN)) {
        Serial.println("Start signal received from stage 2 machine");
        return;
    }
    
    // Check for manual start button press (with debounce)
    if (manualStartButton.rose()) {
        Serial.println("Manual start button pressed");
        return;
    }
}

//! Check if IDLE state should transition to next state
bool shouldExitIdleState() {
    // Update debouncer
    manualStartButton.update();
    
    // Check both start conditions
    return (readPin(START_SENSOR_PIN) || manualStartButton.rose());
}

//! Reset IDLE state for next cycle
void resetIdleState() {
    Serial.println("=== EXITING IDLE STATE ===");
    idleInitialized = false;
}

 