#include "ServoControl.h"

// SERVO CONFIG
// 5V 25kg digital servo. Empirically tuned with load — raise if cylinder
// fires before servo lands; lower if it waits too long.
const float SERVO_MS_PER_DEGREE = 12.0f;
const unsigned long SERVO_MIN_MOVE_MS = 50; // floor for tiny moves / jitter

// Constructor
ServoControl::ServoControl() {
    // Initialize member variables to default values
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

// Initialization
void ServoControl::init(int servoPin, int pwmChannel, int freq, int res) {
    // Store configuration parameters
    pin = servoPin;
    channel = pwmChannel;
    frequency = freq;
    resolution = res;

    // Configure LEDC PWM channel
    ledcSetup(channel, frequency, resolution);
    ledcAttachPin(pin, channel);
}

// Private methods
int ServoControl::angleToDuty(float angle) {
    // Clamp angle to valid range
    if (angle < minAngle) angle = minAngle;
    if (angle > maxAngle) angle = maxAngle;

    // Convert angle to pulse width
    float pulseWidth = map(angle, minAngle, maxAngle, minPulseWidth, maxPulseWidth);

    // Convert pulse width to duty cycle
    int maxDuty = (1 << resolution) - 1;
    int duty = (pulseWidth / (1000000.0 / frequency)) * maxDuty;

    return duty;
}

// Servo control methods
void ServoControl::write(float angle) {
    // Check if servo is initialized
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
    // Check if servo is initialized
    if (channel >= 0) {
        int maxDuty = (1 << resolution) - 1;
        int duty = (microseconds / (1000000.0 / frequency)) * maxDuty;
        ledcWrite(channel, duty);
        lastUpdateTime = millis();   // Record the time of update
    }
}

void ServoControl::detach() {
    // Detach servo from pin and reset channel
    if (channel >= 0) {
        ledcDetachPin(pin);
        channel = -1;
    }
}

// Configuration methods
void ServoControl::setPulseWidthRange(int minUs, int maxUs) {
    minPulseWidth = minUs;
    maxPulseWidth = maxUs;
}

void ServoControl::setAngleRange(int minDeg, int maxDeg) {
    minAngle = minDeg;
    maxAngle = maxDeg;
}

// Status methods
bool ServoControl::hasReachedTarget() {
    // Check if enough time has passed for the commanded move to complete.
    // Duration scales with angle delta (SERVO_MS_PER_DEGREE).
    return millis() - lastUpdateTime >= moveDurationMs;
}
