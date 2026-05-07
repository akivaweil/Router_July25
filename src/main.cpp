//* ************************************************************************
//* ******************** ESP32 ROUTER CONTROL SYSTEM *********************
//* ************************************************************************
//! This program controls a router machine that:
//! 1. Waits for a start signal (IDLE)
//! 2. Feeds wood through router (FEEDING)
//! 3. Flips the wood over with a servo motor (FLIPPING)
//! 4. Feeds wood again (FEEDING2)
//! 5. Goes back to waiting (IDLE)

#include <Arduino.h>
#include <Bounce2.h>
#include <ESPmDNS.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

//* ************************************************************************
//* ********************** PROJECT FILES ***********************************
//* ************************************************************************
#include "ServoControl.h"
#include "WebDashboard.h"
#include "config/Pins_Definitions.h"

//* ************************************************************************
//* ********************** FORWARD DECLARATIONS ****************************
//* ************************************************************************
void initOTA();
void handleOTA();
void handleStateMachine();
void log_state_step(const char* message);
void initEspNow();

//* ************************************************************************
//* ********************** STATE ENUMERATION *******************************
//* ************************************************************************
enum State {
    S_NONE,
    S_IDLE,
    S_FEEDING,
    S_FLIPPING,
    S_FEEDING2
};

//* ************************************************************************
//* ********************** GLOBAL VARIABLES ********************************
//* ************************************************************************

//! ********************** INPUT DEBOUNCERS ********************************
Bounce startSensorDebouncer = Bounce();
Bounce manualStartDebouncer = Bounce();

//! ********************** SERVO CONTROL ***********************************
ServoControl flipServo;
float SERVO_HOME_ANGLE = 90.0f;

//! ********************** WEB DASHBOARD ***********************************
WebDashboard dashboard;

//! ********************** STATE MACHINE VARIABLES *************************
volatile State currentState = S_IDLE;
State lastLoggedState = S_NONE;
float lastLoggedStep = 0.0f;

//! ********************** TIMING VARIABLES ********************************
unsigned long stateStartTime = 0;
unsigned long stepStartTime = 0;
float currentStep = 1.0f;

//! ********************** ESP-NOW ******************************************
volatile bool espNowStartReceived = false;
unsigned long lastEspNowSignalTime = 0;

// Ignore repeated signal=1 messages within this window (Stage 2 sends 3x rapid-fire)
static const unsigned long ESPNOW_DEDUP_MS = 200;

//* ************************************************************************
//* ********************** HELPER FUNCTIONS ********************************
//* ************************************************************************
void log_state_step(const char* message) {
    if (currentState != lastLoggedState || currentStep != lastLoggedStep) {
        Serial.println(message);
        lastLoggedState = currentState;
        lastLoggedStep = currentStep;
    }
}

//* ************************************************************************
//* ********************** ESP-NOW RECEIVER ********************************
//* ************************************************************************
typedef struct { uint8_t signal; } RouterMessage;

void onEspNowReceive(const uint8_t* mac, const uint8_t* data, int len) {
    if (len < 1) return;
    RouterMessage msg;
    memcpy(&msg, data, sizeof(msg));

    Serial.printf("[ESP-NOW] signal=%d  state=%d  timeSinceLast=%lums\n",
                  msg.signal, (int)currentState, millis() - lastEspNowSignalTime);

    //! ************************************************************************
    //! DEDUPLICATE: ignore signal=1 repeats within 200ms window
    //! Stage 2 sends each message 3x rapid-fire (5ms apart) for redundancy
    //! ************************************************************************
    if (msg.signal == 1 && (millis() - lastEspNowSignalTime > ESPNOW_DEDUP_MS)) {
        lastEspNowSignalTime = millis();
        //! Only trigger if we're actually in IDLE — discard pulses mid-cycle
        if (currentState == S_IDLE) {
            espNowStartReceived = true;
            Serial.println("[ESP-NOW] Trigger accepted");
        } else {
            Serial.println("[ESP-NOW] Trigger ignored (not in IDLE)");
        }
    }
}

void initEspNow() {
    //! ************************************************************************
    //! SET MAX TX POWER FOR RELIABLE RECEPTION
    //! ************************************************************************
    esp_wifi_set_max_tx_power(84);

    //! ************************************************************************
    //! INIT ESP-NOW (WiFi must already be connected in STA mode)
    //! ************************************************************************
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        return;
    }
    esp_now_register_recv_cb(onEspNowReceive);
    Serial.println("ESP-NOW receiver ready");
}

