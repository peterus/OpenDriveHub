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
 * Adafruit_PCF8574 simulation shim.
 * Provides a stub so transmitter code compiles on native/sim.
 */

#pragma once

#include <Wire.h>

#include <cstdint>

#ifndef INPUT
#define INPUT 0x0
#endif

class Adafruit_PCF8574 {
public:
    bool begin(uint8_t addr = 0x20, TwoWire *wire = nullptr) {
        (void)addr;
        (void)wire;
        return true;
    }
    void pinMode(uint8_t pin, uint8_t mode) {
        (void)pin;
        (void)mode;
    }
    bool digitalRead(uint8_t pin) {
        (void)pin;
        return true;
    }
    void digitalWrite(uint8_t pin, bool val) {
        (void)pin;
        (void)val;
    }
};
