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
 * EncoderModule – implementation.
 */

#include "EncoderModule.h"

#include <Wire.h>

namespace odh {

static constexpr uint8_t kAS5600_REG_ANGLE = 0x0C;

EncoderModule::EncoderModule(uint8_t slot)
    : IModule(slot, ModuleType::Encoder) {}

bool EncoderModule::begin() {
    // Probe the AS5600 by trying to read the angle register.
    Wire.beginTransmission(kEncoderI2cAddr);
    Wire.write(kAS5600_REG_ANGLE);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    return true;
}

void EncoderModule::update() {
    _rawAngle = readRawAngle();
    _value    = mapToChannel(_rawAngle);
}

uint16_t EncoderModule::inputValue(uint8_t index) const {
    if (index != 0)
        return kChannelMid;
    return _value;
}

uint16_t EncoderModule::readRawAngle() {
    Wire.beginTransmission(kEncoderI2cAddr);
    Wire.write(kAS5600_REG_ANGLE);
    Wire.endTransmission(false);

    Wire.requestFrom(kEncoderI2cAddr, static_cast<uint8_t>(2));
    if (Wire.available() < 2)
        return 0;

    uint16_t hi = Wire.read();
    uint16_t lo = Wire.read();
    return ((hi & 0x0F) << 8) | lo; // 12-bit value
}

uint16_t EncoderModule::mapToChannel(uint16_t raw) {
    if (raw > kEncoderAngleMax)
        raw = kEncoderAngleMax;
    return static_cast<uint16_t>(kChannelMin + static_cast<uint32_t>(raw) * (kChannelMax - kChannelMin) / kEncoderAngleMax);
}

} // namespace odh
