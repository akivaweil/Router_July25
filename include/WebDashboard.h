#ifndef WEB_DASHBOARD_H
#define WEB_DASHBOARD_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>

//* ************************************************************************
//* ********************** WEB DASHBOARD CLASS *****************************
//* ************************************************************************
//! Web dashboard for controlling router machine parameters
//! Provides real-time control via websocket and web interface
//! Includes persistence for configuration settings

class WebDashboard {
private:
    //! ********************** PRIVATE MEMBER VARIABLES *********************
    AsyncWebServer* server;
    WebSocketsServer* webSocket;
    bool isConnected;
    float* homeAnglePtr;  // Pointer to the home angle variable
    
    //! ********************** EEPROM SETTINGS ******************************
    static const int EEPROM_SIZE = 512;
    static const int HOME_ANGLE_ADDR = 0;
    
    //! ********************** PRIVATE METHODS ******************************
    void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
    void sendStatusUpdate();
    void saveHomeAngleToEEPROM();
    void loadHomeAngleFromEEPROM();
    String getDashboardHTML();
    
public:
    //! ********************** CONSTRUCTOR **********************************
    WebDashboard();
    
    //! ********************** INITIALIZATION *******************************
    void init(float* homeAngle);
    void begin();
    
    //! ********************** CONTROL METHODS ******************************
    void setHomeAngle(float angle);
    void update();
    
    //! ********************** STATUS METHODS *******************************
    bool isClientConnected();
    void broadcastStatus();
};

#endif // WEB_DASHBOARD_H
