#include "WebDashboard.h"
#include "ServoControl.h"

//! ************************************************************************
//! DEFINE STATIC CONSTANTS
//! ************************************************************************
const int WebDashboard::DATA_VERSION = 3;
const char* WebDashboard::GOOGLE_SCRIPT_URL = "https://script.google.com/macros/s/YOUR_SCRIPT_ID/exec";

//* ************************************************************************
//* ********************** CONSTRUCTOR *************************************
//* ************************************************************************
WebDashboard::WebDashboard() {
    //! ************************************************************************
    //! INITIALIZE MEMBER VARIABLES TO DEFAULT VALUES
    //! ************************************************************************
    server = nullptr;
    webSocket = nullptr;
    isConnected = false;
    homeAnglePtr = nullptr;
    servoPtr = nullptr;
    
    //! ************************************************************************
    //! INITIALIZE TRIGGER TRACKING VARIABLES
    //! ************************************************************************
    cycleBufferIndex = 0;
    lastCycleTime = 0;
    totalCycles = 0;
    cycleDataLoaded = false;
    
    //! ************************************************************************
    //! INITIALIZE HOURLY TRACKING VARIABLES
    //! ************************************************************************
    hourlyBufferIndex = 0;
    currentHourCycles = 0;
    lastHour = 255; // Invalid hour to force initialization
    lastDay = 0;
    lastMonth = 0;
    
    //! ************************************************************************
    //! CLEAR TRIGGER BUFFER
    //! ************************************************************************
    for (int i = 0; i < MAX_CYCLE_RECORDS; i++) {
        cycleBuffer[i].timestamp = 0;
        cycleBuffer[i].cycle_count = 0;
    }
    
    //! ************************************************************************
    //! CLEAR HOURLY BUFFER
    //! ************************************************************************
    for (int i = 0; i < MAX_HOURLY_RECORDS; i++) {
        hourlyBuffer[i].cycles = 0;
        hourlyBuffer[i].hour = 0;
        hourlyBuffer[i].day = 0;
        hourlyBuffer[i].month = 0;
    }
    
    //! ************************************************************************
    //! INITIALIZE CLOUD SYNC VARIABLES
    //! ************************************************************************
    lastCloudSync = 0;
    lastSyncedTotalCycles = 0;
}

//* ************************************************************************
//* ********************** INITIALIZATION **********************************
//* ************************************************************************
void WebDashboard::init(float* homeAngle, void* servo) {
    //! ************************************************************************
    //! STORE POINTER TO HOME ANGLE VARIABLE AND SERVO OBJECT
    //! ************************************************************************
    homeAnglePtr = homeAngle;
    servoPtr = servo;
    
    //! ************************************************************************
    //! INITIALIZE EEPROM
    //! ************************************************************************
    EEPROM.begin(EEPROM_SIZE);
    
    //! ************************************************************************
    //! LOAD SAVED HOME ANGLE FROM EEPROM
    //! ************************************************************************
    loadHomeAngleFromEEPROM();
    
    //! ************************************************************************
    //! LOAD TRIGGER DATA FROM EEPROM
    //! ************************************************************************
    loadCycleDataFromEEPROM();
    
    //! ************************************************************************
    //! LOAD HOURLY DATA FROM EEPROM
    //! ************************************************************************
    loadHourlyDataFromEEPROM();
    
    //! ************************************************************************
    //! CREATE WEB SERVER AND WEBSOCKET SERVER
    //! ************************************************************************
    server = new AsyncWebServer(80);
    webSocket = new WebSocketsServer(81);
    
    //! ************************************************************************
    //! SETUP WEBSOCKET EVENT HANDLER
    //! ************************************************************************
    webSocket->onEvent([this](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
        this->handleWebSocketEvent(num, type, payload, length);
    });
}

void WebDashboard::begin() {
    //! ************************************************************************
    //! SETUP WEB SERVER ROUTES
    //! ************************************************************************
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/html", getDashboardHTML());
    });
    
    //! ************************************************************************
    //! DAILY STATS API ENDPOINT
    //! ************************************************************************
    server->on("/daily-stats", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("day") && request->hasParam("month")) {
            uint8_t day = request->getParam("day")->value().toInt();
            uint8_t month = request->getParam("month")->value().toInt();
            String json = getDailyStatsJSON(day, month);
            request->send(200, "application/json", json);
        } else {
            request->send(400, "text/plain", "Missing day or month parameter");
        }
    });
    
    //! ************************************************************************
    //! CALENDAR DATA API ENDPOINT
    //! ************************************************************************
    server->on("/calendar-data", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("month")) {
            uint8_t month = request->getParam("month")->value().toInt();
            String json = getCalendarDataJSON(month);
            request->send(200, "application/json", json);
        } else {
            request->send(400, "text/plain", "Missing month parameter");
        }
    });
    
    //! ************************************************************************
    //! START SERVERS
    //! ************************************************************************
    server->begin();
    webSocket->begin();
}

//* ************************************************************************
//* ********************** PRIVATE METHODS **********************************
//* ************************************************************************
void WebDashboard::handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            isConnected = false;
            break;
            
        case WStype_CONNECTED:
            isConnected = true;
            sendStatusUpdate();
            break;
            
        case WStype_TEXT:
            //! ************************************************************************
            //! PARSE INCOMING JSON COMMANDS
            //! ************************************************************************
            String message = String((char*)payload);
            
            if (message.indexOf("\"command\":\"setHomeAngle\"") >= 0) {
                //! ************************************************************************
                //! EXTRACT ANGLE VALUE FROM JSON
                //! ************************************************************************
                int startIndex = message.indexOf("\"angle\":") + 8;
                int endIndex = message.indexOf("}", startIndex);
                if (startIndex > 7 && endIndex > startIndex) {
                    String angleStr = message.substring(startIndex, endIndex);
                    float newAngle = angleStr.toFloat();
                    
                    //! ************************************************************************
                    //! VALIDATE ANGLE RANGE (0-180 degrees)
                    //! ************************************************************************
                    if (newAngle >= 0.0f && newAngle <= 180.0f) {
                        setHomeAngle(newAngle);
                    }
                }
            }
            break;
    }
}

void WebDashboard::sendStatusUpdate() {
    if (isConnected && homeAnglePtr != nullptr) {
        //! ************************************************************************
        //! CALCULATE ALL TIME PERIOD AVERAGES
        //! ************************************************************************
        CycleAverages averages = calculateAllAverages();
        
        String json = "{";
        json += "\"type\":\"status\",";
        json += "\"homeAngle\":" + String(*homeAnglePtr, 1) + ",";
        json += "\"totalCycles\":" + String(totalCycles) + ",";
        json += "\"average1Min\":" + String(averages.average1Min, 1) + ",";
        json += "\"average5Min\":" + String(averages.average5Min, 1) + ",";
        json += "\"average15Min\":" + String(averages.average15Min, 1) + ",";
        json += "\"average30Min\":" + String(averages.average30Min, 1);
        json += "}";
        webSocket->broadcastTXT(json);
    }
}

void WebDashboard::saveHomeAngleToEEPROM() {
    if (homeAnglePtr != nullptr) {
        EEPROM.put(HOME_ANGLE_ADDR, *homeAnglePtr);
        EEPROM.commit();
    }
}

void WebDashboard::loadHomeAngleFromEEPROM() {
    if (homeAnglePtr != nullptr) {
        float savedAngle;
        EEPROM.get(HOME_ANGLE_ADDR, savedAngle);
        
        //! ************************************************************************
        //! VALIDATE LOADED VALUE (check for uninitialized EEPROM)
        //! ************************************************************************
        if (savedAngle >= 0.0f && savedAngle <= 180.0f) {
            *homeAnglePtr = savedAngle;
        }
    }
}

