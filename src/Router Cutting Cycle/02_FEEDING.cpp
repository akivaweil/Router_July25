//* ************************************************************************
//* ************************ FEEDING STATE *******************************
//* ************************************************************************
//! FEEDING state handles the wood feeding process through the router
//! Sequence: Start delay -> Extend cylinder -> Retract cylinder

#include <Arduino.h>
#include "../../include/Config.h"
#include "../../include/Pins_Definitions.h"

//* ************************************************************************
//* ************************ FEEDING STATE VARIABLES *********************
//* ************************************************************************

// State timing variables
static unsigned long feedingStateStartTime = 0;
static unsigned long stepStartTime = 0;

// State flags and step tracking
static bool feedingStateInitialized = false;
static int currentStep = 0;

// Step definitions
enum FeedingSteps {
    STEP_START_DELAY = 1,
    STEP_EXTEND_CYLINDER,
    STEP_RETRACT_CYLINDER,
    STEP_COMPLETE
};

//* ************************************************************************
//* ************************ FORWARD DECLARATIONS ***********************
//* ************************************************************************

void executeStartDelay();
void executeExtendCylinder();
void executeRetractCylinder();
void completeFeeding();

//* ************************************************************************
//* ************************ FEEDING STATE FUNCTIONS *********************
//* ************************************************************************

//! Initialize the FEEDING state
void initFeedingState() {
    if (!feedingStateInitialized) {
        Serial.println();
        Serial.println("=== ENTERING FEEDING STATE ===");
        
        // Record state entry time
        feedingStateStartTime = millis();
        stepStartTime = millis();
        
        // Initialize step tracking
        currentStep = STEP_START_DELAY;
        
        // Set status indicators
        WRITE_PIN_LOW(READY_LED_PIN);
        WRITE_PIN_HIGH(RUNNING_LED_PIN);
        WRITE_PIN_LOW(ERROR_LED_PIN);
        
        Serial.println("Starting wood feeding sequence...");
        Serial.println("============================");
        
        feedingStateInitialized = true;
    }
}

//! Execute FEEDING state logic
void executeFeedingState() {
    switch (currentStep) {
        case STEP_START_DELAY:
            executeStartDelay();
            break;
            
        case STEP_EXTEND_CYLINDER:
            executeExtendCylinder();
            break;
            
        case STEP_RETRACT_CYLINDER:
            executeRetractCylinder();
            break;
            
        case STEP_COMPLETE:
            completeFeeding();
            break;
            
        default:
            Serial.println("ERROR: Unknown feeding step");
            currentStep = STEP_COMPLETE;
            break;
    }
}

//! ************************************************************************
//! STEP 1: START DELAY (50ms)
//! ************************************************************************
void executeStartDelay() {
    static bool stepInitialized = false;
    
    if (!stepInitialized) {
        Serial.println("Step 1: Start delay (50ms)");
        stepStartTime = millis();
        stepInitialized = true;
    }
    
    // Wait for start delay to complete
    if (millis() - stepStartTime >= FEEDING_START_DELAY) {
        Serial.println("Start delay complete");
        currentStep = STEP_EXTEND_CYLINDER;
        stepInitialized = false;
    }
}

//! ************************************************************************
//! STEP 2: EXTEND FEED CYLINDER (2 seconds)
//! ************************************************************************
void executeExtendCylinder() {
    static bool stepInitialized = false;
    
    if (!stepInitialized) {
        Serial.println("Step 2: Extending feed cylinder to push wood through router (2 seconds)");
        
        // Extend the feed cylinder (LOW signal)
        EXTEND_FEED_CYLINDER();
        
        stepStartTime = millis();
        stepInitialized = true;
    }
    
    // Wait for cylinder extend time to complete
    if (millis() - stepStartTime >= FEED_CYLINDER_EXTEND_TIME) {
        Serial.println("Feed cylinder extension complete");
        currentStep = STEP_RETRACT_CYLINDER;
        stepInitialized = false;
    }
}

//! ************************************************************************
//! STEP 3: RETRACT FEED CYLINDER (50ms)
//! ************************************************************************
void executeRetractCylinder() {
    static bool stepInitialized = false;
    
    if (!stepInitialized) {
        Serial.println("Step 3: Retracting feed cylinder to move out of the way (50ms)");
        
        // Retract the feed cylinder (HIGH signal)
        RETRACT_FEED_CYLINDER();
        
        stepStartTime = millis();
        stepInitialized = true;
    }
    
    // Wait for cylinder retract time to complete
    if (millis() - stepStartTime >= FEED_CYLINDER_RETRACT_TIME) {
        Serial.println("Feed cylinder retraction complete");
        currentStep = STEP_COMPLETE;
        stepInitialized = false;
    }
}

//! Complete the feeding sequence
void completeFeeding() {
    Serial.println("=== FEEDING SEQUENCE COMPLETE ===");
    Serial.print("Total feeding time: ");
    Serial.print(millis() - feedingStateStartTime);
    Serial.println(" ms");
    
    // Feeding state is complete - state machine will handle transition to flipping
}

//! Exit the FEEDING state and prepare for next state
void exitFeedingState() {
    Serial.println("=== EXITING FEEDING STATE ===");
    
    // Ensure cylinder is retracted for safety
    RETRACT_FEED_CYLINDER();
    
    // Reset state initialization flag
    feedingStateInitialized = false;
    currentStep = 0;
    
    Serial.println("FEEDING state exited successfully");
}

//! Check if feeding sequence is complete
bool isFeedingComplete() {
    return (currentStep == STEP_COMPLETE);
}

//! Get current feeding step
int getCurrentFeedingStep() {
    return currentStep;
}

//! Get time spent in FEEDING state
unsigned long getFeedingStateTime() {
    if (feedingStateInitialized) {
        return millis() - feedingStateStartTime;
    }
    return 0;
}

//! Emergency stop for feeding state
void emergencyStopFeeding() {
    Serial.println("EMERGENCY STOP - Immediately retracting feed cylinder");
    
    // Immediately retract cylinder for safety
    RETRACT_FEED_CYLINDER();
    
    // Reset state
    feedingStateInitialized = false;
    currentStep = 0;
} 