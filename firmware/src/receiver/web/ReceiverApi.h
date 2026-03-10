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
