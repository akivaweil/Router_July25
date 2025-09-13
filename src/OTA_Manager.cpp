//* ************************************************************************
//* ************************ OTA MANAGER **********************************
//* ************************************************************************
//! Simple OTA (Over-The-Air) update manager for ESP32
//! Handles WiFi connection and OTA updates
//! Provides wireless firmware updates for the router control system

#include <WiFi.h>
#include <ArduinoOTA.h>
#include <Arduino.h>

//* ************************************************************************
//* ************************ NETWORK CONFIGURATION *************************
//* ************************************************************************

//! ********************** WIFI CREDENTIALS ********************************
const char* WIFI_SSID = "Everwood";
const char* WIFI_PASSWORD = "Everwood-Staff";

//! ********************** OTA SETTINGS ************************************
const char* OTA_HOSTNAME = "Router-July25-ESP32";
const char* OTA_PASSWORD = "";  // No password for simplicity

//! ********************** CONNECTION PARAMETERS ***************************
const int WIFI_CONNECT_TIMEOUT = 20;  // Maximum connection attempts (20 * 500ms = 10 seconds)
const int WIFI_CONNECT_DELAY = 500;   // Delay between connection attempts (ms)

//* ************************************************************************
//* ************************ OTA INITIALIZATION ***************************
//* ************************************************************************
void initOTA() {
    Serial.println("=== STARTING OTA SETUP ===");
    
    //! ************************************************************************
    //! CONNECT TO WIFI NETWORK
    //! ************************************************************************
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    //! ************************************************************************
    //! WAIT FOR WIFI CONNECTION WITH TIMEOUT
    //! ************************************************************************
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < WIFI_CONNECT_TIMEOUT) {
        delay(WIFI_CONNECT_DELAY);
        Serial.print(".");
        attempts++;
    }
    
    //! ************************************************************************
    //! CHECK CONNECTION STATUS
    //! ************************************************************************
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.print("✓ WiFi connected! IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println();
        Serial.println("✗ WiFi connection failed - OTA disabled");
        return;
    }
    
    //! ************************************************************************
    //! CONFIGURE OTA SETTINGS
    //! ************************************************************************
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    if (strlen(OTA_PASSWORD) > 0) {
        ArduinoOTA.setPassword(OTA_PASSWORD);
    }
    
    //! ************************************************************************
    //! SETUP OTA EVENT HANDLERS
    //! ************************************************************************
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
        // Progress tracking disabled to reduce spam during motor operations
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
    
    //! ************************************************************************
    //! START OTA SERVICE
    //! ************************************************************************
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
    //! ************************************************************************
    //! HANDLE OTA UPDATE REQUESTS
    //! ************************************************************************
    ArduinoOTA.handle();
} 