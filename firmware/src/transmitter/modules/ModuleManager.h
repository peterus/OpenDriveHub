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
 * ModuleManager – detection, initialisation, and periodic update of
 * all plug-in modules behind the I²C-mux backplane.
 *
 * Detection strategy (no MCU on the modules themselves):
 *   Each mux channel is probed for known I²C addresses in priority order.
 *   Priority: Pot (0x48) > Encoder (0x36) > Switch (0x20) > Button (0x21).
 */

#pragma once

#include "../backplane/Backplane.h"
#include "IModule.h"

#include <array>
#include <cstdint>
#include <memory>

namespace odh {

class ModuleManager {
public:
    /** @param backplane  Reference to an already-initialised Backplane. */
    explicit ModuleManager(Backplane &backplane);

    /**
     * Scan all slots, detect module types and call begin() on each.
     * Safe to call repeatedly (e.g. after hot-swap).
     */
    void scanAndInit();

    /** Read all module values from hardware. Call every control cycle. */
    void update();

    /** Read a single input value from a slot/input pair. */
    uint16_t readInput(uint8_t slot, uint8_t inputIndex) const;

    /** Number of inputs provided by the module in @p slot (0 if empty). */
    uint8_t inputCount(uint8_t slot) const;

    /** Total configured slot count (from backplane). */
    uint8_t slotCount() const {
        return _backplane.slotCount();
    }

    /** Raw pointer to the module at @p slot (nullptr if empty). */
    IModule *moduleAt(uint8_t slot) const;

    /** ModuleType detected at @p slot. */
    ModuleType typeAt(uint8_t slot) const;

private:
    Backplane &_backplane;
    std::array<std::unique_ptr<IModule>, 8> _modules{};

    /** Probe one slot and return a new module, or nullptr. */
    std::unique_ptr<IModule> detectModule(uint8_t slot);
};

} // namespace odh
