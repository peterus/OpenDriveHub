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
 * IRadioLink – abstract interface for ESP-NOW radio links.
 *
 * Both ReceiverRadioLink and TransmitterRadioLink implement this interface.
 * The interface allows mocking in tests and enables the sim layer to
 * provide alternative implementations.
 */

#pragma once

#include "Protocol.h"

#include <cstdint>
#include <functional>

namespace odh {

/// Callback type for received control packets (receiver side).
using ControlCallback = std::function<void(const ControlPacket &pkt)>;

/// Callback type for received telemetry packets (transmitter side).
using TelemetryCallback = std::function<void(const TelemetryPacket &pkt)>;

/// A vehicle discovered via announce broadcasts (transmitter side).
struct DiscoveredVehicle {
    uint8_t mac[6]{};
    uint8_t modelType = 0;
    char name[kVehicleNameMax]{};
    uint32_t lastSeenMs = 0; ///< millis() timestamp of last announce
    int8_t rssi         = 0; ///< RSSI of last announce packet (dBm)
    bool valid          = false;
};

} // namespace odh
