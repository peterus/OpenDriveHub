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
 * OutputManager – owns and manages all output drivers for the receiver.
 *
 * Provides routing from function values to physical output channels
 * via the FunctionMap, failsafe application, and NVS persistence.
 */

#pragma once

#include "Config.h"
#include "FunctionMap.h"
#include "NvsStore.h"
#include "Protocol.h"
#include "output/IOutputDriver.h"

#include <array>
#include <cstdint>
#include <memory>

namespace odh {

class OutputManager {
public:
    OutputManager();
    ~OutputManager() = default;

    /// Initialise hardware (PCA9685 chip, then begin() on all drivers).
    void begin();

    /**
     * Set an output driver for a channel.
     * Takes ownership of the pointer.
     */
    void setDriver(uint8_t channel, std::unique_ptr<IOutputDriver> driver);

    /// Apply a control packet – routes functions to channels via the function map.
    void applyControl(const ControlPacket &pkt);

    /// Apply failsafe values to all channels.
    void applyFailsafe();

    /// Write an individual channel (direct access, bypassing function map).
    void setChannel(uint8_t channel, uint16_t us);

    // ── Function map management ─────────────────────────────────────

    /// Load function map and failsafe values from NVS.
    void loadFromNvs();

    /// Save function map and failsafe values to NVS.
    void saveToNvs();

    /// Set model type and reload default function map.
    void setModelType(ModelType model);

    /// Get current model type.
    ModelType modelType() const {
        return _modelType;
    }

    /// Get the current function mapping.
    const FunctionMapping &functionMap() const {
        return _functionMap;
    }

    /// Set the function mapping directly.
    void setFunctionMap(const FunctionMapping &map) {
        _functionMap = map;
    }

    // ── Failsafe ────────────────────────────────────────────────────

    /// Get failsafe value for a channel.
    uint16_t failsafeValue(uint8_t channel) const;

    /// Set failsafe value for a channel.
    void setFailsafeValue(uint8_t channel, uint16_t us);

    /// Get the channel values array (for telemetry/web).
    const std::array<uint16_t, config::rx::kChannelCount> &channelValues() const {
        return _channelValues;
    }

private:
    std::array<std::unique_ptr<IOutputDriver>, config::rx::kChannelCount> _drivers;
    std::array<uint16_t, config::rx::kChannelCount> _channelValues;
    std::array<uint16_t, config::rx::kChannelCount> _failsafeValues;
    FunctionMapping _functionMap;
    ModelType _modelType = ModelType::Generic;
};

} // namespace odh
