/**
 * TransmitterApi – REST API endpoints for transmitter configuration.
 *
 * Built on top of OdhWebServer (ESPAsyncWebServer).
 * All endpoints under /api/ return JSON (ArduinoJson v7).
 *
 * Endpoints:
 *   GET  /api/status     – link state, battery, RSSI, PPS, modules
 *   GET  /api/config     – model, input maps, battery cells, device name
 *   POST /api/config     – update configuration
 *   POST /api/config/reset – factory-reset NVS
 */

#pragma once

#include "../modules/InputMap.h"
#include "../modules/ModuleManager.h"

#include <BatteryMonitor.h>
#include <OdhWebServer.h>
#include <TelemetryData.h>
#include <TransmitterRadioLink.h>

namespace odh {

class TransmitterApi {
public:
    TransmitterApi(OdhWebServer &server, TransmitterRadioLink &radio, BatteryMonitor &battery, TelemetryData &telemetry, ModuleManager &modules);

    void begin();

private:
    OdhWebServer &_server;
    TransmitterRadioLink &_radio;
    BatteryMonitor &_battery;
    TelemetryData &_telemetry;
    ModuleManager &_modules;

    void handleGetStatus(AsyncWebServerRequest *req);
    void handleGetConfig(AsyncWebServerRequest *req);
    void handlePostConfig(AsyncWebServerRequest *req, const uint8_t *data, size_t len);
    void handlePostReset(AsyncWebServerRequest *req);
};

} // namespace odh
