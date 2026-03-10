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
 * EncoderModule – magnetic rotary encoder.
 *
 * Hardware: AS5600 at I²C address 0x36.
 * Provides one 12-bit absolute angle (0-4095).
 * Linearly mapped to kChannelMin–kChannelMax.
 */

#pragma once

#include "IModule.h"

namespace odh {

inline constexpr uint8_t kEncoderI2cAddr   = 0x36;
inline constexpr uint16_t kEncoderAngleMax = 4095; // 12-bit

class EncoderModule : public IModule {
public:
    explicit EncoderModule(uint8_t slot);

    bool begin() override;
    void update() override;
    uint8_t inputCount() const override {
        return 1;
    }
    uint16_t inputValue(uint8_t index) const override;

    /** Raw 12-bit angle reading. */
    uint16_t rawAngle() const {
        return _rawAngle;
    }

private:
    uint16_t _rawAngle = 0;
    uint16_t _value    = kChannelMid;

    /** Read the 12-bit angle register (0x0C-0x0D) from the AS5600. */
    uint16_t readRawAngle();

    static uint16_t mapToChannel(uint16_t raw);
};

} // namespace odh
