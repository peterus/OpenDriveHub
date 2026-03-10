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
 * display_utils.h – pure, hardware-independent display helper functions.
 *
 * No LVGL or TFT_eSPI dependency – can be compiled and unit-tested on host.
 */

#pragma once

#include <Protocol.h>
#include <cstdint>

namespace odh {

/** Map a channel value (µs) to a fill amount in permille (0–1000). */
constexpr int32_t barPermille(uint16_t valueMicros) {
    uint16_t v = valueMicros;
    if (v < kChannelMin)
        v = kChannelMin;
    if (v > kChannelMax)
        v = kChannelMax;
    return static_cast<int32_t>(static_cast<uint32_t>(v - kChannelMin) * 1000u / static_cast<uint32_t>(kChannelMax - kChannelMin));
}

/** Map a permille value to a pixel fill width. */
constexpr int16_t permilleToPx(int32_t permille, int16_t barWidth) {
    int32_t p = permille;
    if (p < 0)
        p = 0;
    if (p > 1000)
        p = 1000;
    return static_cast<int16_t>((p * static_cast<int32_t>(barWidth)) / 1000);
}

/** LiPo cell percentage from per-cell voltage (mV). 4200=100%, 3000=0%. */
constexpr uint8_t battCellPercent(uint16_t cellMv) {
    if (cellMv == 0)
        return 0;
    if (cellMv >= 4200)
        return 100;
    if (cellMv <= 3000)
        return 0;
    return static_cast<uint8_t>((static_cast<uint32_t>(cellMv - 3000) * 100u) / 1200u);
}

/** Battery voltage to percentage via linear empty/full range. */
constexpr int32_t battPercent(uint16_t battMv, uint16_t emptyMv, uint16_t fullMv) {
    if (battMv <= emptyMv)
        return 0;
    if (battMv >= fullMv)
        return 100;
    return static_cast<int32_t>(static_cast<uint32_t>(battMv - emptyMv) * 100u / static_cast<uint32_t>(fullMv - emptyMv));
}

/** Return short text for a link state. */
constexpr const char *linkStateText(LinkState state) {
    switch (state) {
    case LinkState::Connected:
        return "CONNECTED";
    case LinkState::Binding:
        return "BINDING";
    case LinkState::Failsafe:
        return "FAILSAFE";
    case LinkState::Scanning:
        return "SCANNING";
    default:
        return "---";
    }
}

} // namespace odh
