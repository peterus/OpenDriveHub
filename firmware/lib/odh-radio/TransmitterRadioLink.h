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
 * TransmitterRadioLink – ESP-NOW radio link for the transmitter side.
 *
 * Discovery-based connection model (transmitter side):
 *   1. Transmitter scans for receiver announce broadcasts.
 *   2. Operator selects a vehicle; transmitter sends BindPacket.
 *   3. Receiver replies with BindPacket → link established.
 *   4. On disconnect the transmitter sends Disconnect and resumes scanning.
 */

#pragma once

#include <esp_now.h>

#include "IRadioLink.h"

namespace odh {

class TransmitterRadioLink {
public:
    TransmitterRadioLink();
    ~TransmitterRadioLink()                                       = default;
    TransmitterRadioLink(const TransmitterRadioLink &)            = delete;
    TransmitterRadioLink &operator=(const TransmitterRadioLink &) = delete;

    /// Initialize WiFi and ESP-NOW. Does NOT set channel yet.
    bool begin(TelemetryCallback callback);

    /// Switch to a different WiFi channel at runtime.
    bool setChannel(uint8_t channel);

    /// Get the current WiFi channel.
    uint8_t currentChannel() const {
        return _wifiChannel;
    }

    // ── Discovery ───────────────────────────────────────────────

    /// Send a DiscoveryRequest broadcast (for finding other transmitters).
    bool sendDiscoveryRequest();

    /// Send a DiscoveryResponse (called when a DiscoveryRequest is received).
    bool sendDiscoveryResponse(uint8_t channel);

    /// Send ChannelMigration notification before switching channels.
    bool sendChannelMigration(uint8_t newChannel);

    /// Register callback for DiscoveryResponse packets (from other TXs).
    void onDiscoveryResponse(DiscoveryResponseCallback cb);

    /// Register callback for DiscoveryRequest packets (auto-reply).
    void onDiscoveryRequest(DiscoveryRequestCallback cb);

    // ── Scanning (listen for receiver presence) ─────────────────

    void startScanning();
    bool isScanning() const {
        return _scanning;
    }

    // ── Connect / Disconnect ────────────────────────────────────

    bool connectTo(uint8_t index, uint32_t timeoutMs = 5000);
    void disconnect();
    bool sendControl(const FunctionValue *functions, uint8_t count, uint8_t flags = 0);
    bool isBound() const {
        return _bound;
    }
    bool lastSendOk() const {
        return _lastSendOk;
    }
    int8_t lastRssi() const {
        return _lastRssi;
    }
    void boundMac(uint8_t out[6]) const;
    uint16_t txSequence() const {
        return _txSequence;
    }

    // ── Discovered vehicle list ─────────────────────────────────

    uint8_t discoveredCount() const {
        return _discoveredCount;
    }
    const DiscoveredVehicle *discoveredVehicle(uint8_t index) const;
    void pruneStaleVehicles(uint32_t timeoutMs);

private:
    bool _ready          = false;
    bool _bound          = false;
    bool _scanning       = false;
    bool _lastSendOk     = false;
    int8_t _lastRssi     = 0;
    uint8_t _peerMac[6]  = {};
    uint16_t _txSequence = 0;
    uint8_t _wifiChannel = 0;

    TelemetryCallback _telemetryCallback;
    DiscoveryResponseCallback _discoveryResponseCallback;
    DiscoveryRequestCallback _discoveryRequestCallback;

    DiscoveredVehicle _discovered[kMaxDiscovered] = {};
    uint8_t _discoveredCount                      = 0;

    static void onReceive(const uint8_t *mac, const uint8_t *data, int len);
    static void onSent(const uint8_t *mac, esp_now_send_status_t status);
    void handleReceive(const uint8_t *mac, const uint8_t *data, int len);
    void handleSent(esp_now_send_status_t status);
    bool addPeer(const uint8_t mac[6]);
    void delPeer(const uint8_t mac[6]);

    static TransmitterRadioLink *sInstance;
};

} // namespace odh
