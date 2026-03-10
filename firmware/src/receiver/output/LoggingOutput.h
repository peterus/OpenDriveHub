/*
 * Copyright (C) 2026 Peter Buchegger
 *
 * This file is part of OpenDriveHub.
 *
 * OpenDriveHub is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenDriveHub is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenDriveHub. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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
