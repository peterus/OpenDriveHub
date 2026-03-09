#include "ReceiverApi.h"

#include "ApiHandler.h"
#include "Config.h"
#include "FunctionMap.h"
#include "NvsStore.h"

#include <ArduinoJson.h>

namespace odh {

ReceiverApi::ReceiverApi(OdhWebServer &server, OutputManager &output, ReceiverRadioLink &radio, BatteryMonitor &battery)
    : _server(server),
      _output(output),
      _radio(radio),
      _battery(battery) {}

void ReceiverApi::begin() {
    // GET /api/status
    _server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *req) { handleGetStatus(req); });

    // GET /api/config
    _server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *req) { handleGetConfig(req); });

    // POST /api/config (JSON body)
    _server.onBody(
        "/api/config", HTTP_POST,
        [](AsyncWebServerRequest *req) {
            // Handler called after body is received – see body handler below.
        },
        [this](AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t index, size_t total) { handlePostConfig(req, data, len, index, total); });

    // POST /api/config/reset
    _server.on("/api/config/reset", HTTP_POST, [this](AsyncWebServerRequest *req) { handlePostConfigReset(req); });

    // Serve static files from LittleFS /receiver/ directory.
    _server.serveStatic("/", "/receiver/", "index.html");
}

void ReceiverApi::handleGetStatus(AsyncWebServerRequest *request) {
    JsonDocument doc;

    // Link – nested object matching the transmitter structure.
    const char *state = "disconnected";
    if (_radio.isBound())
        state = "connected";
    else if (_radio.isAnnouncing())
        state = "announcing";
    doc["link"]["state"] = state;
    doc["link"]["rssi"]  = _radio.lastRssi();

    // Battery – RX battery info.
    doc["battery"]["voltage_mv"] = _battery.voltageMv();
    doc["battery"]["cells"]      = _battery.cells();

    // Live channels – array of objects with value and function_name.
    const auto &fmap = _output.functionMap();
    auto channels    = doc["channels"].to<JsonArray>();
    for (uint8_t i = 0; i < config::rx::kChannelCount; i++) {
        auto ch     = channels.add<JsonObject>();
        ch["value"] = _output.channelValues()[i];

        // Look up which function is mapped to this channel.
        const char *fname = "";
        for (uint8_t f = 0; f < fmap.count; ++f) {
            if (fmap.entries[f].channel == i) {
                fname = functionName(fmap.entries[f].function).data();
                break;
            }
        }
        ch["function_name"] = fname;
    }

    sendJson(request, 200, doc);
}

void ReceiverApi::handleGetConfig(AsyncWebServerRequest *request) {
    NvsStore nvs("odh_rx", true);
    JsonDocument doc;

    doc["vehicle_name"] = nvs.getString("veh_name", "Receiver");
    doc["model_type"]   = static_cast<uint8_t>(_output.modelType());
    doc["model_name"]   = modelName(_output.modelType());

    // Function map – one entry per mapped function.
    auto fmap       = doc["function_map"].to<JsonArray>();
    const auto &map = _output.functionMap();
    for (uint8_t i = 0; i < map.count; i++) {
        auto entry             = fmap.add<JsonObject>();
        entry["function"]      = map.entries[i].function;
        entry["function_name"] = functionName(map.entries[i].function);
        entry["channel"]       = map.entries[i].channel;
    }

    // Failsafe value (single global, 0 = hold last).
    doc["failsafe_us"] = nvs.getU16("failsafe_us", 0);

    // Available model types.
    auto models = doc["available_models"].to<JsonArray>();
    for (uint8_t m = 0; m < static_cast<uint8_t>(ModelType::Count); m++) {
        auto entry    = models.add<JsonObject>();
        entry["id"]   = m;
        entry["name"] = modelName(m);
    }

    // Available functions – so the UI can build dropdown menus.
    auto funcs = doc["available_functions"].to<JsonArray>();
    for (uint8_t f = 0; f <= static_cast<uint8_t>(Function::TrackR); f++) {
        auto entry    = funcs.add<JsonObject>();
        entry["id"]   = f;
        entry["name"] = functionName(f);
    }

    sendJson(request, 200, doc);
}

void ReceiverApi::handlePostConfig(AsyncWebServerRequest *request, const uint8_t *data, size_t len, size_t /*index*/, size_t /*total*/) {
    JsonDocument doc;
    if (!parseBody(data, len, doc)) {
        sendError(request, 400, "Invalid JSON");
        return;
    }

    NvsStore nvs("odh_rx", false);

    // Update vehicle name if provided.
    if (doc["vehicle_name"].is<const char *>()) {
        nvs.putString("veh_name", doc["vehicle_name"].as<const char *>());
    }

    // Update model type if provided.
    if (doc["model_type"].is<uint8_t>()) {
        const auto model = static_cast<ModelType>(doc["model_type"].as<uint8_t>());
        _output.setModelType(model);
    }

    // Update function map if provided.
    if (doc["function_map"].is<JsonArray>()) {
        FunctionMapping map{};
        auto arr    = doc["function_map"].as<JsonArray>();
        uint8_t idx = 0;
        for (auto entry : arr) {
            if (idx >= kMaxFunctions)
                break;
            map.entries[idx].function = entry["function"].as<uint8_t>();
            map.entries[idx].channel  = idx; // auto-assign channel = index
            idx++;
        }
        map.count = idx;
        _output.setFunctionMap(map);
    }

    // Update failsafe value if provided.
    if (doc["failsafe_us"].is<uint16_t>()) {
        nvs.putU16("failsafe_us", doc["failsafe_us"].as<uint16_t>());
    }

    _output.saveToNvs();
    sendOk(request, "Configuration saved");
}

void ReceiverApi::handlePostConfigReset(AsyncWebServerRequest *request) {
    _output.setModelType(_output.modelType());
    _output.saveToNvs();
    sendOk(request, "Configuration reset to defaults");
}

} // namespace odh
