//* ************************************************************************
//* ************************ FEEDING STATE *******************************
//* ************************************************************************
//! FEEDING State: Extends feed cylinder to push wood through router
//! Executes the feeding sequence with precise timing

#include <Arduino.h>
#include "../../../include/Config.h"
#include "../../../include/Pins_Definitions.h"
#include "../../../include/FeedCylinder.h"
#include "../../../include/FlipServo.h"
#include "../../../include/StartSensor.h"

//* ************************************************************************
//* ************************ FEEDING STATE VARIABLES ********************
//* ************************************************************************

static unsigned long stateStartTime = 0;
static unsigned long stepStartTime = 0;
static int currentStep = 0;
static bool feedingStateInitialized = false;
static bool feedingComplete = false;

//* ************************************************************************
//* ************************ FEEDING STATE FUNCTIONS ********************
//* ************************************************************************

// Forward declarations for step functions
void executeFeedingStep1();
void executeFeedingStep2();
void executeFeedingStep3();

//! Initialize FEEDING state
void initFeedingState() {
    if (!feedingStateInitialized) {
        Serial.println("=== INITIALIZING FEEDING STATE ===");
        
        // Record state start time
        stateStartTime = millis();
        stepStartTime = millis();
        currentStep = 1;
        feedingComplete = false;
        
        // Ensure safe starting position
        retractFeedCylinder();
        moveFlipServoToZero();
        
        Serial.println("FEEDING State initialized - Starting feeding sequence");
        feedingStateInitialized = true;
    }
}

//! Execute FEEDING state main loop
void executeFeedingState() {
    // Initialize if not already done
    if (!feedingStateInitialized) {
        initFeedingState();
    }
    
    // Check for completion
    if (feedingComplete) {
        return;
    }
    
    // Execute current step
    switch (currentStep) {
        case 1:
            executeFeedingStep1();
            break;
        case 2:
            executeFeedingStep2();
            break;
        case 3:
            executeFeedingStep3();
            break;
        default:
            // All steps complete
            feedingComplete = true;
            Serial.println("FEEDING: Sequence complete - ready for next state");
            break;
    }
}

//! ************************************************************************
//! STEP 1: INITIAL DELAY BEFORE FEEDING
//! ************************************************************************
void executeFeedingStep1() {
    static bool step1Started = false;
    
    if (!step1Started) {
        Serial.println("FEEDING STEP 1: Starting initial delay");
        stepStartTime = millis();
        step1Started = true;
    }
    
    // Wait for initial delay
    if (millis() - stepStartTime >= FEEDING_START_DELAY) {
        Serial.println("FEEDING STEP 1: Initial delay complete");
        
        // Move to next step
        currentStep = 2;
        step1Started = false;
        stepStartTime = millis();
    }
}

//! ************************************************************************
//! STEP 2: EXTEND FEED CYLINDER TO PUSH WOOD
//! ************************************************************************
void executeFeedingStep2() {
    static bool step2Started = false;
    
    if (!step2Started) {
        Serial.println("FEEDING STEP 2: Extending feed cylinder");
        
        // Extend the feed cylinder
        extendFeedCylinder();
        
        stepStartTime = millis();
        step2Started = true;
    }
    
    // Wait for feed cylinder extension time
    if (millis() - stepStartTime >= FEED_CYLINDER_EXTEND_TIME) {
        Serial.println("FEEDING STEP 2: Feed extension complete");
        
        // Move to next step
        currentStep = 3;
        step2Started = false;
        stepStartTime = millis();
    }
}

//! ************************************************************************
//! STEP 3: RETRACT FEED CYLINDER
//! ************************************************************************
void executeFeedingStep3() {
    static bool step3Started = false;
    
    if (!step3Started) {
        Serial.println("FEEDING STEP 3: Retracting feed cylinder");
        
        // Retract the feed cylinder
        retractFeedCylinder();
        
        stepStartTime = millis();
        step3Started = true;
    }
    
    // Wait for feed cylinder retraction time
    if (millis() - stepStartTime >= FEED_CYLINDER_RETRACT_TIME) {
        Serial.println("FEEDING STEP 3: Feed retraction complete");
        
        // All steps complete
        currentStep = 4;
        step3Started = false;
    }
}

//! Check if FEEDING state is complete
bool isFeedingStateComplete() {
    return feedingComplete;
}

//! Get current feeding step
int getCurrentFeedingStep() {
    return currentStep;
}

//! Get time spent in current step
unsigned long getTimeInCurrentStep() {
    return millis() - stepStartTime;
}

//! Get total time in FEEDING state
unsigned long getTotalFeedingTime() {
    return millis() - stateStartTime;
}

//! Get FEEDING state status
const char* getFeedingStateStatus() {
    if (!feedingStateInitialized) {
        return "NOT_INITIALIZED";
    }
    
    if (feedingComplete) {
        return "COMPLETE";
    }
    
    switch (currentStep) {
        case 1: return "INITIAL_DELAY";
        case 2: return "EXTENDING_CYLINDER";
        case 3: return "RETRACTING_CYLINDER";
        default: return "FINISHING";
    }
}

//! Emergency stop FEEDING state
void emergencyStopFeedingState() {
    Serial.println("EMERGENCY STOP: FEEDING State");
    
    // Immediately retract feed cylinder
    emergencyRetractFeedCylinder();
    
    // Mark as complete to exit state
    feedingComplete = true;
    currentStep = 99; // Emergency stop indicator
    
    Serial.println("FEEDING: Emergency stop complete");
}

//! Reset FEEDING state for next cycle
void resetFeedingState() {
    feedingStateInitialized = false;
    feedingComplete = false;
    currentStep = 0;
    stateStartTime = 0;
    stepStartTime = 0;
    
    Serial.println("FEEDING: State reset for next cycle");
}

//! Exit FEEDING state cleanup
void exitFeedingState() {
    Serial.println("FEEDING: Exiting state");
    
    // Ensure cylinder is retracted
    if (isFeedCylinderExtended()) {
        Serial.println("FEEDING: Ensuring cylinder is retracted before exit");
        retractFeedCylinder();
        delay(50);
    }
    
    // Print final statistics
    Serial.print("FEEDING: Total time in state: ");
    Serial.print(getTotalFeedingTime());
    Serial.println(" ms");
    
    Serial.println("FEEDING: Ready for FLIPPING state");
} 