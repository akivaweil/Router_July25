#ifndef SERVO_FUNCTIONS_H
#define SERVO_FUNCTIONS_H

#include <ESP32Servo.h>

//* ************************************************************************
//* ************************ SERVO FUNCTION DECLARATIONS ******************
//* ************************************************************************
// Router servo control functions based on Stage 1 approach

// Servo control functions
void moveServoToHome();
void moveServoToActive();
void moveServoToAngle(int angle);

// Servo getter function
Servo* getFlipServo();

#endif // SERVO_FUNCTIONS_H 