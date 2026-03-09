/**
 * Pca9685Output – servo/ESC output via PCA9685 16-channel I²C PWM driver.
 *
 * A single shared PCA9685 instance is initialised once via initChip().
 * Each Pca9685Output instance maps one logical channel to one PCA9685
 * output pin (0-15).
 *
 * The PCA9685 uses a 25 MHz internal oscillator. At 50 Hz the prescaler
 * is set to 121, giving 4096 counts per 20 ms period (~4.88 µs per count).
 */

#pragma once

#include "IOutputDriver.h"

#ifndef NATIVE_SIM
#include <Wire.h>

namespace odh {

class Pca9685Output : public IOutputDriver {
public:
    /**
     * @param pca9685Channel  PCA9685 output channel (0-15).
     * @param failsafeValue   Failsafe pulse width in µs.
     */
    explicit Pca9685Output(uint8_t pca9685Channel, uint16_t failsafeValue = kChannelMid)
        : IOutputDriver(failsafeValue),
          _pcaCh(pca9685Channel) {}

    /**
     * Initialise the PCA9685 chip (call once before any begin()).
     * Sets prescaler for 50 Hz and enables auto-increment.
     */
    static void initChip(uint8_t addr = 0x40, TwoWire &wire = Wire);

    void begin() override;
    void setChannel(uint16_t us) override;

    uint8_t pcaChannel() const {
        return _pcaCh;
    }

private:
    uint8_t _pcaCh;

    static uint8_t sAddr;
    static TwoWire *sWire;
    static bool sChipReady;

    static void writeReg(uint8_t reg, uint8_t val);
    static void setPwm(uint8_t ch, uint16_t on, uint16_t off);
};

} // namespace odh

#endif // !NATIVE_SIM
