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
    S_FEEDING2,
    S_BOARD_END
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
State currentState = S_IDLE;
State lastLoggedState = S_NONE;
float lastLoggedStep = 0.0f;
bool boardEndModeActive = false;

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

    //! ************************************************************************
    //! DEDUPLICATE: ignore signal=1 repeats within 200ms window
    //! Stage 2 sends each message 3x rapid-fire (5ms apart) for redundancy
    //! ************************************************************************
    if (msg.signal == 1 && (millis() - lastEspNowSignalTime > ESPNOW_DEDUP_MS)) {
        lastEspNowSignalTime = millis();
        espNowStartReceived = true;
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
#include "StateMachine/STATES/04_BOARD_END.h"

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
    dashboard.init(&SERVO_HOME_ANGLE, &flipServo, &boardEndModeActive);
    dashboard.begin();

    Serial.print("Dashboard: http://router.local or http://");
    Serial.println(WiFi.localIP());

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
        case S_BOARD_END:
            handleBoardEndState();
            break;
        default:
            // Handle unexpected state
            currentState = S_IDLE;
            currentStep = 1.0f;
            break;
    }
}