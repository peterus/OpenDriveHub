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

#include "BatteryMonitor.h"

#include <Arduino.h>

namespace odh {

BatteryMonitor::BatteryMonitor(uint8_t adcPin, float dividerRatio, uint16_t adcVrefMv, uint8_t adcBits)
    : _adcPin(adcPin),
      _dividerRatio(dividerRatio),
      _adcVrefMv(adcVrefMv),
      _adcBits(adcBits) {}

void BatteryMonitor::begin() {
    pinMode(_adcPin, INPUT);
}

void BatteryMonitor::sample() {
    const int raw       = analogRead(_adcPin);
    const uint32_t max  = (1u << _adcBits) - 1u;
    const uint32_t vPin = (static_cast<uint32_t>(raw) * _adcVrefMv) / max;
    _voltageMv          = static_cast<uint16_t>(static_cast<float>(vPin) * _dividerRatio);
}

void BatteryMonitor::autoDetectCells() {
    _cells = detectCells(_voltageMv);
}

uint8_t BatteryMonitor::detectCells(uint16_t totalMv) {
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
