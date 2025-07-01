#ifndef FLIPPING_STATE_H
#define FLIPPING_STATE_H

//! FLIPPING state function declarations

// State lifecycle
void initFlippingState();
void executeFlippingState();
void exitFlippingState();

// Step functions
void executeFlippingStep1();
void executeFlippingStep2();
void executeFlippingStep3();
void executeFlippingStep4();

// State status
bool isFlippingStateComplete();
int getCurrentFlippingStep();
unsigned long getTimeInCurrentFlippingStep();
unsigned long getTotalFlippingTime();
const char* getFlippingStateStatus();

// State control
void emergencyStopFlippingState();
void resetFlippingState();

#endif 