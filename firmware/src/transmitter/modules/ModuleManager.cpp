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
 * ModuleManager – implementation.
 */

#include "ModuleManager.h"

#include <Wire.h>

#include "ButtonModule.h"
#include "EncoderModule.h"
#include "PotModule.h"
#include "SwitchModule.h"

namespace odh {

ModuleManager::ModuleManager(Backplane &backplane)
    : _backplane{backplane} {}

/* ── Public ─────────────────────────────────────────────────────────────── */

void ModuleManager::scanAndInit() {
    // Reset all slots.
    for (auto &m : _modules)
        m.reset();

    for (uint8_t slot = 0; slot < _backplane.slotCount(); ++slot) {
        if (!_backplane.selectSlot(slot))
            continue;

        auto mod = detectModule(slot);
        if (mod && mod->begin()) {
            _modules[slot] = std::move(mod);
        }
        _backplane.deselectAll();
    }
}

void ModuleManager::update() {
    for (uint8_t slot = 0; slot < _backplane.slotCount(); ++slot) {
        auto &m = _modules[slot];
        if (!m || !m->isReady())
            continue;
        if (!_backplane.selectSlot(slot))
            continue;

        m->update();
        _backplane.deselectAll();
    }
}

uint16_t ModuleManager::readInput(uint8_t slot, uint8_t inputIndex) const {
    if (slot >= _modules.size() || !_modules[slot] || !_modules[slot]->isReady()) {
        return kChannelMid;
    }
    return _modules[slot]->inputValue(inputIndex);
}

uint8_t ModuleManager::inputCount(uint8_t slot) const {
    if (slot >= _modules.size() || !_modules[slot] || !_modules[slot]->isReady()) {
        return 0;
    }
    return _modules[slot]->inputCount();
}

IModule *ModuleManager::moduleAt(uint8_t slot) const {
    if (slot >= _modules.size())
        return nullptr;
    return _modules[slot].get();
}

ModuleType ModuleManager::typeAt(uint8_t slot) const {
    if (slot >= _modules.size() || !_modules[slot])
        return ModuleType::Unknown;
    return _modules[slot]->type();
}

/* ── Private ────────────────────────────────────────────────────────────── */

std::unique_ptr<IModule> ModuleManager::detectModule(uint8_t slot) {
    // Probe in priority order.

    // Potentiometer: ADS1115 at 0x48
    Wire.beginTransmission(kPotI2cAddr);
    if (Wire.endTransmission() == 0) {
        return std::make_unique<PotModule>(slot);
    }

    // Encoder: AS5600 at 0x36
    Wire.beginTransmission(kEncoderI2cAddr);
    if (Wire.endTransmission() == 0) {
        return std::make_unique<EncoderModule>(slot);
    }

    // Switch: PCF8574 at 0x20
    Wire.beginTransmission(kSwitchI2cAddr);
    if (Wire.endTransmission() == 0) {
        return std::make_unique<SwitchModule>(slot);
    }

    // Button: PCF8574 at 0x21
    Wire.beginTransmission(kButtonI2cAddr);
    if (Wire.endTransmission() == 0) {
        return std::make_unique<ButtonModule>(slot);
    }

    return nullptr;
}

} // namespace odh
