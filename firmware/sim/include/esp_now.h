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
 * esp_now.h – Simulation shim using UDP sockets.
 *
 * Each WiFi channel maps to a shared UDP port (port = 7000 + channel).
 * All devices on the same channel listen and send on the same port.
 * Self-packets are filtered by comparing the source MAC.
 */

#ifndef SIM_ESP_NOW_H
#define SIM_ESP_NOW_H

#include <cstdint>

/* ── ESP-NOW types ──────────────────────────────────────────────────────── */

#define ESP_NOW_ETH_ALEN 6
#define ESP_NOW_MAX_DATA_LEN 250

typedef enum {
    ESP_NOW_SEND_SUCCESS = 0,
    ESP_NOW_SEND_FAIL    = 1,
} esp_now_send_status_t;

typedef void (*esp_now_recv_cb_t)(const uint8_t *mac, const uint8_t *data, int len);
typedef void (*esp_now_send_cb_t)(const uint8_t *mac, esp_now_send_status_t status);

typedef struct {
    uint8_t peer_addr[ESP_NOW_ETH_ALEN];
    uint8_t channel;
    bool encrypt;
} esp_now_peer_info_t;

/* ── ESP-NOW API ────────────────────────────────────────────────────────── */

int esp_now_init();
int esp_now_deinit();
int esp_now_register_recv_cb(esp_now_recv_cb_t cb);
int esp_now_register_send_cb(esp_now_send_cb_t cb);
int esp_now_send(const uint8_t *peer_addr, const uint8_t *data, int len);
int esp_now_add_peer(const esp_now_peer_info_t *peer);
int esp_now_del_peer(const uint8_t *peer_addr);
bool esp_now_is_peer_exist(const uint8_t *peer_addr);

/* ── Simulation control ─────────────────────────────────────────────────── */

/// Set the simulated WiFi channel. Switches the underlying UDP port.
void sim_set_wifi_channel(uint8_t channel);

/// Get the current simulated WiFi channel.
uint8_t sim_get_wifi_channel();

#endif /* SIM_ESP_NOW_H */
