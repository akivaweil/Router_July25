//* ************************************************************************
//* ************************ FEED CYLINDER COMPONENT ********************
//* ************************************************************************
//! Feed cylinder control component for ESP32 Router Control System
//! Handles all feed cylinder operations with safety and timing controls

#include <Arduino.h>
#include "../../include/Config.h"
#include "../../include/Pins_Definitions.h"

//* ************************************************************************
//* ************************ FEED CYLINDER VARIABLES ********************
//* ************************************************************************

static bool feedCylinderInitialized = false;
static bool feedCylinderExtended = false;
static unsigned long lastOperationTime = 0;

//* ************************************************************************
//* ************************ FEED CYLINDER FUNCTIONS ********************
//* ************************************************************************

//! Initialize the feed cylinder component
void initFeedCylinder() {
    if (!feedCylinderInitialized) {
        // Configure feed cylinder pin as output
        configureOutput(FEED_CYLINDER_PIN);
        
        // Start in safe retracted position (HIGH signal)
        writePinHigh(FEED_CYLINDER_PIN);
        feedCylinderExtended = false;
        lastOperationTime = millis();
        
        Serial.println("Feed Cylinder initialized - retracted to safe position");
        feedCylinderInitialized = true;
    }
}

//! Extend the feed cylinder (LOW signal)
void extendFeedCylinder() {
    if (!feedCylinderInitialized) {
        Serial.println("ERROR: Feed cylinder not initialized");
        return;
    }
    
    // Extend the cylinder
    writePinLow(FEED_CYLINDER_PIN);
    feedCylinderExtended = true;
    lastOperationTime = millis();
    
    Serial.println("Feed Cylinder: EXTENDED (pushing wood through router)");
}

//! Retract the feed cylinder (HIGH signal)
void retractFeedCylinder() {
    if (!feedCylinderInitialized) {
        // Allow retraction even if not initialized for safety
        configureOutput(FEED_CYLINDER_PIN);
    }
    
    // Retract the cylinder
    writePinHigh(FEED_CYLINDER_PIN);
    feedCylinderExtended = false;
    lastOperationTime = millis();
    
    Serial.println("Feed Cylinder: RETRACTED (safe position)");
}

//! Check if feed cylinder is extended
bool isFeedCylinderExtended() {
    return feedCylinderExtended;
}

//! Get time since last operation
unsigned long getTimeSinceLastOperation() {
    return millis() - lastOperationTime;
}

//! Emergency retract (immediate safety function)
void emergencyRetractFeedCylinder() {
    Serial.println("EMERGENCY: Immediately retracting feed cylinder");
    
    // Force retraction regardless of initialization state
    configureOutput(FEED_CYLINDER_PIN);
    writePinHigh(FEED_CYLINDER_PIN);
    feedCylinderExtended = false;
    lastOperationTime = millis();
}

//! Perform timed extension (extends for specified duration then retracts)
void timedExtendFeedCylinder(unsigned long extendTime) {
    if (!feedCylinderInitialized) {
        Serial.println("ERROR: Feed cylinder not initialized");
        return;
    }
    
    Serial.print("Feed Cylinder: Timed extension for ");
    Serial.print(extendTime);
    Serial.println(" ms");
    
    // Extend cylinder
    extendFeedCylinder();
    
    // Wait for specified time
    delay(extendTime);
    
    // Retract cylinder
    retractFeedCylinder();
}

//! Check feed cylinder component status
bool checkFeedCylinderStatus() {
    if (!feedCylinderInitialized) {
        return false;
    }
    
    // Could add sensor feedback here if available
    // For now, just return based on commanded state
    return true;
}

//! Get feed cylinder state as string
const char* getFeedCylinderState() {
    if (!feedCylinderInitialized) {
        return "NOT_INITIALIZED";
    }
    
    return feedCylinderExtended ? "EXTENDED" : "RETRACTED";
} 