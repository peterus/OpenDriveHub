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
