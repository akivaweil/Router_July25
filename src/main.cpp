/*
 * Simple ESP32 Router Control System
 * Written by a beginner programmer
 * 
 * This program controls a router machine that:
 * 1. Waits for a start signal (IDLE)
 * 2. Feeds wood through router (FEEDING) 
 * 3. Flips the wood over (FLIPPING)
 * 4. Feeds wood again (FEEDING2)
 * 5. Goes back to waiting (IDLE)
 */

#include <Arduino.h>
#include <ESP32Servo.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// Forward declarations for OTA functions
void initOTA();
void handleOTA();

// Pin numbers for hardware connections
const int START_SENSOR_PIN = 48;     // Button or sensor to start the cycle
const int MANUAL_START_PIN = 19;     // Manual start button
const int FEED_CYLINDER_PIN = 41;    // Controls the feeding cylinder
const int FLIP_SERVO_PIN = 18;       // Controls the flipping servo

// Timing settings (in milliseconds)
const int FEEDING_START_DELAY = 600;        // Wait before starting to feed
const int FEED_TIME = 2200;                 // How long to push wood through router
const int SERVO_MOVE_TIME = 1000;           // Time for servo to move and flip wood

// Servo positions (in degrees)
const int SERVO_HOME_POSITION = 88;        // Normal position
const int SERVO_FLIP_POSITION = 0;          // Position to flip wood

// Create servo object
Servo flipServo;

// Keep track of what the machine is doing
int currentState = 1;  // 1=IDLE, 2=FEEDING, 3=FLIPPING, 4=FEEDING2

// Variables to remember when things started
unsigned long stateStartTime = 0;
unsigned long stepStartTime = 0;
int currentStep = 0;

// Setup function - runs once when ESP32 starts
void setup() {
  // --- DISABLE BROWNOUT DETECTOR ---
  // This is the fix for the ESP32 restarting when the servo moves.
  // It tells the ESP32 to ignore the voltage drop from the servo's high current draw.
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // Start serial communication so we can see what's happening
  Serial.begin(115200);
  delay(500);
  
  Serial.println("Simple Router Control System Starting...");
  
  // Set up the pins
  pinMode(START_SENSOR_PIN, INPUT_PULLDOWN);   // Start sensor
  pinMode(MANUAL_START_PIN, INPUT_PULLDOWN);   // Manual button  
  pinMode(FEED_CYLINDER_PIN, OUTPUT);          // Cylinder control
  
  // Set up the servo with robust attachment
  Serial.println("Setting up servo motor...");
  
  // Try multiple attachment methods to ensure it works
  flipServo.attach(FLIP_SERVO_PIN);                    // Basic attach
  delay(100);
  flipServo.attach(FLIP_SERVO_PIN, 500, 2500);         // With custom pulse widths
  delay(100);
  flipServo.attach(FLIP_SERVO_PIN, 1000, 2000);        // Standard range
  delay(100);
  flipServo.attach(FLIP_SERVO_PIN, 544, 2400);         // Arduino standard
  delay(100);
  
  // Final attach with most common settings
  flipServo.attach(FLIP_SERVO_PIN);
  delay(100);
  
  // Check if servo attached successfully
  if (flipServo.attached()) {
    Serial.println("✓ Servo attached successfully!");
  } else {
    Serial.println("✗ Servo attachment failed - but continuing anyway");
  }
  
  // Force servo to home position with multiple commands
  Serial.println("Moving servo to home position...");
  flipServo.write(SERVO_HOME_POSITION);  
  delay(100);
  flipServo.write(SERVO_HOME_POSITION);  // Send command twice to be sure
  delay(100);
  flipServo.write(SERVO_HOME_POSITION);  // Third time for safety
  delay(100);


  Serial.println("Servo setup complete!");
  
  // Make sure cylinder starts in safe position (extended)
  digitalWrite(FEED_CYLINDER_PIN, LOW);  // LOW = extended = safe
  
  // Initialize OTA functionality
  initOTA();
  
  Serial.println("System ready! Waiting for start signal...");
}

