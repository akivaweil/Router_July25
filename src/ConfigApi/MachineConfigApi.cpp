#include "MachineConfigApi.h"
#include "MachineSettings.h"
#include <ArduinoJson.h>
#include <WiFi.h>

#include "StateMachine/StateMachine_Common.h"
#include "WebDashboard.h"
#include "Config/Pins_Definitions.h"

//* ************************************************************************
//* ********************** SHARED MACHINE CONFIG API ***********************
//* ************************************************************************
//! See docs/DASHBOARD_API_CONTRACT.md. All /api/* responses carry the
//! Access-Control-Allow-Origin: * header. POST /api/config arrives as a
//! text/plain CORS simple request; the body is parsed as JSON regardless.

//! ********************** MACHINE IDENTITY ********************************
static const char* MACHINE_ID   = "router";
static const char* MACHINE_NAME = "Router";

//! ********************** EXTERNAL GLOBALS ********************************
extern WebDashboard dashboard;          // main.cpp (servo write + addr-0 persist)

//! ********************** DEFERRED-APPLY FLAG *****************************
volatile bool configDirty = false;

//! ********************** DEFERRED SERVO-HOME FLAG ************************
//! Set true ONLY when a deferred POST actually staged SERVO_HOME_ANGLE, so
//! the next-IDLE apply moves the servo / commits addr 0 ONLY for POSTs that
//! changed the home angle. Without this guard a deferred non-servo POST (e.g.
//! FEEDING_DURATION_MS only) would, on the next IDLE, revert the servo to the
//! stale staged angle and overwrite a websocket-set home angle at addr 0.
static volatile bool g_servoHomeStaged = false;

//* ************************************************************************
//* ********************** FIELD TABLE *************************************
//* ************************************************************************
//! The fields array IS the curated settings list for this machine and the
//! single source of truth for keys, labels, ranges, and steps. type is
//! "float" for all Router fields.
struct ConfigField {
    const char* key;
    const char* label;
    const char* type;   // "float" | "int"
    float*      value;  // live runtime variable
    float       min;
    float       max;
    float       step;
};

static ConfigField FIELDS[] = {
    { "SERVO_HOME_ANGLE",      "Servo Home Angle (deg)",   "float", &SERVO_HOME_ANGLE,      0.0f,    180.0f,  0.1f   },
    { "IDLE_HOME_OFFSET",      "Idle Home Offset (deg)",   "float", &IDLE_HOME_OFFSET,      0.0f,    30.0f,   0.1f   },
    { "ESPNOW_KICKOFF_OFFSET", "Kickoff Offset (deg)",     "float", &ESPNOW_KICKOFF_OFFSET, 0.0f,    50.0f,   0.1f   },
    { "FEEDING_DURATION_MS",   "Feeding Duration (ms)",    "float", &FEEDING_DURATION_MS,   1000.0f, 5000.0f, 100.0f },
};
static const size_t FIELD_COUNT = sizeof(FIELDS) / sizeof(FIELDS[0]);

static ConfigField* findField(const char* key) {
    for (size_t i = 0; i < FIELD_COUNT; ++i) {
        if (strcmp(FIELDS[i].key, key) == 0) return &FIELDS[i];
    }
    return nullptr;
}

//* ************************************************************************
//* ********************** STATE NAME HELPER *******************************
//* ************************************************************************
static const char* stateName() {
    return currentState == S_IDLE     ? "IDLE"
         : currentState == S_FEEDING  ? "FEEDING"
         : currentState == S_FLIPPING ? "FLIPPING"
         : currentState == S_FEEDING2 ? "FEEDING2"
         : "UNKNOWN";
}

//* ************************************************************************
//* ********************** SAFETY GATE *************************************
//* ************************************************************************
bool isSafeToApplyConfig() {
    return currentState == S_IDLE;
}

//* ************************************************************************
//* ********************** GET /api/status BODY ****************************
//* ************************************************************************
String buildStatusJson() {
    JsonDocument doc;
    doc["id"]       = MACHINE_ID;
    doc["name"]     = MACHINE_NAME;
    doc["state"]    = stateName();
    doc["health"]   = "HEALTHY";
    doc["uptimeMs"] = millis();
    doc["freeHeap"] = ESP.getFreeHeap();
    doc["rssi"]     = WiFi.RSSI();

    JsonObject sensors = doc["sensors"].to<JsonObject>();
    sensors["servoHomeAngle"]     = SERVO_HOME_ANGLE;
    sensors["startSensorActive"]  = digitalRead(START_SENSOR_PIN) == HIGH;
    sensors["manualStartActive"]  = digitalRead(MANUAL_START_PIN) == HIGH;
    sensors["espNowSignalPending"] = espNowStartReceived;
    sensors["configDirty"]        = configDirty;

    String out;
    serializeJson(doc, out);
    return out;
}

