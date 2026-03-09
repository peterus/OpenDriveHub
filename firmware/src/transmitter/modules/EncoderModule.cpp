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
