//* ************************************************************************
//* ************************ MAIN ROUTER CONTROL *************************
//* ************************************************************************
//! Barebones ESP32 Router Control System
//! State Flow: IDLE -> FEEDING -> FLIPPING -> IDLE

#include <Arduino.h>
#include "../include/Config.h"
#include "../include/Pins_Definitions.h"

//* ************************************************************************
//* ************************ STATE MACHINE DEFINITIONS *******************
//* ************************************************************************

// State enumeration
enum SystemState {
    STATE_IDLE = 1,
    STATE_FEEDING,
    STATE_FLIPPING
};

// Current system state
static SystemState currentState = STATE_IDLE;

//* ************************************************************************
//* ************************ FUNCTION DECLARATIONS ***********************
//* ************************************************************************

// IDLE state functions (01_IDLE.cpp)
void initIdleState();
void executeIdleState();
bool shouldExitIdleState();
void resetIdleState();

// FEEDING state functions (Router Cutting Cycle/02_FEEDING.cpp)
void initFeedingState();
void executeFeedingState();
bool isFeedingComplete();
void resetFeedingState();

// FLIPPING state functions (Router Cutting Cycle/03_FLIPPING.cpp)
void initFlippingState();
void executeFlippingState();
bool isFlippingComplete();
void resetFlippingState();

//* ************************************************************************
//* ************************ MAIN SETUP AND LOOP *************************
//* ************************************************************************

//! Arduino setup function
void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    delay(1000); // Allow serial to initialize
    
    Serial.println();
    Serial.println("===========================================");
    Serial.println("ESP32 Router Control System - Barebones");
    Serial.println("===========================================");
    Serial.println("State Flow: IDLE -> FEEDING -> FLIPPING -> IDLE");
    Serial.println();
    
    // Initialize with IDLE state
    currentState = STATE_IDLE;
    initIdleState();
    
    Serial.println("System initialized and ready");
}

//! Arduino main loop function
void loop() {
    // Execute current state
    switch (currentState) {
        case STATE_IDLE:
            executeIdleState();
            
            // Check for transition to FEEDING
            if (shouldExitIdleState()) {
                currentState = STATE_FEEDING;
                initFeedingState();
            }
            break;
            
        case STATE_FEEDING:
            executeFeedingState();
            
            // Check for transition to FLIPPING
            if (isFeedingComplete()) {
                resetFeedingState();
                currentState = STATE_FLIPPING;
                initFlippingState();
            }
            break;
            
        case STATE_FLIPPING:
            executeFlippingState();
            
            // Check for transition back to IDLE
            if (isFlippingComplete()) {
                resetFlippingState();
                currentState = STATE_IDLE;
                initIdleState();
            }
            break;
            
        default:
            // Should never reach here, but safety fallback
            Serial.println("ERROR: Unknown state, returning to IDLE");
            currentState = STATE_IDLE;
            initIdleState();
            break;
    }
    
    // Small delay to prevent excessive CPU usage
    delay(10);
}