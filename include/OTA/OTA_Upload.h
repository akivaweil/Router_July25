#pragma once

// OTA Upload
// Wireless firmware updates for the router control system.

// Connect WiFi (if needed) and start the ArduinoOTA service.
void setupOTA();

// Service pending OTA requests. Call from loop().
void handleOTA();
