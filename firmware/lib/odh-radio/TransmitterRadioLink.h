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
    ~TransmitterRadioLink() = default;

    // Non-copyable, non-movable.
    TransmitterRadioLink(const TransmitterRadioLink &)            = delete;
    TransmitterRadioLink &operator=(const TransmitterRadioLink &) = delete;

    /**
     * Initialise WiFi in STA mode and start ESP-NOW.
     *
     * @param wifiChannel  WiFi channel to use (1-13).
     * @param callback     Called when a telemetry packet arrives.
     * @return true on success.
     */
    bool begin(uint8_t wifiChannel, TelemetryCallback callback);

    /// Enter scanning mode – listen for receiver announces.
    void startScanning();

    /// True if currently scanning.
    bool isScanning() const {
        return _scanning;
    }

    /**
     * Connect to a discovered vehicle by index.
     * Sends a bind packet and waits up to @p timeoutMs for a reply.
     *
     * @return true if the connection was established.
     */
    bool connectTo(uint8_t index, uint32_t timeoutMs = 5000);

    /// Disconnect from the current receiver and resume scanning.
    void disconnect();

    /**
     * Send a control packet to the connected receiver.
     *
     * @param functions  Array of function-value pairs.
     * @param count      Number of entries (≤ kMaxFunctions).
     * @param flags      Packet flags.
     * @return true if ESP-NOW accepted the packet.
     */
    bool sendControl(const FunctionValue *functions, uint8_t count, uint8_t flags = 0);

    /// True if a receiver is currently connected.
    bool isBound() const {
        return _bound;
    }

    /// True if the last sendControl() was acknowledged.
    bool lastSendOk() const {
        return _lastSendOk;
    }

    /// RSSI of the last received packet (dBm).
    int8_t lastRssi() const {
        return _lastRssi;
    }

    /// Copy the bound receiver's MAC address into @p out.
    void boundMac(uint8_t out[6]) const;

    /// Sequence counter for the next outgoing control packet.
    uint16_t txSequence() const {
        return _txSequence;
    }

    // ── Discovered vehicle list ─────────────────────────────────────

    /// Number of currently discovered vehicles.
    uint8_t discoveredCount() const {
        return _discoveredCount;
    }

    /// Access a discovered vehicle by index (nullptr if out of range).
    const DiscoveredVehicle *discoveredVehicle(uint8_t index) const;

    /// Remove stale vehicles not seen for @p timeoutMs.
    void pruneStaleVehicles(uint32_t timeoutMs);

private:
    bool _ready          = false;
    bool _bound          = false;
    bool _scanning       = false;
    bool _lastSendOk     = false;
    int8_t _lastRssi     = 0;
    uint8_t _peerMac[6]  = {};
    uint16_t _txSequence = 0;
    TelemetryCallback _telemetryCallback;
    uint8_t _wifiChannel = 1;

    DiscoveredVehicle _discovered[kMaxDiscovered] = {};
    uint8_t _discoveredCount                      = 0;

    // ESP-NOW static callbacks
    static void onReceive(const uint8_t *mac, const uint8_t *data, int len);
    static void onSent(const uint8_t *mac, esp_now_send_status_t status);

    void handleReceive(const uint8_t *mac, const uint8_t *data, int len);
    void handleSent(esp_now_send_status_t status);

    bool addPeer(const uint8_t mac[6]);
    void delPeer(const uint8_t mac[6]);

    static TransmitterRadioLink *sInstance;
};

} // namespace odh
