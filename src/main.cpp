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
#include <esp_task_wdt.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// Project files
#include "ServoControl.h"
#include "OTA/OTA_Upload.h"
#include "WebDashboard/WebDashboard.h"
#include "Config/Pins_Definitions.h"
#include "ConfigApi/MachineConfigApi.h"
#include "ConfigApi/MachineSettings.h"

// Task watchdog timeout (seconds): resets the chip if the loop task stalls longer
// than this. The flip/feed states are non-blocking, so the loop feeds it each
// iteration (and the OTA progress callback feeds it during an upload).
static const uint32_t WATCHDOG_TIMEOUT_S = 15;

// Forward declarations
void handleStateMachine();
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

// Timing variables
unsigned long stateStartTime = 0;
unsigned long stepStartTime = 0;
float currentStep = 1.0f;

// ESP-NOW
volatile bool espNowStartReceived = false;
unsigned long lastEspNowSignalTime = 0;

// Ignore repeated signal=1 messages within this window (Stage 2 sends 3x rapid-fire)
static const unsigned long ESPNOW_DEDUP_MS = 200;

// ESP-NOW protocol: the start command value Stage 2 sends in RouterMessage.signal.
static const uint8_t ESPNOW_CMD_START = 1;

// Max TX power for esp_wifi_set_max_tx_power() (units: 0.25 dBm steps).
static const int8_t ESPNOW_TX_POWER = 84;

// Servo PWM: LEDC frequency / resolution for the flip servo.
static const int SERVO_PWM_FREQ_HZ = 50;
static const int SERVO_PWM_RESOLUTION_BITS = 14;

// Input debounce interval (ms) for the start sensor / manual start switches.
static const uint16_t START_DEBOUNCE_MS = 3;

// ESP-NOW receiver
typedef struct { uint8_t signal; } RouterMessage;

void onEspNowReceive(const uint8_t* mac, const uint8_t* data, int len) {
    if (len < 1) return;
    RouterMessage msg;
    memcpy(&msg, data, sizeof(msg));

    // Deduplicate: ignore signal=1 repeats within 200ms window.
    // Stage 2 sends each message 3x rapid-fire (5ms apart) for redundancy.
    if (msg.signal == ESPNOW_CMD_START && (millis() - lastEspNowSignalTime > ESPNOW_DEDUP_MS)) {
        lastEspNowSignalTime = millis();
        // Only trigger if we're actually in IDLE — discard pulses mid-cycle
        if (currentState == STATE_IDLE) {
            espNowStartReceived = true;
        }
    }
}

void initEspNow() {
    // Set max TX power for reliable reception
    esp_wifi_set_max_tx_power(ESPNOW_TX_POWER);

    // Init ESP-NOW (WiFi must already be connected in STA mode)
    if (esp_now_init() != ESP_OK) {
        return;
    }
    esp_now_register_recv_cb(onEspNowReceive);
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
    Serial.println("[Router] booting");

    // Brownout detector left ENABLED (hardware default): on a voltage sag the
    // chip resets cleanly instead of risking a corrupted EEPROM/NVS write or a
    // half-driven motor/servo. (Previously this line disabled it.)

    // Configure input pins
    pinMode(START_SENSOR_PIN, INPUT_PULLDOWN);
    pinMode(MANUAL_START_PIN, INPUT_PULLDOWN);
    pinMode(FEED_CYLINDER_PIN, OUTPUT);

    // Setup input debouncers
    startSensorDebouncer.attach(START_SENSOR_PIN);
    startSensorDebouncer.interval(START_DEBOUNCE_MS); // 3ms debounce
    manualStartDebouncer.attach(MANUAL_START_PIN);
    manualStartDebouncer.interval(START_DEBOUNCE_MS); // 3ms debounce

    // Initialize feed cylinder to safe position
    digitalWrite(FEED_CYLINDER_PIN, LOW); // LOW = extended = safe position

    // Configure servo motor
    flipServo.init(FLIP_SERVO_PIN, 0, SERVO_PWM_FREQ_HZ, SERVO_PWM_RESOLUTION_BITS); // pin, channel, frequency, resolution
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

    // Report network result on boot.
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("[Router] wifi ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("[Router] wifi unavailable - standalone");
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

    // Initialize OTA functionality
    setupOTA();

    // Subscribe the loop task to the task watchdog (last, so the boot-time WiFi
    // connect isn't watched). esp_task_wdt_init() reconfigures the core's TWDT to
    // our timeout; add(NULL) watches this (loop) task.
    esp_task_wdt_init(WATCHDOG_TIMEOUT_S, true);
    esp_task_wdt_add(NULL);

    Serial.println("[Router] ready");
}

// Loop
void loop() {
    // Feed the task watchdog each iteration.
    esp_task_wdt_reset();

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
