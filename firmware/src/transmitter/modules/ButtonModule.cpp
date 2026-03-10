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
 * ButtonModule – implementation.
 */

#include "ButtonModule.h"

#include <Wire.h>

namespace odh {

ButtonModule::ButtonModule(uint8_t slot)
    : IModule(slot, ModuleType::Button) {}

bool ButtonModule::begin() {
    if (!_pcf.begin(kButtonI2cAddr, &Wire)) {
        return false;
    }
    for (uint8_t i = 0; i < kButtonCount; ++i) {
        _pcf.pinMode(i, INPUT);
    }
    return true;
}

void ButtonModule::update() {
    Wire.requestFrom(kButtonI2cAddr, static_cast<uint8_t>(1));
    if (Wire.available()) {
        _state = Wire.read();
    }
}

uint16_t ButtonModule::inputValue(uint8_t index) const {
    if (index >= kButtonCount)
        return kChannelMid;
    return isPressed(index) ? kChannelMax : kChannelMin;
}

bool ButtonModule::isPressed(uint8_t index) const {
    if (index >= kButtonCount)
        return false;
    // Active-low: bit 0 = pressed.
    return !(_state & (1u << index));
}

} // namespace odh
