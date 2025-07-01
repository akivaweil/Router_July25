//* ************************************************************************
//* ************************ STATE MACHINE MANAGER **********************
//* ************************************************************************
//! Central state machine manager for ESP32 Router Control System
//! Coordinates all state transitions and manages system flow

#include <Arduino.h>
#include "../../../include/Config.h"
#include "../../../include/Pins_Definitions.h"

// Component includes
#include "../../../include/FeedCylinder.h"
#include "../../../include/FlipServo.h"
#include "../../../include/StartSensor.h"

// State includes
#include "../../../include/IdleState.h"
#include "../../../include/FeedingState.h"
#include "../../../include/FlippingState.h"

//* ************************************************************************
//* ************************ STATE DEFINITIONS ***************************
//* ************************************************************************

// State enumeration
enum SystemState {
    STATE_IDLE = 1,
    STATE_FEEDING = 2,
    STATE_FLIPPING = 3,
    STATE_ERROR = 4,
    STATE_EMERGENCY_STOP = 5,
    STATE_SHUTDOWN = 6
};

//* ************************************************************************
//* ************************ STATE MACHINE VARIABLES ********************
//* ************************************************************************

// Current state tracking
static SystemState currentState = STATE_IDLE;
static SystemState previousState = STATE_IDLE;
static SystemState stateHistory[MAX_STATE_HISTORY];
static int stateHistoryIndex = 0;

// State timing
static unsigned long stateStartTime = 0;
static unsigned long lastStateUpdate = 0;

// State machine flags
static bool stateMachineInitialized = false;
static bool emergencyStopActive = false;
static bool systemPaused = false;

//* ************************************************************************
//* ************************ FORWARD DECLARATIONS ***********************
//* ************************************************************************

// State functions from individual state files
void initIdleState();
void executeIdleState();
void exitIdleState();
bool isSystemReady();
bool isStartCommandActive();

void initFeedingState();
void executeFeedingState();
void exitFeedingState();
bool isFeedingComplete();
void emergencyStopFeeding();

void initFlippingState();
void executeFlippingState();
void exitFlippingState();
bool isFlippingComplete();
void emergencyStopFlipping();
void detachFlipServo();

// State machine functions
void initializeStateMachine();
void updateStateMachine();
void transitionToState(SystemState newState);
void recordStateHistory(SystemState state);
void handleEmergencyStop();
void handleSystemPause();
void exitCurrentState();
void initializeCurrentState();
void handleEmergencyStopState();
void handleShutdownState();
const char* getStateName(SystemState state);

//* ************************************************************************
//* ************************ STATE MACHINE INITIALIZATION ***************
//* ************************************************************************

//! Initialize the state machine system
void initializeStateMachine() {
    if (!stateMachineInitialized) {
        Serial.println();
        Serial.println("=== INITIALIZING STATE MACHINE ===");
        
        // Initialize state history
        for (int i = 0; i < MAX_STATE_HISTORY; i++) {
            stateHistory[i] = STATE_IDLE;
        }
        
        // Set initial state
        currentState = STATE_IDLE;
        previousState = STATE_IDLE;
        stateStartTime = millis();
        lastStateUpdate = millis();
        
        // Initialize hardware pins for state management
        configureOutput(STATUS_LED_PIN);
        configureOutput(ERROR_LED_PIN);
        configureOutput(READY_LED_PIN);
        configureOutput(RUNNING_LED_PIN);
        
        // Initialize router-specific output pins
        configureOutput(FEED_CYLINDER_PIN);
        configureOutput(FLIP_SERVO_PIN);
        
        // Initialize input pins
        configureInputPulldown(START_BUTTON_PIN);
        configureInputPulldown(STOP_BUTTON_PIN);
        configureInputPullup(EMERGENCY_STOP_PIN);
        configureInputPulldown(RESET_BUTTON_PIN);
        configureInputPulldown(START_SENSOR_PIN);
        
        // Record initial state
        recordStateHistory(currentState);
        
        Serial.println("State machine initialized successfully");
        Serial.println("Starting in IDLE state");
        Serial.println("===============================");
        
        stateMachineInitialized = true;
    }
}

//* ************************************************************************
//* ************************ STATE MACHINE EXECUTION ********************
//* ************************************************************************

//! Main state machine update function (call from main loop)
void updateStateMachine() {
    // Check for emergency stop first (highest priority)
    if (readPin(EMERGENCY_STOP_PIN) && !emergencyStopActive) {
        handleEmergencyStop();
        return;
    }
    
    // Check for system pause
    if (readPin(STOP_BUTTON_PIN) && !systemPaused) {
        handleSystemPause();
    }
    
    // Execute current state logic and handle transitions
    switch (currentState) {
        case STATE_IDLE:
            executeIdleState();
            // Check for transition to feeding state
            if (isStartSensorRisingEdge()) {
                transitionToState(STATE_FEEDING);
            }
            break;
            
        case STATE_FEEDING:
            executeFeedingState();
            // Check for transition to flipping state
            if (isFeedingStateComplete()) {
                transitionToState(STATE_FLIPPING);
            }
            break;
            
        case STATE_FLIPPING:
            executeFlippingState();
            // Check for transition back to idle state
            if (isFlippingStateComplete()) {
                transitionToState(STATE_IDLE);
            }
            break;
            
        case STATE_ERROR:
            // Error state logic will be added here
            Serial.println("ERROR state - logic to be implemented");
            // For now, transition back to idle after a delay
            if (millis() - stateStartTime > 5000) {
                transitionToState(STATE_IDLE);
            }
            break;
            
        case STATE_EMERGENCY_STOP:
            // Emergency stop state logic
            handleEmergencyStopState();
            break;
            
        case STATE_SHUTDOWN:
            // Shutdown state logic
            handleShutdownState();
            break;
            
        default:
            Serial.println("Unknown state - transitioning to IDLE");
            transitionToState(STATE_IDLE);
            break;
    }
    
    // Update state timing
    lastStateUpdate = millis();
}

