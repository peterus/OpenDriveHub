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
 * InputMap – physical input → logical function assignment.
 *
 * Each entry maps a physical module input (slot + inputIndex) to a
 * logical odh::Function.  The transmitter control task iterates the map,
 * reads each physical input from the ModuleManager and packages the
 * result as a FunctionValue for radio transmission.
 *
 * NVS keys (per model m):
 *   "imapc_<m>"  – uint8_t count of entries
 *   "imape_<m>"  – blob of InputAssignment[]
 */

#pragma once

#include <Protocol.h>
#include <cstdint>

namespace odh {

/** Sentinel: slot / inputIndex not assigned. */
inline constexpr uint8_t kInputUnassigned = 0xFF;

/**
 * A single input assignment entry.
 * Stored sequentially in NVS and used at runtime.
 */
struct InputAssignment {
    uint8_t slot       = kInputUnassigned; ///< Backplane slot index.
    uint8_t inputIndex = 0;                ///< Input index within the module.
    uint8_t function   = 0;                ///< odh::Function (cast to uint8_t).
    int8_t trim        = 0;                ///< Trim offset ±100.

    bool isAssigned() const {
        return slot != kInputUnassigned;
    }
};

} // namespace odh
