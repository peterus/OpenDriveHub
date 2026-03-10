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
 * IOutputDriver – abstract interface for model outputs.
 *
 * Each IOutputDriver subclass maps a single logical channel to a physical
 * output (PCA9685 I²C PWM, Logging stub, etc.).
 */

#pragma once

#include "Protocol.h"

#include <cstdint>

namespace odh {

class IOutputDriver {
public:
    explicit IOutputDriver(uint16_t failsafeValue = kChannelMid)
        : _failsafeValue(failsafeValue) {}

    virtual ~IOutputDriver() = default;

    /// Initialise the output hardware.
    virtual void begin() = 0;

    /// Apply a channel value (RC pulse width in µs [1000, 2000]).
    virtual void setChannel(uint16_t us) = 0;

    /// Apply the failsafe position immediately.
    void applyFailsafe() {
        setChannel(_failsafeValue);
    }

    /// Get current failsafe value.
    uint16_t failsafeValue() const {
        return _failsafeValue;
    }

    /// Update failsafe value at runtime (e.g. from NVS).
    void setFailsafeValue(uint16_t us) {
        _failsafeValue = us;
    }

protected:
    uint16_t _failsafeValue;

    /// Clamp a value to the valid channel range.
    static uint16_t clamp(uint16_t us) {
        if (us < kChannelMin)
            return kChannelMin;
        if (us > kChannelMax)
            return kChannelMax;
        return us;
    }
};

} // namespace odh