//* ************************************************************************
//* ********************** STATE MACHINE FILES *****************************
//* ************************************************************************
#include "StateMachine/STATES/00_IDLE.h"
#include "StateMachine/STATES/01_FEEDING.h"
#include "StateMachine/STATES/02_FLIPPING.h"
#include "StateMachine/STATES/03_FEEDING2.h"

//* ************************************************************************
//* **************************** SETUP *************************************
//* ************************************************************************
void setup() {
    //! ************************************************************************
    //! INITIALIZE SERIAL COMMUNICATION
    //! ************************************************************************
    Serial.begin(115200);
    
    //! ************************************************************************
    //! DISABLE BROWNOUT DETECTOR
    //! ************************************************************************
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    //! ************************************************************************
    //! CONFIGURE INPUT PINS
    //! ************************************************************************
    pinMode(START_SENSOR_PIN, INPUT_PULLDOWN);
    pinMode(MANUAL_START_PIN, INPUT_PULLDOWN);
    pinMode(FEED_CYLINDER_PIN, OUTPUT);

    //! ************************************************************************
    //! SETUP INPUT DEBOUNCERS
    //! ************************************************************************
    startSensorDebouncer.attach(START_SENSOR_PIN);
    startSensorDebouncer.interval(3); // 3ms debounce
    manualStartDebouncer.attach(MANUAL_START_PIN);
    manualStartDebouncer.interval(3); // 3ms debounce

    //! ************************************************************************
    //! INITIALIZE FEED CYLINDER TO SAFE POSITION
    //! ************************************************************************
    digitalWrite(FEED_CYLINDER_PIN, LOW); // LOW = extended = safe position

    //! ************************************************************************
    //! CONFIGURE SERVO MOTOR
    //! ************************************************************************
    flipServo.init(FLIP_SERVO_PIN, 0, 50, 14); // pin, channel, frequency, resolution
    flipServo.write(SERVO_HOME_ANGLE);

    //! ************************************************************************
    //! CONNECT TO WIFI
    //! ************************************************************************
    WiFi.begin("Everwood", "Everwood-Staff");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
    }

    //! ************************************************************************
    //! INITIALIZE ESP-NOW RECEIVER
    //! ************************************************************************
    initEspNow();

    if (MDNS.begin("router")) {
        MDNS.addService("http", "tcp", 80);
    }

    //! ************************************************************************
    //! INITIALIZE WEB DASHBOARD
    //! ************************************************************************
    dashboard.init(&SERVO_HOME_ANGLE, &flipServo);
    dashboard.begin();

    Serial.print("Dashboard: http://router.local or http://");
    Serial.println(WiFi.localIP());
    Serial.printf("WiFi channel: %d  (Stage 2 must match this)\n", WiFi.channel());

    //! ************************************************************************
    //! INITIALIZE OTA FUNCTIONALITY
    //! ************************************************************************
    initOTA();
}

//* ************************************************************************
//* ***************************** LOOP *************************************
//* ************************************************************************
void loop() {
    //! ************************************************************************
    //! UPDATE INPUT DEBOUNCERS
    //! ************************************************************************
    startSensorDebouncer.update();
    manualStartDebouncer.update();

    //! ************************************************************************
    //! DRAIN START EDGES OUTSIDE IDLE — only IDLE should ever start a new cycle
    //! (Bounce.rose() latches the edge until consumed; without this, an edge
    //! during FEEDING/FLIPPING/FEEDING2 fires immediately on return to IDLE)
    //! ************************************************************************
    if (currentState != S_IDLE) {
        startSensorDebouncer.rose();
        manualStartDebouncer.rose();
    }

    //! ************************************************************************
    //! HANDLE OVER-THE-AIR UPDATES
    //! ************************************************************************
    handleOTA();

    //! ************************************************************************
    //! UPDATE WEB DASHBOARD
    //! ************************************************************************
    dashboard.update();

    //! ************************************************************************
    //! RUN STATE MACHINE
    //! ************************************************************************
    handleStateMachine();
}

//* ************************************************************************
//* ********************** STATE MACHINE HANDLER ***************************
//* ************************************************************************
void handleStateMachine() {
    switch (currentState) {
        case S_IDLE:
            handleIdleState();
            break;
        case S_FEEDING:
            handleFeedingState();
            break;
        case S_FLIPPING:
            handleFlippingState();
            break;
        case S_FEEDING2:
            handleFeeding2State();
            break;
        default:
            // Handle unexpected state
            currentState = S_IDLE;
            currentStep = 1.0f;
            break;
    }
}