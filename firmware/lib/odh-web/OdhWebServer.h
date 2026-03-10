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
 * WebServer – ESPAsyncWebServer + LittleFS wrapper.
 *
 * Provides a thin abstraction for starting an async web server that
 * serves static files from LittleFS and supports REST API routes.
 */

#pragma once

#include <ESPAsyncWebServer.h>
#include <cstdint>
#include <functional>

#ifndef NATIVE_SIM
#include <LittleFS.h>
#endif

namespace odh {

/**
 * Thin wrapper around ESPAsyncWebServer with LittleFS support.
 *
 * Usage:
 *   WebServer server;
 *   server.begin("MyAP", "password", 80);
 *   server.serveStatic("/", LittleFS, "/www/").setDefaultFile("index.html");
 *   server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest* req) { ... });
 */
class OdhWebServer {
public:
    explicit OdhWebServer(uint16_t port = 80);
    ~OdhWebServer() = default;

    // Non-copyable.
    OdhWebServer(const OdhWebServer &)            = delete;
    OdhWebServer &operator=(const OdhWebServer &) = delete;

    /**
     * Start WiFi AP and HTTP server.
     *
     * @param ssid       AP SSID.
     * @param password   AP password (empty for open).
     * @param port       HTTP port.
     * @return true on success.
     */
    bool begin(const char *ssid, const char *password, uint16_t port = 80);

    /// Stop the server and WiFi AP.
    void stop();

    /// True if the server is running.
    bool isRunning() const {
        return _running;
    }

    /// Direct access to the underlying AsyncWebServer.
    AsyncWebServer &server() {
        return _server;
    }

    /**
     * Convenience: add a request handler.
     */
    void on(const char *uri, WebRequestMethodComposite method, ArRequestHandlerFunction handler) {
        _server.on(uri, method, handler);
    }

    /**
     * Convenience: add a body request handler (for JSON POST/PUT).
     */
    void onBody(const char *uri, WebRequestMethodComposite method, ArRequestHandlerFunction handler, ArBodyHandlerFunction bodyHandler) {
        _server.on(uri, method, handler, nullptr, bodyHandler);
    }

    /**
     * Serve static files from LittleFS.
     */
    void serveStatic(const char *uri, const char *fsPath, const char *defaultFile = "index.html");

private:
    uint16_t _port;
    bool _running = false;

    AsyncWebServer _server;
};

} // namespace odh
