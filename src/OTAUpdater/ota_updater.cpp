#include "OTAUpdater/ota_updater.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//* ************************************************************************
//* *********************** OTA UPDATER IMPLEMENTATION *********************
//* ************************************************************************
// Handles WiFi connection and Over-The-Air updates for the ESP32.

const char* ssid = "Everwood";
const char* password = "Everwood-Staff";

void setupOTA() {
  //serial.println("Booting for OTA...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    //serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname("router-esp32s3");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else { // U_SPIFFS
        type = "filesystem";
      }
      // NOTE: if updating SPIFFS, ensure SPIFFS is mounted via SPIFFS.begin()
      //serial.println("Start updating " + type);
      //serial.println("OTA Upload started");
    })
    .onEnd([]() {
      //serial.println("\nOTA Upload completed!");
      //serial.println("OTA completion finished");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      // Progress indication can be added here if needed
    })
    .onError([](ota_error_t error) {
      if (error == OTA_AUTH_ERROR) {
        //serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        //serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        //serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        //serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        //serial.println("End Failed");
      }
      //serial.println("OTA error indication completed");
    });

  ArduinoOTA.begin();

  //serial.println("OTA Initialized");
  //serial.print("IP address: ");
  //serial.println(WiFi.localIP());
}

void handleOTA() {
  ArduinoOTA.handle();
} 