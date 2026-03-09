/**
 * ButtonModule – 8-way momentary push-button bank.
 *
 * Hardware: PCF8574 GPIO-expander at 0x21.
 * Buttons are active-low (pulled HIGH when released).
 * Pressed → kChannelMax, released → kChannelMin.
 */

#pragma once

#include <Adafruit_PCF8574.h>

#include "IModule.h"

namespace odh {

inline constexpr uint8_t kButtonI2cAddr = 0x21;
inline constexpr uint8_t kButtonCount   = 8;

class ButtonModule : public IModule {
public:
    explicit ButtonModule(uint8_t slot);

    bool begin() override;
    void update() override;
    uint8_t inputCount() const override {
        return kButtonCount;
    }
    uint16_t inputValue(uint8_t index) const override;

    /** True while physical button @p index is held down. */
    bool isPressed(uint8_t index) const;

private:
    Adafruit_PCF8574 _pcf;
    uint8_t _state = 0xFF; // active-low bit-field
};

} // namespace odh
