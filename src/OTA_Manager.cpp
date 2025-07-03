//* ************************************************************************
//* ************************ OTA MANAGER **********************************
//* ************************************************************************
//! Simple OTA (Over-The-Air) update manager for ESP32
//! Handles WiFi connection and OTA updates

#include <WiFi.h>
#include <ArduinoOTA.h>
#include <Arduino.h>

// WiFi credentials
const char* WIFI_SSID = "Everwood";
const char* WIFI_PASSWORD = "Everwood-Staff";

// OTA settings
const char* OTA_HOSTNAME = "Router-ESP32";
const char* OTA_PASSWORD = "";  // No password for simplicity

//* ************************************************************************
//* ************************ OTA INITIALIZATION ***************************
//* ************************************************************************

void initOTA() {
    Serial.println("=== STARTING OTA SETUP ===");
    
    // Connect to WiFi
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // Wait for WiFi connection (max 10 seconds)
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.print("✓ WiFi connected! IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println();
        Serial.println("✗ WiFi connection failed - OTA disabled");
        return;
    }
    
    // Configure OTA
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    if (strlen(OTA_PASSWORD) > 0) {
        ArduinoOTA.setPassword(OTA_PASSWORD);
    }
    
    // OTA event handlers
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_SPIFFS
            type = "filesystem";
        }
        Serial.println("OTA Update starting: " + type);
    });
    
    ArduinoOTA.onEnd([]() {
        Serial.println("\nOTA Update complete!");
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("OTA Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
    });
    
    ArduinoOTA.begin();
    Serial.println("✓ OTA ready!");
    Serial.print("OTA hostname: ");
    Serial.println(OTA_HOSTNAME);
    Serial.println("=== OTA SETUP COMPLETE ===");
}

//* ************************************************************************
//* ************************ OTA HANDLER **********************************
//* ************************************************************************

void handleOTA() {
    ArduinoOTA.handle();
} 