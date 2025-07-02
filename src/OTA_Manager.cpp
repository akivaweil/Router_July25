/*
 * OTA_Manager.cpp - Over-The-Air Update Manager for ESP32-S3
 * 
 * SETUP INSTRUCTIONS:
 * 1. This file contains all OTA functionality - no header file needed
 * 2. WiFi credentials are configured in Config.h (Everwood network)
 * 3. In your main.cpp:
 *    - Call initOTA() in setup()
 *    - Call handleOTA() in loop() (already integrated in IDLE state)
 * 4. Use 'pio run -t upload -e esp32s3_ota' for OTA uploads
 * 5. Monitor serial output to see IP address assigned to ESP32-S3
 * 6. Update platformio.ini upload_port with the assigned IP
 * 
 * NETWORK REQUIREMENTS:
 * - ESP32-S3 and computer must be on same network (Everwood)
 * - Port 3232 must be open for OTA communication
 * 
 * ESP32-S3 SPECIFIC CONFIGURATION:
 * - Optimized for ESP32-S3 WiFi stack
 * - Uses proper ESP32-S3 hostname and identification
 * - Compatible with PlatformIO ESP32-S3 environments
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "../include/Config.h"

//* ************************************************************************
//* ************************ ESP32-S3 WIFI CONNECTION FUNCTIONS **********
//* ************************************************************************

void initWiFi() {
    Serial.println("\n===========================================");
    Serial.println("ESP32-S3 Router Control - OTA Setup");
    Serial.println("===========================================");
    
    //! Step 1: Connect to Everwood WiFi network
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    Serial.print("Connecting to Everwood WiFi");
    float connectionStartTime = millis();
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        
        // Timeout after 30 seconds
        if (millis() - connectionStartTime > 30000) {
            Serial.println("");
            Serial.println("WiFi connection timeout - continuing without OTA");
            return;
        }
    }
    
    Serial.println("");
    Serial.println("✓ WiFi connected successfully!");
    Serial.print("✓ ESP32-S3 IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("✓ MAC address: ");
    Serial.println(WiFi.macAddress());
}

//* ************************************************************************
//* ************************ ESP32-S3 OTA SETUP FUNCTIONS ****************
//* ************************************************************************

void initOTA() {
    // Initialize WiFi first
    initWiFi();
    
    // Only proceed if WiFi is connected
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Skipping OTA setup - WiFi not connected");
        return;
    }
    
    //! Step 2: Configure OTA for ESP32-S3
    ArduinoOTA.setHostname("ESP32-S3-Router");
    
    // Enhanced OTA callbacks with detailed feedback
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_SPIFFS
            type = "filesystem";
        }
        Serial.println("");
        Serial.println("=== OTA UPDATE STARTING ===");
        Serial.println("Updating " + type);
        Serial.println("WARNING: Do not power off device during update!");
    });
    
    ArduinoOTA.onEnd([]() {
        Serial.println("");
        Serial.println("=== OTA UPDATE COMPLETE ===");
        Serial.println("Rebooting ESP32-S3...");
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        static float lastPrintTime = 0;
        // Print progress every 1 second to avoid spam
        if (millis() - lastPrintTime > 1000) {
            Serial.printf("OTA Progress: %u%% (%u/%u bytes)\r", 
                         (progress / (total / 100)), progress, total);
            lastPrintTime = millis();
        }
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.println("");
        Serial.printf("=== OTA ERROR [%u] ===\n", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Authentication Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
        Serial.println("OTA failed - system will continue normal operation");
    });
    
    //! Step 3: Start OTA service for ESP32-S3
    ArduinoOTA.begin();
    Serial.println("✓ OTA service started and ready");
    Serial.println("✓ ESP32-S3 ready for remote uploads!");
    Serial.print("✓ Use this IP for OTA uploads: ");
    Serial.println(WiFi.localIP());
    Serial.println("===========================================");
}

//* ************************************************************************
//* ************************ ESP32-S3 OTA RUNTIME FUNCTIONS **************
//* ************************************************************************

void handleOTA() {
    // Only handle OTA if WiFi is still connected
    if (WiFi.status() == WL_CONNECTED) {
        ArduinoOTA.handle();
    }
}

void displayOTAStatus() {
    //! Display IP and connection status every 30 seconds during IDLE
    static float lastStatusPrint = 0;
    if (millis() - lastStatusPrint > 30000) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("--- OTA Status ---");
            Serial.print("ESP32-S3 IP: ");
            Serial.println(WiFi.localIP());
            Serial.println("OTA ready for uploads");
        } else {
            Serial.println("--- OTA Status ---");
            Serial.println("WiFi disconnected - OTA unavailable");
        }
        lastStatusPrint = millis();
    }
}

//* ************************************************************************
//* ************************ WIFI STATUS FUNCTIONS ***********************
//* ************************************************************************

bool isWiFiConnected() {
    return (WiFi.status() == WL_CONNECTED);
}

String getWiFiIP() {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.localIP().toString();
    }
    return "No Connection";
} 