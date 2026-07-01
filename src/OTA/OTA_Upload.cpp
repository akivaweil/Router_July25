// OTA Upload
// Simple OTA (Over-The-Air) update manager for ESP32.
// Handles WiFi connection and OTA updates.

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>

#include "OTA/OTA_Upload.h"

// Network configuration

// OTA settings
const char* OTA_HOSTNAME = "Router-July25-ESP32";
const char* OTA_PASSWORD = "";  // No password for simplicity

// OTA initialization
void setupOTA() {
    // WiFi is already connected in setup() (before ESP-NOW is initialized). Do
    // NOT re-init WiFi here: a second WiFi.mode()/WiFi.begin() after esp_now is
    // bound can disturb the radio/STA channel that ESP-NOW rides on. If WiFi
    // never came up, skip OTA — the flip/feed cycle still runs standalone.
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    // Configure OTA settings
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    if (strlen(OTA_PASSWORD) > 0) {
        ArduinoOTA.setPassword(OTA_PASSWORD);
    }

    // Setup OTA event handlers
    ArduinoOTA.onStart([]() {
        Serial.println("[Router] OTA start");
    });

    ArduinoOTA.onEnd([]() {
        Serial.println("[Router] OTA done");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        esp_task_wdt_reset();  // upload blocks one loop iteration — feed the WDT
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("[Router] OTA error %u\n", error);
    });

    // Start OTA service
    ArduinoOTA.begin();
}

// OTA handler
void handleOTA() {
    // Handle OTA update requests
    ArduinoOTA.handle();
}
