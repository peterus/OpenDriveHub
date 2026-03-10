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
 * sim_keyboard.h – Keyboard-to-channel mapping for the TX simulator.
 *
 * SDL2 key events are captured in the display touch callback and stored
 * here.  The control task applies them to the channel array.
 *
 * Key bindings:
 *   1-8          Select active channel
 *   ↑ / ↓        Increase / decrease active channel by 10 µs
 *   Page Up/Down Increase / decrease active channel by 100 µs
 *   Home         Center active channel (1500 µs)
 *   Space        Center ALL channels
 */

#ifndef SIM_KEYBOARD_H
#define SIM_KEYBOARD_H

#include <cstdint>
#include <mutex>

struct SimKeyboardState {
    uint16_t channels[16];
    uint8_t activeChannel;
    bool quit;
    std::mutex mtx;

    SimKeyboardState();
};

extern SimKeyboardState g_simKeyboard;

/**
 * Apply the keyboard-controlled input values to the given array.
 * Called from the control task after moduleManager.update().
 */
void sim_apply_keyboard_inputs(uint16_t *channels, uint8_t count);

#endif /* SIM_KEYBOARD_H */
