//* ************************************************************************
//* ************************ 03_FLIPPING STATE ***************************
//* ************************************************************************
//! FLIPPING State: Flips wood using servo and returns cylinder to safe position
//! Sequence: Move servo to 100° -> Wait 200ms -> Return to 0° -> Extend cylinder LOW

#include <Arduino.h>
#include "../../include/Config.h"
#include "../../include/Pins_Definitions.h"
#include <ESP32Servo.h>

//* ************************************************************************
//* ************************ FLIPPING STATE VARIABLES ********************
//* ************************************************************************

static unsigned long stateStartTime = 0;
static unsigned long stepStartTime = 0;
static int currentStep = 0;
static bool flippingInitialized = false;
static bool flippingComplete = false;

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
void executeFlippingRetractCylinder();
void completeFlippingSequence();

//* ************************************************************************
//* ************************ FLIPPING STATE FUNCTIONS ********************
//* ************************************************************************

//! Initialize FLIPPING state
void initFlippingState() {
    if (!flippingInitialized) {
        Serial.println("=== ENTERING FLIPPING STATE ===");
        
        // Record state start time
        stateStartTime = millis();
        stepStartTime = millis();
        currentStep = STEP_MOVE_SERVO;
        flippingComplete = false;
        
        // Initialize servo if not already attached
        if (!servoAttached) {
            flipServo.attach(FLIP_SERVO_PIN);
            servoAttached = true;
            Serial.println("Flip servo attached and initialized");
        }
        
        Serial.println("Starting wood flipping sequence...");
        flippingInitialized = true;
    }
}

//! Execute FLIPPING state main loop
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
            executeFlippingRetractCylinder();
            break;
        case STEP_COMPLETE:
            completeFlippingSequence();
            break;
    }
}

//! ************************************************************************
//! STEP 1: MOVE SERVO TO 100 DEGREES
//! ************************************************************************
void executeMoveServo() {
    static bool stepStarted = false;
    
    if (!stepStarted) {
        Serial.println("FLIPPING STEP 1: Moving flip servo to 100 degrees");
        
        // Move servo to 100 degrees position for flipping
        flipServo.write(FLIP_SERVO_FLIP_POSITION);
        
        stepStartTime = millis();
        stepStarted = true;
    }
    
    // Move immediately to next step (servo movement is non-blocking)
    currentStep = STEP_WAIT_SERVO;
    stepStarted = false;
}

//! ************************************************************************
//! STEP 2: WAIT 200ms FOR SERVO TO REACH POSITION
//! ************************************************************************
void executeWaitServo() {
    static bool stepStarted = false;
    
    if (!stepStarted) {
        Serial.println("FLIPPING STEP 2: Waiting 200ms for servo to reach position");
        stepStartTime = millis();
        stepStarted = true;
    }
    
    // Wait for servo movement delay
    if (millis() - stepStartTime >= FLIP_SERVO_MOVE_DELAY) {
        Serial.println("Servo movement delay complete");
        currentStep = STEP_RETURN_SERVO;
        stepStarted = false;
    }
}

//! ************************************************************************
//! STEP 3: RETURN SERVO TO ZERO POSITION IMMEDIATELY
//! ************************************************************************
void executeReturnServo() {
    static bool stepStarted = false;
    
    if (!stepStarted) {
        Serial.println("FLIPPING STEP 3: Returning servo to zero position immediately");
        
        // Return servo to zero position (same as initial position)
        flipServo.write(FLIP_SERVO_ZERO_POSITION);
        
        stepStartTime = millis();
        stepStarted = true;
    }
    
    // Move immediately to next step (immediate return as specified)
    currentStep = STEP_RETRACT_CYLINDER;
    stepStarted = false;
}

//! ************************************************************************
//! STEP 4: EXTEND FEED CYLINDER TO SAFE DEFAULT POSITION
//! ************************************************************************
void executeFlippingRetractCylinder() {
    static bool stepStarted = false;
    
    if (!stepStarted) {
        Serial.println("FLIPPING STEP 4: Extending feed cylinder to safe default position");
        
        // Return cylinder to extended (safe default position) after cutting cycle
        extendFeedCylinder();
        
        stepStartTime = millis();
        stepStarted = true;
    }
    
    // Move immediately to complete (retraction is immediate as specified)
    currentStep = STEP_COMPLETE;
    stepStarted = false;
}

//! Complete the flipping sequence
void completeFlippingSequence() {
    Serial.println("=== FLIPPING SEQUENCE COMPLETE ===");
    Serial.print("Total flipping time: ");
    Serial.print(millis() - stateStartTime);
    Serial.println(" ms");
    flippingComplete = true;
}

//! Check if FLIPPING state is complete
bool isFlippingComplete() {
    return flippingComplete;
}

//! Reset FLIPPING state for next cycle
void resetFlippingState() {
    Serial.println("=== EXITING FLIPPING STATE ===");
    flippingInitialized = false;
    flippingComplete = false;
    currentStep = 0;
} 