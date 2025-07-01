//* ************************************************************************
//* ************************ IDLE STATE **********************************
//* ************************************************************************
//! IDLE State: System waits for start command or OTA operations
//! This is the default state where the system monitors for start conditions

#include <Arduino.h>
#include "../../../include/Config.h"
#include "../../../include/Pins_Definitions.h"
#include "../../../include/FeedCylinder.h"
#include "../../../include/FlipServo.h"
#include "../../../include/StartSensor.h"

//* ************************************************************************
//* ************************ IDLE STATE VARIABLES ***********************
//* ************************************************************************

static unsigned long lastHealthCheck = 0;
static unsigned long lastSensorCheck = 0;
static bool idleStateInitialized = false;

//* ************************************************************************
//* ************************ IDLE STATE FUNCTIONS ***********************
//* ************************************************************************

// Forward declarations for helper functions
void performIdleHealthCheck();
void monitorSensorStates();

//! Initialize IDLE state
void initIdleState() {
    if (!idleStateInitialized) {
        Serial.println("=== INITIALIZING IDLE STATE ===");
        
        // Ensure all hardware is in safe position
        retractFeedCylinder();
        moveFlipServoToZero();
        
        // Reset timers
        lastHealthCheck = millis();
        lastSensorCheck = millis();
        
        Serial.println("IDLE State initialized - Ready for commands");
        idleStateInitialized = true;
    }
}

//! Execute IDLE state main loop
void executeIdleState() {
    // Initialize if not already done
    if (!idleStateInitialized) {
        initIdleState();
    }
    
    //! ************************************************************************
    //! STEP 1: CHECK FOR START SENSOR ACTIVATION
    //! ************************************************************************
    
    if (isStartSensorRisingEdge()) {
        Serial.println("IDLE: Start sensor activated - Beginning cutting cycle");
        // State machine will handle transition to FEEDING
        return;
    }
    
    //! ************************************************************************
    //! STEP 2: PERIODIC HEALTH MONITORING
    //! ************************************************************************
    
    if (millis() - lastHealthCheck >= HEALTH_CHECK_INTERVAL) {
        performIdleHealthCheck();
        lastHealthCheck = millis();
    }
    
    //! ************************************************************************
    //! STEP 3: SENSOR STATE MONITORING
    //! ************************************************************************
    
    if (millis() - lastSensorCheck >= SENSOR_CHECK_INTERVAL) {
        monitorSensorStates();
        lastSensorCheck = millis();
    }
    
    //! ************************************************************************
    //! STEP 4: MAINTAIN SAFE HARDWARE POSITIONS
    //! ************************************************************************
    
    // Ensure feed cylinder is retracted
    if (isFeedCylinderExtended()) {
        Serial.println("IDLE: Feed cylinder extended - retracting for safety");
        retractFeedCylinder();
    }
    
    // Ensure flip servo is at zero
    if (!isFlipServoAtZero()) {
        Serial.println("IDLE: Flip servo not at zero - moving to safe position");
        moveFlipServoToZero();
    }
    
    //! ************************************************************************
    //! STEP 5: HANDLE OTA OPERATIONS
    //! ************************************************************************
    
    // Handle OTA operations (from main loop)
    // This allows wireless updates while in IDLE state
}

//! Perform health check while in IDLE
void performIdleHealthCheck() {
    static int healthCheckCount = 0;
    healthCheckCount++;
    
    // Check component status
    bool feedCylinderOK = checkFeedCylinderStatus();
    bool flipServoOK = checkFlipServoStatus();
    bool startSensorOK = checkStartSensorStatus();
    
    // Print status every 10th check (reduce serial output)
    if (healthCheckCount % 10 == 0) {
        Serial.println("=== IDLE HEALTH CHECK ===");
        Serial.print("Feed Cylinder: ");
        Serial.println(feedCylinderOK ? "OK" : "ERROR");
        Serial.print("Flip Servo: ");
        Serial.println(flipServoOK ? "OK" : "ERROR");
        Serial.print("Start Sensor: ");
        Serial.println(startSensorOK ? "OK" : "ERROR");
        Serial.println("========================");
    }
    
    // Handle any errors
    if (!feedCylinderOK) {
        Serial.println("IDLE: Feed cylinder error detected");
        // Could implement error recovery here
    }
    
    if (!flipServoOK) {
        Serial.println("IDLE: Flip servo error detected");
        // Could implement error recovery here
    }
    
    if (!startSensorOK) {
        Serial.println("IDLE: Start sensor error detected");
        // Could implement error recovery here
    }
}

//! Monitor sensor states while in IDLE
void monitorSensorStates() {
    // Read start sensor to keep debouncing active
    readStartSensor();
    
    // Check for sensor state changes
    static bool lastStartSensorState = false;
    bool currentStartSensorState = isStartSensorActive();
    
    if (currentStartSensorState != lastStartSensorState) {
        Serial.print("IDLE: Start sensor state: ");
        Serial.println(getStartSensorState());
        lastStartSensorState = currentStartSensorState;
    }
}

//! Check if ready to exit IDLE state
bool isReadyToExitIdle() {
    // Check if start sensor is activated
    if (isStartSensorActive()) {
        // Verify all systems are ready
        bool allSystemsReady = checkFeedCylinderStatus() && 
                               checkFlipServoStatus() && 
                               checkStartSensorStatus();
        
        if (allSystemsReady) {
            Serial.println("IDLE: All systems ready - can proceed to FEEDING");
            return true;
        } else {
            Serial.println("IDLE: Start requested but systems not ready");
            return false;
        }
    }
    
    return false;
}

//! Get IDLE state status
const char* getIdleStateStatus() {
    if (!idleStateInitialized) {
        return "NOT_INITIALIZED";
    }
    
    if (isStartSensorActive()) {
        return "WAITING_FOR_SYSTEMS";
    }
    
    return "READY";
}

//! Exit IDLE state cleanup
void exitIdleState() {
    Serial.println("IDLE: Exiting state - preparing for next state");
    
    // Final safety checks
    if (!isFeedCylinderExtended() && isFlipServoAtZero()) {
        Serial.println("IDLE: Hardware in safe position for state transition");
    } else {
        Serial.println("IDLE: WARNING - Hardware not in expected safe position");
        retractFeedCylinder();
        moveFlipServoToZero();
        delay(100);
    }
} 