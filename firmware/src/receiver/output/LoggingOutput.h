/**
 * LoggingOutput – simulation-only output driver that logs values.
 */

#pragma once

#include "IOutputDriver.h"

#ifdef NATIVE_SIM

namespace odh {

class LoggingOutput : public IOutputDriver {
public:
    explicit LoggingOutput(uint8_t channel, uint16_t failsafeValue = kChannelMid)
        : IOutputDriver(failsafeValue),
          _channel(channel),
          _lastUs(failsafeValue) {}

    void begin() override {}

    void setChannel(uint16_t us) override {
        _lastUs = clamp(us);
    }

    uint16_t lastUs() const {
        return _lastUs;
    }
    uint8_t channelIndex() const {
        return _channel;
    }

private:
    uint8_t _channel;
    uint16_t _lastUs;
};

} // namespace odh

#endif // NATIVE_SIM
