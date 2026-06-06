#pragma once

// Pin definitions

// Input pins
const int START_SENSOR_PIN = 48;     // Button or sensor to start the cycle (Active HIGH - input pulldown)
const int MANUAL_START_PIN = 36;     // Manual start button (Active HIGH - input pulldown)

// Output pins
const int FEED_CYLINDER_PIN = 41;    // Controls the feeding cylinder (LOW = extended/safe, HIGH = retracted/active)

// Servo pin
const int FLIP_SERVO_PIN = 15;       // Pin for the flipping servo motor
