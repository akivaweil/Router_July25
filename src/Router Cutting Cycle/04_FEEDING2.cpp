//* ************************************************************************
//* ************************ 04_FEEDING2 STATE ****************************
//* ************************************************************************
//! FEEDING2 State: Second feeding cycle after flipping
//! Identical sequence to first feeding: Start delay -> Retract (2s) -> Extend (50ms)

#include <Arduino.h>
#include "../../include/Config.h"
#include "../../include/Pins_Definitions.h"

//* ************************************************************************
//* ************************ FEEDING2 STATE VARIABLES *********************
//* ************************************************************************

static unsigned long stateStartTime = 0;
static unsigned long stepStartTime = 0;
static int currentStep = 0;
static bool feeding2Initialized = false;
static bool feeding2Complete = false;

// Step definitions
enum Feeding2Steps {
    STEP_START_DELAY = 1,
    STEP_RETRACT_CYLINDER,
    STEP_EXTEND_CYLINDER,
    STEP_COMPLETE
};

//* ************************************************************************
//* ************************ FORWARD DECLARATIONS ***********************
//* ************************************************************************

void executeStartDelay2();
void executeRetractCylinder2();
void executeExtendCylinder2();
void completeFeedingSequence2();

//* ************************************************************************
//* ************************ FEEDING2 STATE FUNCTIONS ********************
//* ************************************************************************

//! Initialize FEEDING2 state
void initFeeding2State() {
    if (!feeding2Initialized) {
        Serial.println("=== ENTERING FEEDING2 STATE ===");
        
        // Record state start time
        stateStartTime = millis();
        stepStartTime = millis();
        currentStep = STEP_START_DELAY;
        feeding2Complete = false;
        
        Serial.println("Starting second feeding sequence...");
        feeding2Initialized = true;
    }
}

//! Execute FEEDING2 state main loop
void executeFeeding2State() {
    switch (currentStep) {
        case STEP_START_DELAY:
            executeStartDelay2();
            break;
        case STEP_RETRACT_CYLINDER:
            executeRetractCylinder2();
            break;
        case STEP_EXTEND_CYLINDER:
            executeExtendCylinder2();
            break;
        case STEP_COMPLETE:
            completeFeedingSequence2();
            break;
    }
}

//! ************************************************************************
//! STEP 1: START DELAY (50ms)
//! ************************************************************************
void executeStartDelay2() {
    static bool stepStarted = false;
    
    if (!stepStarted) {
        Serial.println("FEEDING2 STEP 1: Start delay (50ms)");
        stepStartTime = millis();
        stepStarted = true;
    }
    
    // Check if start delay is complete
    if (millis() - stepStartTime >= FEEDING_START_DELAY) {
        Serial.println("Start delay complete");
        currentStep = STEP_RETRACT_CYLINDER;
        stepStarted = false;
    }
}

//! ************************************************************************
//! STEP 2: RETRACT FEED CYLINDER WITH HIGH SIGNAL (2 seconds)
//! ************************************************************************
void executeRetractCylinder2() {
    static bool stepStarted = false;
    
    if (!stepStarted) {
        Serial.println("FEEDING2 STEP 2: Retracting feed cylinder with HIGH signal (2 seconds)");
        
        // Retract cylinder (HIGH retracts for cutting cycle)
        retractFeedCylinder();
        
        stepStartTime = millis();
        stepStarted = true;
    }
    
    // Check if retract time is complete
    if (millis() - stepStartTime >= FEED_CYLINDER_EXTEND_TIME) {
        Serial.println("Cylinder retraction complete");
        currentStep = STEP_EXTEND_CYLINDER;
        stepStarted = false;
    }
}

//! ************************************************************************
//! STEP 3: EXTEND FEED CYLINDER (50ms)
//! ************************************************************************
void executeExtendCylinder2() {
    static bool stepStarted = false;
    
    if (!stepStarted) {
        Serial.println("FEEDING2 STEP 3: Extending feed cylinder to feed wood (50ms)");
        
        // Extend cylinder (LOW extends - return to safe default position)
        extendFeedCylinder();
        
        stepStartTime = millis();
        stepStarted = true;
    }
    
    // Check if extend time is complete
    if (millis() - stepStartTime >= FEED_CYLINDER_RETRACT_TIME) {
        Serial.println("Cylinder extension complete");
        currentStep = STEP_COMPLETE;
        stepStarted = false;
    }
}

//! Complete the feeding sequence
void completeFeedingSequence2() {
    Serial.println("=== FEEDING2 SEQUENCE COMPLETE ===");
    Serial.print("Total feeding2 time: ");
    Serial.print(millis() - stateStartTime);
    Serial.println(" ms");
    feeding2Complete = true;
}

//! Check if FEEDING2 state is complete
bool isFeeding2Complete() {
    return feeding2Complete;
}

//! Reset FEEDING2 state for next cycle
void resetFeeding2State() {
    Serial.println("=== EXITING FEEDING2 STATE ===");
    feeding2Initialized = false;
    feeding2Complete = false;
    currentStep = 0;
} 