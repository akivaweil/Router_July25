//* ************************************************************************
//* ************************ MAIN ROUTER CONTROL *************************
//* ************************************************************************
//! Barebones ESP32 Router Control System
//! State Flow: IDLE -> FEEDING -> FLIPPING -> IDLE

#include <Arduino.h>
#include "../include/Config.h"
#include "../include/Pins_Definitions.h"
#include "../include/ServoMotor.h"

//* ************************************************************************
//* ************************ STATE MACHINE DEFINITIONS *******************
//* ************************************************************************

// State enumeration
enum SystemState {
    STATE_IDLE = 1,
    STATE_FEEDING,
    STATE_FLIPPING,
    STATE_FEEDING2
};

// Current system state
static SystemState currentState = STATE_IDLE;

//* ************************************************************************
//* ************************ SERVO MOTOR INSTANCE ************************
//* ************************************************************************

// Global servo motor instance using the robust ServoMotor class
ServoMotor flipServo(FLIP_SERVO_PIN);

//* ************************************************************************
//* ************************ FUNCTION DECLARATIONS ***********************
//* ************************************************************************

// IDLE state functions (01_IDLE.cpp)
void initIdleState();
void executeIdleState();
bool shouldExitIdleState();
void resetIdleState();

// FEEDING state functions (Router Cutting Cycle/02_FEEDING.cpp)
void initFeedingState();
void executeFeedingState();
bool isFeedingComplete();
void resetFeedingState();

// FLIPPING state functions (Router Cutting Cycle/03_FLIPPING.cpp)
void initFlippingState();
void executeFlippingState();
bool isFlippingComplete();
void resetFlippingState();

// FEEDING2 state functions (same as FEEDING - second cycle)
void initFeeding2State();
void executeFeeding2State();
bool isFeeding2Complete();
void resetFeeding2State();

//* ************************************************************************
//* ************************ MAIN SETUP AND LOOP *************************
//* ************************************************************************

//! Arduino setup function
void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    delay(300); // Allow serial to initialize
    
    Serial.println();
    Serial.println("===========================================");
    Serial.println("ESP32 Router Control System - Barebones");
    Serial.println("===========================================");
    Serial.println("State Flow: IDLE -> FEEDING -> FLIPPING -> FEEDING2 -> IDLE");
    Serial.println();

    // Configure pins
    configureInputPulldown(START_SENSOR_PIN);
    configureInputPulldown(MANUAL_START_PIN);
    configureOutput(FEED_CYLINDER_PIN);
    
    // Initialize servo motor with starting angle
    flipServo.init(0.0f);  // Initialize at 0 degrees
    Serial.println("Servo motor initialized successfully");
    
    // Initialize with IDLE state
    currentState = STATE_IDLE;
    initIdleState();

    digitalWrite(FEED_CYLINDER_PIN, HIGH);  // HIGH retracts the cylinder (cutting cycle position)
    delay(1000);
    digitalWrite(FEED_CYLINDER_PIN, LOW);  // LOW extends the cylinder (idle position)
    
    Serial.println("System initialized and ready");
}

//! Arduino main loop function
void loop() {
    // Execute current state
    switch (currentState) {
        case STATE_IDLE:
            executeIdleState();
            
            // Check for transition to FEEDING
            if (shouldExitIdleState()) {
                currentState = STATE_FEEDING;
                initFeedingState();
            }
            break;
            
        case STATE_FEEDING:
            executeFeedingState();
            
            // Check for transition to FLIPPING
            if (isFeedingComplete()) {
                resetFeedingState();
                currentState = STATE_FLIPPING;
                initFlippingState();
            }
            break;
            
        case STATE_FLIPPING:
            executeFlippingState();
            
            // Check for transition to FEEDING2
            if (isFlippingComplete()) {
                resetFlippingState();
                currentState = STATE_FEEDING2;
                initFeeding2State();
            }
            break;
            
        case STATE_FEEDING2:
            executeFeeding2State();
            
            // Check for transition back to IDLE
            if (isFeeding2Complete()) {
                resetFeeding2State();
                currentState = STATE_IDLE;
                initIdleState();
            }
            break;
            
        default:
            // Should never reach here, but safety fallback
            Serial.println("ERROR: Unknown state, returning to IDLE");
            currentState = STATE_IDLE;
            initIdleState();
            break;
    }
    
    // Small delay to prevent excessive CPU usage
    delay(10);
}