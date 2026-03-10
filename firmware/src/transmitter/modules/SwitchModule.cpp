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
 * SwitchModule – implementation.
 */

#include "SwitchModule.h"

#include <Wire.h>

namespace odh {

SwitchModule::SwitchModule(uint8_t slot)
    : IModule(slot, ModuleType::Switch) {}

bool SwitchModule::begin() {
    if (!_pcf.begin(kSwitchI2cAddr, &Wire)) {
        return false;
    }
    // All pins as inputs (PCF8574 defaults to HIGH / input).
    for (uint8_t i = 0; i < kSwitchCount; ++i) {
        _pcf.pinMode(i, INPUT);
    }
    return true;
}

void SwitchModule::update() {
    // Read all 8 pins at once via I²C.
    Wire.requestFrom(kSwitchI2cAddr, static_cast<uint8_t>(1));
    if (Wire.available()) {
        _state = Wire.read();
    }
}

uint16_t SwitchModule::inputValue(uint8_t index) const {
    if (index >= kSwitchCount)
        return kChannelMid;
    return isOn(index) ? kChannelMin : kChannelMax;
}

bool SwitchModule::isOn(uint8_t index) const {
    if (index >= kSwitchCount)
        return false;
    // Active-low: bit 0 means switch is ON.
    return !(_state & (1u << index));
}

} // namespace odh
