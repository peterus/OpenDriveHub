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
 * ApiHandler – JSON REST API helper utilities.
 *
 * Provides common patterns for handling JSON requests/responses
 * with ESPAsyncWebServer and ArduinoJson v7.
 */

#pragma once

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <cstdint>

namespace odh {

/// Send a JSON response from a JsonDocument.
inline void sendJson(AsyncWebServerRequest *request, int code, const JsonDocument &doc) {
    String output;
    serializeJson(doc, output);
    request->send(code, "application/json", output);
}

/// Send a simple JSON error response.
inline void sendError(AsyncWebServerRequest *request, int code, const char *message) {
    JsonDocument doc;
    doc["error"]  = message;
    doc["status"] = code;
    sendJson(request, code, doc);
}

/// Send a simple JSON success response.
inline void sendOk(AsyncWebServerRequest *request, const char *message = "ok") {
    JsonDocument doc;
    doc["status"]  = 200;
    doc["message"] = message;
    sendJson(request, 200, doc);
}

/// Parse a JSON body. Returns true on success.
inline bool parseBody(const uint8_t *data, size_t len, JsonDocument &doc) {
    DeserializationError err = deserializeJson(doc, data, len);
    return err == DeserializationError::Ok;
}

} // namespace odh
