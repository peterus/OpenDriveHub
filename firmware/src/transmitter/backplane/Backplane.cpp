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
