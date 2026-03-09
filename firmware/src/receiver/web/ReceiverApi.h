/**
 * ReceiverApi – REST API endpoints for the receiver web configuration.
 *
 * All endpoints use JSON request/response format via ArduinoJson v7.
 *
 * Endpoints:
 *   GET  /api/status        – link state, battery, model info
 *   GET  /api/config        – current model type, function map, failsafe values
 *   POST /api/config        – update model type, function map, failsafe values
 *   POST /api/config/reset  – reset to defaults for current model type
 */

#pragma once

#include "../OutputManager.h"
#include "BatteryMonitor.h"
#include "OdhWebServer.h"
#include "ReceiverRadioLink.h"

namespace odh {

class ReceiverApi {
public:
    /**
     * @param server   The web server instance to register routes on.
     * @param output   Output manager (for config read/write).
     * @param radio    Radio link (for status info).
     * @param battery  Battery monitor (for voltage reading).
     */
    ReceiverApi(OdhWebServer &server, OutputManager &output, ReceiverRadioLink &radio, BatteryMonitor &battery);

    /// Register all API routes on the web server.
    void begin();

private:
    OdhWebServer &_server;
    OutputManager &_output;
    ReceiverRadioLink &_radio;
    BatteryMonitor &_battery;

    void handleGetStatus(AsyncWebServerRequest *request);
    void handleGetConfig(AsyncWebServerRequest *request);
    void handlePostConfig(AsyncWebServerRequest *request, const uint8_t *data, size_t len, size_t index, size_t total);
    void handlePostConfigReset(AsyncWebServerRequest *request);
};

} // namespace odh
