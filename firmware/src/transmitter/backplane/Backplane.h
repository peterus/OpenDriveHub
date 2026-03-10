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
 * Backplane – TCA9548A I²C-multiplexer manager.
 *
 * Provides isolated I²C channels for up to kModuleSlotCount plug-in
 * modules.  Every slot runs on its own mux channel so identical I²C
 * addresses do not collide.
 */

#pragma once

#include <cstdint>

namespace odh {

inline constexpr uint8_t kNoSlot = 0xFF;

class Backplane {
public:
    /**
     * @param muxAddress  I²C address of the TCA9548A (default 0x70).
     * @param slotCount   Number of populated slots (max 8).
     */
    explicit Backplane(uint8_t muxAddress = 0x70, uint8_t slotCount = 8);

    /** Initialise the mux and verify it is reachable.  Call after Wire.begin(). */
    bool begin();

    /** Select one mux channel by slot index (0-based). */
    bool selectSlot(uint8_t slot);

    /** Deactivate all mux channels. */
    void deselectAll();

    /** Return the currently active slot, or kNoSlot if none. */
    uint8_t activeSlot() const {
        return _activeSlot;
    }

    /**
     * Probe every slot for a responding device at @p probeAddress.
     * @return Bitmask – bit N set means slot N has the device.
     */
    uint8_t scanSlots(uint8_t probeAddress);

    uint8_t slotCount() const {
        return _slotCount;
    }
    bool isReady() const {
        return _ready;
    }

private:
    uint8_t _muxAddress;
    uint8_t _slotCount;
    uint8_t _activeSlot = kNoSlot;
    bool _ready         = false;

    bool writeMux(uint8_t mask);
};

} // namespace odh
