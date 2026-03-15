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
 * ReceiverRadioLink – ESP-NOW radio link for the receiver side.
 *
 * Discovery-based connection model (receiver side):
 *   1. Receiver broadcasts ReceiverPresencePacket periodically.
 *   2. Transmitter selects this receiver and sends BindPacket.
 *   3. Receiver replies with its own BindPacket and stops announcing.
 *   4. Subsequent communication is directed (TX MAC <-> RX MAC).
 *   5. On disconnect the receiver resumes announcing.
 *
 * Singleton pattern is eliminated – the instance is owned by ReceiverApp.
 * ESP-NOW static callbacks forward to the active instance via a static pointer.
 */

#pragma once

#include <esp_now.h>

#include "IRadioLink.h"

namespace odh {

class ReceiverRadioLink {
public:
    ReceiverRadioLink();
    ~ReceiverRadioLink()                                    = default;
    ReceiverRadioLink(const ReceiverRadioLink &)            = delete;
    ReceiverRadioLink &operator=(const ReceiverRadioLink &) = delete;

    /// Initialize WiFi and ESP-NOW. Does NOT set channel yet.
    bool begin(ControlCallback controlCallback);

    /// Switch to a different WiFi channel at runtime.
    bool setChannel(uint8_t channel);

    /// Get the current WiFi channel.
    uint8_t currentChannel() const {
        return _wifiChannel;
    }

    // ── Presence mode (replaces old announcing) ──────────────────

    /// Configure presence parameters (call before beginPresence).
    void configurePresence(const char *name, uint8_t modelType);

    /// Start periodic presence broadcasts on the current channel.
    void beginPresence();

    /// Send one presence broadcast if enough time has elapsed.
    void tickPresence(uint32_t intervalMs);

    /// True if currently broadcasting presence.
    bool isPresencing() const {
        return _presencing;
    }

    // ── Discovery ───────────────────────────────────────────────

    /// Send a DiscoveryRequest broadcast on the current channel.
    bool sendDiscoveryRequest();

    /// Register callback for DiscoveryResponse packets.
    void onDiscoveryResponse(DiscoveryResponseCallback cb);

    /// Register callback for ChannelMigration packets.
    void onChannelMigration(ChannelMigrationCallback cb);

    // ── Telemetry & link state ──────────────────────────────────

    bool sendTelemetry(uint16_t batteryMv, int8_t rssi, uint8_t linkState, uint8_t modelType, uint8_t modelFlags, const uint16_t *sensors, uint8_t sensorCount);
    bool isBound() const {
        return _bound;
    }
    uint32_t msSinceLastControl() const;
    int8_t lastRssi() const {
        return _lastRssi;
    }
    bool checkLinkTimeout(uint32_t timeoutMs);

private:
    bool _ready                        = false;
    bool _bound                        = false;
    bool _presencing                   = false;
    uint8_t _txMac[6]                  = {};
    uint16_t _txSequence               = 0;
    uint32_t _lastControlMs            = 0;
    uint32_t _lastPresenceMs           = 0;
    uint8_t _wifiChannel               = 0;
    uint8_t _modelType                 = 0;
    char _vehicleName[kVehicleNameMax] = {};

    ControlCallback _controlCallback;
    DiscoveryResponseCallback _discoveryResponseCallback;
    ChannelMigrationCallback _channelMigrationCallback;
    volatile int8_t _lastRssi = 0;

    static void onReceive(const uint8_t *mac, const uint8_t *data, int len);
    static void onSent(const uint8_t *mac, esp_now_send_status_t status);
    void handleReceive(const uint8_t *mac, const uint8_t *data, int len);

    bool addPeer(const uint8_t mac[6]);
    void delPeer(const uint8_t mac[6]);
    void sendPresencePacket();
    void stopPresence();
    void resumePresence();

    static ReceiverRadioLink *sInstance;
};

} // namespace odh
