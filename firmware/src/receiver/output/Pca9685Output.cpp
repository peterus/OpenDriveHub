#include "Pca9685Output.h"

#ifndef NATIVE_SIM

#include <Arduino.h>

namespace odh {

// PCA9685 register addresses
static constexpr uint8_t kMode1    = 0x00;
static constexpr uint8_t kMode2    = 0x01;
static constexpr uint8_t kLed0OnL  = 0x06;
static constexpr uint8_t kPrescale = 0xFE;

// MODE1 bits
static constexpr uint8_t kRestart = 0x80;
static constexpr uint8_t kAI      = 0x20; // auto-increment
static constexpr uint8_t kSleep   = 0x10;
static constexpr uint8_t kAllCall = 0x01;

// 4096 counts per cycle; at 50 Hz period = 20000 µs
static constexpr uint16_t kCounts = 4096;
static constexpr float kUsPerCnt  = 20000.0f / kCounts;

// Static members
uint8_t Pca9685Output::sAddr   = 0x40;
TwoWire *Pca9685Output::sWire  = nullptr;
bool Pca9685Output::sChipReady = false;

void Pca9685Output::initChip(uint8_t addr, TwoWire &wire) {
    sAddr = addr;
    sWire = &wire;

    // Sleep to change prescaler.
    writeReg(kMode1, kSleep | kAI | kAllCall);
    delay(1);

    // Prescaler for 50 Hz: round(25 MHz / (4096 × 50)) − 1 = 121
    writeReg(kPrescale, 121);
    delay(1);

    // Wake up.
    writeReg(kMode1, kAI | kAllCall);
    delay(1);

    // Restart.
    writeReg(kMode1, kRestart | kAI | kAllCall);
    delay(1);

    // Totem-pole outputs.
    writeReg(kMode2, 0x04);

    sChipReady = true;
}

void Pca9685Output::begin() {
    setChannel(_failsafeValue);
}

void Pca9685Output::setChannel(uint16_t us) {
    if (!sChipReady)
        return;
    us = clamp(us);

    auto off = static_cast<uint16_t>(static_cast<float>(us) / kUsPerCnt);
    if (off >= kCounts)
        off = kCounts - 1;
    setPwm(_pcaCh, 0, off);
}

void Pca9685Output::writeReg(uint8_t reg, uint8_t val) {
    sWire->beginTransmission(sAddr);
    sWire->write(reg);
    sWire->write(val);
    sWire->endTransmission();
}

void Pca9685Output::setPwm(uint8_t ch, uint16_t on, uint16_t off) {
    const uint8_t reg = kLed0OnL + 4 * ch;
    sWire->beginTransmission(sAddr);
    sWire->write(reg);
    sWire->write(on & 0xFF);
    sWire->write(on >> 8);
    sWire->write(off & 0xFF);
    sWire->write(off >> 8);
    sWire->endTransmission();
}

} // namespace odh

#endif // !NATIVE_SIM
