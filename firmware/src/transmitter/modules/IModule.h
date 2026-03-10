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
 * IModule – abstract interface for all plug-in input modules.
 *
 * Every module type inherits from this interface and implements:
 *   - begin()      : initialise the underlying I²C device(s)
 *   - update()     : read new values from hardware
 *   - isReady()    : true if the module initialised successfully
 *   - inputCount() : how many inputs this module provides
 *   - inputValue() : read a single input value by index
 */

#pragma once

#include "Protocol.h"

#include <cstdint>

namespace odh {

/// Module type identifiers.
enum class ModuleType : uint8_t {
    Unknown       = 0x00,
    Switch        = 0x01, ///< Toggle switch panel (PCF8574 at 0x20)
    Button        = 0x02, ///< Momentary button panel (PCF8574 at 0x21)
    Potentiometer = 0x03, ///< Potentiometer / fader bank (ADS1115 at 0x48)
    Encoder       = 0x04, ///< Rotary encoder (AS5600 at 0x36)
};

class IModule {
public:
    explicit IModule(uint8_t slot, ModuleType type)
        : _slot(slot),
          _type(type) {}

    virtual ~IModule() = default;

    /// Initialise the module hardware (mux channel must be pre-selected).
    virtual bool begin() = 0;

    /// Read the latest values (mux channel must be pre-selected).
    virtual void update() = 0;

    /// True if begin() completed successfully.
    bool isReady() const {
        return _ready;
    }

    /// Slot index this module occupies (0-based).
    uint8_t slot() const {
        return _slot;
    }

    /// Module type identifier.
    ModuleType type() const {
        return _type;
    }

    /// Number of inputs this module provides.
    virtual uint8_t inputCount() const = 0;

    /// Read a single input value by index.
    virtual uint16_t inputValue(uint8_t index) const = 0;

protected:
    uint8_t _slot;
    ModuleType _type;
    bool _ready = false;
};

} // namespace odh
