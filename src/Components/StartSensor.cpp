//* ************************************************************************
//* ************************ START SENSOR COMPONENT *********************
//* ************************************************************************
//! Start sensor control component for ESP32 Router Control System
//! Handles sensor reading with debouncing and state monitoring

#include <Arduino.h>
#include "../../include/Config.h"
#include "../../include/Pins_Definitions.h"

//* ************************************************************************
//* ************************ START SENSOR VARIABLES *********************
//* ************************************************************************

static bool sensorInitialized = false;
static bool currentState = false;
static bool lastState = false;
static bool stableState = false;
static unsigned long lastChangeTime = 0;
static unsigned long lastReadTime = 0;
static int stableReadings = 0;

//* ************************************************************************
//* ************************ START SENSOR FUNCTIONS *********************
//* ************************************************************************

//! Initialize the start sensor component
void initStartSensor() {
    if (!sensorInitialized) {
        // Configure sensor pin as input with pulldown (Active HIGH)
        configureInputPulldown(START_SENSOR_PIN);
        
        // Read initial state
        currentState = readPin(START_SENSOR_PIN);
        lastState = currentState;
        stableState = currentState;
        lastChangeTime = millis();
        lastReadTime = millis();
        
        Serial.print("Start Sensor initialized - Initial state: ");
        Serial.println(currentState ? "ACTIVE" : "INACTIVE");
        sensorInitialized = true;
    }
}

//! Read start sensor with debouncing
bool readStartSensor() {
    if (!sensorInitialized) {
        Serial.println("ERROR: Start sensor not initialized");
        return false;
    }
    
    // Check if enough time has passed for next reading
    if (millis() - lastReadTime < SENSOR_READ_INTERVAL) {
        return stableState;
    }
    
    lastReadTime = millis();
    
    // Read current pin state
    bool rawState = readPin(START_SENSOR_PIN);
    
    // Check if state has changed
    if (rawState != lastState) {
        // State changed, reset debounce timer
        lastChangeTime = millis();
        stableReadings = 0;
        lastState = rawState;
    } else {
        // State is same as last reading
        stableReadings++;
    }
    
    // Check if state has been stable long enough
    if ((millis() - lastChangeTime > DEBOUNCE_DELAY) && 
        (stableReadings >= SENSOR_STABLE_COUNT)) {
        
        // State change confirmed
        if (rawState != stableState) {
            stableState = rawState;
            
            Serial.print("Start Sensor state changed to: ");
            Serial.println(stableState ? "ACTIVE" : "INACTIVE");
        }
    }
    
    currentState = rawState;
    return stableState;
}

//! Check if start sensor is currently active
bool isStartSensorActive() {
    return readStartSensor();
}

//! Check if start sensor just became active (rising edge)
bool isStartSensorRisingEdge() {
    static bool lastStableState = false;
    bool currentStableState = readStartSensor();
    
    bool risingEdge = (currentStableState && !lastStableState);
    lastStableState = currentStableState;
    
    if (risingEdge) {
        Serial.println("Start Sensor: RISING EDGE detected");
    }
    
    return risingEdge;
}

//! Check if start sensor just became inactive (falling edge)
bool isStartSensorFallingEdge() {
    static bool lastStableState = false;
    bool currentStableState = readStartSensor();
    
    bool fallingEdge = (!currentStableState && lastStableState);
    lastStableState = currentStableState;
    
    if (fallingEdge) {
        Serial.println("Start Sensor: FALLING EDGE detected");
    }
    
    return fallingEdge;
}

//! Get raw sensor reading (without debouncing)
bool readStartSensorRaw() {
    if (!sensorInitialized) {
        return false;
    }
    
    return readPin(START_SENSOR_PIN);
}

//! Get time since last state change
unsigned long getTimeSinceLastChange() {
    return millis() - lastChangeTime;
}

//! Wait for start sensor activation
bool waitForStartSensorActivation(unsigned long timeout) {
    Serial.println("Waiting for start sensor activation...");
    
    unsigned long startTime = millis();
    
    while (millis() - startTime < timeout) {
        if (isStartSensorRisingEdge()) {
            Serial.println("Start sensor activated!");
            return true;
        }
        delay(10);
    }
    
    Serial.println("Timeout waiting for start sensor");
    return false;
}

//! Check sensor component status
bool checkStartSensorStatus() {
    if (!sensorInitialized) {
        return false;
    }
    
    // Read sensor to verify it's responsive
    readStartSensor();
    return true;
}

//! Get sensor state as string
const char* getStartSensorState() {
    if (!sensorInitialized) {
        return "NOT_INITIALIZED";
    }
    
    return stableState ? "ACTIVE" : "INACTIVE";
}

//! Get sensor statistics for debugging
void printStartSensorStats() {
    Serial.println("=== START SENSOR STATISTICS ===");
    Serial.print("Initialized: ");
    Serial.println(sensorInitialized ? "YES" : "NO");
    Serial.print("Current State: ");
    Serial.println(getStartSensorState());
    Serial.print("Raw Reading: ");
    Serial.println(readStartSensorRaw() ? "HIGH" : "LOW");
    Serial.print("Stable Readings: ");
    Serial.println(stableReadings);
    Serial.print("Time Since Last Change: ");
    Serial.print(getTimeSinceLastChange());
    Serial.println(" ms");
    Serial.println("===============================");
} 