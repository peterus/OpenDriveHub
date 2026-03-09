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
