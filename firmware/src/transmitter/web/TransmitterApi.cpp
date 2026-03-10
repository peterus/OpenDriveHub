/*
 * Copyright (C) 2026 Peter Buchegger
 *
 * This file is part of OpenDriveHub.
 *
 * OpenDriveHub is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenDriveHub is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenDriveHub. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * TransmitterApi – implementation.
 */

#include "TransmitterApi.h"

#include <ApiHandler.h>
#include <Config.h>
#include <FunctionMap.h>
#include <NvsStore.h>
#include <Protocol.h>

namespace odh {

TransmitterApi::TransmitterApi(OdhWebServer &server, TransmitterRadioLink &radio, BatteryMonitor &battery, TelemetryData &telemetry, ModuleManager &modules)
    : _server{server},
      _radio{radio},
      _battery{battery},
      _telemetry{telemetry},
      _modules{modules} {}

void TransmitterApi::begin() {
    _server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *r) { handleGetStatus(r); });

    _server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *r) { handleGetConfig(r); });

    // cppcheck-suppress constParameterPointer
    _server.onBody("/api/config", HTTP_POST, [](AsyncWebServerRequest *) {}, [this](AsyncWebServerRequest *r, uint8_t *d, size_t l, size_t, size_t) { handlePostConfig(r, d, l); });

    _server.on("/api/config/reset", HTTP_POST, [this](AsyncWebServerRequest *r) { handlePostReset(r); });

    // Serve static files from LittleFS /transmitter/ directory.
    _server.serveStatic("/", "/transmitter/", "index.html");
}

/* ── GET /api/status ─────────────────────────────────────────────────────── */

void TransmitterApi::handleGetStatus(AsyncWebServerRequest *req) {
    JsonDocument doc;

    // Link
    doc["link"]["state"] = _radio.isBound() ? "connected" : "scanning";
    doc["link"]["rssi"]  = _telemetry.rssi();

    // TX battery
    doc["battery"]["tx"]["voltage_mv"] = _battery.voltageMv();
    doc["battery"]["tx"]["cells"]      = _battery.cells();

    // RX battery (from telemetry)
    if (_telemetry.hasData()) {
        doc["battery"]["rx"]["voltage_mv"] = _telemetry.rxBatteryMv();
        doc["battery"]["rx"]["cells"]      = _telemetry.rxCells();
        doc["telemetry"]["pps"]            = _telemetry.packetsPerSecond();
        doc["telemetry"]["uptime_ms"]      = _telemetry.connectionUptimeMs();
    }

    // Modules
    JsonArray mods = doc["modules"].to<JsonArray>();
    for (uint8_t s = 0; s < _modules.slotCount(); ++s) {
        JsonObject m     = mods.add<JsonObject>();
        m["slot"]        = s;
        m["type"]        = static_cast<uint8_t>(_modules.typeAt(s));
        m["input_count"] = _modules.inputCount(s);
    }

    // Discovered vehicles (scan mode)
    if (_radio.isScanning()) {
        JsonArray vehs = doc["vehicles"].to<JsonArray>();
        for (uint8_t i = 0; i < _radio.discoveredCount(); ++i) {
            const auto *v = _radio.discoveredVehicle(i);
            if (!v || !v->valid)
                continue;
            JsonObject vj    = vehs.add<JsonObject>();
            vj["index"]      = i;
            vj["name"]       = v->name;
            vj["model_type"] = v->modelType;
            vj["rssi"]       = v->rssi;
        }
    }

    sendJson(req, 200, doc);
}

/* ── GET /api/config ─────────────────────────────────────────────────────── */

