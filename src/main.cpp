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
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

//* ************************************************************************
//* ********************** PROJECT FILES ***********************************
//* ************************************************************************
#include "ServoControl.h"
#include "WebDashboard.h"
#include "config/Pins_Definitions.h"
#include "config/Config.h"

//* ************************************************************************
//* ********************** FORWARD DECLARATIONS ****************************
//* ************************************************************************
void initOTA();
void handleOTA();
void handleStateMachine();
void log_state_step(const char* message);

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

//! ********************** WEB DASHBOARD ***********************************
WebDashboard dashboard;

//! ********************** STATE MACHINE VARIABLES *************************
State currentState = S_IDLE;
State lastLoggedState = S_NONE;
float lastLoggedStep = 0.0f;

//! ********************** TIMING VARIABLES ********************************
unsigned long stateStartTime = 0;
unsigned long stepStartTime = 0;
float currentStep = 1.0f;

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
    startSensorDebouncer.interval(5); // 5ms debounce
    manualStartDebouncer.attach(MANUAL_START_PIN);
    manualStartDebouncer.interval(30); // 30ms debounce

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
    //! CONFIGURE TIME (NTP) - PACIFIC TIME
    //! ************************************************************************
    // Pacific Time: UTC-8 (PST) or UTC-7 (PDT)
    // Using UTC-8 for Pacific Standard Time
    configTime(-8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    while (time(nullptr) < 1000000000) {
        delay(1000);
    }

    //! ************************************************************************
    //! INITIALIZE WEB DASHBOARD
    //! ************************************************************************
    dashboard.init(&SERVO_HOME_ANGLE, &flipServo);
    dashboard.begin();

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
    //! UPDATE WEB DASHBOARD (ONLY WHEN IN IDLE STATE)
    //! ************************************************************************
    dashboard.update(currentState == S_IDLE);

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