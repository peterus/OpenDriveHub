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
 * ButtonModule – 8-way momentary push-button bank.
 *
 * Hardware: PCF8574 GPIO-expander at 0x21.
 * Buttons are active-low (pulled HIGH when released).
 * Pressed → kChannelMax, released → kChannelMin.
 */

#pragma once

#include <Adafruit_PCF8574.h>

#include "IModule.h"

namespace odh {

inline constexpr uint8_t kButtonI2cAddr = 0x21;
inline constexpr uint8_t kButtonCount   = 8;

class ButtonModule : public IModule {
public:
    explicit ButtonModule(uint8_t slot);

    bool begin() override;
    void update() override;
    uint8_t inputCount() const override {
        return kButtonCount;
    }
    uint16_t inputValue(uint8_t index) const override;

    /** True while physical button @p index is held down. */
    bool isPressed(uint8_t index) const;

private:
    Adafruit_PCF8574 _pcf;
    uint8_t _state = 0xFF; // active-low bit-field
};

} // namespace odh
