//* ************************************************************************
//* ************************ FLIPPING STATE ******************************
//* ************************************************************************
//! FLIPPING state handles the wood flipping process using a servo
//! Sequence: Move servo to 0 degrees -> Wait 200ms -> Return to zero -> Retract feed cylinder

#include <Arduino.h>
#include "../../include/Config.h"
#include "../../include/Pins_Definitions.h"
#include <ESP32Servo.h>

//* ************************************************************************
//* ************************ FLIPPING STATE VARIABLES ********************
//* ************************************************************************

// State timing variables
static unsigned long flippingStateStartTime = 0;
static unsigned long stepStartTime = 0;

// State flags and step tracking
static bool flippingStateInitialized = false;
static int currentStep = 0;

// Servo object
static Servo flipServo;
static bool servoAttached = false;

// Step definitions
enum FlippingSteps {
    STEP_MOVE_SERVO = 1,
    STEP_WAIT_SERVO,
    STEP_RETURN_SERVO,
    STEP_RETRACT_CYLINDER,
    STEP_COMPLETE
};

//* ************************************************************************
//* ************************ FORWARD DECLARATIONS ***********************
//* ************************************************************************

void executeMoveServo();
void executeWaitServo();
void executeReturnServo();
void executeRetractCylinderFlipping();
void completeFlipping();

//* ************************************************************************
//* ************************ FLIPPING STATE FUNCTIONS ********************
//* ************************************************************************

//! Initialize the FLIPPING state
void initFlippingState() {
    if (!flippingStateInitialized) {
        Serial.println();
        Serial.println("=== ENTERING FLIPPING STATE ===");
        
        // Record state entry time
        flippingStateStartTime = millis();
        stepStartTime = millis();
        
        // Initialize step tracking
        currentStep = STEP_MOVE_SERVO;
        
        // Initialize servo if not already attached
        if (!servoAttached) {
            flipServo.attach(FLIP_SERVO_PIN);
            servoAttached = true;
            Serial.println("Flip servo attached and initialized");
        }
        
        // Set status indicators
        WRITE_PIN_LOW(READY_LED_PIN);
        WRITE_PIN_HIGH(RUNNING_LED_PIN);
        WRITE_PIN_LOW(ERROR_LED_PIN);
        
        Serial.println("Starting wood flipping sequence...");
        Serial.println("============================");
        
        flippingStateInitialized = true;
    }
}

//! Execute FLIPPING state logic
void executeFlippingState() {
    switch (currentStep) {
        case STEP_MOVE_SERVO:
            executeMoveServo();
            break;
            
        case STEP_WAIT_SERVO:
            executeWaitServo();
            break;
            
        case STEP_RETURN_SERVO:
            executeReturnServo();
            break;
            
        case STEP_RETRACT_CYLINDER:
            executeRetractCylinderFlipping();
            break;
            
        case STEP_COMPLETE:
            completeFlipping();
            break;
            
        default:
            Serial.println("ERROR: Unknown flipping step");
            currentStep = STEP_COMPLETE;
            break;
    }
}

//! ************************************************************************
//! STEP 1: MOVE SERVO TO 0 DEGREES
//! ************************************************************************
void executeMoveServo() {
    static bool stepInitialized = false;
    
    if (!stepInitialized) {
        Serial.println("Step 1: Moving flip servo to 0 degrees");
        
        // Move servo to 0 degrees position
        flipServo.write(FLIP_SERVO_ZERO_POSITION);
        
        stepStartTime = millis();
        stepInitialized = true;
    }
    
    // Move immediately to next step (servo movement is non-blocking)
    currentStep = STEP_WAIT_SERVO;
    stepInitialized = false;
}

//! ************************************************************************
//! STEP 2: WAIT 200ms FOR SERVO TO REACH POSITION
//! ************************************************************************
void executeWaitServo() {
    static bool stepInitialized = false;
    
    if (!stepInitialized) {
        Serial.println("Step 2: Waiting 200ms for servo to reach position");
        stepStartTime = millis();
        stepInitialized = true;
    }
    
    // Wait for servo movement delay
    if (millis() - stepStartTime >= FLIP_SERVO_MOVE_DELAY) {
        Serial.println("Servo movement delay complete");
        currentStep = STEP_RETURN_SERVO;
        stepInitialized = false;
    }
}

//! ************************************************************************
//! STEP 3: RETURN SERVO TO ZERO POSITION IMMEDIATELY
//! ************************************************************************
void executeReturnServo() {
    static bool stepInitialized = false;
    
    if (!stepInitialized) {
        Serial.println("Step 3: Returning servo to zero position immediately");
        
        // Return servo to zero position (same as initial position in this case)
        flipServo.write(FLIP_SERVO_ZERO_POSITION);
        
        stepStartTime = millis();
        stepInitialized = true;
    }
    
    // Move immediately to next step (immediate return as specified)
    currentStep = STEP_RETRACT_CYLINDER;
    stepInitialized = false;
}

//! ************************************************************************
//! STEP 4: RETRACT FEED CYLINDER
//! ************************************************************************
void executeRetractCylinderFlipping() {
    static bool stepInitialized = false;
    
    if (!stepInitialized) {
        Serial.println("Step 4: Retracting feed cylinder with HIGH signal");
        
        // Retract the feed cylinder (HIGH signal)
        RETRACT_FEED_CYLINDER();
        
        stepStartTime = millis();
        stepInitialized = true;
    }
    
    // Move immediately to complete (retraction is immediate as specified)
    currentStep = STEP_COMPLETE;
    stepInitialized = false;
}

//! Complete the flipping sequence
void completeFlipping() {
    Serial.println("=== FLIPPING SEQUENCE COMPLETE ===");
    Serial.print("Total flipping time: ");
    Serial.print(millis() - flippingStateStartTime);
    Serial.println(" ms");
    
    // Flipping state is complete - ready to return to idle
}

//! Exit the FLIPPING state and prepare for next state
void exitFlippingState() {
    Serial.println("=== EXITING FLIPPING STATE ===");
    
    // Ensure cylinder is retracted for safety
    RETRACT_FEED_CYLINDER();
    
    // Ensure servo is in safe position
    if (servoAttached) {
        flipServo.write(FLIP_SERVO_ZERO_POSITION);
    }
    
    // Reset state initialization flag
    flippingStateInitialized = false;
    currentStep = 0;
    
    Serial.println("FLIPPING state exited successfully");
}

//! Check if flipping sequence is complete
bool isFlippingComplete() {
    return (currentStep == STEP_COMPLETE);
}

//! Get current flipping step
int getCurrentFlippingStep() {
    return currentStep;
}

//! Get time spent in FLIPPING state
unsigned long getFlippingStateTime() {
    if (flippingStateInitialized) {
        return millis() - flippingStateStartTime;
    }
    return 0;
}

//! Emergency stop for flipping state
void emergencyStopFlipping() {
    Serial.println("EMERGENCY STOP - Immediately stopping flip sequence");
    
    // Immediately retract cylinder for safety
    RETRACT_FEED_CYLINDER();
    
    // Return servo to safe position
    if (servoAttached) {
        flipServo.write(FLIP_SERVO_ZERO_POSITION);
    }
    
    // Reset state
    flippingStateInitialized = false;
    currentStep = 0;
}

//! Detach servo (call during system shutdown)
void detachFlipServo() {
    if (servoAttached) {
        flipServo.detach();
        servoAttached = false;
        Serial.println("Flip servo detached");
    }
} 