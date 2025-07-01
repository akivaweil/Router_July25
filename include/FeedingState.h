#ifndef FEEDING_STATE_H
#define FEEDING_STATE_H

//! FEEDING state function declarations

// State lifecycle
void initFeedingState();
void executeFeedingState();
void exitFeedingState();

// Step functions
void executeFeedingStep1();
void executeFeedingStep2();
void executeFeedingStep3();

// State status
bool isFeedingStateComplete();
int getCurrentFeedingStep();
unsigned long getTimeInCurrentStep();
unsigned long getTotalFeedingTime();
const char* getFeedingStateStatus();

// State control
void emergencyStopFeedingState();
void resetFeedingState();

#endif 