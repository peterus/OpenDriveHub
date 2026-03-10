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
 * sim_async_webserver.cpp – Working ESPAsyncWebServer simulation.
 *
 * Uses a non-blocking listening socket and a background thread to
 * handle HTTP requests, matching the async behaviour of the real
 * ESPAsyncWebServer on ESP32.
 *
 * Ports < 1024 are remapped to port + 8000 so no root privileges are
 * needed (80 → 8080).
 */

#include "ESPAsyncWebServer.h"

#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

static uint16_t simPort(uint16_t p) {
    if (p >= 1024u)
        return p;
#ifdef SIM_RX
    return static_cast<uint16_t>(p + 8001u); /* RX: 80 → 8081 */
#else
    return static_cast<uint16_t>(p + 8000u); /* TX: 80 → 8080 */
#endif
}

static void writeAll(int fd, const char *data, size_t len) {
    while (len > 0) {
        ssize_t sent = ::send(fd, data, len, MSG_NOSIGNAL);
        if (sent <= 0)
            break;
        data += sent;
        len -= static_cast<size_t>(sent);
    }
}

static const char *statusText(int code) {
    switch (code) {
    case 200:
        return "OK";
    case 303:
        return "See Other";
    case 400:
        return "Bad Request";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 500:
        return "Internal Server Error";
    default:
        return "Error";
    }
}

/* ── AsyncWebServerRequest ───────────────────────────────────────────────── */

AsyncStaticWebHandler &AsyncStaticWebHandler::setDefaultFile(const char *f) {
    defaultFile = f ? f : "";
    if (_owner) {
        std::lock_guard<std::mutex> lock(_owner->_routesMutex);
        if (_mountIndex < _owner->_staticMounts.size())
            _owner->_staticMounts[_mountIndex].defaultFile = defaultFile;
    }
    return *this;
}

AsyncWebServerRequest::AsyncWebServerRequest(int clientSock, int method, std::string body, std::string queryString)
    : _clientSock(clientSock),
      _method(method),
      _body(std::move(body)),
      _queryString(std::move(queryString)) {}

void AsyncWebServerRequest::send(int code, const char *type, const char *body) {
    if (_responseSent || _clientSock < 0)
        return;
    _responseSent = true;

    size_t bodyLen = body ? strlen(body) : 0;
    char header[512];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 %d %s\r\n"
                        "Content-Type: %s\r\n"
                        "Content-Length: %zu\r\n"
                        "Access-Control-Allow-Origin: *\r\n"
                        "Connection: close\r\n"
                        "\r\n",
                        code, statusText(code), type ? type : "text/plain", bodyLen);
    writeAll(_clientSock, header, static_cast<size_t>(hlen));
    if (body && bodyLen > 0) {
        writeAll(_clientSock, body, bodyLen);
    }
}

void AsyncWebServerRequest::send(int code, const char *type, const String &body) {
    send(code, type, body.c_str());
}

void AsyncWebServerRequest::send(AsyncWebServerResponse *) {
    /* Stub – response already handled via the other send overloads. */
}

AsyncWebServerResponse *AsyncWebServerRequest::beginResponse(int code, const char *type, const char *body) {
    send(code, type, body);
    static AsyncWebServerResponse r;
    return &r;
}

/* ── AsyncWebServer ──────────────────────────────────────────────────────── */

AsyncWebServer::AsyncWebServer(uint16_t port)
    : _port(simPort(port)) {}

AsyncWebServer::~AsyncWebServer() {
    end();
}

