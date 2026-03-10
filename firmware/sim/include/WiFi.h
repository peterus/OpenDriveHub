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
 * WiFi.h – Simulation shim (stubs).
 */

#ifndef SIM_WIFI_H
#define SIM_WIFI_H

#include "Arduino.h"

#include <cstdint>

typedef enum {
    WIFI_STA,
    WIFI_AP,
    WIFI_AP_STA,
    WIFI_OFF,
} wifi_mode_t;

class IPAddress {
    uint32_t _addr;

public:
    IPAddress()
        : _addr(0x0100007F) {} /* 127.0.0.1 */
    /* Implicit conversion lets Serial.print(ip) use the const char* overload. */
    operator const char *() const {
        return "127.0.0.1";
    }
};

class WiFiClass {
public:
    void mode(wifi_mode_t) {}
    void disconnect(bool = false) {}
    bool softAP(const char *, const char * = nullptr) {
        return true;
    }
    void softAPdisconnect(bool = false) {}
    IPAddress softAPIP() {
        return IPAddress();
    }
};

extern WiFiClass WiFi;

#endif /* SIM_WIFI_H */