//* ************************************************************************
//* ********************** GET /api/config BODY ****************************
//* ************************************************************************
String buildConfigJson() {
    JsonDocument doc;
    doc["id"]     = MACHINE_ID;
    doc["schema"] = 1;

    JsonArray fields = doc["fields"].to<JsonArray>();
    for (size_t i = 0; i < FIELD_COUNT; ++i) {
        JsonObject f = fields.add<JsonObject>();
        f["key"]   = FIELDS[i].key;
        f["label"] = FIELDS[i].label;
        f["type"]  = FIELDS[i].type;
        f["value"] = *FIELDS[i].value;
        f["min"]   = FIELDS[i].min;
        f["max"]   = FIELDS[i].max;
        f["step"]  = FIELDS[i].step;
    }

    String out;
    serializeJson(doc, out);
    return out;
}

//* ************************************************************************
//* ********************** APPLY HELPERS **********************************
//* ************************************************************************
//! Apply SERVO_HOME_ANGLE live: write the live global, persist addr 0, and
//! move the servo. dashboard.setHomeAngle() does all three (and pushes a
//! websocket status update). Source of truth is the STAGED servo-home angle
//! so we never read a half-written live global.
static void applyServoHomeAngleLive() {
    dashboard.setHomeAngle(getStagedServoHomeAngle());
}

//* ************************************************************************
//* ********************** POST /api/config CORE ***************************
//* ************************************************************************
bool applyConfigJson(const String& body, bool& outDeferred, String& outMsg) {
    outDeferred = false;

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err || !doc.is<JsonObject>()) {
        outMsg = "invalid JSON";
        return false;
    }

    JsonObject obj = doc.as<JsonObject>();

    //! ********************** PASS 1: VALIDATE ALL KEYS *******************
    //! Validate everything BEFORE mutating anything so a bad key leaves
    //! state untouched (contract: unknown/out-of-range => change nothing).
    for (JsonPair kv : obj) {
        ConfigField* field = findField(kv.key().c_str());
        if (field == nullptr) {
            outMsg = String(kv.key().c_str()) + " invalid/out of range";
            return false;
        }
        if (!kv.value().is<float>() && !kv.value().is<int>()) {
            outMsg = String(field->key) + " invalid/out of range";
            return false;
        }
        float v = kv.value().as<float>();
        if (v < field->min || v > field->max) {
            outMsg = String(field->key) + " invalid/out of range";
            return false;
        }
    }

    //! ********************** PASS 2: STAGE ACCEPTED VALUES ***************
    //! Write accepted values into the in-RAM STAGING area only (the persist
    //! struct + staged servo-home mirror). NO live runtime global is touched
    //! here -- FEEDING_DURATION_MS et al. are read live mid-cycle, so a
    //! mid-feed POST must not mutate them. Live globals change only in the
    //! IDLE branch below (or on the next IDLE entry for the deferred path).
    bool touchedServoHome = false;
    for (JsonPair kv : obj) {
        ConfigField* field = findField(kv.key().c_str());
        stageSettingByKey(field->key, kv.value().as<float>());
        if (strcmp(field->key, "SERVO_HOME_ANGLE") == 0) touchedServoHome = true;
    }

    //! ********************** ALWAYS PERSIST IMMEDIATELY *****************
    //! Flush the staged struct (addr 4) and, if touched, the servo-home
    //! mirror (addr 0). Both persist WITHOUT moving the servo or writing a
    //! live global, so this is safe on the deferred path.
    saveSettings();
    if (touchedServoHome) persistServoHomeAngle();

    //! ********************** APPLY NOW OR DEFER **************************
    if (isSafeToApplyConfig()) {
        //! Truly-motionless IDLE: copy staged struct -> live globals now and
        //! move the servo to the staged home angle.
        applySettings();
        if (touchedServoHome) applyServoHomeAngleLive();
        outDeferred = false;
        outMsg = "saved";
        return true;
    }

    //! Mid-cycle (HOMING / FEEDING / etc.): values are persisted + staged,
    //! but do NOT touch ANY live runtime variable. The next IDLE entry copies
    //! staged -> live and moves the servo via configDirty. Only flag the
    //! servo-home apply if THIS POST actually staged SERVO_HOME_ANGLE, so a
    //! non-servo deferred POST cannot revert a websocket-set home angle.
    if (touchedServoHome) g_servoHomeStaged = true;
    configDirty = true;
    outDeferred = true;
    outMsg = "pending — applies at next idle";
    return true;
}

