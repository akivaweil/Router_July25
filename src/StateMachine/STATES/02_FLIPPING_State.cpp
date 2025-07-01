//* ************************************************************************
//* ************************ FLIPPING STATE ******************************
//* ************************************************************************
//! FLIPPING State: Operates flip servo to flip wood piece
//! Executes the flipping sequence with precise timing

#include <Arduino.h>
#include "../../../include/Config.h"
#include "../../../include/Pins_Definitions.h"
#include "../../../include/FeedCylinder.h"
#include "../../../include/FlipServo.h"
#include "../../../include/StartSensor.h"

//* ************************************************************************
//* ************************ FLIPPING STATE VARIABLES *******************
//* ************************************************************************

static unsigned long stateStartTime = 0;
static unsigned long stepStartTime = 0;
static int currentStep = 0;
static bool flippingStateInitialized = false;
static bool flippingComplete = false;

//* ************************************************************************
//* ************************ FLIPPING STATE FUNCTIONS *******************
//* ************************************************************************

// Forward declarations for step functions
void executeFlippingStep1();
void executeFlippingStep2();
void executeFlippingStep3();
void executeFlippingStep4();

//! Initialize FLIPPING state
void initFlippingState() {
    if (!flippingStateInitialized) {
        Serial.println("=== INITIALIZING FLIPPING STATE ===");
        
        // Record state start time
        stateStartTime = millis();
        stepStartTime = millis();
        currentStep = 1;
        flippingComplete = false;
        
        // Ensure safe starting position
        retractFeedCylinder();
        
        Serial.println("FLIPPING State initialized - Starting flipping sequence");
        flippingStateInitialized = true;
    }
}

//! Execute FLIPPING state main loop
void executeFlippingState() {
    // Initialize if not already done
    if (!flippingStateInitialized) {
        initFlippingState();
    }
    
    // Check for completion
    if (flippingComplete) {
        return;
    }
    
    // Execute current step
    switch (currentStep) {
        case 1:
            executeFlippingStep1();
            break;
        case 2:
            executeFlippingStep2();
            break;
        case 3:
            executeFlippingStep3();
            break;
        case 4:
            executeFlippingStep4();
            break;
        default:
            // All steps complete
            flippingComplete = true;
            Serial.println("FLIPPING: Sequence complete - returning to IDLE");
            break;
    }
}

//! ************************************************************************
//! STEP 1: MOVE FLIP SERVO TO ZERO DEGREES
//! ************************************************************************
void executeFlippingStep1() {
    static bool step1Started = false;
    
    if (!step1Started) {
        Serial.println("FLIPPING STEP 1: Moving flip servo to zero degrees");
        
        // Move servo to zero position
        moveFlipServoToZero();
        
        stepStartTime = millis();
        step1Started = true;
    }
    
    // Move to next step immediately (servo movement is non-blocking)
    currentStep = 2;
    step1Started = false;
    stepStartTime = millis();
}

//! ************************************************************************
//! STEP 2: WAIT FOR SERVO POSITIONING
//! ************************************************************************
void executeFlippingStep2() {
    static bool step2Started = false;
    
    if (!step2Started) {
        Serial.println("FLIPPING STEP 2: Waiting for servo positioning");
        stepStartTime = millis();
        step2Started = true;
    }
    
    // Wait for servo positioning delay
    if (millis() - stepStartTime >= FLIP_SERVO_MOVE_DELAY) {
        Serial.println("FLIPPING STEP 2: Servo positioning complete");
        
        // Move to next step
        currentStep = 3;
        step2Started = false;
        stepStartTime = millis();
    }
}

//! ************************************************************************
//! STEP 3: RETURN SERVO TO ZERO (IMMEDIATE)
//! ************************************************************************
void executeFlippingStep3() {
    static bool step3Started = false;
    
    if (!step3Started) {
        Serial.println("FLIPPING STEP 3: Returning servo to zero position");
        
        // Return servo to zero position
        moveFlipServoToZero();
        
        stepStartTime = millis();
        step3Started = true;
    }
    
    // Move to next step immediately
    currentStep = 4;
    step3Started = false;
    stepStartTime = millis();
}

//! ************************************************************************
//! STEP 4: RETRACT FEED CYLINDER WITH HIGH SIGNAL
//! ************************************************************************
void executeFlippingStep4() {
    static bool step4Started = false;
    
    if (!step4Started) {
        Serial.println("FLIPPING STEP 4: Retracting feed cylinder");
        
        // Retract feed cylinder (HIGH signal)
        retractFeedCylinder();
        
        stepStartTime = millis();
        step4Started = true;
    }
    
    // Wait a brief moment then complete
    if (millis() - stepStartTime >= 50) {
        Serial.println("FLIPPING STEP 4: Feed cylinder retraction complete");
        
        // All steps complete
        currentStep = 5;
        step4Started = false;
    }
}

//! Check if FLIPPING state is complete
bool isFlippingStateComplete() {
    return flippingComplete;
}

//! Get current flipping step
int getCurrentFlippingStep() {
    return currentStep;
}

//! Get time spent in current step
unsigned long getTimeInCurrentFlippingStep() {
    return millis() - stepStartTime;
}

//! Get total time in FLIPPING state
unsigned long getTotalFlippingTime() {
    return millis() - stateStartTime;
}

//! Get FLIPPING state status
const char* getFlippingStateStatus() {
    if (!flippingStateInitialized) {
        return "NOT_INITIALIZED";
    }
    
    if (flippingComplete) {
        return "COMPLETE";
    }
    
    switch (currentStep) {
        case 1: return "MOVING_SERVO_TO_ZERO";
        case 2: return "WAITING_FOR_POSITIONING";
        case 3: return "RETURNING_SERVO";
        case 4: return "RETRACTING_CYLINDER";
        default: return "FINISHING";
    }
}

//! Emergency stop FLIPPING state
void emergencyStopFlippingState() {
    Serial.println("EMERGENCY STOP: FLIPPING State");
    
    // Stop servo and retract cylinder immediately
    emergencyStopFlipServo();
    emergencyRetractFeedCylinder();
    
    // Mark as complete to exit state
    flippingComplete = true;
    currentStep = 99; // Emergency stop indicator
    
    Serial.println("FLIPPING: Emergency stop complete");
}

//! Reset FLIPPING state for next cycle
void resetFlippingState() {
    flippingStateInitialized = false;
    flippingComplete = false;
    currentStep = 0;
    stateStartTime = 0;
    stepStartTime = 0;
    
    Serial.println("FLIPPING: State reset for next cycle");
}

//! Exit FLIPPING state cleanup
void exitFlippingState() {
    Serial.println("FLIPPING: Exiting state");
    
    // Ensure all hardware is in safe position
    if (isFeedCylinderExtended()) {
        Serial.println("FLIPPING: Ensuring cylinder is retracted before exit");
        retractFeedCylinder();
        delay(50);
    }
    
    if (!isFlipServoAtZero()) {
        Serial.println("FLIPPING: Ensuring servo is at zero before exit");
        moveFlipServoToZero();
        delay(100);
    }
    
    // Print final statistics
    Serial.print("FLIPPING: Total time in state: ");
    Serial.print(getTotalFlippingTime());
    Serial.println(" ms");
    
    Serial.println("FLIPPING: Returning to IDLE state");
} 