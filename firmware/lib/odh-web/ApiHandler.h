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