//* ************************************************************************
//* ********************** DEFERRED APPLY (MAIN LOOP) **********************
//* ************************************************************************
void applyPendingConfigIfIdle() {
    if (!configDirty) return;
    if (currentState != S_IDLE) return;

    //! Copy the persisted/staged struct into the live globals. This is the
    //! ONLY place the deferred non-servo values reach the live runtime.
    applySettings();

    //! Move the servo to the staged home angle / commit addr 0 ONLY if this
    //! deferred batch actually staged SERVO_HOME_ANGLE. Otherwise we would
    //! revert the servo to a stale staged angle and clobber a websocket-set
    //! home angle (the websocket path updates the live angle + addr 0 but not
    //! the staged mirror). Mirrors the apply-now touchedServoHome guard.
    if (g_servoHomeStaged) {
        applyServoHomeAngleLive();
        g_servoHomeStaged = false;
    }
    configDirty = false;
}

//* ************************************************************************
//* ********************** ROUTE REGISTRATION ******************************
//* ************************************************************************
static void addCors(AsyncWebServerResponse* response) {
    response->addHeader("Access-Control-Allow-Origin", "*");
}

void setupConfigApi(AsyncWebServer& server) {
    //! ********************** GET /api/status ****************************
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest* request) {
        AsyncWebServerResponse* response =
            request->beginResponse(200, "application/json", buildStatusJson());
        addCors(response);
        request->send(response);
    });

    //! ********************** GET /api/config ****************************
    server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest* request) {
        AsyncWebServerResponse* response =
            request->beginResponse(200, "application/json", buildConfigJson());
        addCors(response);
        request->send(response);
    });

    //! ********************** POST /api/config ***************************
    //! Raw onBody accumulator (the me-no-dev fork may not expose
    //! AsyncCallbackJsonWebHandler). Accumulate chunks into a String keyed
    //! by request, then on the final chunk validate/persist/apply.
    //!
    //! IMPORTANT (me-no-dev fork): onRequest ALWAYS runs after onBody. Do
    //! NOT gate it on a _tempObject sentinel -- onBody nulls _tempObject on
    //! the final chunk, so that would make every real POST fall through to a
    //! bogus empty-body 400 that REPLACES the real response. Gate strictly on
    //! request->contentLength() == 0: the genuine empty-body case.
    server.on(
        "/api/config", HTTP_POST,
        //! onRequest: the body path (onBody) already sent the real response
        //! when there was a body. Here we only handle the empty-body POST.
        [](AsyncWebServerRequest* request) {
            if (request->contentLength() == 0) {
                JsonDocument doc;
                doc["ok"]      = false;
                doc["message"] = "empty body";
                String out;
                serializeJson(doc, out);
                AsyncWebServerResponse* response =
                    request->beginResponse(400, "application/json", out);
                addCors(response);
                request->send(response);
            }
        },
        nullptr,
        //! onBody: accumulate the raw body across chunks.
        [](AsyncWebServerRequest* request, uint8_t* data, size_t len,
           size_t index, size_t total) {
            if (index == 0) {
                request->_tempObject = new String();
                ((String*)request->_tempObject)->reserve(total);
                //! Harden against a mid-body abort: the fork frees
                //! _tempObject with C free() (skips ~String, leaking the char
                //! buffer). Delete it properly on disconnect; we re-null
                //! _tempObject below before this can double-fire on success.
                request->onDisconnect([request]() {
                    if (request->_tempObject != nullptr) {
                        delete (String*)request->_tempObject;
                        request->_tempObject = nullptr;
                    }
                });
            }
            String* buf = (String*)request->_tempObject;
            for (size_t i = 0; i < len; ++i) buf->concat((char)data[i]);

            //! Final chunk: process and respond.
            if (index + len == total) {
                bool deferred = false;
                String msg;
                bool ok = applyConfigJson(*buf, deferred, msg);

                JsonDocument doc;
                doc["ok"] = ok;
                if (ok) {
                    doc["applied"]  = !deferred;
                    doc["deferred"] = deferred;
                }
                doc["message"] = msg;
                String out;
                serializeJson(doc, out);

                AsyncWebServerResponse* response =
                    request->beginResponse(ok ? 200 : 400, "application/json", out);
                addCors(response);
                request->send(response);

                delete buf;
                request->_tempObject = nullptr;
            }
        });
}
