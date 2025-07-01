#ifndef FLIP_SERVO_H
#define FLIP_SERVO_H

//! Flip servo component function declarations

// Initialization
void initFlipServo();

// Movement functions
void moveFlipServoToPosition(int position);
void moveFlipServoToZero();

// Sequence functions
void performFlipSequence(int flipPosition, unsigned long waitTime);
void performStandardFlip();

// Status functions
int getFlipServoPosition();
bool isFlipServoAtZero();
unsigned long getTimeSinceLastMove();
bool checkFlipServoStatus();
const char* getFlipServoState();

// Control functions
void detachFlipServo();
void reattachFlipServo();
void emergencyStopFlipServo();

#endif 