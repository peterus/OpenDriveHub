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
 * sim_keyboard.cpp – Keyboard channel control implementation.
 */

#include "sim_keyboard.h"

#include <algorithm>
#include <cstring>

SimKeyboardState g_simKeyboard;

SimKeyboardState::SimKeyboardState()
    : activeChannel(0),
      quit(false) {
    for (uint8_t i = 0; i < 16; i++) {
        channels[i] = 1500;
    }
}

void sim_apply_keyboard_inputs(uint16_t *channels, uint8_t count) {
    std::lock_guard<std::mutex> lock(g_simKeyboard.mtx);
    uint8_t n = count < 16 ? count : 16;
    for (uint8_t i = 0; i < n; i++) {
        channels[i] = g_simKeyboard.channels[i];
    }
}
