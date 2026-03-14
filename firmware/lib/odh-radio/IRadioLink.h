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

/// Callback when a DiscoveryResponse is received.
using DiscoveryResponseCallback = std::function<void(uint8_t channel, int8_t rssi, uint8_t deviceCount)>;

/// Callback when a ChannelMigration is received.
using ChannelMigrationCallback = std::function<void(uint8_t newChannel)>;

/// Callback when a DiscoveryRequest is received (transmitter answers these).
using DiscoveryRequestCallback = std::function<void(const uint8_t *mac, DeviceRole role)>;

} // namespace odh
