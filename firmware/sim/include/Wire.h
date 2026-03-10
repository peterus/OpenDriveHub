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
 * Wire.h – Simulation shim (I²C stub).
 *
 * All I²C transactions return NACK (no device), so module detection will
 * find no hardware modules.  This is expected for the simulation.
 */

#ifndef SIM_WIRE_H
#define SIM_WIRE_H

#include <cstddef>
#include <cstdint>

class TwoWire {
public:
    void begin(int = -1, int = -1) {}
    void setClock(uint32_t) {}

    void beginTransmission(uint8_t) {}
    /** Always returns 2 (NACK) – no I²C devices present. */
    uint8_t endTransmission(bool = true) {
        return 2;
    }

    uint8_t requestFrom(uint8_t, uint8_t) {
        return 0;
    }
    size_t write(uint8_t) {
        return 1;
    }
    size_t write(const uint8_t *, size_t len) {
        return len;
    }
    int available() {
        return 0;
    }
    int read() {
        return -1;
    }
};

extern TwoWire Wire;

#endif /* SIM_WIRE_H */
