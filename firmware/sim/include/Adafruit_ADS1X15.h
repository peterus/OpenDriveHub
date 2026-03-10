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
 * Adafruit_ADS1X15.h – Simulation stub for ADS1115 ADC.
 *
 * The PotModule uses Adafruit_ADS1115 over I²C.  In simulation, no
 * I²C hardware exists, so we stub all methods.  readADC_SingleEnded()
 * returns mid-scale (16384) which PotModule maps to ~1500 µs.
 */

#ifndef SIM_ADAFRUIT_ADS1X15_H
#define SIM_ADAFRUIT_ADS1X15_H

#include <cstdint>

typedef enum {
    GAIN_TWOTHIRDS = 0x0000,
    GAIN_ONE       = 0x0200,
    GAIN_TWO       = 0x0400,
    GAIN_FOUR      = 0x0600,
    GAIN_EIGHT     = 0x0800,
    GAIN_SIXTEEN   = 0x0A00,
} adsGain_t;

typedef enum {
    RATE_ADS1115_8SPS   = 0x0000,
    RATE_ADS1115_16SPS  = 0x0020,
    RATE_ADS1115_32SPS  = 0x0040,
    RATE_ADS1115_64SPS  = 0x0060,
    RATE_ADS1115_128SPS = 0x0080,
    RATE_ADS1115_250SPS = 0x00A0,
    RATE_ADS1115_475SPS = 0x00C0,
    RATE_ADS1115_860SPS = 0x00E0,
} ads1115DataRate_t;

class Adafruit_ADS1115 {
public:
    Adafruit_ADS1115(uint8_t addr = 0x48) {
        (void)addr;
    }
    bool begin(uint8_t addr = 0x48) {
        (void)addr;
        return true;
    }
    void setGain(adsGain_t g) {
        (void)g;
    }
    void setDataRate(ads1115DataRate_t r) {
        (void)r;
    }
    int16_t readADC_SingleEnded(uint8_t channel) {
        (void)channel;
        return 16384;
    }
};

#endif /* SIM_ADAFRUIT_ADS1X15_H */
