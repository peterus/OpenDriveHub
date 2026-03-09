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
