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
 *   1. Receiver broadcasts AnnouncePacket periodically.
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
    ~ReceiverRadioLink() = default;

    // Non-copyable, non-movable.
    ReceiverRadioLink(const ReceiverRadioLink &)            = delete;
    ReceiverRadioLink &operator=(const ReceiverRadioLink &) = delete;

    /**
     * Initialise WiFi in STA mode and start ESP-NOW.
     *
     * @param wifiChannel  WiFi channel to listen on (must match the transmitter).
     * @param callback     Called when a valid control packet arrives.
     * @return true on success.
     */
    bool begin(uint8_t wifiChannel, ControlCallback callback);

    /**
     * Begin periodic announce broadcasts.
     *
     * @param name       Vehicle name (max kVehicleNameMax chars).
     * @param modelType  ModelType identifying this vehicle.
     */
    void beginAnnouncing(const char *name, uint8_t modelType);

    /**
     * Send one announce broadcast if enough time has elapsed.
     * Call periodically from a FreeRTOS task.
     */
    void tickAnnounce(uint32_t intervalMs);

    /// True if the receiver is currently announcing.
    bool isAnnouncing() const {
        return _announcing;
    }

    /**
     * Send a telemetry packet to the bound transmitter.
     */
    bool sendTelemetry(uint16_t batteryMv, int8_t rssi, uint8_t linkState, uint8_t modelType, uint8_t modelFlags, const uint16_t *sensors, uint8_t sensorCount);

    /// True if a transmitter is currently connected.
    bool isBound() const {
        return _bound;
    }

    /// Milliseconds since the last control packet was received.
    uint32_t msSinceLastControl() const;

    /// RSSI (dBm) of the last received control packet.
    int8_t lastRssi() const {
        return _lastRssi;
    }

    /**
     * Check link timeout.  If no control packet for @p timeoutMs,
     * automatically disconnects and resumes announcing.
     *
     * @return true if the link was dropped.
     */
    bool checkLinkTimeout(uint32_t timeoutMs);

private:
    bool _ready                        = false;
    bool _bound                        = false;
    bool _announcing                   = false;
    uint8_t _txMac[6]                  = {};
    uint16_t _txSequence               = 0;
    uint32_t _lastControlMs            = 0;
    uint32_t _lastAnnounceMs           = 0;
    uint8_t _wifiChannel               = 1;
    uint8_t _modelType                 = 0;
    char _vehicleName[kVehicleNameMax] = {};
    ControlCallback _controlCallback;
    volatile int8_t _lastRssi = 0;

    // ESP-NOW static callbacks
    static void onReceive(const uint8_t *mac, const uint8_t *data, int len);
    static void onSent(const uint8_t *mac, esp_now_send_status_t status);

    void handleReceive(const uint8_t *mac, const uint8_t *data, int len);
    bool addPeer(const uint8_t mac[6]);
    void delPeer(const uint8_t mac[6]);
    void sendAnnounce();
    void stopAnnouncing();
    void resumeAnnouncing();

    // Active instance for static callback forwarding.
    static ReceiverRadioLink *sInstance;
};

} // namespace odh
