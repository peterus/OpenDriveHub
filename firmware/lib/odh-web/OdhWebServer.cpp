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

#include "OdhWebServer.h"

#ifndef NATIVE_SIM
#include <WiFi.h>
#endif

namespace odh {

OdhWebServer::OdhWebServer(uint16_t port)
    : _port(port),
      _server(port) {}

bool OdhWebServer::begin(const char *ssid, const char *password, uint16_t port) {
    _port = port;

#ifndef NATIVE_SIM
    // Mount LittleFS for static file serving.
    if (!LittleFS.begin(true)) {
        // Format on first mount failure.
        LittleFS.begin(true);
    }

    // Start WiFi AP.
    WiFi.softAP(ssid, password);
    delay(100); // Allow AP to stabilise.
#else
    (void)ssid;
    (void)password;
#endif

    _server.begin();
    _running = true;
    return true;
}

void OdhWebServer::stop() {
    _server.end();
#ifndef NATIVE_SIM
    WiFi.softAPdisconnect(true);
#endif
    _running = false;
}

#ifndef NATIVE_SIM
void OdhWebServer::serveStatic(const char *uri, const char *fsPath, const char *defaultFile) {
    _server.serveStatic(uri, LittleFS, fsPath).setDefaultFile(defaultFile);
}
#else
void OdhWebServer::serveStatic(const char *uri, const char *fsPath, const char *defaultFile) {
    _server.serveStatic(uri, nullptr, fsPath).setDefaultFile(defaultFile);
}
#endif

} // namespace odh
