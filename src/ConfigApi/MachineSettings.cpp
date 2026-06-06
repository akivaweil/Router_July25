#include "ConfigApi/MachineSettings.h"

#include <Arduino.h>
#include <EEPROM.h>

#include "StateMachine/StateMachine.h"

// Machine settings (persistence)
// Persists the 3 NEW runtime-tunable settings in a versioned struct at
// EEPROM addr 4, plus a mirror of SERVO_HOME_ANGLE at addr 0 (shared with
// WebDashboard). The in-RAM copies here are the STAGING area: values are
// staged here first and only copied into the live globals by
// applySettings() (boot + next-IDLE deferred apply).

// In-RAM staging snapshot
static MachineSettings g_settings;
static float g_stagedServoHomeAngle = 0.0f; // mirror of addr-0 SERVO_HOME_ANGLE

// Apply: struct -> live globals
// Copies the staged struct into the live globals. SERVO_HOME_ANGLE is NOT
// touched here: moving the servo is the "apply live" action and is driven
// by the config API via the dashboard, not by this copy.
void applySettings() {
    IDLE_HOME_OFFSET      = g_settings.idleHomeOffset;
    ESPNOW_KICKOFF_OFFSET = g_settings.espnowKickoffOffset;
    FEEDING_DURATION_MS   = g_settings.feedingDurationMs;
}

// Save: staged struct -> EEPROM
// Flushes the STAGED struct to addr 4. Does NOT read the live globals, so
// it is safe to call on the deferred path (no live mutation implied).
void saveSettings() {
    g_settings.magic = MACHINE_SETTINGS_MAGIC;
    EEPROM.put(MACHINE_SETTINGS_ADDR, g_settings);
    EEPROM.commit();
}

// Load: EEPROM -> staged -> globals
void loadSettings() {
    // NOTE: EEPROM.begin() is already called by WebDashboard::init(), which
    // also restores SERVO_HOME_ANGLE from addr 0 into its live global.
    // Seed the staged servo-home mirror from that already-restored live value.
    g_stagedServoHomeAngle = SERVO_HOME_ANGLE;

    EEPROM.get(MACHINE_SETTINGS_ADDR, g_settings);

    if (g_settings.magic != MACHINE_SETTINGS_MAGIC) {
        // First boot: seed from defaults
        // The live globals still hold their compile-time defaults here, so
        // snapshot them into the staging struct and persist.
        g_settings.magic               = MACHINE_SETTINGS_MAGIC;
        g_settings.idleHomeOffset      = IDLE_HOME_OFFSET;
        g_settings.espnowKickoffOffset = ESPNOW_KICKOFF_OFFSET;
        g_settings.feedingDurationMs   = FEEDING_DURATION_MS;
        EEPROM.put(MACHINE_SETTINGS_ADDR, g_settings);
        EEPROM.commit();
        return;
    }

    // Valid: push into live globals
    applySettings();
}

// Staging (no live writes)
bool stageSettingByKey(const char* key, float value) {
    if (strcmp(key, "SERVO_HOME_ANGLE") == 0) {
        g_stagedServoHomeAngle = value;
        return true;
    }
    if (strcmp(key, "IDLE_HOME_OFFSET") == 0) {
        g_settings.idleHomeOffset = value;
        return true;
    }
    if (strcmp(key, "ESPNOW_KICKOFF_OFFSET") == 0) {
        g_settings.espnowKickoffOffset = value;
        return true;
    }
    if (strcmp(key, "FEEDING_DURATION_MS") == 0) {
        g_settings.feedingDurationMs = value;
        return true;
    }
    return false;
}

float getStagedServoHomeAngle() {
    return g_stagedServoHomeAngle;
}

void persistServoHomeAngle() {
    EEPROM.put(SERVO_HOME_ANGLE_ADDR, g_stagedServoHomeAngle);
    EEPROM.commit();
}
