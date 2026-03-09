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