// Main loop - runs over and over
void loop() {
  
  // Handle Over-The-Air updates
  handleOTA();

  // STATE 1: IDLE - Waiting for start signal
  if (currentState == 1) {
    // Check if start button pressed or sensor triggered
    if (digitalRead(START_SENSOR_PIN) || digitalRead(MANUAL_START_PIN)) {
      Serial.println("Starting router cycle!");
      currentState = 2;  // Go to FEEDING state
      stateStartTime = millis();
      currentStep = 1;
    }
  }
  
  // STATE 2: FEEDING - Push wood through router
  else if (currentState == 2) {
    
    // Step 1: Wait a little bit before starting
    if (currentStep == 1) {
      if (millis() - stateStartTime >= FEEDING_START_DELAY) {
        Serial.println("Starting to feed wood...");
        digitalWrite(FEED_CYLINDER_PIN, HIGH);  // HIGH = retract = push wood
        stepStartTime = millis();
        currentStep = 2;
      }
    }
    
    // Step 2: Keep pushing for the feed time
    else if (currentStep == 2) {
      if (millis() - stepStartTime >= FEED_TIME) {
        Serial.println("Done feeding, retracting cylinder...");
        digitalWrite(FEED_CYLINDER_PIN, LOW);   // LOW = extend = safe position
        currentState = 3;  // Go to FLIPPING state
        stateStartTime = millis();
        currentStep = 1;
      }
    }
  }
  
  // STATE 3: FLIPPING - Flip the wood over
  else if (currentState == 3) {
    
    // Step 1: Move servo to flip the wood
    if (currentStep == 1) {
      Serial.println("Flipping wood...");
      Serial.print("Moving servo to flip position: ");
      Serial.print(SERVO_FLIP_POSITION);
      Serial.println(" degrees");
      flipServo.write(SERVO_FLIP_POSITION);  // Move to flip position
      stepStartTime = millis();
      currentStep = 2;
    }
    
    // Step 2: Wait for servo to finish moving to flip position
    else if (currentStep == 2) {
      if (millis() - stepStartTime >= SERVO_MOVE_TIME) {
        Serial.println("Moving servo back home...");
        Serial.print("Moving servo to home position: ");
        Serial.print(SERVO_HOME_POSITION);
        Serial.println(" degrees");
        flipServo.write(SERVO_HOME_POSITION);  // Move back to home
        stepStartTime = millis();  // Reset timer for return movement
        currentStep = 3;  // Go to step 3 to wait for return
      }
    }
    
    // Step 3: Wait for servo to return to home position
    else if (currentStep == 3) {
      if (millis() - stepStartTime >= SERVO_MOVE_TIME) {
        Serial.println("Servo returned home, starting second feed...");
        currentState = 4;  // Go to second feeding
        stateStartTime = millis();
        currentStep = 1;
      }
    }
  }
  
  // STATE 4: FEEDING2 - Feed the flipped wood through router again
  else if (currentState == 4) {
    
    // Step 1: Wait a little bit before starting
    if (currentStep == 1) {
      if (millis() - stateStartTime >= 0) {
        Serial.println("Starting second feed...");
        digitalWrite(FEED_CYLINDER_PIN, HIGH);  // HIGH = retract = push wood
        stepStartTime = millis();
        currentStep = 2;
      }
    }
    
    // Step 2: Keep pushing for the feed time
    else if (currentStep == 2) {
      if (millis() - stepStartTime >= FEED_TIME) {
        Serial.println("Done with second feed!");
        digitalWrite(FEED_CYLINDER_PIN, LOW);   // LOW = extend = safe position
        currentState = 1;  // Go back to IDLE state
        currentStep = 1;
        Serial.println("Cycle complete! Ready for next piece...");
      }
    }
  }
  
  // Small delay so the program doesn't run too fast
  delay(10);
}