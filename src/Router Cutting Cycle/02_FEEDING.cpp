//* ************************************************************************
//* ************************ 02_FEEDING STATE ****************************
//* ************************************************************************
//! FEEDING State: Feeds wood through router using pneumatic cylinder
//! Sequence: Start delay -> Extend cylinder (HIGH) -> Retract cylinder (HIGH)

#include <Arduino.h>
#include "../../include/Config.h"
#include "../../include/Pins_Definitions.h"

//* ************************************************************************
//* ************************ FEEDING STATE VARIABLES *********************
//* ************************************************************************

static unsigned long stateStartTime = 0;
static unsigned long stepStartTime = 0;
static int currentStep = 0;
static bool feedingInitialized = false;
static bool feedingComplete = false;

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
void executeFeedingRetractCylinder();
void completeFeedingSequence();

//* ************************************************************************
//* ************************ FEEDING STATE FUNCTIONS *********************
//* ************************************************************************

//! Initialize FEEDING state
void initFeedingState() {
    if (!feedingInitialized) {
        Serial.println("=== ENTERING FEEDING STATE ===");
        
        // Record state start time
        stateStartTime = millis();
        stepStartTime = millis();
        currentStep = STEP_START_DELAY;
        feedingComplete = false;
        
        Serial.println("Starting wood feeding sequence...");
        feedingInitialized = true;
    }
}

//! Execute FEEDING state main loop
void executeFeedingState() {
    switch (currentStep) {
        case STEP_START_DELAY:
            executeStartDelay();
            break;
        case STEP_EXTEND_CYLINDER:
            executeExtendCylinder();
            break;
        case STEP_RETRACT_CYLINDER:
            executeFeedingRetractCylinder();
            break;
        case STEP_COMPLETE:
            completeFeedingSequence();
            break;
    }
}

//! ************************************************************************
//! STEP 1: START DELAY (50ms)
//! ************************************************************************
void executeStartDelay() {
    static bool stepStarted = false;
    
    if (!stepStarted) {
        Serial.println("FEEDING STEP 1: Start delay (50ms)");
        stepStartTime = millis();
        stepStarted = true;
    }
    
    // Wait for start delay to complete
    if (millis() - stepStartTime >= FEEDING_START_DELAY) {
        Serial.println("Start delay complete");
        currentStep = STEP_EXTEND_CYLINDER;
        stepStarted = false;
    }
}

//! ************************************************************************
//! STEP 2: RETRACT FEED CYLINDER WITH HIGH SIGNAL (2 seconds)
//! ************************************************************************
void executeExtendCylinder() {
    static bool stepStarted = false;
    
    if (!stepStarted) {
        Serial.println("FEEDING STEP 2: Retracting feed cylinder with HIGH signal (2 seconds)");
        
        // Retract cylinder (HIGH retracts)
        retractFeedCylinder();
        
        stepStartTime = millis();
        stepStarted = true;
    }
    
    // Wait for cylinder extend time to complete
    if (millis() - stepStartTime >= FEED_CYLINDER_EXTEND_TIME) {
        Serial.println("Feed cylinder extension complete");
        currentStep = STEP_RETRACT_CYLINDER;
        stepStarted = false;
    }
}

//! ************************************************************************
//! STEP 3: EXTEND FEED CYLINDER (50ms)
//! ************************************************************************
void executeFeedingRetractCylinder() {
    static bool stepStarted = false;
    
    if (!stepStarted) {
        Serial.println("FEEDING STEP 3: Extending feed cylinder to feed wood (50ms)");
        
        // Extend cylinder (LOW extends)
        extendFeedCylinder();
        
        stepStartTime = millis();
        stepStarted = true;
    }
    
    // Wait for cylinder retract time to complete
    if (millis() - stepStartTime >= FEED_CYLINDER_RETRACT_TIME) {
        Serial.println("Feed cylinder retraction complete");
        currentStep = STEP_COMPLETE;
        stepStarted = false;
    }
}

//! Complete the feeding sequence
void completeFeedingSequence() {
    Serial.println("=== FEEDING SEQUENCE COMPLETE ===");
    Serial.print("Total feeding time: ");
    Serial.print(millis() - stateStartTime);
    Serial.println(" ms");
    feedingComplete = true;
}

//! Check if FEEDING state is complete
bool isFeedingComplete() {
    return feedingComplete;
}

//! Reset FEEDING state for next cycle
void resetFeedingState() {
    Serial.println("=== EXITING FEEDING STATE ===");
    feedingInitialized = false;
    feedingComplete = false;
    currentStep = 0;
} 