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