void AsyncWebServer::begin() {
    if (_running)
        return;

    _listenSock = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenSock < 0) {
        perror("[SIM] AsyncWebServer socket");
        return;
    }

    int opt = 1;
    setsockopt(_listenSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    fcntl(_listenSock, F_SETFL, O_NONBLOCK);

    struct sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(_port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(_listenSock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        perror("[SIM] AsyncWebServer bind");
        close(_listenSock);
        _listenSock = -1;
        return;
    }

    listen(_listenSock, 5);
    printf("[SIM] Web server available at http://localhost:%u\n", _port);

    _running      = true;
    _serverThread = std::thread(&AsyncWebServer::serverLoop, this);
}

void AsyncWebServer::end() {
    _running = false;
    if (_serverThread.joinable())
        _serverThread.join();
    if (_listenSock >= 0) {
        close(_listenSock);
        _listenSock = -1;
    }
}

void AsyncWebServer::on(const char *uri, int method, ArRequestHandlerFunction handler) {
    std::lock_guard<std::mutex> lock(_routesMutex);
    _routes.push_back({uri, method, std::move(handler), nullptr});
}

void AsyncWebServer::on(const char *uri, int method, ArRequestHandlerFunction handler, std::nullptr_t, ArBodyHandlerFunction bodyHandler) {
    std::lock_guard<std::mutex> lock(_routesMutex);
    _routes.push_back({uri, method, std::move(handler), std::move(bodyHandler)});
}

void AsyncWebServer::onNotFound(ArRequestHandlerFunction handler) {
    _notFound = std::move(handler);
}

AsyncStaticWebHandler &AsyncWebServer::serveStatic(const char *uri, void *fs, const char *path) {
    (void)fs;
    std::lock_guard<std::mutex> lock(_routesMutex);
    _staticMounts.push_back({uri ? uri : "/", path ? path : "/", ""});
    _lastStaticIndex           = _staticMounts.size() - 1;
    _staticHandler._owner      = this;
    _staticHandler._mountIndex = _lastStaticIndex;
    return _staticHandler;
}

/* ── Server loop (runs in background thread) ─────────────────────────────── */

void AsyncWebServer::serverLoop() {
    while (_running) {
        int client = accept(_listenSock, nullptr, nullptr);
        if (client < 0) {
            usleep(10'000); /* 10 ms */
            continue;
        }
        handleConnection(client);
        close(client);
    }
}

void AsyncWebServer::handleConnection(int clientSock) {
    /* Give the client up to 2 s to finish sending the request. */
    struct timeval tv{};
    tv.tv_sec  = 2;
    tv.tv_usec = 0;
    setsockopt(clientSock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    char buf[8192]{};
    ssize_t n = recv(clientSock, buf, sizeof(buf) - 1, 0);
    if (n <= 0)
        return;
    buf[n] = '\0';
    std::string raw(buf, static_cast<size_t>(n));

    /* Parse: METHOD SP URI SP HTTP/x.x CRLF … */
    std::string method, uri;
    size_t sp1 = raw.find(' ');
    size_t sp2 = raw.find(' ', sp1 + 1);
    if (sp1 == std::string::npos || sp2 == std::string::npos)
        return;
    method = raw.substr(0, sp1);
    uri    = raw.substr(sp1 + 1, sp2 - sp1 - 1);

    /* Split URI into path and query string. */
    size_t qpos         = uri.find('?');
    std::string uriPath = uri.substr(0, qpos);
    std::string query   = (qpos != std::string::npos) ? uri.substr(qpos + 1) : "";

    /* Body starts after the blank CRLF line. */
    size_t bodyPos   = raw.find("\r\n\r\n");
    std::string body = (bodyPos != std::string::npos) ? raw.substr(bodyPos + 4) : "";
    int httpMethod   = (method == "POST") ? HTTP_POST : (method == "PUT") ? HTTP_PUT : (method == "DELETE") ? HTTP_DELETE : HTTP_GET;

    /* Find matching route. */
    std::lock_guard<std::mutex> lock(_routesMutex);

    for (auto &r : _routes) {
        if (r.uri == uriPath && (r.method & httpMethod)) {
            AsyncWebServerRequest req(clientSock, httpMethod, body, query);

            /* If the route has a body handler, call it first. */
            if (r.bodyHandler && !body.empty()) {
                auto *bodyData = reinterpret_cast<uint8_t *>(body.data());
                r.bodyHandler(&req, bodyData, body.size(), 0, body.size());
            }

            /* Call the request handler (may be a no-op for body-only routes). */
            if (!req._responseSent && r.handler) {
                r.handler(&req);
            }

            /* If neither handler sent a response, send a default. */
            if (!req._responseSent) {
                req.send(200, "text/plain", "OK");
            }
            return;
        }
    }

    /* No route matched → try static file serving. */
    if (httpMethod == HTTP_GET && tryServeStatic(clientSock, uriPath))
        return;

    /* Nothing matched → 404. */
    AsyncWebServerRequest req(clientSock, httpMethod, body, query);
    if (_notFound) {
        _notFound(&req);
    }
    if (!req._responseSent) {
        req.send(404, "text/plain", "Not Found");
    }
}

/* ── Static file serving ─────────────────────────────────────────────────── */

static const char *mimeType(const std::string &path) {
    if (path.size() >= 5 && path.substr(path.size() - 5) == ".html")
        return "text/html";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".css")
        return "text/css";
    if (path.size() >= 3 && path.substr(path.size() - 3) == ".js")
        return "application/javascript";
    if (path.size() >= 5 && path.substr(path.size() - 5) == ".json")
        return "application/json";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".png")
        return "image/png";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".svg")
        return "image/svg+xml";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".ico")
        return "image/x-icon";
    return "application/octet-stream";
}

bool AsyncWebServer::tryServeStatic(int clientSock, const std::string &uriPath) {
    for (auto &mount : _staticMounts) {
        /* Check if the URI starts with the mount prefix. */
        if (uriPath.compare(0, mount.uriPrefix.size(), mount.uriPrefix) != 0)
            continue;

        /* Build the relative path after the URI prefix. */
        std::string relPath = uriPath.substr(mount.uriPrefix.size());
        if (relPath.empty() || relPath == "/" || relPath.back() == '/') {
            relPath += mount.defaultFile.empty() ? "index.html" : mount.defaultFile;
        }

        /* Resolve against data/<fsPath>/<relPath>.
         * firmware/data/ is relative to the working directory. */
        std::string fsPath = mount.fsPath;
        /* Strip leading slash from fsPath. */
        if (!fsPath.empty() && fsPath[0] == '/')
            fsPath = fsPath.substr(1);
        /* Strip trailing slash. */
        if (!fsPath.empty() && fsPath.back() == '/')
            fsPath.pop_back();

        std::string filePath = "data/" + fsPath + "/" + relPath;

        struct stat st{};
        if (stat(filePath.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
            continue;

        std::ifstream ifs(filePath, std::ios::binary);
        if (!ifs)
            continue;

        std::ostringstream oss;
        oss << ifs.rdbuf();
        std::string content = oss.str();

        char header[512];
        int hlen = snprintf(header, sizeof(header),
                            "HTTP/1.1 200 OK\r\n"
                            "Content-Type: %s\r\n"
                            "Content-Length: %zu\r\n"
                            "Access-Control-Allow-Origin: *\r\n"
                            "Connection: close\r\n"
                            "\r\n",
                            mimeType(filePath), content.size());
        writeAll(clientSock, header, static_cast<size_t>(hlen));
        writeAll(clientSock, content.data(), content.size());
        return true;
    }
    return false;
}
