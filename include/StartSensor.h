#ifndef START_SENSOR_H
#define START_SENSOR_H

//! Start sensor component function declarations

// Initialization
void initStartSensor();

// Reading functions
bool readStartSensor();
bool readStartSensorRaw();
bool isStartSensorActive();

// Edge detection
bool isStartSensorRisingEdge();
bool isStartSensorFallingEdge();

// Status functions
unsigned long getTimeSinceLastChange();
bool checkStartSensorStatus();
const char* getStartSensorState();

// Utility functions
bool waitForStartSensorActivation(unsigned long timeout);
void printStartSensorStats();

#endif 