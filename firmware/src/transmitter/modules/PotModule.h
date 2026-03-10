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
 * PotModule – potentiometer / fader bank module.
 *
 * Hardware: ADS1115 16-bit ADC at 0x48.
 * Provides 4 single-ended analogue inputs.
 */

#pragma once

#include <Adafruit_ADS1X15.h>

#include "IModule.h"

namespace odh {

inline constexpr uint8_t kPotI2cAddr = 0x48;
inline constexpr uint8_t kPotCount   = 4;
inline constexpr int32_t kPotAdcMax  = 32767;

class PotModule : public IModule {
public:
    explicit PotModule(uint8_t slot);

    bool begin() override;
    void update() override;
    uint8_t inputCount() const override {
        return kPotCount;
    }
    uint16_t inputValue(uint8_t index) const override;

    int16_t rawValue(uint8_t index) const;

private:
    Adafruit_ADS1115 _adc;
    int16_t _raw[kPotCount]     = {};
    uint16_t _values[kPotCount] = {};

    static uint16_t mapToChannel(int32_t raw);
};

} // namespace odh
