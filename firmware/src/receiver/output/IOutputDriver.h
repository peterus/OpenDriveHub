/**
 * IOutputDriver – abstract interface for model outputs.
 *
 * Each IOutputDriver subclass maps a single logical channel to a physical
 * output (PCA9685 I²C PWM, Logging stub, etc.).
 */

#pragma once

#include "Protocol.h"

#include <cstdint>

namespace odh {

class IOutputDriver {
public:
    explicit IOutputDriver(uint16_t failsafeValue = kChannelMid)
        : _failsafeValue(failsafeValue) {}

    virtual ~IOutputDriver() = default;

    /// Initialise the output hardware.
    virtual void begin() = 0;

    /// Apply a channel value (RC pulse width in µs [1000, 2000]).
    virtual void setChannel(uint16_t us) = 0;

    /// Apply the failsafe position immediately.
    void applyFailsafe() {
        setChannel(_failsafeValue);
    }

    /// Get current failsafe value.
    uint16_t failsafeValue() const {
        return _failsafeValue;
    }

    /// Update failsafe value at runtime (e.g. from NVS).
    void setFailsafeValue(uint16_t us) {
        _failsafeValue = us;
    }

protected:
    uint16_t _failsafeValue;

    /// Clamp a value to the valid channel range.
    static uint16_t clamp(uint16_t us) {
        if (us < kChannelMin)
            return kChannelMin;
        if (us > kChannelMax)
            return kChannelMax;
        return us;
    }
};

} // namespace odh
