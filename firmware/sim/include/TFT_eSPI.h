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
 * TFT_eSPI.h – Simulation shim (stub).
 *
 * Under NATIVE_SIM, Display.cpp uses SDL2 directly instead of TFT_eSPI.
 * This stub provides the class interface so the code compiles, but the
 * methods are only called in the non-NATIVE_SIM code path.
 */

#ifndef SIM_TFT_ESPI_H
#define SIM_TFT_ESPI_H

#include <cstdint>

#define TFT_BLACK 0x0000

class TFT_eSPI {
public:
    void begin() {}
    void setRotation(uint8_t) {}
    void fillScreen(uint16_t) {}
    void setTouch(uint16_t *) {}
    bool getTouch(uint16_t *, uint16_t *, uint16_t = 600) {
        return false;
    }

    void startWrite() {}
    void endWrite() {}
    void setAddrWindow(int32_t, int32_t, uint32_t, uint32_t) {}
    void pushColors(uint16_t *, uint32_t, bool = true) {}
};

#endif /* SIM_TFT_ESPI_H */
