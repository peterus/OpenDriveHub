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