//* ************************************************************************
//* ************************ STATE TRANSITION FUNCTIONS *****************
//* ************************************************************************

//! Transition to a new state
void transitionToState(SystemState newState) {
    if (newState != currentState) {
        Serial.println();
        Serial.print("STATE TRANSITION: ");
        Serial.print(getStateName(currentState));
        Serial.print(" -> ");
        Serial.println(getStateName(newState));
        
        // Exit current state
        exitCurrentState();
        
        // Record state change
        previousState = currentState;
        currentState = newState;
        stateStartTime = millis();
        
        // Record in history
        recordStateHistory(newState);
        
        // Initialize new state
        initializeCurrentState();
        
        Serial.println("State transition completed");
        Serial.println();
    }
}

//! Exit the current state
void exitCurrentState() {
    switch (currentState) {
        case STATE_IDLE:
            exitIdleState();
            break;
            
        case STATE_FEEDING:
            exitFeedingState();
            break;
            
        case STATE_FLIPPING:
            exitFlippingState();
            break;
            
        default:
            break;
    }
}

//! Initialize the current state
void initializeCurrentState() {
    switch (currentState) {
        case STATE_IDLE:
            initIdleState();
            break;
            
        case STATE_FEEDING:
            initFeedingState();
            break;
            
        case STATE_FLIPPING:
            initFlippingState();
            break;
            
        default:
            break;
    }
}

//* ************************************************************************
//* ************************ UTILITY FUNCTIONS **************************
//* ************************************************************************

//! Record state in history
void recordStateHistory(SystemState state) {
    stateHistory[stateHistoryIndex] = state;
    stateHistoryIndex = (stateHistoryIndex + 1) % MAX_STATE_HISTORY;
}

//! Get state name as string
const char* getStateName(SystemState state) {
    switch (state) {
        case STATE_IDLE: return "IDLE";
        case STATE_FEEDING: return "FEEDING";
        case STATE_FLIPPING: return "FLIPPING";
        case STATE_ERROR: return "ERROR";
        case STATE_EMERGENCY_STOP: return "EMERGENCY_STOP";
        case STATE_SHUTDOWN: return "SHUTDOWN";
        default: return "UNKNOWN";
    }
}

//! Handle emergency stop condition
void handleEmergencyStop() {
    Serial.println("!!! EMERGENCY STOP ACTIVATED !!!");
    emergencyStopActive = true;
    
    // Emergency stop specific actions based on current state
    switch (currentState) {
        case STATE_FEEDING:
            emergencyStopFeedingState();
            break;
        case STATE_FLIPPING:
            emergencyStopFlippingState();
            break;
        default:
            break;
    }
    
    transitionToState(STATE_EMERGENCY_STOP);
    
    // Set error indicators
    writePinHigh(ERROR_LED_PIN);
    writePinLow(RUNNING_LED_PIN);
    writePinLow(READY_LED_PIN);
}

//! Handle emergency stop state
void handleEmergencyStopState() {
    static unsigned long emergencyStartTime = 0;
    static bool emergencyInitialized = false;
    
    if (!emergencyInitialized) {
        emergencyStartTime = millis();
        emergencyInitialized = true;
        Serial.println("Emergency stop state active - system halted");
        
        // Ensure all systems are in safe state
        emergencyRetractFeedCylinder();
        emergencyStopFlipServo();
    }
    
    // Check if emergency stop is released
    if (!readPin(EMERGENCY_STOP_PIN)) {
        Serial.println("Emergency stop released - resetting system");
        emergencyStopActive = false;
        emergencyInitialized = false;
        transitionToState(STATE_IDLE);
    }
    
    // Periodic safety message
    if (millis() - emergencyStartTime > 10000) {
        Serial.println("EMERGENCY STOP ACTIVE - Release emergency stop to resume");
        emergencyStartTime = millis();
    }
}

//! Handle system pause
void handleSystemPause() {
    Serial.println("System pause requested");
    systemPaused = true;
    
    // For router system, pause means return to idle
    transitionToState(STATE_IDLE);
}

//! Handle shutdown state
void handleShutdownState() {
    static bool shutdownInitialized = false;
    
    if (!shutdownInitialized) {
        Serial.println("=== SYSTEM SHUTDOWN ===");
        
        // Safe shutdown procedures
        emergencyRetractFeedCylinder();
        emergencyStopFlipServo();
        
        // Turn off all LEDs except error LED
        writePinLow(READY_LED_PIN);
        writePinLow(RUNNING_LED_PIN);
        writePinHigh(ERROR_LED_PIN);
        
        Serial.println("System shutdown complete");
        shutdownInitialized = true;
    }
    
    // Stay in shutdown state until reset
}

//! Get current state
SystemState getCurrentState() {
    return currentState;
}

//! Get time in current state
unsigned long getTimeInCurrentState() {
    return millis() - stateStartTime;
}

//! Check if state machine is initialized
bool isStateMachineInitialized() {
    return stateMachineInitialized;
}

//! Get current state as string for display
const char* getCurrentStateName() {
    return getStateName(currentState);
} 