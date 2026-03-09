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
