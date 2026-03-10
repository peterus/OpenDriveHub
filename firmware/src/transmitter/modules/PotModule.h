/**
 * PotModule – potentiometer / fader bank module.
 *
 * Hardware: ADS1115 16-bit ADC at 0x48.
 * Provides 4 single-ended analogue inputs.
 */

#pragma once

#include <Adafruit_ADS1X15.h>

#include "IModule.h"

namespace odh {

inline constexpr uint8_t kPotI2cAddr = 0x48;
inline constexpr uint8_t kPotCount   = 4;
inline constexpr int32_t kPotAdcMax  = 32767;

class PotModule : public IModule {
public:
    explicit PotModule(uint8_t slot);

    bool begin() override;
    void update() override;
    uint8_t inputCount() const override {
        return kPotCount;
    }
    uint16_t inputValue(uint8_t index) const override;

    int16_t rawValue(uint8_t index) const;

private:
    Adafruit_ADS1115 _adc;
    int16_t _raw[kPotCount]     = {};
    uint16_t _values[kPotCount] = {};

    static uint16_t mapToChannel(int32_t raw);
};

} // namespace odh
