#ifndef SERVOMOTOR_H
#define SERVOMOTOR_H

#include <Arduino.h>
#include <ESP32Servo.h>

//* ************************************************************************
//* ************************* SERVO MOTOR CLASS ****************************
//* ************************************************************************
// Servo motor control class for ESP32-S3 with proper PWM configuration
// Uses float precision for smooth servo movements and accurate positioning

class ServoMotor {
public:
    ServoMotor(int pin);
    void init(float initialAngle);
    void setAngle(float angle);
    float getCurrentAngle();
    bool isAttached();
    void detach();
    void reattach();
    
    // Servo configuration constants
    static const float MIN_ANGLE;
    static const float MAX_ANGLE;
    static const int SERVO_MIN_PULSE_WIDTH;
    static const int SERVO_MAX_PULSE_WIDTH;
    
private:
    Servo servo;
    int servoPin;
    float currentAngle;
    bool attached;
    
    // Helper methods
    bool isValidAngle(float angle);
    float constrainAngle(float angle);
};

#endif // SERVOMOTOR_H 