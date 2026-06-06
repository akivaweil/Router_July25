// ESP32 Router control system
// This program controls a router machine that:
// 1. Waits for a start signal (IDLE)
// 2. Feeds wood through router (FEEDING)
// 3. Flips the wood over with a servo motor (FLIPPING)
// 4. Feeds wood again (FEEDING2)
// 5. Goes back to waiting (IDLE)

#include <Arduino.h>
#include <Bounce2.h>
#include <ESPmDNS.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// Project files
#include "ServoControl.h"
#include "OTA/OTA_Upload.h"
#include "WebDashboard/WebDashboard.h"
#include "Config/Pins_Definitions.h"
#include "ConfigApi/MachineConfigApi.h"
#include "ConfigApi/MachineSettings.h"

// Forward declarations
void handleStateMachine();
void log_state_step(const char* message);
void initEspNow();

// State enumeration
enum SystemState {
    STATE_NONE,
    STATE_IDLE,
    STATE_FEEDING,
    STATE_FLIPPING,
    STATE_FEEDING2
};

// Global variables

// Input debouncers
Bounce startSensorDebouncer = Bounce();
Bounce manualStartDebouncer = Bounce();

// Servo control
ServoControl flipServo;
float SERVO_HOME_ANGLE = 90.0f;

// Web dashboard
WebDashboard dashboard;

// State machine variables
volatile SystemState currentState = STATE_IDLE;
SystemState lastLoggedState = STATE_NONE;
float lastLoggedStep = 0.0f;

// Timing variables
unsigned long stateStartTime = 0;
unsigned long stepStartTime = 0;
float currentStep = 1.0f;

// ESP-NOW
volatile bool espNowStartReceived = false;
unsigned long lastEspNowSignalTime = 0;

// Ignore repeated signal=1 messages within this window (Stage 2 sends 3x rapid-fire)
static const unsigned long ESPNOW_DEDUP_MS = 200;

// Helper functions
void log_state_step(const char* message) {
    if (currentState != lastLoggedState || currentStep != lastLoggedStep) {
        Serial.println(message);
        lastLoggedState = currentState;
        lastLoggedStep = currentStep;
    }
}

// ESP-NOW receiver
typedef struct { uint8_t signal; } RouterMessage;

void onEspNowReceive(const uint8_t* mac, const uint8_t* data, int len) {
    if (len < 1) return;
    RouterMessage msg;
    memcpy(&msg, data, sizeof(msg));

    Serial.printf("[ESP-NOW] signal=%d  state=%d  timeSinceLast=%lums\n",
                  msg.signal, (int)currentState, millis() - lastEspNowSignalTime);

    // Deduplicate: ignore signal=1 repeats within 200ms window.
    // Stage 2 sends each message 3x rapid-fire (5ms apart) for redundancy.
    if (msg.signal == 1 && (millis() - lastEspNowSignalTime > ESPNOW_DEDUP_MS)) {
        lastEspNowSignalTime = millis();
        // Only trigger if we're actually in IDLE — discard pulses mid-cycle
        if (currentState == STATE_IDLE) {
            espNowStartReceived = true;
            Serial.println("[ESP-NOW] Trigger accepted");
        } else {
            Serial.println("[ESP-NOW] Trigger ignored (not in IDLE)");
        }
    }
}

void initEspNow() {
    // Set max TX power for reliable reception
    esp_wifi_set_max_tx_power(84);

    // Init ESP-NOW (WiFi must already be connected in STA mode)
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        return;
    }
    esp_now_register_recv_cb(onEspNowReceive);
    Serial.println("ESP-NOW receiver ready");
}

// State machine files
#include "StateMachine/STATES/00_IDLE.h"
#include "StateMachine/STATES/01_FEEDING.h"
#include "StateMachine/STATES/02_FLIPPING.h"
#include "StateMachine/STATES/03_FEEDING2.h"

// Setup
void setup() {
    // Initialize serial communication
    Serial.begin(115200);

    // Disable brownout detector
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    // Configure input pins
    pinMode(START_SENSOR_PIN, INPUT_PULLDOWN);
    pinMode(MANUAL_START_PIN, INPUT_PULLDOWN);
    pinMode(FEED_CYLINDER_PIN, OUTPUT);

    // Setup input debouncers
    startSensorDebouncer.attach(START_SENSOR_PIN);
    startSensorDebouncer.interval(3); // 3ms debounce
    manualStartDebouncer.attach(MANUAL_START_PIN);
    manualStartDebouncer.interval(3); // 3ms debounce

    // Initialize feed cylinder to safe position
    digitalWrite(FEED_CYLINDER_PIN, LOW); // LOW = extended = safe position

    // Configure servo motor
    flipServo.init(FLIP_SERVO_PIN, 0, 50, 14); // pin, channel, frequency, resolution
    flipServo.write(SERVO_HOME_ANGLE);

    // Connect to WiFi
    // Bounded connect attempt, then proceed so the machine still boots and runs
    // its flip/feed cycle on locally-saved EEPROM settings if the network/TA is
    // down. WiFi keeps retrying in the background after the loop falls through.
    const uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000;
    const uint32_t WIFI_CONNECT_POLL_MS = 250;
    WiFi.begin("Everwood", "Everwood-Staff");
    WiFi.setAutoReconnect(true);
    uint32_t wifiConnectStart = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - wifiConnectStart < WIFI_CONNECT_TIMEOUT_MS) {
        delay(WIFI_CONNECT_POLL_MS);
    }

    // Initialize ESP-NOW receiver
    initEspNow();

    if (MDNS.begin("router")) {
        MDNS.addService("http", "tcp", 80);
    }

    // Initialize web dashboard
    dashboard.init(&SERVO_HOME_ANGLE, &flipServo);

    // Load persisted machine settings (3 new settings, EEPROM addr 4+).
    // EEPROM is already begun by dashboard.init(); SERVO_HOME_ANGLE (addr 0)
    // was loaded there. Seeds defaults on first boot.
    loadSettings();

    dashboard.begin();

    // Register shared config + status API on the port-80 async server
    setupConfigApi(*dashboard.getServer());

    Serial.print("Dashboard: http://router.local or http://");
    Serial.println(WiFi.localIP());
    Serial.printf("WiFi channel: %d  (Stage 2 must match this)\n", WiFi.channel());

    // Initialize OTA functionality
    setupOTA();
}

// Loop
void loop() {
    // Update input debouncers
    startSensorDebouncer.update();
    manualStartDebouncer.update();

    // Drain start edges outside IDLE — only IDLE should ever start a new cycle.
    // (Bounce.rose() latches the edge until consumed; without this, an edge
    // during FEEDING/FLIPPING/FEEDING2 fires immediately on return to IDLE.)
    if (currentState != STATE_IDLE) {
        startSensorDebouncer.rose();
        manualStartDebouncer.rose();
    }

    // Handle over-the-air updates (only allowed while IDLE)
    if (currentState == STATE_IDLE) {
        handleOTA();

        // Apply any config changes deferred mid-cycle (now back in IDLE)
        applyPendingConfigIfIdle();
    }

    // Update web dashboard
    dashboard.update();

    // Run state machine
    handleStateMachine();
}

// State machine handler
void handleStateMachine() {
    switch (currentState) {
        case STATE_IDLE:
            handleIdleState();
            break;
        case STATE_FEEDING:
            handleFeedingState();
            break;
        case STATE_FLIPPING:
            handleFlippingState();
            break;
        case STATE_FEEDING2:
            handleFeeding2State();
            break;
        default:
            // Handle unexpected state
            currentState = STATE_IDLE;
            currentStep = 1.0f;
            break;
    }
}
