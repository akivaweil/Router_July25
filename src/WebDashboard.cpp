#include "WebDashboard.h"
#include "ServoControl.h"
#include "StateMachine/StateMachine_Common.h"

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
    boardEndModePtr = nullptr;
}

//* ************************************************************************
//* ********************** INITIALIZATION **********************************
//* ************************************************************************
void WebDashboard::init(float* homeAngle, void* servo, bool* boardEndMode) {
    //! ************************************************************************
    //! STORE POINTER TO HOME ANGLE VARIABLE AND SERVO OBJECT
    //! ************************************************************************
    homeAnglePtr = homeAngle;
    servoPtr = servo;
    boardEndModePtr = boardEndMode;
    
    //! ************************************************************************
    //! INITIALIZE EEPROM
    //! ************************************************************************
    EEPROM.begin(EEPROM_SIZE);
    
    //! ************************************************************************
    //! LOAD SAVED HOME ANGLE FROM EEPROM
    //! ************************************************************************
    loadHomeAngleFromEEPROM();
    
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
            else if (message.indexOf("\"command\":\"setBoardEndMode\"") >= 0) {
                //! ************************************************************************
                //! EXTRACT BOARD END MODE VALUE FROM JSON
                //! ************************************************************************
                int startIndex = message.indexOf("\"enabled\":") + 10;
                int endIndex = message.indexOf("}", startIndex);
                if (startIndex > 9 && endIndex > startIndex) {
                    String enabledStr = message.substring(startIndex, endIndex);
                    bool enabled = (enabledStr == "true");

                    //! ************************************************************************
                    //! SET BOARD END MODE
                    //! ************************************************************************
                    setBoardEndMode(enabled);
                }
            }
            break;
    }
}

void WebDashboard::sendStatusUpdate() {
    if (isConnected && homeAnglePtr != nullptr && boardEndModePtr != nullptr) {
        String json = "{";
        json += "\"type\":\"status\",";
        json += "\"homeAngle\":" + String(*homeAnglePtr, 1) + ",";
        json += "\"boardEndMode\":" + String(*boardEndModePtr ? "true" : "false");
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
            max-width: 500px;
            width: 90%;
            text-align: center;
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

        .mode-toggle {
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 15px;
            margin: 20px 0;
        }

        .toggle-switch {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 34px;
        }

        .toggle-switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }

        .toggle-slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            transition: .4s;
            border-radius: 34px;
        }

        .toggle-slider:before {
            position: absolute;
            content: "";
            height: 26px;
            width: 26px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
        }

        input:checked + .toggle-slider {
            background-color: #667eea;
        }

        input:checked + .toggle-slider:before {
            transform: translateX(26px);
        }

        .toggle-label {
            font-size: 1.1em;
            font-weight: 600;
            color: #444;
            min-width: 80px;
            text-align: center;
        }

        .mode-description {
            font-size: 0.9em;
            color: #666;
            text-align: center;
            margin-top: 10px;
            font-style: italic;
        }
    </style>
</head>
<body>
    <div class="dashboard">
        <h1 class="title">Router Control</h1>
        <p class="subtitle">Home Angle Control Dashboard</p>
        
        <div class="control-group">
            <label class="control-label">Home Angle (degrees)</label>
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

        <div class="control-group">
            <label class="control-label">Board End Mode</label>
            <div class="mode-toggle">
                <label class="toggle-switch">
                    <input type="checkbox" id="boardEndToggle">
                    <span class="toggle-slider"></span>
                </label>
                <span class="toggle-label" id="boardEndLabel">Disabled</span>
            </div>
            <div class="mode-description">
                When enabled, servo stays at home angle + 30° and skips rotation during cycles
            </div>
        </div>

        <div class="status disconnected" id="connectionStatus">
            Disconnected
        </div>
    </div>

    <script>
        let ws;
        let currentAngle = 90.0;
        let boardEndModeActive = false;
        
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
                        boardEndModeActive = data.boardEndMode === 'true';
                        updateDisplay(currentAngle);
                        updateBoardEndModeDisplay();
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
        //! UPDATE BOARD END MODE DISPLAY
        //! ************************************************************************
        function updateBoardEndModeDisplay() {
            const toggle = document.getElementById('boardEndToggle');
            const label = document.getElementById('boardEndLabel');

            toggle.checked = boardEndModeActive;
            label.textContent = boardEndModeActive ? 'Enabled' : 'Disabled';
            label.style.color = boardEndModeActive ? '#667eea' : '#444';
        }

        //! ************************************************************************
        //! SET BOARD END MODE
        //! ************************************************************************
        function setBoardEndMode(enabled) {
            if (ws && ws.readyState === WebSocket.OPEN) {
                const command = {
                    command: 'setBoardEndMode',
                    enabled: enabled
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

        document.getElementById('boardEndToggle').addEventListener('change', function() {
            setBoardEndMode(this.checked);
        });
        
        //! ************************************************************************
        //! INITIALIZE ON PAGE LOAD
        //! ************************************************************************
        window.addEventListener('load', function() {
            initWebSocket();
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

void WebDashboard::setBoardEndMode(bool enabled) {
    if (boardEndModePtr != nullptr) {
        *boardEndModePtr = enabled;

        if (enabled) {
            //! ************************************************************************
            //! ACTIVATE BOARD END MODE - TRANSITION TO BOARD END STATE
            //! ************************************************************************
            Serial.println("Board end mode activated from dashboard.");
            currentState = S_BOARD_END;
            stateStartTime = millis();
            currentStep = 1.0f;
        } else {
            //! ************************************************************************
            //! DEACTIVATE BOARD END MODE - RETURN TO IDLE STATE
            //! ************************************************************************
            Serial.println("Board end mode deactivated from dashboard.");
            currentState = S_IDLE;
            stateStartTime = millis();
            currentStep = 1.0f;
        }

        sendStatusUpdate();
    }
}

void WebDashboard::update() {
    if (webSocket != nullptr) {
        webSocket->loop();
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
