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
