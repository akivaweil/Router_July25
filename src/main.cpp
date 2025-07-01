#include <Arduino.h>
#include <WiFi.h>
#include "../include/Config.h"
#include "../include/Pins_Definitions.h"

// Component includes
#include "../include/FeedCylinder.h"
#include "../include/FlipServo.h"
#include "../include/StartSensor.h"

// Forward declarations for OTA functions from OTA_Manager.cpp
void initOTA();
void handleOTA();
void displayOTAStatus();
bool isWiFiConnected();

// Forward declarations for State Machine functions
void initializeStateMachine();
void updateStateMachine();
bool isStateMachineInitialized();

//* ************************************************************************
//* ************************ WIFI CONFIGURATION ************************
//* ************************************************************************
// WiFi credentials
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

//* ************************************************************************
//* ************************ GLOBAL VARIABLES ***************************
//* ************************************************************************
bool otaInitialized = false;
bool systemInitialized = false;

//* ************************************************************************
//* ************************ SETUP ************************************
//* ************************************************************************
void setup() {
  // Initialize serial communication (UART)
  Serial.begin(SERIAL_BAUD_RATE);
  delay(SERIAL_TIMEOUT);  // Give time for serial to initialize
  
  Serial.println();
  Serial.println("ESP32 Router Control System with OTA");
  Serial.println("====================================");
  Serial.print("System: ");
  Serial.println(SYSTEM_NAME);
  Serial.print("Version: ");
  Serial.println(VERSION);
  Serial.print("Chip: ");
  Serial.println(ESP.getChipModel());
  Serial.print("Chip Revision: ");
  Serial.println(ESP.getChipRevision());
  Serial.print("Free Heap: ");
  Serial.println(ESP.getFreeHeap());
  Serial.println();
  
  // Initialize hardware components
  Serial.println("Initializing Hardware Components...");
  initFeedCylinder();
  initFlipServo();
  initStartSensor();
  
  // Initialize State Machine
  Serial.println("Initializing State Machine...");
  initializeStateMachine();
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < (WIFI_CONNECTION_TIMEOUT / 500)) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi connected successfully!");
    Serial.println();
    
    // Print network information
    Serial.println("Network Information:");
    Serial.println("-------------------");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("Subnet Mask: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("DNS: ");
    Serial.println(WiFi.dnsIP());
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
    Serial.print("Signal Strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    
    // Initialize OTA after WiFi is connected
    initOTA();
    otaInitialized = true;
    
  } else {
    Serial.println();
    Serial.println("WiFi connection failed!");
    Serial.println("Will continue trying in main loop...");
    Serial.println("OTA will be unavailable until WiFi connects");
  }
  
  // Mark system as initialized
  systemInitialized = true;
  
  Serial.println();
  Serial.println("Router system initialization complete");
  Serial.println("Ready for router control operations");
  Serial.println("====================================");
}

//* ************************************************************************
//* ************************ MAIN LOOP ********************************
//* ************************************************************************
void loop() {
  // Handle OTA updates if initialized
  if (otaInitialized) {
    handleOTA();
  }
  
  // Update State Machine
  if (isStateMachineInitialized()) {
    updateStateMachine();
  }
  
  // Check WiFi connection status
  static unsigned long lastWiFiCheck = 0;
  if (millis() - lastWiFiCheck > WIFI_RECONNECT_DELAY) {
    if (WiFi.status() == WL_CONNECTED) {
      // Initialize OTA if not already done and WiFi just connected
      if (!otaInitialized) {
        initOTA();
        otaInitialized = true;
      }
      
      // Display status periodically
      static unsigned long lastStatusDisplay = 0;
      if (millis() - lastStatusDisplay > STATUS_UPDATE_INTERVAL) {
        Serial.print("Status - IP: ");
        Serial.print(WiFi.localIP());
        Serial.print(" | RSSI: ");
        Serial.print(WiFi.RSSI());
        Serial.print(" dBm | Router: ");
        Serial.print(systemInitialized ? "Ready" : "Initializing");
        Serial.println(" | OTA: Ready");
        
        lastStatusDisplay = millis();
      }
      
      // Display OTA status periodically
      displayOTAStatus();
      
    } else {
      Serial.println("WiFi disconnected! Attempting to reconnect...");
      WiFi.begin(ssid, password);
      otaInitialized = false; // Reset OTA flag when WiFi disconnects
    }
    
    lastWiFiCheck = millis();
  }
  
  // Main loop delay for system timing
  delay(MAIN_LOOP_DELAY);
}