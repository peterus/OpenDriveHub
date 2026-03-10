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
 * Backplane – implementation.
 */

#include "Backplane.h"

#include <Wire.h>

namespace odh {

Backplane::Backplane(uint8_t muxAddress, uint8_t slotCount)
    : _muxAddress{muxAddress},
      _slotCount{static_cast<uint8_t>(slotCount > 8 ? 8 : slotCount)} {}

bool Backplane::begin() {
    _ready = writeMux(0x00);
    return _ready;
}

bool Backplane::selectSlot(uint8_t slot) {
    if (!_ready || slot >= _slotCount)
        return false;
    if (writeMux(static_cast<uint8_t>(1u << slot))) {
        _activeSlot = slot;
        return true;
    }
    return false;
}

void Backplane::deselectAll() {
    writeMux(0x00);
    _activeSlot = kNoSlot;
}

uint8_t Backplane::scanSlots(uint8_t probeAddress) {
    uint8_t present = 0;
    for (uint8_t s = 0; s < _slotCount; ++s) {
        if (!selectSlot(s))
            continue;
        Wire.beginTransmission(probeAddress);
        if (Wire.endTransmission() == 0) {
            present |= static_cast<uint8_t>(1u << s);
        }
    }
    deselectAll();
    return present;
}

bool Backplane::writeMux(uint8_t mask) {
    Wire.beginTransmission(_muxAddress);
    Wire.write(mask);
    return Wire.endTransmission() == 0;
}

} // namespace odh
