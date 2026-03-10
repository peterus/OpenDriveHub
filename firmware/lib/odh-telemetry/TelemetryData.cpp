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

#include "TelemetryData.h"

#include <Arduino.h>

namespace odh {

void TelemetryData::onPacketReceived(const TelemetryPacket &pkt) {
    _rxBatteryMv = pkt.battery_mv;
    _rssi        = pkt.rssi;
    _linkState   = pkt.link_state;
    _rxModelType = pkt.model_type;
    _modelFlags  = pkt.model_flags;
    _sensorCount = (pkt.sensor_count < kMaxSensors) ? pkt.sensor_count : kMaxSensors;

    for (uint8_t i = 0; i < _sensorCount; i++) {
        _sensors[i] = pkt.sensors[i];
    }

    _lastPacketMs = millis();
    _hasData      = true;
    _packetCount++;
}

void TelemetryData::reset() {
    _hasData           = false;
    _lastPacketMs      = 0;
    _packetCount       = 0;
    _connectionStartMs = millis();
    _rxBatteryMv       = 0;
    _rssi              = 0;
    _linkState         = static_cast<uint8_t>(LinkState::Disconnected);
}

uint16_t TelemetryData::sensor(uint8_t index) const {
    if (index >= kMaxSensors)
        return 0;
    return _sensors[index];
}

uint32_t TelemetryData::msSinceLastPacket() const {
    if (!_hasData)
        return UINT32_MAX;
    return millis() - _lastPacketMs;
}

uint32_t TelemetryData::connectionUptimeMs() const {
    return millis() - _connectionStartMs;
}

float TelemetryData::packetsPerSecond() const {
    const uint32_t uptime = connectionUptimeMs();
    if (uptime < 500 || _packetCount == 0)
        return 0.0f;
    return static_cast<float>(_packetCount) * 1000.0f / static_cast<float>(uptime);
}

void TelemetryData::autoDetectRxCells() {
    _rxCells = detectCells(_rxBatteryMv);
}

uint8_t TelemetryData::detectCells(uint16_t totalMv) {
    if (totalMv == 0)
        return 0;
    if (totalMv < 5000)
        return 1;
    if (totalMv < 9000)
        return 2;
    if (totalMv < 13200)
        return 3;
    return 4;
}

} // namespace odh