void TransmitterApi::handleGetConfig(AsyncWebServerRequest *req) {
    NvsStore nvs("odh", true);
    JsonDocument doc;

    doc["device_name"]   = nvs.getString("dev_name", "TX");
    doc["radio_channel"] = nvs.getU8("radio_ch", 1);

    uint8_t model          = nvs.getU8("model_type", static_cast<uint8_t>(ModelType::Generic));
    doc["model_type"]      = model;
    doc["model_type_name"] = modelName(static_cast<ModelType>(model)).data();
    doc["tx_cells"]        = nvs.getU8("tx_cells", 0);
    doc["rx_cells"]        = nvs.getU8("rx_cells", 0);

    // Available models
    JsonArray models = doc["available_models"].to<JsonArray>();
    for (uint8_t m = 0; m < static_cast<uint8_t>(ModelType::Count); ++m) {
        JsonObject mo = models.add<JsonObject>();
        mo["id"]      = m;
        mo["name"]    = modelName(static_cast<ModelType>(m)).data();
    }

    // Input maps for all models
    JsonObject maps = doc["input_maps"].to<JsonObject>();
    for (uint8_t m = 0; m < static_cast<uint8_t>(ModelType::Count); ++m) {
        char ck[16], ek[16];
        snprintf(ck, sizeof(ck), "imapc_%u", m);
        snprintf(ek, sizeof(ek), "imape_%u", m);

        uint8_t count = nvs.getU8(ck, 0);
        InputAssignment entries[kMaxFunctions];

        if (count > 0 && count <= kMaxFunctions) {
            nvs.getBytes(ek, entries, count * sizeof(InputAssignment));
        } else {
            // Build defaults from function map
            auto fmap  = defaultFunctionMap(static_cast<ModelType>(m));
            count      = fmap.count;
            uint8_t sl = 0, inp = 0;
            for (uint8_t i = 0; i < count; ++i) {
                entries[i].function   = fmap.entries[i].function;
                entries[i].slot       = sl;
                entries[i].inputIndex = inp;
                entries[i].trim       = 0;
                if (++inp >= 4) {
                    inp = 0;
                    ++sl;
                }
            }
        }

        char mk[4];
        snprintf(mk, sizeof(mk), "%u", m);
        JsonArray arr = maps[mk].to<JsonArray>();
        for (uint8_t i = 0; i < count; ++i) {
            JsonObject e       = arr.add<JsonObject>();
            e["function"]      = entries[i].function;
            e["function_name"] = functionName(static_cast<Function>(entries[i].function)).data();
            e["slot"]          = entries[i].slot;
            e["input_index"]   = entries[i].inputIndex;
            e["trim"]          = entries[i].trim;
        }
    }

    // Modules (so the UI can show physical input pickers)
    JsonArray mods = doc["modules"].to<JsonArray>();
    for (uint8_t s = 0; s < _modules.slotCount(); ++s) {
        JsonObject mo     = mods.add<JsonObject>();
        mo["slot"]        = s;
        mo["type"]        = static_cast<uint8_t>(_modules.typeAt(s));
        mo["input_count"] = _modules.inputCount(s);
    }

    sendJson(req, 200, doc);
}

/* ── POST /api/config ────────────────────────────────────────────────────── */

void TransmitterApi::handlePostConfig(AsyncWebServerRequest *req, const uint8_t *data, size_t len) {
    JsonDocument doc;
    if (!parseBody(data, len, doc)) {
        sendError(req, 400, "Invalid JSON");
        return;
    }

    NvsStore nvs("odh", false);

    if (doc["device_name"].is<const char *>()) {
        nvs.putString("dev_name", doc["device_name"].as<const char *>());
    }
    if (doc["radio_channel"].is<uint8_t>()) {
        uint8_t ch = doc["radio_channel"].as<uint8_t>();
        if (ch >= 1 && ch <= 13)
            nvs.putU8("radio_ch", ch);
    }
    if (doc["model_type"].is<uint8_t>()) {
        uint8_t m = doc["model_type"].as<uint8_t>();
        if (m < static_cast<uint8_t>(ModelType::Count))
            nvs.putU8("model_type", m);
    }
    if (doc["tx_cells"].is<uint8_t>()) {
        uint8_t c = doc["tx_cells"].as<uint8_t>();
        if (c <= 4)
            nvs.putU8("tx_cells", c);
    }
    if (doc["rx_cells"].is<uint8_t>()) {
        uint8_t c = doc["rx_cells"].as<uint8_t>();
        if (c <= 4)
            nvs.putU8("rx_cells", c);
    }

    // Input maps per model
    if (doc["input_maps"].is<JsonObject>()) {
        for (uint8_t m = 0; m < static_cast<uint8_t>(ModelType::Count); ++m) {
            char mk[4];
            snprintf(mk, sizeof(mk), "%u", m);
            if (!doc["input_maps"][mk].is<JsonArray>())
                continue;

            JsonArray arr = doc["input_maps"][mk].as<JsonArray>();
            uint8_t count = 0;
            InputAssignment entries[kMaxFunctions];

            for (JsonObject e : arr) {
                if (count >= kMaxFunctions)
                    break;
                entries[count].function   = e["function"].as<uint8_t>();
                entries[count].slot       = e["slot"].as<uint8_t>();
                entries[count].inputIndex = e["input_index"].as<uint8_t>();
                entries[count].trim       = e["trim"].as<int8_t>();
                ++count;
            }

            char ck[16], ek[16];
            snprintf(ck, sizeof(ck), "imapc_%u", m);
            snprintf(ek, sizeof(ek), "imape_%u", m);
            nvs.putU8(ck, count);
            nvs.putBytes(ek, entries, count * sizeof(InputAssignment));
        }
    }

    sendOk(req, "Configuration saved");
}

/* ── POST /api/config/reset ──────────────────────────────────────────────── */

void TransmitterApi::handlePostReset(AsyncWebServerRequest *req) {
    Preferences p;
    p.begin("odh", false);
    p.clear();
    p.end();

    sendOk(req, "Configuration reset – reboot recommended");
}

} // namespace odh
