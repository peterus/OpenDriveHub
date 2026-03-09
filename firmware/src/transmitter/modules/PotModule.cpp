/**
 * PotModule – implementation.
 */

#include "PotModule.h"

namespace odh {

PotModule::PotModule(uint8_t slot)
    : IModule(slot, ModuleType::Potentiometer) {}

bool PotModule::begin() {
    if (!_adc.begin(kPotI2cAddr)) {
        return false;
    }
    _adc.setGain(GAIN_ONE); // ±4.096 V
    _adc.setDataRate(RATE_ADS1115_860SPS);
    return true;
}

void PotModule::update() {
    for (uint8_t ch = 0; ch < kPotCount; ++ch) {
        _raw[ch]    = _adc.readADC_SingleEnded(ch);
        _values[ch] = mapToChannel(_raw[ch]);
    }
}

uint16_t PotModule::inputValue(uint8_t index) const {
    if (index >= kPotCount)
        return kChannelMid;
    return _values[index];
}

int16_t PotModule::rawValue(uint8_t index) const {
    if (index >= kPotCount)
        return 0;
    return _raw[index];
}

uint16_t PotModule::mapToChannel(int16_t raw) {
    if (raw < 0)
        raw = 0;
    if (raw > kPotAdcMax)
        raw = kPotAdcMax;
    return static_cast<uint16_t>(kChannelMin + static_cast<uint32_t>(raw) * (kChannelMax - kChannelMin) / kPotAdcMax);
}

} // namespace odh
