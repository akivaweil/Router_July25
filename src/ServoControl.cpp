#include "ServoControl.h"
#include "config/Config.h" // Include config for SERVO_MOVE_DELAY

ServoControl::ServoControl() {
    pin = -1;
    channel = -1;
    frequency = 50;
    resolution = 14;
    minPulseWidth = 500;  // Corresponds to 0 degrees
    maxPulseWidth = 2500; // Corresponds to 180 degrees
    minAngle = 0;
    maxAngle = 180;
}

void ServoControl::init(int servoPin, int pwmChannel, int freq, int res) {
    pin = servoPin;
    channel = pwmChannel;
    frequency = freq;
    resolution = res;
    
    ledcSetup(channel, frequency, resolution);
    ledcAttachPin(pin, channel);
}

int ServoControl::angleToDuty(float angle) {
    if (angle < minAngle) angle = minAngle;
    if (angle > maxAngle) angle = maxAngle;
    
    float pulseWidth = map(angle, minAngle, maxAngle, minPulseWidth, maxPulseWidth);
    
    int maxDuty = (1 << resolution) - 1;
    int duty = (pulseWidth / (1000000.0 / frequency)) * maxDuty;
    
    return duty;
}

void ServoControl::write(float angle) {
    if (channel >= 0) {
        int duty = angleToDuty(angle);
        ledcWrite(channel, duty);
        targetAngle = angle; // Store the target angle
        lastUpdateTime = millis(); // Record the time of update
    }
}

void ServoControl::writeMicroseconds(int microseconds) {
    if (channel >= 0) {
        int maxDuty = (1 << resolution) - 1;
        int duty = (microseconds / (1000000.0 / frequency)) * maxDuty;
        ledcWrite(channel, duty);
    }
}

void ServoControl::detach() {
    if (channel >= 0) {
        ledcDetachPin(pin);
        channel = -1;
    }
}

void ServoControl::setPulseWidthRange(int minUs, int maxUs) {
    minPulseWidth = minUs;
    maxPulseWidth = maxUs;
}

void ServoControl::setAngleRange(int minDeg, int maxDeg) {
    minAngle = minDeg;
    maxAngle = maxDeg;
} 

bool ServoControl::hasReachedTarget() {
    // Check if enough time has passed since the last write() command
    return millis() - lastUpdateTime >= SERVO_MOVE_DELAY;
} 