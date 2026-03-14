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
#include "esp_now.h"

#include <cstdint>

/* ── esp_wifi types (matching ESP-IDF) ──────────────────────────────────── */

typedef enum {
    WIFI_STA,
    WIFI_AP,
    WIFI_AP_STA,
    WIFI_OFF,
} wifi_mode_t;

typedef enum {
    WIFI_SECOND_CHAN_NONE = 0,
} wifi_second_chan_t;

typedef struct {
    int8_t rssi;
} wifi_pkt_rx_ctrl_t;

typedef struct {
    wifi_pkt_rx_ctrl_t rx_ctrl;
    uint8_t payload[0];
} wifi_promiscuous_pkt_t;

typedef enum {
    WIFI_PKT_MGMT = 0,
    WIFI_PKT_CTRL,
    WIFI_PKT_DATA,
    WIFI_PKT_MISC,
} wifi_promiscuous_pkt_type_t;

typedef void (*wifi_promiscuous_cb_t)(void *buf, wifi_promiscuous_pkt_type_t type);

/* ── esp_wifi API shim ──────────────────────────────────────────────────── */

void esp_wifi_set_channel(uint8_t channel, wifi_second_chan_t secondary);
void esp_wifi_set_promiscuous(bool enable);
void esp_wifi_set_promiscuous_rx_cb(wifi_promiscuous_cb_t cb);

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
    uint8_t channel() {
        return sim_get_wifi_channel();
    }
};

extern WiFiClass WiFi;

#endif /* SIM_WIFI_H */
