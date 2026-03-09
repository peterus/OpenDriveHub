/**
 * SwitchModule – 8-way switch bank.
 *
 * Hardware: PCF8574 GPIO-expander at 0x20.
 * Inputs are active-low (pulled HIGH internally).
 * Each switch maps to kChannelMin (on) or kChannelMax (off).
 */

#pragma once

#include <Adafruit_PCF8574.h>

#include "IModule.h"

namespace odh {

inline constexpr uint8_t kSwitchI2cAddr = 0x20;
inline constexpr uint8_t kSwitchCount   = 8;

class SwitchModule : public IModule {
public:
    explicit SwitchModule(uint8_t slot);

    bool begin() override;
    void update() override;
    uint8_t inputCount() const override {
        return kSwitchCount;
    }
    uint16_t inputValue(uint8_t index) const override;

    /** True when the physical switch at @p index is in the ON position. */
    bool isOn(uint8_t index) const;

private:
    Adafruit_PCF8574 _pcf;
    uint8_t _state = 0xFF; // bit-field, high = off (active-low)
};

} // namespace odh
