/**
 * EncoderModule – magnetic rotary encoder.
 *
 * Hardware: AS5600 at I²C address 0x36.
 * Provides one 12-bit absolute angle (0-4095).
 * Linearly mapped to kChannelMin–kChannelMax.
 */

#pragma once

#include "IModule.h"

namespace odh {

inline constexpr uint8_t kEncoderI2cAddr   = 0x36;
inline constexpr uint16_t kEncoderAngleMax = 4095; // 12-bit

class EncoderModule : public IModule {
public:
    explicit EncoderModule(uint8_t slot);

    bool begin() override;
    void update() override;
    uint8_t inputCount() const override {
        return 1;
    }
    uint16_t inputValue(uint8_t index) const override;

    /** Raw 12-bit angle reading. */
    uint16_t rawAngle() const {
        return _rawAngle;
    }

private:
    uint16_t _rawAngle = 0;
    uint16_t _value    = kChannelMid;

    /** Read the 12-bit angle register (0x0C-0x0D) from the AS5600. */
    uint16_t readRawAngle();

    static uint16_t mapToChannel(uint16_t raw);
};

} // namespace odh
