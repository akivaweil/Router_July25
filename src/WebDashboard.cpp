#include "WebDashboard.h"
#include "ServoControl.h"

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
        String json = "{";
        json += "\"type\":\"status\",";
        json += "\"homeAngle\":" + String(*homeAnglePtr, 1) + ",";
        json += "\"totalCycles\":" + String(totalCycles) + ",";
        json += "\"average3Min\":" + String(calculateAverageCycles3Min(), 1) + ",";
        json += "\"average15Min\":" + String(calculateAverageCycles(), 1) + ",";
        json += "\"average1Hour\":" + String(calculateAverageCycles1Hour(), 1);
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
            grid-template-columns: repeat(4, 1fr);
            gap: 15px;
            margin: 20px 0;
        }
        
        .stat-item {
            text-align: center;
            flex: 1;
        }
        
        .stat-value {
            font-size: 2.5em;
            font-weight: bold;
            color: #667eea;
            margin-bottom: 5px;
        }
        
        .stat-label {
            font-size: 0.9em;
            color: #666;
            font-weight: 500;
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
                            <div class="stat-value" id="average3Min">0.00</div>
                            <div class="stat-label">Avg/Min (3min)</div>
                        </div>
                        <div class="stat-item">
                            <div class="stat-value" id="average15Min">0.00</div>
                            <div class="stat-label">Avg/Min (15min)</div>
                        </div>
                        <div class="stat-item">
                            <div class="stat-value" id="average1Hour">0.00</div>
                            <div class="stat-label">Avg/Min (1hr)</div>
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
        let average3Min = 0.0;
        let average15Min = 0.0;
        let average1Hour = 0.0;
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
                        average3Min = data.average3Min || 0.0;
                        average15Min = data.average15Min || 0.0;
                        average1Hour = data.average1Hour || 0.0;
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
            document.getElementById('average3Min').textContent = average3Min.toFixed(1);
            document.getElementById('average15Min').textContent = average15Min.toFixed(1);
            document.getElementById('average1Hour').textContent = average1Hour.toFixed(1);
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
            //! KEEP EXACTLY 15 DATA POINTS (ONE PER MINUTE)
            //! ************************************************************************
            if (graphData.length > 15) {
                graphData = graphData.slice(-15);
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
            //! DRAW GRID LINES (0-10 RANGE)
            //! ************************************************************************
            ctx.strokeStyle = '#e0e0e0';
            ctx.lineWidth = 1;
            for (let i = 0; i <= 10; i++) {
                const y = padding + (i * graphHeight / 10);
                ctx.beginPath();
                ctx.moveTo(padding, y);
                ctx.lineTo(canvas.width - padding, y);
                ctx.stroke();
            }
            
            //! ************************************************************************
            //! DRAW Y-AXIS LABELS (0-10 RANGE)
            //! ************************************************************************
            ctx.fillStyle = '#666';
            ctx.font = '12px Arial';
            ctx.textAlign = 'right';
            for (let i = 0; i <= 10; i++) {
                const value = 10 - i;
                const y = padding + (i * graphHeight / 10) + 4;
                ctx.fillText(value.toString(), padding - 5, y);
            }
            
            //! ************************************************************************
            //! DRAW GRAPH LINE AND DOTS (15 POINTS, 0-10 RANGE)
            //! ************************************************************************
            ctx.strokeStyle = '#667eea';
            ctx.lineWidth = 2;
            ctx.beginPath();
            
            // Draw line connecting the points
            for (let i = 0; i < graphData.length; i++) {
                const x = padding + (i * graphWidth / 14); // 14 intervals for 15 points
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
                const x = padding + (i * graphWidth / 14); // 14 intervals for 15 points
                const y = canvas.height - padding - ((graphData[i].value - minValue) / valueRange * graphHeight);
                
                ctx.beginPath();
                ctx.arc(x, y, 4, 0, 2 * Math.PI);
                ctx.fill();
            }
            
            
            //! ************************************************************************
            //! DRAW LABELS
            //! ************************************************************************
            ctx.fillStyle = '#666';
            ctx.font = '12px Arial';
            ctx.textAlign = 'center';
            
            // Y-axis labels
            for (let i = 0; i <= 4; i++) {
                const value = minValue + (i * valueRange / 4);
                const y = canvas.height - padding - (i * graphHeight / 4);
                ctx.fillText(value.toFixed(1), padding - 10, y + 4);
            }
            
            // Title
            ctx.textAlign = 'center';
            ctx.font = 'bold 14px Arial';
            ctx.fillText('Cycles per Minute (15 min avg)', canvas.width / 2, 20);
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
                        peakHour + ':00 (' + maxCycles + ' cycles)';
                    
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
            //! DRAW HOUR LABELS
            //! ************************************************************************
            ctx.fillStyle = '#666';
            ctx.font = '10px Arial';
            ctx.textAlign = 'center';
            for (let i = 0; i < 24; i += 4) {
                ctx.fillText(i + ':00', i * barWidth + barWidth / 2, canvas.height - 10);
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
            generateCalendar();
        }
        
        function nextMonth() {
            currentDate.setMonth(currentDate.getMonth() + 1);
            generateCalendar();
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
            generateCalendar();
            
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
            EEPROM.put(TOTAL_CYCLES_ADDR, totalCycles);
            saveCycleDataToEEPROM();
            EEPROM.commit();
            
            // Update display immediately after cycle completion
            if (isConnected) {
                sendStatusUpdate();
                lastStatusUpdate = currentTime;
            }
        } else {
            // Machine was already idle, ping websocket every 3 seconds with fresh data
            if (isConnected && currentTime - lastStatusUpdate > 3000) {
                sendStatusUpdate();
                lastStatusUpdate = currentTime;
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
    // EEPROM operations and status updates will be handled in update() when idle
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

float WebDashboard::calculateAverageCycles() {
    //! ************************************************************************
    //! CALCULATE AVERAGE TRIGGERS OVER PAST 15 MINUTES
    //! ************************************************************************
    unsigned long currentTime = millis();
    unsigned long fifteenMinutesAgo = currentTime - (15 * 60 * 1000);
    
    int validRecords = 0;
    
    for (int i = 0; i < MAX_CYCLE_RECORDS; i++) {
        if (cycleBuffer[i].timestamp > fifteenMinutesAgo && cycleBuffer[i].timestamp > 0) {
            validRecords++;
        }
    }
    
    if (validRecords == 0) {
        return 0.0f;
    }
    
    //! ************************************************************************
    //! CONVERT TO TRIGGERS PER MINUTE (MORE PRECISE CALCULATION)
    //! ************************************************************************
    float averagePerMinute = (float)validRecords / 15.0f;
    return averagePerMinute;
}

float WebDashboard::calculateAverageCycles3Min() {
    //! ************************************************************************
    //! CALCULATE AVERAGE TRIGGERS OVER PAST 3 MINUTES
    //! ************************************************************************
    unsigned long currentTime = millis();
    unsigned long threeMinutesAgo = currentTime - (3 * 60 * 1000);
    
    int validRecords = 0;
    
    for (int i = 0; i < MAX_CYCLE_RECORDS; i++) {
        //! ************************************************************************
        //! CHECK IF RECORD IS WITHIN 3 MINUTE WINDOW AND NOT EMPTY
        //! ************************************************************************
        if (cycleBuffer[i].timestamp > threeMinutesAgo && 
            cycleBuffer[i].timestamp > 0 && 
            cycleBuffer[i].timestamp <= currentTime) {
            validRecords++;
        }
    }
    
    if (validRecords == 0) {
        return 0.0f;
    }
    
    //! ************************************************************************
    //! CONVERT TO TRIGGERS PER MINUTE (CORRECT CALCULATION)
    //! ************************************************************************
    float averagePerMinute = (float)validRecords / 3.0f;
    
    //! ************************************************************************
    //! DEBUG: TEMPORARY SERIAL OUTPUT TO UNDERSTAND THE CALCULATION
    //! ************************************************************************
    Serial.print("3min calc: ");
    Serial.print(validRecords);
    Serial.print(" records in 3min = ");
    Serial.print(averagePerMinute);
    Serial.println(" per minute");
    
    return averagePerMinute;
}

float WebDashboard::calculateAverageCycles1Hour() {
    //! ************************************************************************
    //! CALCULATE AVERAGE TRIGGERS OVER PAST 1 HOUR
    //! ************************************************************************
    unsigned long currentTime = millis();
    unsigned long oneHourAgo = currentTime - (60 * 60 * 1000);
    
    int validRecords = 0;
    
    for (int i = 0; i < MAX_CYCLE_RECORDS; i++) {
        if (cycleBuffer[i].timestamp > oneHourAgo && cycleBuffer[i].timestamp > 0) {
            validRecords++;
        }
    }
    
    if (validRecords == 0) {
        return 0.0f;
    }
    
    //! ************************************************************************
    //! CONVERT TO TRIGGERS PER MINUTE (MORE PRECISE CALCULATION)
    //! ************************************************************************
    float averagePerMinute = (float)validRecords / 60.0f;
    return averagePerMinute;
}

void WebDashboard::saveCycleDataToEEPROM() {
    //! ************************************************************************
    //! SAVE CYCLE BUFFER TO EEPROM
    //! ************************************************************************
    EEPROM.put(TRIGGER_DATA_ADDR, cycleBuffer);
    EEPROM.put(TRIGGER_DATA_ADDR + CYCLE_BUFFER_SIZE, cycleBufferIndex);
    EEPROM.commit();
}

void WebDashboard::loadCycleDataFromEEPROM() {
    //! ************************************************************************
    //! LOAD CRITICAL DATA (TOTAL CYCLES) FROM EEPROM
    //! ************************************************************************
    EEPROM.get(TOTAL_CYCLES_ADDR, totalCycles);
    
    //! ************************************************************************
    //! LOAD CYCLE BUFFER FROM EEPROM
    //! ************************************************************************
    EEPROM.get(TRIGGER_DATA_ADDR, cycleBuffer);
    EEPROM.get(TRIGGER_DATA_ADDR + CYCLE_BUFFER_SIZE, cycleBufferIndex);
    
    //! ************************************************************************
    //! VALIDATE LOADED DATA
    //! ************************************************************************
    if (cycleBufferIndex >= MAX_CYCLE_RECORDS) {
        cycleBufferIndex = 0;
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
    
    json += "],";
    json += "\"totalCycles\":" + String(totalDayCycles);
    json += "}";
    
    return json;
}

