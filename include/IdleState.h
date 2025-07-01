#ifndef IDLE_STATE_H
#define IDLE_STATE_H

//! IDLE state function declarations

// State lifecycle
void initIdleState();
void executeIdleState();
void exitIdleState();

// State monitoring
void performIdleHealthCheck();
void monitorSensorStates();

// State status
bool isReadyToExitIdle();
const char* getIdleStateStatus();

#endif 