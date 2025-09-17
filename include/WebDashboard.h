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
    void* servoPtr;       // Pointer to the servo object
    
    //! ********************** EEPROM SETTINGS ******************************
    static const int EEPROM_SIZE = 8192; // Increased to accommodate both buffers
    static const int HOME_ANGLE_ADDR = 0;
    static const int TOTAL_CYCLES_ADDR = 16;  // Critical data - saved every cycle
    static const int DATA_VERSION_ADDR = 20;  // Version check to detect structure changes
    static const int DATA_VERSION;  // Increment when data structure changes
    static const int TRIGGER_DATA_ADDR = 32;  // Buffer data - saved every 10 cycles
    
    //! ********************** CYCLE TRACKING ******************************
    struct CycleData {
        unsigned long timestamp;
        uint16_t cycle_count;
    };
    
    struct CycleAverages {
        float average1Min;
        float average5Min;
        float average15Min;
        float average30Min;
    };
    
    struct HourlyData {
        uint16_t cycles;
        uint16_t hour;  // 0-23
        uint16_t day;   // 1-31
        uint16_t month; // 1-12
    };
    
    static const int MAX_CYCLE_RECORDS = 15; // 15 minutes * 1 record per minute
    static const int CYCLE_RECORD_SIZE = sizeof(CycleData);
    static const int CYCLE_BUFFER_SIZE = MAX_CYCLE_RECORDS * CYCLE_RECORD_SIZE;
    
    //! ********************** HOURLY TRACKING *******************************
    static const int MAX_HOURLY_RECORDS = 744; // 31 days * 24 hours
    static const int HOURLY_RECORD_SIZE = sizeof(HourlyData);
    static const int HOURLY_DATA_ADDR = 120; // Start after cycle buffer (32 + 90 + padding)
    
    CycleData cycleBuffer[MAX_CYCLE_RECORDS];
    int cycleBufferIndex;
    unsigned long lastCycleTime;
    uint16_t totalCycles;
    bool cycleDataLoaded;
    
    //! ********************** HOURLY DATA ************************************
    HourlyData hourlyBuffer[MAX_HOURLY_RECORDS];
    int hourlyBufferIndex;
    uint16_t currentHourCycles;
    uint8_t lastHour;
    uint8_t lastDay;
    uint8_t lastMonth;
    
    //! ********************** PRIVATE METHODS ******************************
    void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
    void sendStatusUpdate();
    void saveHomeAngleToEEPROM();
    void loadHomeAngleFromEEPROM();
    void saveCycleDataToEEPROM();
    void loadCycleDataFromEEPROM();
    void addCycleRecord();
    CycleAverages calculateAllAverages();
    void updateHourlyData();
    void saveHourlyDataToEEPROM();
    void loadHourlyDataFromEEPROM();
    String getDailyStatsJSON(uint8_t day, uint8_t month);
    String getDashboardHTML();
    
public:
    //! ********************** CONSTRUCTOR **********************************
    WebDashboard();
    
    //! ********************** INITIALIZATION *******************************
    void init(float* homeAngle, void* servo);
    void begin();
    
    //! ********************** CONTROL METHODS ******************************
    void setHomeAngle(float angle);
    void update();
    void update(bool isIdleState);
    
    //! ********************** STATUS METHODS *******************************
    bool isClientConnected();
    void broadcastStatus();
    
    //! ********************** CYCLE TRACKING METHODS *********************
    void recordCycle();
    void updateCycleDisplay();
};

#endif // WEB_DASHBOARD_H
