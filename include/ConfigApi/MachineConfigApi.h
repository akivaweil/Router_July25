#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

// Shared machine config API
// Implements the CANONICAL cross-machine REST config + status contract
// (see docs/DASHBOARD_API_CONTRACT.md). Three routes on port 80:
//   GET  /api/status  -> live read-only status
//   GET  /api/config  -> current editable settings (self-describing)
//   POST /api/config  -> validate + persist + apply-or-defer
//
// Router is an async machine, so setupConfigApi takes an AsyncWebServer&.

// Route registration
void setupConfigApi(AsyncWebServer& server);

// Response / handler core
String buildStatusJson();   // GET /api/status body
String buildConfigJson();   // GET /api/config body
// applyConfigJson: parse+validate the flat key->value POST body. On any
// unknown key or out-of-range value, returns false and changes nothing.
// On success persists immediately; applies live now (outDeferred=false) if
// isSafeToApplyConfig(), otherwise sets configDirty (outDeferred=true).
bool applyConfigJson(const String& body, bool& outDeferred, String& outMsg);

// isSafeToApplyConfig: true only when the machine is IDLE.
bool isSafeToApplyConfig();

// Deferred-apply hook
// Set when a POST is accepted mid-cycle. The main loop calls
// applyPendingConfigIfIdle() so the persisted values take effect on the
// next entry to IDLE.
extern volatile bool configDirty;
void applyPendingConfigIfIdle();
