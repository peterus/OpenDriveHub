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

#include <cstdint>

namespace odh {
namespace channel {

/// The three non-overlapping 2.4 GHz WiFi channels used for discovery.
inline constexpr uint8_t kCandidateChannels[]   = {1, 6, 11};
inline constexpr uint8_t kCandidateChannelCount = 3;

/// Returns true only for channels 1, 6, or 11.
inline constexpr bool isValidChannel(uint8_t ch) {
    return ch == 1 || ch == 6 || ch == 11;
}

/// Fallback channel when NVS is empty or contains an invalid value.
inline constexpr uint8_t kDefaultChannel = 1;

// -- Discovery timing constants --

/// Wait after switching WiFi channel before sending (ms).
inline constexpr uint32_t kChannelSettleMs = 10;

/// Wait for a discovery response after sending a request (ms).
inline constexpr uint32_t kDiscoveryWaitMs = 25;

/// Number of discovery requests per channel scan attempt.
inline constexpr uint8_t kDiscoveryRetries = 2;

/// Approximate wall-clock time for one full scan of all 3 channels (ms).
inline constexpr uint32_t kScanRoundMs = 180;

/// Minimum random backoff before a receiver claims a channel (ms).
inline constexpr uint32_t kFoundingBackoffMinMs = 200;

/// Maximum random backoff before a receiver claims a channel (ms).
inline constexpr uint32_t kFoundingBackoffMaxMs = 700;

/// Interval at which a receiver broadcasts its presence (ms).
inline constexpr uint32_t kPresenceIntervalMs = 500;

/// Receiver re-enters discovery if it hears no transmitter for this long (ms).
inline constexpr uint32_t kTransmitterLossTimeoutMs = 3000;

/// Transmitter prunes receivers not seen for this long (ms).
inline constexpr uint32_t kPresenceStaleTimeoutMs = 5000;

/// Maps a WiFi channel number to a UDP simulation port.
/// ch 1 → 7001, ch 6 → 7006, ch 11 → 7011, anything else → 7001.
inline constexpr uint16_t channelToSimPort(uint8_t ch) {
    if (ch == 1)
        return 7001;
    if (ch == 6)
        return 7006;
    if (ch == 11)
        return 7011;
    return 7001;
}

} // namespace channel
} // namespace odh
