#pragma once

#include <Arduino.h>

// Machine settings (persistence)
// Owns the EEPROM-persisted settings for the runtime-tunable values
// (SERVO_HOME_ANGLE, IDLE_HOME_OFFSET, ESPNOW_KICKOFF_OFFSET,
// FEEDING_DURATION_MS).
//
// The in-RAM struct is the STAGING area: the POST core stages accepted
// values here (NOT into the live globals) so a mid-cycle POST never
// mutates an in-flight tunable. saveSettings() flushes the staged struct
// to EEPROM; applySettings() copies the staged struct -> live globals
// (called on boot and on the next IDLE entry for deferred changes).
//
// EEPROM LAYOUT (begun as EEPROM.begin(512) by WebDashboard::init()):
//   addr 0 : SERVO_HOME_ANGLE (float) -- legacy slot, shared with the
//            WebDashboard's saveHomeAngleToEEPROM(); we mirror it here so
//            the deferred path can persist it WITHOUT moving the servo.
//   addr 4 : MachineSettings struct (the 3 new settings + magic).

// Versioned persisted struct
struct MachineSettings {
    uint32_t magic;                 // sentinel; absent on first boot
    float    idleHomeOffset;
    float    espnowKickoffOffset;
    float    feedingDurationMs;
};

// EEPROM layout
static const int SERVO_HOME_ANGLE_ADDR = 0;             // float, shared w/ WebDashboard
static const int MACHINE_SETTINGS_ADDR = 4;             // struct, the 3 new settings
static const uint32_t MACHINE_SETTINGS_MAGIC = 0x52544331; // 'RTC1' = Router seTtings v1

// Public API
// loadSettings() : called once in init. On a valid magic it copies the
// persisted values into the live globals; on first boot (no/garbage magic)
// it seeds the staging struct from the current compile-time defaults and
// persists. SERVO_HOME_ANGLE is loaded by WebDashboard at addr 0.
void loadSettings();

// saveSettings() : flush the STAGED struct (addr 4) to EEPROM (commit).
// Does NOT read or write the live globals.
void saveSettings();

// applySettings() : push the STAGED struct values into the live globals.
void applySettings();

// Staging (no live writes)
// stageSettingByKey() : write an accepted value into the in-RAM staging
// struct (or the staged servo-home angle) by config key, WITHOUT touching
// any live runtime global. Returns false for an unknown key. Used by the
// POST core on BOTH the apply-now and deferred paths.
bool stageSettingByKey(const char* key, float value);

// getStagedServoHomeAngle() : current staged SERVO_HOME_ANGLE (addr-0
// mirror). Seeded from the live SERVO_HOME_ANGLE in loadSettings().
float getStagedServoHomeAngle();

// persistServoHomeAngle() : write the staged SERVO_HOME_ANGLE to EEPROM
// addr 0 (commit) WITHOUT moving the servo. For the deferred path.
void persistServoHomeAngle();
