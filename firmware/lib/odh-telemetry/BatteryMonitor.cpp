#include "BatteryMonitor.h"

#include <Arduino.h>

namespace odh {

BatteryMonitor::BatteryMonitor(uint8_t adcPin, float dividerRatio, uint16_t adcVrefMv, uint8_t adcBits)
    : _adcPin(adcPin),
      _dividerRatio(dividerRatio),
      _adcVrefMv(adcVrefMv),
      _adcBits(adcBits) {}

void BatteryMonitor::begin() {
    pinMode(_adcPin, INPUT);
}

void BatteryMonitor::sample() {
    const int raw       = analogRead(_adcPin);
    const uint32_t max  = (1u << _adcBits) - 1u;
    const uint32_t vPin = (static_cast<uint32_t>(raw) * _adcVrefMv) / max;
    _voltageMv          = static_cast<uint16_t>(static_cast<float>(vPin) * _dividerRatio);
}

void BatteryMonitor::autoDetectCells() {
    _cells = detectCells(_voltageMv);
}

uint8_t BatteryMonitor::detectCells(uint16_t totalMv) {
    if (totalMv == 0)
        return 0;
    if (totalMv < 5000)
        return 1;
    if (totalMv < 9000)
        return 2;
    if (totalMv < 13200)
        return 3;
    return 4;
}

} // namespace odh
