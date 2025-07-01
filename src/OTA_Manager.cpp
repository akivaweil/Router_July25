//* ************************************************************************
//* ************************ OTA MANAGER ********************************
//* ************************************************************************
//! Over-The-Air Update Manager for ESP32 Router Project
//! Integrates with main WiFi connection for seamless OTA updates
//! All OTA functionality contained in this single file

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

//* ************************************************************************
//* ************************ OTA CONFIGURATION **************************
//* ************************************************************************

// WiFi credentials (using Everwood network)
#define OTA_WIFI_SSID "Everwood"
#define OTA_WIFI_PASSWORD "Everwood-Staff"

// OTA settings
#define OTA_HOSTNAME "ESP32-Router"
#define OTA_PORT 3232

//* ************************************************************************
//* ************************ FUNCTION DECLARATIONS *********************
//* ************************************************************************

void initOTA();
void handleOTA();
void initWiFiForOTA();
bool isWiFiConnected();
void displayOTAStatus();

//* ************************************************************************
//* ************************ OTA SETUP FUNCTIONS ************************
//* ************************************************************************

//! Initialize OTA functionality (assumes WiFi is already connected)
void initOTA() {
    // Check if WiFi is connected before setting up OTA
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected - OTA initialization skipped");
        return;
    }
    
    Serial.println();
    Serial.println("=== Initializing OTA ===");
    
    //! Configure OTA settings
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.setPort(OTA_PORT);
    
    //! Set up OTA callbacks
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else {
            type = "filesystem";
        }
        Serial.println();
        Serial.println("=== OTA UPDATE STARTING ===");
        Serial.println("Type: " + type);
    });
    
    ArduinoOTA.onEnd([]() {
        Serial.println();
        Serial.println("=== OTA UPDATE COMPLETE ===");
        Serial.println("Rebooting...");
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        unsigned int percent = (progress / (total / 100));
        Serial.printf("Progress: %u%% (%u/%u bytes)\r", percent, progress, total);
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.println();
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
    });
    
    //! Start OTA service
    ArduinoOTA.begin();
    
    Serial.println("OTA Ready!");
    Serial.print("Hostname: ");
    Serial.println(OTA_HOSTNAME);
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Port: ");
    Serial.println(OTA_PORT);
    Serial.println("Device ready for OTA uploads!");
    Serial.println("=========================");
}

//! Handle OTA updates (call in main loop)
void handleOTA() {
    ArduinoOTA.handle();
}

//* ************************************************************************
//* ************************ WIFI MANAGEMENT ****************************
//* ************************************************************************

//! Initialize WiFi specifically for OTA (backup function)
void initWiFiForOTA() {
    if (WiFi.status() == WL_CONNECTED) {
        return; // Already connected
    }
    
    Serial.println("Initializing WiFi for OTA...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(OTA_WIFI_SSID, OTA_WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.println("WiFi connected for OTA");
    } else {
        Serial.println();
        Serial.println("WiFi connection failed for OTA");
    }
}

//! Check if WiFi is connected
bool isWiFiConnected() {
    return (WiFi.status() == WL_CONNECTED);
}

//* ************************************************************************
//* ************************ STATUS FUNCTIONS ***************************
//* ************************************************************************

//! Display OTA status information
void displayOTAStatus() {
    static unsigned long lastDisplay = 0;
    
    if (millis() - lastDisplay > 30000) { // Every 30 seconds
        if (isWiFiConnected()) {
            Serial.println();
            Serial.println("=== OTA STATUS ===");
            Serial.print("Ready for OTA at: ");
            Serial.println(WiFi.localIP());
            Serial.print("Hostname: ");
            Serial.println(OTA_HOSTNAME);
            Serial.println("==================");
        }
        lastDisplay = millis();
    }
} 