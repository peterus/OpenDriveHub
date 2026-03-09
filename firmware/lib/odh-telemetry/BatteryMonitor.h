/**
 * BatteryMonitor – unified ADC battery voltage measurement.
 *
 * Used by both receiver and transmitter for local battery monitoring.
 * Includes cell count auto-detection for LiPo batteries.
 */

#pragma once

#include <cstdint>

namespace odh {

class BatteryMonitor {
public:
    /**
     * @param adcPin        GPIO pin connected to the battery voltage divider.
     * @param dividerRatio  (R1+R2)/R2 of the resistor divider.
     * @param adcVrefMv     ADC reference voltage in mV.
     * @param adcBits       ADC resolution in bits.
     */
    BatteryMonitor(uint8_t adcPin, float dividerRatio, uint16_t adcVrefMv, uint8_t adcBits);

    /// Initialise the ADC pin mode.
    void begin();

    /// Sample the battery voltage and update the internal reading.
    void sample();

    /// Battery voltage in millivolts.
    uint16_t voltageMv() const {
        return _voltageMv;
    }

    /// Set battery cell count manually (1-4), or 0 for auto-detect.
    void setCells(uint8_t cells) {
        _cells = cells;
    }

    /// Auto-detect cell count from current voltage reading.
    void autoDetectCells();

    /// Cell count (0 = not yet detected).
    uint8_t cells() const {
        return _cells;
    }

    /// Voltage per cell in millivolts (0 if cells unknown).
    uint16_t cellVoltageMv() const {
        return (_cells > 0) ? (_voltageMv / _cells) : 0;
    }

private:
    uint8_t _adcPin;
    float _dividerRatio;
    uint16_t _adcVrefMv;
    uint8_t _adcBits;
    uint16_t _voltageMv = 0;
    uint8_t _cells      = 0;

    /// Detect cell count from total voltage.
    static uint8_t detectCells(uint16_t totalMv);
};

} // namespace odh
