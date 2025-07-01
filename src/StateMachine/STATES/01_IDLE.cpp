//* ************************************************************************
//* ************************ IDLE STATE **********************************
//* ************************************************************************
//! The IDLE state is where the machine waits for start commands or OTA uploads
//! This is the default state the system returns to after operations

#include <Arduino.h>
#include <WiFi.h>
#include "../../../include/Config.h"
#include "../../../include/Pins_Definitions.h"

//* ************************************************************************
//* ************************ IDLE STATE VARIABLES ***********************
//* ************************************************************************

// State timing variables
static unsigned long idleStateStartTime = 0;
static unsigned long lastStatusUpdate = 0;

// State flags
static bool idleStateInitialized = false;
static bool systemReady = false;

//* ************************************************************************
//* ************************ IDLE STATE FUNCTIONS ***********************
//* ************************************************************************

//! Initialize the IDLE state
void initIdleState() {
    if (!idleStateInitialized) {
        Serial.println();
        Serial.println("=== ENTERING IDLE STATE ===");
        
        // Record state entry time
        idleStateStartTime = millis();
        lastStatusUpdate = millis();
        
        // Initialize system ready flag
        systemReady = true;
        
        // Ensure feed cylinder is retracted in idle state
        RETRACT_FEED_CYLINDER();
        
        // Set status indicators
        WRITE_PIN_HIGH(READY_LED_PIN);
        WRITE_PIN_LOW(RUNNING_LED_PIN);
        WRITE_PIN_LOW(ERROR_LED_PIN);
        
        Serial.println("Router ready and waiting for start command");
        Serial.println("Feed cylinder retracted to safe position");
        Serial.println("============================");
        
        idleStateInitialized = true;
    }
}

//! Execute IDLE state logic
void executeIdleState() {
    // Update status periodically
    if (millis() - lastStatusUpdate > STATUS_UPDATE_INTERVAL) {
        Serial.print("IDLE - Router Ready | Uptime: ");
        Serial.print((millis() - idleStateStartTime) / 1000);
        Serial.println(" seconds");
        
        lastStatusUpdate = millis();
    }
    
    // Check for input conditions that would trigger state transitions
    checkIdleStateTransitions();
    
    // Perform any background tasks while idle
    performIdleBackgroundTasks();
}

//! Check for conditions that would cause state transitions from IDLE
void checkIdleStateTransitions() {
    // Check for start sensor activation (main trigger for cutting cycle)
    if (READ_PIN(START_SENSOR_PIN)) {
        Serial.println("Start sensor activated - beginning cutting cycle");
        // Transition to feeding state will be handled by state machine
        // This function just detects the condition
        return;
    }
    
    // Check for manual start button press
    if (READ_PIN(START_BUTTON_PIN)) {
        Serial.println("Manual start button pressed - beginning cutting cycle");
        // Transition logic will be added here
        return;
    }
    
    // Check for emergency stop
    if (READ_PIN(EMERGENCY_STOP_PIN)) {
        Serial.println("Emergency stop activated from IDLE state");
        // Emergency stop logic will be added here
        return;
    }
    
    // Check for mode changes
    if (READ_PIN(AUTO_MODE_PIN)) {
        Serial.println("Auto mode selected");
        // Mode change logic will be added here
    }
    
    if (READ_PIN(MANUAL_MODE_PIN)) {
        Serial.println("Manual mode selected");
        // Mode change logic will be added here
    }
}

//! Perform background tasks while in IDLE state
void performIdleBackgroundTasks() {
    // Monitor system health
    monitorSystemHealth();
    
    // Check sensor states
    checkSensorStates();
    
    // Maintain communication
    maintainCommunication();
    
    // Ensure safety systems are active
    maintainSafetySystems();
}

//! Monitor overall system health while idle
void monitorSystemHealth() {
    static unsigned long lastHealthCheck = 0;
    
    if (millis() - lastHealthCheck > 10000) { // Every 10 seconds
        // Check free memory
        if (ESP.getFreeHeap() < 10000) { // Less than 10KB free
            Serial.println("WARNING: Low memory detected");
        }
        
        // Check WiFi connection
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WARNING: WiFi connection lost");
            systemReady = false;
        } else {
            systemReady = true;
        }
        
        lastHealthCheck = millis();
    }
}

//! Check sensor states for any unexpected conditions
void checkSensorStates() {
    // Check limit switches
    static bool lastLimitStates[4] = {false, false, false, false};
    bool currentLimitStates[4];
    
    currentLimitStates[0] = READ_PIN(LIMIT_SWITCH_1_PIN);
    currentLimitStates[1] = READ_PIN(LIMIT_SWITCH_2_PIN);
    currentLimitStates[2] = READ_PIN(LIMIT_SWITCH_3_PIN);
    currentLimitStates[3] = READ_PIN(LIMIT_SWITCH_4_PIN);
    
    // Report any changes in limit switch states
    for (int i = 0; i < 4; i++) {
        if (currentLimitStates[i] != lastLimitStates[i]) {
            Serial.print("Limit Switch ");
            Serial.print(i + 1);
            Serial.print(" state changed to: ");
            Serial.println(currentLimitStates[i] ? "ACTIVE" : "INACTIVE");
            lastLimitStates[i] = currentLimitStates[i];
        }
    }
    
    // Check start sensor state changes
    static bool lastStartSensorState = false;
    bool currentStartSensorState = READ_PIN(START_SENSOR_PIN);
    
    if (currentStartSensorState != lastStartSensorState) {
        Serial.print("Start Sensor state changed to: ");
        Serial.println(currentStartSensorState ? "ACTIVE" : "INACTIVE");
        lastStartSensorState = currentStartSensorState;
    }
}

//! Maintain communication systems while idle
void maintainCommunication() {
    // This function can be expanded to handle communication protocols
    // Currently just ensures basic connectivity is maintained
    
    static unsigned long lastCommCheck = 0;
    
    if (millis() - lastCommCheck > 30000) { // Every 30 seconds
        if (WiFi.status() == WL_CONNECTED) {
            Serial.print("Communication OK - IP: ");
            Serial.println(WiFi.localIP());
        }
        lastCommCheck = millis();
    }
}

//! Maintain safety systems while idle
void maintainSafetySystems() {
    static unsigned long lastSafetyCheck = 0;
    
    if (millis() - lastSafetyCheck > 5000) { // Every 5 seconds
        // Ensure feed cylinder is in safe retracted position
        RETRACT_FEED_CYLINDER();
        
        // Check emergency stop system
        if (READ_PIN(EMERGENCY_STOP_PIN)) {
            Serial.println("Emergency stop detected during safety check");
        }
        
        lastSafetyCheck = millis();
    }
}

//! Exit the IDLE state and prepare for next state
void exitIdleState() {
    Serial.println("=== EXITING IDLE STATE ===");
    
    // Turn off ready LED, turn on running LED
    WRITE_PIN_LOW(READY_LED_PIN);
    WRITE_PIN_HIGH(RUNNING_LED_PIN);
    
    // Reset state initialization flag
    idleStateInitialized = false;
    
    Serial.println("IDLE state exited successfully");
}

//! Get the current system ready status
bool isSystemReady() {
    return systemReady;
}

//! Get time spent in IDLE state
unsigned long getIdleStateTime() {
    if (idleStateInitialized) {
        return millis() - idleStateStartTime;
    }
    return 0;
}

//! Check if start command is active
bool isStartCommandActive() {
    return READ_PIN(START_SENSOR_PIN) || READ_PIN(START_BUTTON_PIN);
} 