String WebDashboard::getDashboardHTML() {
    return R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Router Control Dashboard</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            color: #333;
        }
        
        .dashboard {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 20px;
            padding: 40px;
            box-shadow: 0 20px 40px rgba(0, 0, 0, 0.1);
            backdrop-filter: blur(10px);
            max-width: 1400px;
            width: 95%;
            text-align: center;
            margin: 20px auto;
        }
        
        .main-layout {
            display: grid;
            grid-template-columns: 1fr 2fr;
            gap: 30px;
            margin: 30px 0;
        }
        
        .left-column, .right-column {
            display: flex;
            flex-direction: column;
            gap: 20px;
        }
        
        .calendar-section {
            margin-top: 30px;
        }
        
        .calendar-layout {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 30px;
            align-items: start;
        }
        
        .title {
            font-size: 2.5em;
            margin-bottom: 10px;
            background: linear-gradient(45deg, #667eea, #764ba2);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
        }
        
        .subtitle {
            color: #666;
            margin-bottom: 40px;
            font-size: 1.1em;
        }
        
        .control-group {
            margin-bottom: 30px;
        }
        
        .control-label {
            display: block;
            font-size: 1.2em;
            margin-bottom: 15px;
            font-weight: 600;
            color: #444;
        }
        
        .angle-display {
            font-size: 3em;
            font-weight: bold;
            color: #667eea;
            margin: 20px 0;
            text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.1);
        }
        
        .slider-container {
            position: relative;
            margin: 20px 0;
        }
        
        .angle-slider {
            width: 100%;
            height: 8px;
            border-radius: 5px;
            background: #ddd;
            outline: none;
            -webkit-appearance: none;
            appearance: none;
        }
        
        .angle-slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 25px;
            height: 25px;
            border-radius: 50%;
            background: linear-gradient(45deg, #667eea, #764ba2);
            cursor: pointer;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
        }
        
        .angle-slider::-moz-range-thumb {
            width: 25px;
            height: 25px;
            border-radius: 50%;
            background: linear-gradient(45deg, #667eea, #764ba2);
            cursor: pointer;
            border: none;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
        }
        
        .angle-input {
            width: 100px;
            padding: 10px;
            border: 2px solid #ddd;
            border-radius: 10px;
            font-size: 1.1em;
            text-align: center;
            margin: 0 10px;
            transition: border-color 0.3s ease;
        }
        
        .angle-input:focus {
            outline: none;
            border-color: #667eea;
        }
        
        .set-button {
            background: linear-gradient(45deg, #667eea, #764ba2);
            color: white;
            border: none;
            padding: 12px 30px;
            border-radius: 25px;
            font-size: 1.1em;
            cursor: pointer;
            transition: transform 0.2s ease, box-shadow 0.2s ease;
            margin: 10px;
        }
        
        .set-button:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 16px rgba(0, 0, 0, 0.2);
        }
        
        .set-button:active {
            transform: translateY(0);
        }
        
        .status {
            margin-top: 30px;
            padding: 15px;
            border-radius: 10px;
            font-weight: 600;
        }
        
        .status.connected {
            background: rgba(76, 175, 80, 0.2);
            color: #2e7d32;
            border: 2px solid #4caf50;
        }
        
        .status.disconnected {
            background: rgba(244, 67, 54, 0.2);
            color: #c62828;
            border: 2px solid #f44336;
        }
        
        .preset-buttons {
            display: flex;
            justify-content: center;
            gap: 10px;
            margin: 20px 0;
            flex-wrap: wrap;
        }
        
        .preset-btn {
            background: rgba(102, 126, 234, 0.1);
            color: #667eea;
            border: 2px solid #667eea;
            padding: 8px 16px;
            border-radius: 20px;
            cursor: pointer;
            transition: all 0.3s ease;
            font-size: 0.9em;
        }
        
        .preset-btn:hover {
            background: #667eea;
            color: white;
        }
        
        .stats-container {
            display: grid;
            grid-template-columns: repeat(5, 1fr);
            gap: 15px;
            margin: 20px 0;
        }
        
        .stat-item {
            text-align: center;
            flex: 1;
            position: relative;
        }
        
        .stat-value {
            font-size: 2.5em;
            font-weight: bold;
            color: #667eea;
            margin-bottom: 5px;
            transition: all 0.3s ease;
        }
        
        .stat-value.collecting {
            color: #ff9500;
            animation: pulse 2s infinite;
        }
        
        .stat-value.waiting {
            color: #999;
            opacity: 0.7;
        }
        
        .stat-label {
            font-size: 0.9em;
            color: #666;
            font-weight: 500;
        }
        
        .stat-label.waiting {
            color: #999;
        }
        
        .time-indicator {
            font-size: 0.7em;
            color: #ff9500;
            font-weight: 600;
            margin-top: 2px;
            animation: blink 1.5s infinite;
        }
        
        .time-indicator.waiting {
            color: #999;
            animation: none;
        }
        
        @keyframes pulse {
            0% { transform: scale(1); }
            50% { transform: scale(1.05); }
            100% { transform: scale(1); }
        }
        
        @keyframes blink {
            0%, 50% { opacity: 1; }
            51%, 100% { opacity: 0.3; }
        }
        
        .graph-container {
            margin-top: 20px;
            text-align: center;
        }
        
        #triggerGraph {
            border: 2px solid #ddd;
            border-radius: 10px;
            background: #f9f9f9;
            max-width: 100%;
            height: auto;
        }
        
        .calendar-container {
            background: white;
            border-radius: 15px;
            padding: 20px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            margin: 20px 0;
        }
        
        .calendar-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 20px;
        }
        
        .nav-btn {
            background: #667eea;
            color: white;
            border: none;
            width: 40px;
            height: 40px;
            border-radius: 50%;
            font-size: 20px;
            cursor: pointer;
            transition: all 0.3s ease;
        }
        
        .nav-btn:hover {
            background: #5a6fd8;
            transform: scale(1.1);
        }
        
        #calendarMonth {
            margin: 0;
            color: #333;
            font-size: 1.3em;
        }
        
        .calendar-grid {
            display: grid;
            grid-template-columns: repeat(7, 1fr);
            gap: 8px;
        }
        
        .calendar-day {
            aspect-ratio: 1;
            display: flex;
            align-items: center;
            justify-content: center;
            border-radius: 8px;
            cursor: pointer;
            transition: all 0.3s ease;
            font-weight: 500;
            position: relative;
        }
        
        .calendar-day:hover {
            background: #f0f0f0;
            transform: scale(1.05);
        }
        
        .calendar-day.has-data {
            background: linear-gradient(135deg, #667eea, #764ba2);
            color: white;
        }
        
        .calendar-day.has-data:hover {
            background: linear-gradient(135deg, #5a6fd8, #6a4190);
        }
        
        .calendar-day.selected {
            background: #ff6b6b;
            color: white;
            transform: scale(1.1);
        }
        
        .calendar-day.other-month {
            color: #ccc;
        }
        
        .calendar-day.today {
            border: 2px solid #667eea;
        }
        
        .daily-stats {
            background: white;
            border-radius: 15px;
            padding: 20px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            margin-top: 20px;
        }
        
        .daily-stats h4 {
            margin: 0 0 15px 0;
            color: #333;
            text-align: center;
        }
        
        .hourly-chart {
            text-align: center;
            margin: 20px 0;
        }
        
        #hourlyChart {
            border: 2px solid #ddd;
            border-radius: 10px;
            background: #f9f9f9;
            max-width: 100%;
            height: auto;
        }
        
        .daily-summary {
            display: flex;
            justify-content: space-around;
            margin-top: 20px;
        }
        
        .summary-item {
            text-align: center;
        }
        
        .summary-label {
            display: block;
            font-size: 0.9em;
            color: #666;
            margin-bottom: 5px;
        }
        
        .summary-value {
            display: block;
            font-size: 1.5em;
            font-weight: bold;
            color: #667eea;
        }
        
        /*! ************************************************************************
        //! RESPONSIVE DESIGN
        //! ************************************************************************
        @media (max-width: 1200px) {
            .main-layout {
                grid-template-columns: 1fr;
                gap: 20px;
            }
            
            .calendar-layout {
                grid-template-columns: 1fr;
                gap: 20px;
            }
            
            .stats-container {
                grid-template-columns: repeat(2, 1fr);
            }
        }
        
        @media (max-width: 768px) {
            .dashboard {
                padding: 20px;
                margin: 10px;
                width: calc(100% - 20px);
            }
            
            .stats-container {
                grid-template-columns: 1fr;
                gap: 10px;
            }
            
            .preset-buttons {
                flex-wrap: wrap;
                gap: 5px;
            }
            
            .preset-btn {
                flex: 1;
                min-width: 60px;
            }
            
            #triggerGraph, #hourlyChart {
                width: 100% !important;
                height: auto !important;
            }
        }
    </style>
</head>
<body>
    <div class="dashboard">
        <h1 class="title">Router Control Dashboard</h1>
        <p class="subtitle">Production Monitoring & Control System</p>
        
        <div class="main-layout">
            <!-- LEFT COLUMN -->
            <div class="left-column">
                <div class="control-group">
                    <label class="control-label">Servo Control</label>
                    <div class="angle-display" id="angleDisplay">90.0°</div>
                    
                    <div class="slider-container">
                        <input type="range" class="angle-slider" id="angleSlider" 
                               min="0" max="180" step="0.1" value="90">
                    </div>
                    
                    <div>
                        <input type="number" class="angle-input" id="angleInput" 
                               min="0" max="180" step="0.1" value="90">
                        <button class="set-button" onclick="setAngle()">Set Angle</button>
                    </div>
                    
                    <div class="preset-buttons">
                        <button class="preset-btn" onclick="setPresetAngle(0)">0°</button>
                        <button class="preset-btn" onclick="setPresetAngle(45)">45°</button>
                        <button class="preset-btn" onclick="setPresetAngle(90)">90°</button>
                        <button class="preset-btn" onclick="setPresetAngle(135)">135°</button>
                        <button class="preset-btn" onclick="setPresetAngle(180)">180°</button>
                    </div>
                </div>
                
                <div class="status disconnected" id="connectionStatus">
                    Disconnected
                </div>
            </div>
            
            <!-- RIGHT COLUMN -->
            <div class="right-column">
                <div class="control-group">
                    <label class="control-label">Real-Time Statistics</label>
                    <div class="stats-container">
                        <div class="stat-item">
                            <div class="stat-value" id="totalCycles">0</div>
                            <div class="stat-label">Total Cycles</div>
                        </div>
                        <div class="stat-item">
                            <div class="stat-value" id="average1Min">0</div>
                            <div class="stat-label">Cycles (1min)</div>
                            <div class="time-indicator" id="indicator1Min"></div>
                        </div>
                        <div class="stat-item">
                            <div class="stat-value" id="average5Min">0.00</div>
                            <div class="stat-label">Cycles/Min (5min)</div>
                            <div class="time-indicator" id="indicator5Min"></div>
                        </div>
                        <div class="stat-item">
                            <div class="stat-value" id="average15Min">0.00</div>
                            <div class="stat-label">Cycles/Min (10min)</div>
                            <div class="time-indicator" id="indicator15Min"></div>
                        </div>
                        <div class="stat-item">
                            <div class="stat-value" id="average30Min">0.00</div>
                            <div class="stat-label">Cycles/Min (15min)</div>
                            <div class="time-indicator" id="indicator30Min"></div>
                        </div>
                    </div>
                    <div class="graph-container">
                        <canvas id="triggerGraph" width="600" height="200"></canvas>
                    </div>
                </div>
            </div>
        </div>
        
        <!-- FULL WIDTH CALENDAR SECTION -->
        <div class="control-group calendar-section">
            <label class="control-label">Daily Production Calendar</label>
            <div class="calendar-layout">
                <div class="calendar-container">
                    <div class="calendar-header">
                        <button class="nav-btn" onclick="previousMonth()">‹</button>
                        <h3 id="calendarMonth">January 2024</h3>
                        <button class="nav-btn" onclick="nextMonth()">›</button>
                    </div>
                    <div class="calendar-grid" id="calendarGrid">
                        <!-- Calendar will be generated here -->
                    </div>
                </div>
                
                <div class="daily-stats" id="dailyStats" style="display: none;">
                    <h4 id="selectedDate">Selected Date</h4>
                    <div class="hourly-chart">
                        <canvas id="hourlyChart" width="600" height="200"></canvas>
                    </div>
                    <div class="daily-summary">
                        <div class="summary-item">
                            <span class="summary-label">Total Cycles:</span>
                            <span class="summary-value" id="dailyTotal">0</span>
                        </div>
                        <div class="summary-item">
                            <span class="summary-label">Peak Hour:</span>
                            <span class="summary-value" id="peakHour">-</span>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <script>
        let ws;
        let currentAngle = 90.0;
        let totalCycles = 0;
        let average1Min = 0.0;
        let average5Min = 0.0;
        let average15Min = 0.0;
        let average30Min = 0.0;
        let graphData = [];
        let canvas, ctx;
        
        //! ************************************************************************
        //! CALENDAR VARIABLES
        //! ************************************************************************
        let currentDate = new Date();
        let selectedDate = null;
        let hourlyCanvas, hourlyCtx;
        let dailyData = {};
        
        //! ************************************************************************
        //! INITIALIZE WEBSOCKET CONNECTION
        //! ************************************************************************
        function initWebSocket() {
            const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
            const wsUrl = protocol + '//' + window.location.hostname + ':81';
            
            ws = new WebSocket(wsUrl);
            
            ws.onopen = function() {
                updateConnectionStatus(true);
            };
            
            ws.onclose = function() {
                updateConnectionStatus(false);
                //! ************************************************************************
                //! RECONNECT AFTER 3 SECONDS
                //! ************************************************************************
                setTimeout(initWebSocket, 3000);
            };
            
            ws.onmessage = function(event) {
                try {
                    const data = JSON.parse(event.data);
                    if (data.type === 'status') {
                        currentAngle = data.homeAngle;
                        totalCycles = data.totalCycles || 0;
                        average1Min = data.average1Min || 0.0;
                        average5Min = data.average5Min || 0.0;
                        average15Min = data.average15Min || 0.0;
                        average30Min = data.average30Min || 0.0;
                        updateDisplay(currentAngle);
                        updateStatistics();
                        updateGraph();
                    }
                } catch (e) {
                    console.error('Error parsing message:', e);
                }
            };
            
            ws.onerror = function(error) {
                console.error('WebSocket error:', error);
            };
        }
        
        //! ************************************************************************
        //! UPDATE CONNECTION STATUS DISPLAY
        //! ************************************************************************
        function updateConnectionStatus(connected) {
            const status = document.getElementById('connectionStatus');
            if (connected) {
                status.textContent = 'Connected';
                status.className = 'status connected';
            } else {
                status.textContent = 'Disconnected';
                status.className = 'status disconnected';
            }
        }
        
        //! ************************************************************************
        //! UPDATE ANGLE DISPLAY
        //! ************************************************************************
        function updateDisplay(angle) {
            document.getElementById('angleDisplay').textContent = angle.toFixed(1) + '°';
            document.getElementById('angleSlider').value = angle;
            document.getElementById('angleInput').value = angle;
        }
        
        //! ************************************************************************
        //! SET ANGLE FROM INPUT OR SLIDER
        //! ************************************************************************
        function setAngle() {
            const input = document.getElementById('angleInput');
            const angle = parseFloat(input.value);
            
            if (angle >= 0 && angle <= 180) {
                sendAngleCommand(angle);
            } else {
                alert('Please enter an angle between 0 and 180 degrees');
            }
        }
        
        //! ************************************************************************
        //! SET PRESET ANGLE
        //! ************************************************************************
        function setPresetAngle(angle) {
            sendAngleCommand(angle);
        }
        
        //! ************************************************************************
        //! SEND ANGLE COMMAND TO ESP32
        //! ************************************************************************
        function sendAngleCommand(angle) {
            if (ws && ws.readyState === WebSocket.OPEN) {
                const command = {
                    command: 'setHomeAngle',
                    angle: angle
                };
                ws.send(JSON.stringify(command));
            }
        }
        
        //! ************************************************************************
        //! EVENT LISTENERS
        //! ************************************************************************
        document.getElementById('angleSlider').addEventListener('input', function() {
            const angle = parseFloat(this.value);
            document.getElementById('angleInput').value = angle;
            document.getElementById('angleDisplay').textContent = angle.toFixed(1) + '°';
        });
        
        document.getElementById('angleInput').addEventListener('input', function() {
            const angle = parseFloat(this.value);
            if (angle >= 0 && angle <= 180) {
                document.getElementById('angleSlider').value = angle;
                document.getElementById('angleDisplay').textContent = angle.toFixed(1) + '°';
            }
        });
        
        document.getElementById('angleInput').addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                setAngle();
            }
        });
        
        //! ************************************************************************
        //! UPDATE STATISTICS DISPLAY
        //! ************************************************************************
        function updateStatistics() {
            document.getElementById('totalCycles').textContent = totalCycles;
            
            //! ************************************************************************
            //! UPDATE 1 MINUTE AVERAGE
            //! ************************************************************************
            const avg1MinEl = document.getElementById('average1Min');
            const indicator1MinEl = document.getElementById('indicator1Min');
            avg1MinEl.textContent = Math.round(average1Min);
            
            if (average1Min > 0) {
                avg1MinEl.className = 'stat-value collecting';
                indicator1MinEl.textContent = '';
                indicator1MinEl.className = 'time-indicator';
            } else {
                avg1MinEl.className = 'stat-value';
                indicator1MinEl.textContent = '';
                indicator1MinEl.className = 'time-indicator';
            }
            
            //! ************************************************************************
            //! UPDATE 5 MINUTE AVERAGE
            //! ************************************************************************
            const avg5MinEl = document.getElementById('average5Min');
            const indicator5MinEl = document.getElementById('indicator5Min');
            avg5MinEl.textContent = average5Min.toFixed(1);
            
            if (average5Min > 0) {
                avg5MinEl.className = 'stat-value collecting';
                indicator5MinEl.textContent = '';
                indicator5MinEl.className = 'time-indicator';
            } else {
                avg5MinEl.className = 'stat-value waiting';
                indicator5MinEl.textContent = '';
                indicator5MinEl.className = 'time-indicator waiting';
            }
            
            //! ************************************************************************
            //! UPDATE 10 MINUTE AVERAGE
            //! ************************************************************************
            const avg15MinEl = document.getElementById('average15Min');
            const indicator15MinEl = document.getElementById('indicator15Min');
            avg15MinEl.textContent = average15Min.toFixed(1);
            
            if (average15Min > 0) {
                avg15MinEl.className = 'stat-value collecting';
                indicator15MinEl.textContent = '';
                indicator15MinEl.className = 'time-indicator';
            } else {
                avg15MinEl.className = 'stat-value waiting';
                indicator15MinEl.textContent = '';
                indicator15MinEl.className = 'time-indicator waiting';
            }
            
            //! ************************************************************************
            //! UPDATE 15 MINUTE AVERAGE
            //! ************************************************************************
            const avg30MinEl = document.getElementById('average30Min');
            const indicator30MinEl = document.getElementById('indicator30Min');
            avg30MinEl.textContent = average30Min.toFixed(1);
            
            if (average30Min > 0) {
                avg30MinEl.className = 'stat-value collecting';
                indicator30MinEl.textContent = '';
                indicator30MinEl.className = 'time-indicator';
            } else {
                avg30MinEl.className = 'stat-value waiting';
                indicator30MinEl.textContent = '';
                indicator30MinEl.className = 'time-indicator waiting';
            }
        }
        
        //! ************************************************************************
        //! UPDATE GRAPH DISPLAY
        //! ************************************************************************
        function updateGraph() {
            if (!canvas || !ctx) return;
            
            //! ************************************************************************
            //! ADD NEW DATA POINT
            //! ************************************************************************
            const now = new Date();
            graphData.push({
                time: now,
                value: average15Min
            });
            
        //! ************************************************************************
        //! KEEP EXACTLY 10 DATA POINTS (ONE PER MINUTE)
        //! ************************************************************************
        if (graphData.length > 10) {
            graphData = graphData.slice(-10);
        }
        
        //! ************************************************************************
        //! CLEAR GRAPH DATA IF NO RECENT CYCLES (RESET AFTER 5 MINUTES OF INACTIVITY)
        //! ************************************************************************
        const fiveMinutesAgo = new Date(now.getTime() - 5 * 60 * 1000);
        if (graphData.length > 0 && graphData[graphData.length - 1].time < fiveMinutesAgo) {
            graphData = [];
        }
            
            //! ************************************************************************
            //! CLEAR CANVAS
            //! ************************************************************************
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            
            if (graphData.length < 2) return;
            
            //! ************************************************************************
            //! DRAW GRAPH WITH FIXED Y-AXIS (0-10)
            //! ************************************************************************
            const padding = 40;
            const graphWidth = canvas.width - 2 * padding;
            const graphHeight = canvas.height - 2 * padding;
            
            //! ************************************************************************
            //! FIXED Y-AXIS RANGE: 0-10
            //! ************************************************************************
            const minValue = 0;
            const maxValue = 10;
            const valueRange = maxValue - minValue;
            
            //! ************************************************************************
            //! DRAW GRID LINES (0-10 RANGE) - CLEANER SPACING
            //! ************************************************************************
            ctx.strokeStyle = '#e0e0e0';
            ctx.lineWidth = 1;
            // Only draw grid lines every 2 units to match labels
            for (let i = 0; i <= 10; i += 2) {
                const y = padding + (i * graphHeight / 10);
                ctx.beginPath();
                ctx.moveTo(padding, y);
                ctx.lineTo(canvas.width - padding, y);
                ctx.stroke();
            }
            
            //! ************************************************************************
            //! DRAW Y-AXIS LABELS (0-10 RANGE) - CLEANER SPACING
            //! ************************************************************************
            ctx.fillStyle = '#666';
            ctx.font = '12px Arial';
            ctx.textAlign = 'right';
            // Only show every other label to avoid overlap
            for (let i = 0; i <= 10; i += 2) {
                const value = 10 - i;
                const y = padding + (i * graphHeight / 10) + 4;
                ctx.fillText(value.toString(), padding - 5, y);
            }
            
            //! ************************************************************************
            //! DRAW GRAPH LINE AND DOTS (10 POINTS, 0-10 RANGE)
            //! ************************************************************************
            ctx.strokeStyle = '#667eea';
            ctx.lineWidth = 2;
            ctx.beginPath();
            
            // Draw line connecting the points
            for (let i = 0; i < graphData.length; i++) {
                const x = padding + (i * graphWidth / 9); // 9 intervals for 10 points
                const y = canvas.height - padding - ((graphData[i].value - minValue) / valueRange * graphHeight);
                
                if (i === 0) {
                    ctx.moveTo(x, y);
                } else {
                    ctx.lineTo(x, y);
                }
            }
            ctx.stroke();
            
            // Draw dots for each data point
            ctx.fillStyle = '#667eea';
            for (let i = 0; i < graphData.length; i++) {
                const x = padding + (i * graphWidth / 9); // 9 intervals for 10 points
                const y = canvas.height - padding - ((graphData[i].value - minValue) / valueRange * graphHeight);
                
                ctx.beginPath();
                ctx.arc(x, y, 4, 0, 2 * Math.PI);
                ctx.fill();
            }
            
            
            //! ************************************************************************
            //! DRAW TITLE
            //! ************************************************************************
            ctx.textAlign = 'center';
            ctx.font = 'bold 14px Arial';
            ctx.fillText('Real-Time Production Rate (10 min rolling avg)', canvas.width / 2, 20);
        }
        
        //! ************************************************************************
        //! TIME FORMATTING FUNCTIONS
        //! ************************************************************************
        function format12Hour(hour24) {
            if (hour24 === 0) return '12:00 AM';
            if (hour24 < 12) return hour24 + ':00 AM';
            if (hour24 === 12) return '12:00 PM';
            return (hour24 - 12) + ':00 PM';
        }
        
        //! ************************************************************************
        //! CALENDAR FUNCTIONS
        //! ************************************************************************
        function generateCalendar() {
            const year = currentDate.getFullYear();
            const month = currentDate.getMonth();
            const monthNames = ['January', 'February', 'March', 'April', 'May', 'June',
                               'July', 'August', 'September', 'October', 'November', 'December'];
            
            document.getElementById('calendarMonth').textContent = monthNames[month] + ' ' + year;
            
            const firstDay = new Date(year, month, 1);
            const lastDay = new Date(year, month + 1, 0);
            const daysInMonth = lastDay.getDate();
            const startingDayOfWeek = firstDay.getDay();
            
            const calendarGrid = document.getElementById('calendarGrid');
            calendarGrid.innerHTML = '';
            
            //! ************************************************************************
            //! ADD DAY HEADERS
            //! ************************************************************************
            const dayHeaders = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
            dayHeaders.forEach(day => {
                const dayHeader = document.createElement('div');
                dayHeader.textContent = day;
                dayHeader.style.fontWeight = 'bold';
                dayHeader.style.textAlign = 'center';
                dayHeader.style.padding = '10px 0';
                dayHeader.style.color = '#666';
                calendarGrid.appendChild(dayHeader);
            });
            
            //! ************************************************************************
            //! ADD EMPTY CELLS FOR DAYS BEFORE MONTH STARTS
            //! ************************************************************************
            for (let i = 0; i < startingDayOfWeek; i++) {
                const emptyDay = document.createElement('div');
                emptyDay.className = 'calendar-day other-month';
                calendarGrid.appendChild(emptyDay);
            }
            
            //! ************************************************************************
            //! ADD DAYS OF THE MONTH
            //! ************************************************************************
            const today = new Date();
            for (let day = 1; day <= daysInMonth; day++) {
                const dayElement = document.createElement('div');
                dayElement.className = 'calendar-day';
                dayElement.textContent = day;
                
                //! ************************************************************************
                //! CHECK IF THIS IS TODAY
                //! ************************************************************************
                if (year === today.getFullYear() && month === today.getMonth() && day === today.getDate()) {
                    dayElement.classList.add('today');
                }
                
                //! ************************************************************************
                //! CHECK IF THIS DAY HAS DATA
                //! ************************************************************************
                const dayKey = month + 1 + '-' + day;
                if (dailyData[dayKey]) {
                    dayElement.classList.add('has-data');
                }
                
                //! ************************************************************************
                //! ADD CLICK EVENT
                //! ************************************************************************
                dayElement.addEventListener('click', () => selectDate(day, month + 1));
                
                calendarGrid.appendChild(dayElement);
            }
        }
        
        //! ************************************************************************
        //! LOAD CALENDAR DATA FOR CURRENT MONTH
        //! ************************************************************************
        function loadCalendarData() {
            const month = currentDate.getMonth() + 1; // JavaScript months are 0-based
            fetch(`/calendar-data?month=${month}`)
                .then(response => response.json())
                .then(data => {
                    //! ************************************************************************
                    //! CLEAR EXISTING DATA
                    //! ************************************************************************
                    dailyData = {};
                    
                    //! ************************************************************************
                    //! POPULATE DAILY DATA OBJECT
                    //! ************************************************************************
                    data.daysWithData.forEach(day => {
                        const dayKey = month + '-' + day;
                        dailyData[dayKey] = true;
                    });
                    
                    //! ************************************************************************
                    //! REGENERATE CALENDAR WITH NEW DATA
                    //! ************************************************************************
                    generateCalendar();
                })
                .catch(error => {
                    console.error('Error loading calendar data:', error);
                });
        }
        
        function selectDate(day, month) {
            selectedDate = { day: day, month: month };
            
            //! ************************************************************************
            //! UPDATE SELECTED DAY VISUALLY
            //! ************************************************************************
            document.querySelectorAll('.calendar-day').forEach(el => {
                el.classList.remove('selected');
            });
            event.target.classList.add('selected');
            
            //! ************************************************************************
            //! LOAD DAILY STATS
            //! ************************************************************************
            loadDailyStats(day, month);
        }
        
        function loadDailyStats(day, month) {
            fetch(`/daily-stats?day=${day}&month=${month}`)
                .then(response => response.json())
                .then(data => {
                    document.getElementById('selectedDate').textContent = 
                        `Production Data - ${month}/${day}`;
                    document.getElementById('dailyTotal').textContent = data.totalCycles;
                    
                    //! ************************************************************************
                    //! FIND PEAK HOUR
                    //! ************************************************************************
                    let peakHour = 0;
                    let maxCycles = 0;
                    data.hourlyData.forEach(hour => {
                        if (hour.cycles > maxCycles) {
                            maxCycles = hour.cycles;
                            peakHour = hour.hour;
                        }
                    });
                    
                    document.getElementById('peakHour').textContent = 
                        format12Hour(peakHour) + ' (' + maxCycles + ' cycles)';
                    
                    //! ************************************************************************
                    //! DRAW HOURLY CHART
                    //! ************************************************************************
                    drawHourlyChart(data.hourlyData);
                    
                    //! ************************************************************************
                    //! SHOW DAILY STATS
                    //! ************************************************************************
                    document.getElementById('dailyStats').style.display = 'block';
                })
                .catch(error => {
                    console.error('Error loading daily stats:', error);
                });
        }
        
        function drawHourlyChart(hourlyData) {
            if (!hourlyCanvas || !hourlyCtx) return;
            
            const canvas = hourlyCanvas;
            const ctx = hourlyCtx;
            
            //! ************************************************************************
            //! CLEAR CANVAS
            //! ************************************************************************
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            
            if (hourlyData.length === 0) {
                ctx.fillStyle = '#666';
                ctx.font = '16px Arial';
                ctx.textAlign = 'center';
                ctx.fillText('No data available for this day', canvas.width / 2, canvas.height / 2);
                return;
            }
            
            //! ************************************************************************
            //! PREPARE DATA
            //! ************************************************************************
            const maxCycles = Math.max(...hourlyData.map(h => h.cycles));
            const barWidth = canvas.width / 24;
            const maxHeight = canvas.height - 60;
            
            //! ************************************************************************
            //! DRAW BARS
            //! ************************************************************************
            hourlyData.forEach(hour => {
                const x = hour.hour * barWidth;
                const height = (hour.cycles / maxCycles) * maxHeight;
                const y = canvas.height - height - 30;
                
                //! ************************************************************************
                //! DRAW BAR
                //! ************************************************************************
                ctx.fillStyle = '#667eea';
                ctx.fillRect(x + 2, y, barWidth - 4, height);
                
                //! ************************************************************************
                //! DRAW VALUE LABEL
                //! ************************************************************************
                if (hour.cycles > 0) {
                    ctx.fillStyle = '#333';
                    ctx.font = '10px Arial';
                    ctx.textAlign = 'center';
                    ctx.fillText(hour.cycles.toString(), x + barWidth / 2, y - 5);
                }
            });
            
            //! ************************************************************************
            //! DRAW HOUR LABELS (12-HOUR FORMAT)
            //! ************************************************************************
            ctx.fillStyle = '#666';
            ctx.font = '10px Arial';
            ctx.textAlign = 'center';
            for (let i = 0; i < 24; i += 4) {
                const timeLabel = format12Hour(i);
                ctx.fillText(timeLabel, i * barWidth + barWidth / 2, canvas.height - 10);
            }
            
            //! ************************************************************************
            //! DRAW TITLE
            //! ************************************************************************
            ctx.fillStyle = '#333';
            ctx.font = 'bold 14px Arial';
            ctx.textAlign = 'center';
            ctx.fillText('Hourly Production', canvas.width / 2, 20);
        }
        
        function previousMonth() {
            currentDate.setMonth(currentDate.getMonth() - 1);
            loadCalendarData();
        }
        
        function nextMonth() {
            currentDate.setMonth(currentDate.getMonth() + 1);
            loadCalendarData();
        }
        
        //! ************************************************************************
        //! INITIALIZE ON PAGE LOAD
        //! ************************************************************************
        window.addEventListener('load', function() {
            canvas = document.getElementById('triggerGraph');
            ctx = canvas.getContext('2d');
            
            hourlyCanvas = document.getElementById('hourlyChart');
            hourlyCtx = hourlyCanvas.getContext('2d');
            
            initWebSocket();
            loadCalendarData();
            
            //! ************************************************************************
            //! UPDATE GRAPH EVERY 30 SECONDS
            //! ************************************************************************
            setInterval(updateGraph, 30000);
        });
    </script>
</body>
</html>
)rawliteral";
}

//* ************************************************************************
//* ********************** CONTROL METHODS **********************************
//* ************************************************************************
void WebDashboard::setHomeAngle(float angle) {
    if (homeAnglePtr != nullptr && angle >= 0.0f && angle <= 180.0f) {
        *homeAnglePtr = angle;
        saveHomeAngleToEEPROM();
        
        //! ************************************************************************
        //! IMMEDIATELY MOVE SERVO TO NEW ANGLE
        //! ************************************************************************
        if (servoPtr != nullptr) {
            ServoControl* servo = static_cast<ServoControl*>(servoPtr);
            servo->write(angle);
        }
        
        sendStatusUpdate();
    }
}

void WebDashboard::update() {
    //! ************************************************************************
    //! DEFAULT UPDATE - ASSUME IDLE STATE FOR BACKWARD COMPATIBILITY
    //! ************************************************************************
    update(true);
}

void WebDashboard::update(bool isIdleState) {
    //! ************************************************************************
    //! ONLY PROCESS WEBSOCKET EVENTS WHEN MACHINE IS IN IDLE STATE
    //! ************************************************************************
    static bool wasInActiveCycle = false;
    static unsigned long lastStatusUpdate = 0;
    
    if (webSocket != nullptr && isIdleState) {
        webSocket->loop();
        
        //! ************************************************************************
        //! HANDLE DEFERRED OPERATIONS WHEN IDLE
        //! ************************************************************************
        unsigned long currentTime = millis();
        
        //! ************************************************************************
        //! DETECT CYCLE COMPLETION (TRANSITION FROM ACTIVE CYCLE TO IDLE)
        //! ************************************************************************
        if (wasInActiveCycle) {
            // Machine just completed a cycle and returned to idle - save EEPROM data
            saveTotalCyclesToEEPROM();
            saveCycleDataToEEPROM();
            
            // Update display immediately after cycle completion
            if (isConnected) {
                sendStatusUpdate();
                lastStatusUpdate = currentTime;
            }
        } else {
            // Machine was already idle, ping websocket every 3 seconds with fresh data
            if (isConnected && currentTime - lastStatusUpdate > 2000) {
                sendStatusUpdate();
                lastStatusUpdate = currentTime;
            }
            
            //! ************************************************************************
            //! SYNC TO GOOGLE SHEETS EVERY 30 SECONDS
            //! ************************************************************************
            if (currentTime - lastCloudSync > CLOUD_SYNC_INTERVAL) {
                syncToGoogleSheets();
                lastCloudSync = currentTime;
            }
        }
        
        wasInActiveCycle = false; // Reset flag for next cycle
    } else {
        // Machine is in active cycle
        wasInActiveCycle = true;
    }
}

//* ************************************************************************
//* ********************** STATUS METHODS ***********************************
//* ************************************************************************
bool WebDashboard::isClientConnected() {
    return isConnected;
}

void WebDashboard::broadcastStatus() {
    sendStatusUpdate();
}

//* ************************************************************************
//* ********************** TRIGGER TRACKING METHODS ************************
//* ************************************************************************

void WebDashboard::recordCycle() {
    //! ************************************************************************
    //! INCREMENT TOTAL CYCLE COUNT (MINIMAL OPERATION)
    //! ************************************************************************
    totalCycles++;
    
    //! ************************************************************************
    //! IMMEDIATELY SAVE TOTAL CYCLES TO EEPROM FOR MAXIMUM RELIABILITY
    //! ************************************************************************
    saveTotalCyclesToEEPROM();
    
    //! ************************************************************************
    //! ADD NEW CYCLE RECORD (MINIMAL OPERATION)
    //! ************************************************************************
    addCycleRecord();
    
    //! ************************************************************************
    //! UPDATE HOURLY DATA (MINIMAL OPERATION)
    //! ************************************************************************
    updateHourlyData();
    
    //! ************************************************************************
    //! DEFER HEAVY OPERATIONS TO NON-CRITICAL TIME
    //! ************************************************************************
    // Other EEPROM operations and status updates will be handled in update() when idle
}

void WebDashboard::addCycleRecord() {
    //! ************************************************************************
    //! ADD CURRENT CYCLE TO BUFFER
    //! ************************************************************************
    cycleBuffer[cycleBufferIndex].timestamp = millis();
    cycleBuffer[cycleBufferIndex].cycle_count = totalCycles;
    
    //! ************************************************************************
    //! ADVANCE BUFFER INDEX (CIRCULAR)
    //! ************************************************************************
    cycleBufferIndex = (cycleBufferIndex + 1) % MAX_CYCLE_RECORDS;
}

WebDashboard::CycleAverages WebDashboard::calculateAllAverages() {
    //! ************************************************************************
    //! CALCULATE ALL TIME PERIOD AVERAGES - SIMPLE AND RELIABLE
    //! ************************************************************************
    CycleAverages averages = {0.0f, 0.0f, 0.0f, 0.0f};
    
    unsigned long currentTime = millis();
    unsigned long oneMinuteAgo = currentTime - (1 * 60 * 1000);
    unsigned long fiveMinutesAgo = currentTime - (5 * 60 * 1000);
    unsigned long tenMinutesAgo = currentTime - (10 * 60 * 1000);
    unsigned long fifteenMinutesAgo = currentTime - (15 * 60 * 1000);
    
    int validRecords1Min = 0;
    int validRecords5Min = 0;
    int validRecords10Min = 0;
    int validRecords15Min = 0;
    
    //! ************************************************************************
    //! COUNT CYCLES IN EACH TIME PERIOD
    //! ************************************************************************
    for (int i = 0; i < MAX_CYCLE_RECORDS; i++) {
        if (cycleBuffer[i].timestamp > 0) {
            //! ************************************************************************
            //! 1 MINUTE WINDOW
            //! ************************************************************************
            if (cycleBuffer[i].timestamp > oneMinuteAgo) {
                validRecords1Min++;
            }
            
            //! ************************************************************************
            //! 5 MINUTE WINDOW
            //! ************************************************************************
            if (cycleBuffer[i].timestamp > fiveMinutesAgo) {
                validRecords5Min++;
            }
            
            //! ************************************************************************
            //! 10 MINUTE WINDOW
            //! ************************************************************************
            if (cycleBuffer[i].timestamp > tenMinutesAgo) {
                validRecords10Min++;
            }
            
            //! ************************************************************************
            //! 15 MINUTE WINDOW
            //! ************************************************************************
            if (cycleBuffer[i].timestamp > fifteenMinutesAgo) {
                validRecords15Min++;
            }
        }
    }
    
    //! ************************************************************************
    //! CALCULATE CYCLES PER MINUTE FOR EACH TIME PERIOD
    //! ************************************************************************
    averages.average1Min = (float)validRecords1Min; // Cycles in past 1 minute
    averages.average5Min = (float)validRecords5Min / 5.0f; // Cycles per minute over 5 minutes
    averages.average15Min = (float)validRecords10Min / 10.0f; // Cycles per minute over 10 minutes
    averages.average30Min = (float)validRecords15Min / 15.0f; // Cycles per minute over 15 minutes
    
    return averages;
}

void WebDashboard::saveCycleDataToEEPROM() {
    //! ************************************************************************
    //! SAVE CYCLE BUFFER TO EEPROM
    //! ************************************************************************
    EEPROM.put(TRIGGER_DATA_ADDR, cycleBuffer);
    EEPROM.put(TRIGGER_DATA_ADDR + CYCLE_BUFFER_SIZE, cycleBufferIndex);
    EEPROM.commit();
}

void WebDashboard::saveTotalCyclesToEEPROM() {
    //! ************************************************************************
    //! SAVE TOTAL CYCLES WITH BACKUP FOR MAXIMUM RELIABILITY
    //! ************************************************************************
    EEPROM.put(TOTAL_CYCLES_ADDR, totalCycles);
    EEPROM.put(TOTAL_CYCLES_BACKUP_ADDR, totalCycles);
    EEPROM.commit();
}

void WebDashboard::loadCycleDataFromEEPROM() {
    //! ************************************************************************
    //! CHECK DATA VERSION COMPATIBILITY
    //! ************************************************************************
    int storedVersion = 0;
    EEPROM.get(DATA_VERSION_ADDR, storedVersion);
    
    //! ************************************************************************
    //! LOAD TOTAL CYCLES WITH BACKUP VERIFICATION - NEVER RESET THIS VALUE
    //! ************************************************************************
    uint32_t primaryCycles = 0;
    uint32_t backupCycles = 0;
    
    EEPROM.get(TOTAL_CYCLES_ADDR, primaryCycles);
    EEPROM.get(TOTAL_CYCLES_BACKUP_ADDR, backupCycles);
    
    //! ************************************************************************
    //! VALIDATE LOADED VALUES - DETECT UNINITIALIZED OR CORRUPTED EEPROM
    //! ************************************************************************
    // Check for uninitialized EEPROM (0xFFFFFFFF = 4294967295) or unreasonably high values
    // Set reasonable maximum of 1 million cycles (1,000,000)
    const uint32_t MAX_REASONABLE_CYCLES = 1000000;
    
    if (primaryCycles == 0xFFFFFFFF || primaryCycles > MAX_REASONABLE_CYCLES) primaryCycles = 0;
    if (backupCycles == 0xFFFFFFFF || backupCycles > MAX_REASONABLE_CYCLES) backupCycles = 0;
    
    //! ************************************************************************
    //! USE THE HIGHER VALUE BETWEEN PRIMARY AND BACKUP (SAFETY MECHANISM)
    //! ************************************************************************
    totalCycles = max(primaryCycles, backupCycles);
    
    if (storedVersion != DATA_VERSION) {
        //! ************************************************************************
        //! VERSION MISMATCH - PRESERVE TOTAL CYCLES BUT CLEAR BUFFER DATA
        //! This happens when EEPROM layout changes (e.g., memory overlap fixes)
        //! ************************************************************************
        cycleBufferIndex = 0;
        
        // Clear the cycle buffer (but keep totalCycles intact)
        for (int i = 0; i < MAX_CYCLE_RECORDS; i++) {
            cycleBuffer[i].timestamp = 0;
            cycleBuffer[i].cycle_count = 0;
        }
        
        // Save the new version and preserve totalCycles with backup
        EEPROM.put(DATA_VERSION_ADDR, DATA_VERSION);
        saveTotalCyclesToEEPROM();
        EEPROM.commit();
        
        cycleDataLoaded = true;
        return;
    }
    
    //! ************************************************************************
    //! LOAD CYCLE BUFFER FROM EEPROM
    //! ************************************************************************
    EEPROM.get(TRIGGER_DATA_ADDR, cycleBuffer);
    EEPROM.get(TRIGGER_DATA_ADDR + CYCLE_BUFFER_SIZE, cycleBufferIndex);
    
    //! ************************************************************************
    //! VALIDATE LOADED DATA AND CLEAR OLD TIMESTAMPS
    //! ************************************************************************
    if (cycleBufferIndex >= MAX_CYCLE_RECORDS) {
        cycleBufferIndex = 0;
    }
    
    //! ************************************************************************
    //! CLEAR OLD TIMESTAMPS TO PREVENT STALE DATA FROM AFFECTING AVERAGES
    //! ************************************************************************
    unsigned long currentTime = millis();
    unsigned long fifteenMinutesAgo = currentTime - (15 * 60 * 1000);
    
    for (int i = 0; i < MAX_CYCLE_RECORDS; i++) {
        // If timestamp is older than 15 minutes or is 0 (uninitialized), clear it
        if (cycleBuffer[i].timestamp == 0 || cycleBuffer[i].timestamp < fifteenMinutesAgo) {
            cycleBuffer[i].timestamp = 0;
            cycleBuffer[i].cycle_count = 0;
        }
    }
    
    cycleDataLoaded = true;
}

void WebDashboard::updateCycleDisplay() {
    //! ************************************************************************
    //! SEND UPDATED TRIGGER DATA TO WEBSOCKET CLIENTS
    //! ************************************************************************
    if (isConnected) {
        sendStatusUpdate();
    }
}

//* ************************************************************************
//* ********************** HOURLY TRACKING METHODS **************************
//* ************************************************************************

void WebDashboard::updateHourlyData() {
    //! ************************************************************************
    //! GET CURRENT TIME
    //! ************************************************************************
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    
    uint8_t currentHour = timeinfo->tm_hour;
    uint8_t currentDay = timeinfo->tm_mday;
    uint8_t currentMonth = timeinfo->tm_mon + 1; // tm_mon is 0-11
    
    //! ************************************************************************
    //! CHECK IF WE'RE STILL IN THE SAME HOUR
    //! ************************************************************************
    if (lastHour == 255 || lastHour != currentHour || lastDay != currentDay || lastMonth != currentMonth) {
        //! ************************************************************************
        //! SAVE PREVIOUS HOUR'S DATA IF IT EXISTS
        //! ************************************************************************
        if (lastHour != 255) {
            hourlyBuffer[hourlyBufferIndex].cycles = currentHourCycles;
            hourlyBuffer[hourlyBufferIndex].hour = lastHour;
            hourlyBuffer[hourlyBufferIndex].day = lastDay;
            hourlyBuffer[hourlyBufferIndex].month = lastMonth;
            
            hourlyBufferIndex = (hourlyBufferIndex + 1) % MAX_HOURLY_RECORDS;
            
            //! ************************************************************************
            //! SAVE HOURLY DATA TO EEPROM EVERY HOUR
            //! ************************************************************************
            saveHourlyDataToEEPROM();
        }
        
        //! ************************************************************************
        //! START NEW HOUR
        //! ************************************************************************
        currentHourCycles = 1;
        lastHour = currentHour;
        lastDay = currentDay;
        lastMonth = currentMonth;
    } else {
        //! ************************************************************************
        //! SAME HOUR - INCREMENT COUNTER
        //! ************************************************************************
        currentHourCycles++;
    }
}

void WebDashboard::saveHourlyDataToEEPROM() {
    //! ************************************************************************
    //! SAVE HOURLY BUFFER TO EEPROM
    //! ************************************************************************
    EEPROM.put(HOURLY_DATA_ADDR, hourlyBuffer);
    EEPROM.put(HOURLY_DATA_ADDR + (MAX_HOURLY_RECORDS * HOURLY_RECORD_SIZE), hourlyBufferIndex);
    EEPROM.commit();
}

void WebDashboard::loadHourlyDataFromEEPROM() {
    //! ************************************************************************
    //! LOAD HOURLY BUFFER FROM EEPROM
    //! ************************************************************************
    EEPROM.get(HOURLY_DATA_ADDR, hourlyBuffer);
    EEPROM.get(HOURLY_DATA_ADDR + (MAX_HOURLY_RECORDS * HOURLY_RECORD_SIZE), hourlyBufferIndex);
    
    //! ************************************************************************
    //! VALIDATE LOADED DATA
    //! ************************************************************************
    if (hourlyBufferIndex >= MAX_HOURLY_RECORDS) {
        hourlyBufferIndex = 0;
    }
}

String WebDashboard::getDailyStatsJSON(uint8_t day, uint8_t month) {
    //! ************************************************************************
    //! CREATE JSON WITH HOURLY DATA FOR SPECIFIED DAY
    //! ************************************************************************
    String json = "{";
    json += "\"day\":" + String(day) + ",";
    json += "\"month\":" + String(month) + ",";
    json += "\"hourlyData\":[";
    
    bool firstHour = true;
    uint16_t totalDayCycles = 0;
    
    //! ************************************************************************
    //! ADD SAVED HOURLY DATA FROM EEPROM
    //! ************************************************************************
    for (int i = 0; i < MAX_HOURLY_RECORDS; i++) {
        if (hourlyBuffer[i].day == day && hourlyBuffer[i].month == month && hourlyBuffer[i].cycles > 0) {
            if (!firstHour) json += ",";
            json += "{";
            json += "\"hour\":" + String(hourlyBuffer[i].hour) + ",";
            json += "\"cycles\":" + String(hourlyBuffer[i].cycles);
            json += "}";
            firstHour = false;
            totalDayCycles += hourlyBuffer[i].cycles;
        }
    }
    
    //! ************************************************************************
    //! ADD CURRENT HOUR'S DATA IF IT'S THE SAME DAY
    //! ************************************************************************
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    uint8_t currentDay = timeinfo->tm_mday;
    uint8_t currentMonth = timeinfo->tm_mon + 1;
    
    if (currentDay == day && currentMonth == month && currentHourCycles > 0) {
        if (!firstHour) json += ",";
        json += "{";
        json += "\"hour\":" + String(lastHour) + ",";
        json += "\"cycles\":" + String(currentHourCycles);
        json += "}";
        totalDayCycles += currentHourCycles;
    }
    
    json += "],";
    json += "\"totalCycles\":" + String(totalDayCycles);
    json += "}";
    
    return json;
}

String WebDashboard::getCalendarDataJSON(uint8_t month) {
    //! ************************************************************************
    //! CREATE JSON WITH CALENDAR DATA FOR SPECIFIED MONTH
    //! ************************************************************************
    String json = "{";
    json += "\"month\":" + String(month) + ",";
    json += "\"daysWithData\":[";
    
    bool firstDay = true;
    
    //! ************************************************************************
    //! CHECK SAVED HOURLY DATA FROM EEPROM
    //! ************************************************************************
    for (int i = 0; i < MAX_HOURLY_RECORDS; i++) {
        if (hourlyBuffer[i].month == month && hourlyBuffer[i].cycles > 0) {
            String dayStr = String(hourlyBuffer[i].day);
            if (!firstDay) json += ",";
            json += dayStr;
            firstDay = false;
        }
    }
    
    //! ************************************************************************
    //! CHECK CURRENT HOUR'S DATA IF IT'S THE SAME MONTH
    //! ************************************************************************
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    uint8_t currentDay = timeinfo->tm_mday;
    uint8_t currentMonth = timeinfo->tm_mon + 1;
    
    if (currentMonth == month && currentHourCycles > 0) {
        String dayStr = String(currentDay);
        if (!firstDay) json += ",";
        json += dayStr;
    }
    
    json += "]}";
    
    return json;
}

//* ************************************************************************
//* ********************** GOOGLE SHEETS SYNC ******************************
//* ************************************************************************

void WebDashboard::syncToGoogleSheets() {
    //! ************************************************************************
    //! ONLY SYNC IF TOTAL CYCLES HAVE CHANGED
    //! ************************************************************************
    if (totalCycles == lastSyncedTotalCycles) {
        return; // No new data to sync
    }
    
    //! ************************************************************************
    //! CREATE JSON DATA FOR GOOGLE SHEETS
    //! ************************************************************************
    StaticJsonDocument<512> doc;
    
    // Get current time
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    
    // Add timestamp
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    doc["timestamp"] = timestamp;
    
    // Add production data
    doc["total_cycles"] = totalCycles;
    doc["current_hour_cycles"] = currentHourCycles;
    doc["hour"] = timeinfo->tm_hour;
    doc["day"] = timeinfo->tm_mday;
    doc["month"] = timeinfo->tm_mon + 1;
    doc["year"] = timeinfo->tm_year + 1900;
    
    // Add averages
    CycleAverages averages = calculateAllAverages();
    doc["avg_1min"] = averages.average1Min;
    doc["avg_5min"] = averages.average5Min;
    doc["avg_15min"] = averages.average15Min;
    doc["avg_30min"] = averages.average30Min;
    
    //! ************************************************************************
    //! SEND DATA TO GOOGLE SHEETS
    //! ************************************************************************
    HTTPClient http;
    http.begin(GOOGLE_SCRIPT_URL);
    http.addHeader("Content-Type", "application/json");
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    int httpResponseCode = http.POST(jsonString);
    
    if (httpResponseCode > 0) {
        String response = http.getString();
        if (httpResponseCode == 200) {
            lastSyncedTotalCycles = totalCycles;
        }
    }
    
    http.end();
}

