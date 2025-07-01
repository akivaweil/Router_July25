#ifndef FEED_CYLINDER_H
#define FEED_CYLINDER_H

//! Feed cylinder component function declarations

// Initialization
void initFeedCylinder();

// Basic control functions
void extendFeedCylinder();
void retractFeedCylinder();

// Status functions
bool isFeedCylinderExtended();
unsigned long getTimeSinceLastOperation();
bool checkFeedCylinderStatus();
const char* getFeedCylinderState();

// Advanced control functions
void timedExtendFeedCylinder(unsigned long extendTime);
void emergencyRetractFeedCylinder();

#endif 