#include "ServoControl.h"

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚔️ SERVO CONFIG ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
// 5V 25kg digital servo. Empirically tuned with load — raise if cylinder
// fires before servo lands; lower if it waits too long.
const float SERVO_MS_PER_DEGREE = 8.0f;
const unsigned long SERVO_MIN_MOVE_MS = 50; // floor for tiny moves / jitter

//* ************************************************************************
//* ********************** CONSTRUCTOR *************************************
//* ************************************************************************
ServoControl::ServoControl() {
    //! ************************************************************************
    //! INITIALIZE MEMBER VARIABLES TO DEFAULT VALUES
    //! ************************************************************************
    pin = -1;
    channel = -1;
    frequency = 50;          // Standard servo frequency
    resolution = 14;         // 14-bit resolution for smooth operation
    minPulseWidth = 500;     // Corresponds to 0 degrees
    maxPulseWidth = 2500;    // Corresponds to 180 degrees
    minAngle = 0;            // Minimum servo angle
    maxAngle = 180;          // Maximum servo angle
    targetAngle = 90.0f;     // Default to center position
    currentAngle = 90.0f;    // Assume starting at center
    lastUpdateTime = 0;      // Initialize timestamp
    moveDurationMs = 0;      // No move pending
}

//* ************************************************************************
//* ********************** INITIALIZATION **********************************
//* ************************************************************************
void ServoControl::init(int servoPin, int pwmChannel, int freq, int res) {
    //! ************************************************************************
    //! STORE CONFIGURATION PARAMETERS
    //! ************************************************************************
    pin = servoPin;
    channel = pwmChannel;
    frequency = freq;
    resolution = res;
    
    //! ************************************************************************
    //! CONFIGURE LEDC PWM CHANNEL
    //! ************************************************************************
    ledcSetup(channel, frequency, resolution);
    ledcAttachPin(pin, channel);
}

//* ************************************************************************
//* ********************** PRIVATE METHODS **********************************
//* ************************************************************************
int ServoControl::angleToDuty(float angle) {
    //! ************************************************************************
    //! CLAMP ANGLE TO VALID RANGE
    //! ************************************************************************
    if (angle < minAngle) angle = minAngle;
    if (angle > maxAngle) angle = maxAngle;
    
    //! ************************************************************************
    //! CONVERT ANGLE TO PULSE WIDTH
    //! ************************************************************************
    float pulseWidth = map(angle, minAngle, maxAngle, minPulseWidth, maxPulseWidth);
    
    //! ************************************************************************
    //! CONVERT PULSE WIDTH TO DUTY CYCLE
    //! ************************************************************************
    int maxDuty = (1 << resolution) - 1;
    int duty = (pulseWidth / (1000000.0 / frequency)) * maxDuty;
    
    return duty;
}

//* ************************************************************************
//* ********************** SERVO CONTROL METHODS ***************************
//* ************************************************************************
void ServoControl::write(float angle) {
    //! ************************************************************************
    //! CHECK IF SERVO IS INITIALIZED
    //! ************************************************************************
    if (channel >= 0) {
        int duty = angleToDuty(angle);
        ledcWrite(channel, duty);
        float delta = fabs(angle - currentAngle);
        unsigned long computed = (unsigned long)(delta * SERVO_MS_PER_DEGREE);
        moveDurationMs = computed < SERVO_MIN_MOVE_MS ? SERVO_MIN_MOVE_MS : computed;
        currentAngle = angle;
        targetAngle = angle;         // Store the target angle
        lastUpdateTime = millis();   // Record the time of update
    }
}

void ServoControl::writeMicroseconds(int microseconds) {
    //! ************************************************************************
    //! CHECK IF SERVO IS INITIALIZED
    //! ************************************************************************
    if (channel >= 0) {
        int maxDuty = (1 << resolution) - 1;
        int duty = (microseconds / (1000000.0 / frequency)) * maxDuty;
        ledcWrite(channel, duty);
        lastUpdateTime = millis();   // Record the time of update
    }
}

void ServoControl::detach() {
    //! ************************************************************************
    //! DETACH SERVO FROM PIN AND RESET CHANNEL
    //! ************************************************************************
    if (channel >= 0) {
        ledcDetachPin(pin);
        channel = -1;
    }
}

//* ************************************************************************
//* ********************** CONFIGURATION METHODS ***************************
//* ************************************************************************
void ServoControl::setPulseWidthRange(int minUs, int maxUs) {
    minPulseWidth = minUs;
    maxPulseWidth = maxUs;
}

void ServoControl::setAngleRange(int minDeg, int maxDeg) {
    minAngle = minDeg;
    maxAngle = maxDeg;
}

//* ************************************************************************
//* ********************** STATUS METHODS ***********************************
//* ************************************************************************
bool ServoControl::hasReachedTarget() {
    //! ************************************************************************
    //! CHECK IF ENOUGH TIME HAS PASSED FOR THE COMMANDED MOVE TO COMPLETE
    //! Duration scales with angle delta (SERVO_MS_PER_DEGREE).
    //! ************************************************************************
    return millis() - lastUpdateTime >= moveDurationMs;
} 