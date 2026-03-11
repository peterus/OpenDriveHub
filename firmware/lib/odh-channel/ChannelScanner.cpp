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

#include "ChannelScanner.h"

#include <cstdlib>

namespace odh {

ChannelScanner::ChannelScanner(SetChannelFn setChannel, SendDiscoveryFn sendDiscovery, DelayFn delayMs)
    : _setChannel(std::move(setChannel)),
      _sendDiscovery(std::move(sendDiscovery)),
      _delay(std::move(delayMs)) {}

ScanResult ChannelScanner::scanChannel(uint8_t channel) {
    ScanResult result;
    result.channel = channel;

    // Switch to channel and wait for it to settle
    _setChannel(channel);
    _delay(channel::kChannelSettleMs);

    // Reset response flag
    _responseReceived    = false;
    _responseRssi        = -127;
    _responseDeviceCount = 0;

    // Send discovery requests with retries
    for (uint8_t attempt = 0; attempt < channel::kDiscoveryRetries; ++attempt) {
        _sendDiscovery(channel);
        _delay(channel::kDiscoveryWaitMs);

        if (_responseReceived) {
            result.foundTransmitter = true;
            result.rssi             = _responseRssi;
            result.deviceCount      = _responseDeviceCount;
            return result;
        }
    }

    return result;
}

void ChannelScanner::scanAllChannels(ScanResult results[channel::kCandidateChannelCount]) {
    for (uint8_t i = 0; i < channel::kCandidateChannelCount; ++i) {
        results[i] = scanChannel(channel::kCandidateChannels[i]);
    }
}

uint8_t ChannelScanner::bestChannel(const ScanResult results[channel::kCandidateChannelCount]) {
    // First: prefer channels with an active transmitter (best RSSI)
    int8_t bestRssi = -128;
    uint8_t bestCh  = channel::kCandidateChannels[0];
    bool foundAny   = false;

    for (uint8_t i = 0; i < channel::kCandidateChannelCount; ++i) {
        if (results[i].foundTransmitter && results[i].rssi > bestRssi) {
            bestRssi = results[i].rssi;
            bestCh   = results[i].channel;
            foundAny = true;
        }
    }

    if (foundAny) {
        return bestCh;
    }

    // No transmitter found: return first candidate (channel selection for
    // founding a new cluster – caller may do channel quality evaluation)
    return channel::kCandidateChannels[0];
}

void ChannelScanner::onDiscoveryResponse(uint8_t /*channel*/, int8_t rssi, uint8_t deviceCount) {
    _responseRssi        = rssi;
    _responseDeviceCount = deviceCount;
    _responseReceived    = true;
}

} // namespace odh
