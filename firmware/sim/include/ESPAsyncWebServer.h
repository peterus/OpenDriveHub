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
 * ESPAsyncWebServer simulation shim.
 *
 * Provides a working HTTP/1.1 server backed by POSIX TCP sockets and a
 * background thread, matching the subset of the ESPAsyncWebServer API
 * used by OdhWebServer / ReceiverApi / TransmitterApi.
 *
 * Ports below 1024 are remapped by +8000 so no root is required:
 *   80 → 8080, etc.
 */

#pragma once

#include "Arduino.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

/* Forward declarations. */
class AsyncWebServerRequest;
class AsyncWebServerResponse;
class AsyncWebServer;

/* Handler typedefs (matching the real library). */
using ArRequestHandlerFunction  = std::function<void(AsyncWebServerRequest *)>;
using ArBodyHandlerFunction     = std::function<void(AsyncWebServerRequest *, uint8_t *, size_t, size_t, size_t)>;
using WebRequestMethodComposite = int;

/* HTTP method constants (matching ESPAsyncWebServer). */
#define HTTP_GET 0x0001
#define HTTP_POST 0x0002
#define HTTP_DELETE 0x0004
#define HTTP_PUT 0x0008

/* ── AsyncWebServerResponse ──────────────────────────────────────────────── */

class AsyncWebServerResponse {
public:
    void addHeader(const char *, const char *) {}
};

/* ── AsyncWebServerRequest ───────────────────────────────────────────────── */

class AsyncWebServerRequest {
public:
    AsyncWebServerRequest(int clientSock, int method, std::string body, std::string queryString);

    void send(int code, const char *type = "", const char *body = "");
    void send(int code, const char *type, const String &body);
    void send(AsyncWebServerResponse *resp);

    AsyncWebServerResponse *beginResponse(int code, const char *type, const char *body);

private:
    int _clientSock;
    int _method;
    std::string _body;
    std::string _queryString;
    bool _responseSent = false;

    friend class AsyncWebServer;
};

/* ── AsyncStaticWebHandler (stores static file config for sim) ────────────── */

class AsyncWebServer; /* forward decl */

class AsyncStaticWebHandler {
public:
    AsyncStaticWebHandler &setDefaultFile(const char *f);
    std::string defaultFile;
    AsyncWebServer *_owner = nullptr;
    size_t _mountIndex     = 0;
};

/* ── AsyncWebServer ──────────────────────────────────────────────────────── */

class AsyncWebServer {
public:
    explicit AsyncWebServer(uint16_t port = 80);
    ~AsyncWebServer();

    void begin();
    void end();

    void on(const char *uri, int method, ArRequestHandlerFunction handler);
    void on(const char *uri, int method, ArRequestHandlerFunction handler, std::nullptr_t, ArBodyHandlerFunction bodyHandler);
    void onNotFound(ArRequestHandlerFunction handler);

    AsyncStaticWebHandler &serveStatic(const char *uri, void *fs, const char *path);

    friend class AsyncStaticWebHandler;

private:
    struct Route {
        std::string uri;
        int method;
        ArRequestHandlerFunction handler;
        ArBodyHandlerFunction bodyHandler;
    };

    struct StaticMount {
        std::string uriPrefix;
        std::string fsPath;
        std::string defaultFile;
    };

    uint16_t _port;
    int _listenSock = -1;
    std::vector<Route> _routes;
    std::vector<StaticMount> _staticMounts;
    ArRequestHandlerFunction _notFound;
    std::thread _serverThread;
    std::atomic<bool> _running{false};
    std::mutex _routesMutex;
    AsyncStaticWebHandler _staticHandler;

    size_t _lastStaticIndex = 0;

    void serverLoop();
    void handleConnection(int clientSock);
    bool tryServeStatic(int clientSock, const std::string &uriPath);
};
