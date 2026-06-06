#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>

// Web dashboard class
// Web dashboard for controlling router machine parameters.
// Provides real-time control via websocket and web interface.
// Includes persistence for configuration settings.

class WebDashboard {
private:
    // Private member variables
    AsyncWebServer* server;
    WebSocketsServer* webSocket;
    bool isConnected;
    float* homeAnglePtr;  // Pointer to the home angle variable
    void* servoPtr;       // Pointer to the servo object

    // EEPROM settings
    static const int EEPROM_SIZE = 512;
    static const int HOME_ANGLE_ADDR = 0;

    // Private methods
    void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
    void sendStatusUpdate();
    void saveHomeAngleToEEPROM();
    void loadHomeAngleFromEEPROM();
    String getDashboardHTML();

public:
    // Constructor
    WebDashboard();

    // Initialization
    void init(float* homeAngle, void* servo);
    void begin();

    // Server access
    // Exposes the port-80 AsyncWebServer so the shared config API can
    // register /api/* routes on it after begin().
    AsyncWebServer* getServer() { return server; }

    // Control methods
    void setHomeAngle(float angle);
    void update();

    // Status methods
    bool isClientConnected();
    void broadcastStatus();
};
