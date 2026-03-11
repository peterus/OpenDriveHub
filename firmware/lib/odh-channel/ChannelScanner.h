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

#pragma once

#include "Channel.h"

#include <Protocol.h>
#include <cstdint>
#include <functional>

namespace odh {

/// Result of scanning a single WiFi channel for transmitter activity.
struct ScanResult {
    uint8_t channel       = 0;
    bool foundTransmitter = false;
    int8_t rssi           = -127;
    uint8_t deviceCount   = 0; ///< From DiscoveryResponse
};

/// Callback types for platform-abstracted channel operations.
using SetChannelFn    = std::function<bool(uint8_t channel)>;
using SendDiscoveryFn = std::function<bool(uint8_t channel)>;
using DelayFn         = std::function<void(uint32_t ms)>;

/**
 * Shared channel scanner used by both transmitter and receiver.
 *
 * Platform-agnostic: uses callbacks for channel switching, packet
 * sending, and delays so it works on both ESP32 and the simulator.
 */
class ChannelScanner {
public:
    ChannelScanner(SetChannelFn setChannel, SendDiscoveryFn sendDiscovery, DelayFn delayMs);

    /// Scan a single channel: switch, send discovery request(s), wait for response.
    ScanResult scanChannel(uint8_t channel);

    /// Scan all candidate channels (1, 6, 11). Returns results for each.
    void scanAllChannels(ScanResult results[channel::kCandidateChannelCount]);

    /// Pick the best channel from scan results.
    /// Prefers channels with an active transmitter. Among those, picks best RSSI.
    /// If none have a transmitter, returns the channel that appears quietest.
    static uint8_t bestChannel(const ScanResult results[channel::kCandidateChannelCount]);

    /// Called by the radio layer when a DiscoveryResponse is received.
    /// Thread-safe: sets internal flag for the current scan wait.
    void onDiscoveryResponse(uint8_t channel, int8_t rssi, uint8_t deviceCount);

private:
    SetChannelFn _setChannel;
    SendDiscoveryFn _sendDiscovery;
    DelayFn _delay;

    // Volatile scan state (set by onDiscoveryResponse, read by scanChannel)
    volatile bool _responseReceived       = false;
    volatile int8_t _responseRssi         = -127;
    volatile uint8_t _responseDeviceCount = 0;
};

} // namespace odh
