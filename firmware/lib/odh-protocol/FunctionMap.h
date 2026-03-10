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
 * FunctionMap – vehicle model presets and function-to-channel mapping utilities
 *
 * Modern C++ version using constexpr string_view, std::optional, and
 * std::array-based lookup tables.
 */

#pragma once

#include "Protocol.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <optional>

#ifdef __has_include
#if __has_include(<string_view>)
#include <string_view>
#define ODH_HAS_STRING_VIEW 1
#endif
#endif

namespace odh {

// ── Name lookups ────────────────────────────────────────────────────────

#ifdef ODH_HAS_STRING_VIEW
using NameView = std::string_view;
#else
using NameView = const char *;
#endif

/**
 * Return a short human-readable label for the given function.
 */
inline constexpr NameView functionName(Function func) {
    switch (func) {
    case Function::Drive:
        return "Drive";
    case Function::Steering:
        return "Steering";
    case Function::Aux1:
        return "Aux 1";
    case Function::Aux2:
        return "Aux 2";
    case Function::Aux3:
        return "Aux 3";
    case Function::Aux4:
        return "Aux 4";
    case Function::BoomUD:
        return "Boom U/D";
    case Function::BoomLR:
        return "Boom L/R";
    case Function::Bucket:
        return "Bucket";
    case Function::Swing:
        return "Swing";
    case Function::DumpBed:
        return "Dump Bed";
    case Function::Pto:
        return "PTO";
    case Function::Hitch:
        return "Hitch";
    case Function::Winch:
        return "Winch";
    case Function::TrackL:
        return "Track L";
    case Function::TrackR:
        return "Track R";
    default:
        return "Unknown";
    }
}

/**
 * Return a short human-readable label for the given function (by raw uint8_t).
 */
inline constexpr NameView functionName(uint8_t func) {
    return functionName(static_cast<Function>(func));
}

/**
 * Return a short human-readable label for the given model type.
 */
inline constexpr NameView modelName(ModelType model) {
    switch (model) {
    case ModelType::Generic:
        return "Generic";
    case ModelType::DumpTruck:
        return "Dump Truck";
    case ModelType::Excavator:
        return "Excavator";
    case ModelType::Tractor:
        return "Tractor";
    case ModelType::Crane:
        return "Crane";
    default:
        return "Unknown";
    }
}

/**
 * Return a short human-readable label for the given model type (by raw uint8_t).
 */
inline constexpr NameView modelName(uint8_t model) {
    return modelName(static_cast<ModelType>(model));
}

// ── Default mapping presets per model type ──────────────────────────────

/// Result of a defaultFunctionMap() call – contains entries and their count.
struct FunctionMapping {
    std::array<FunctionMapEntry, kMaxFunctions> entries{};
    uint8_t count = 0;

    /// Index operator for convenience.
    constexpr const FunctionMapEntry &operator[](uint8_t i) const {
        return entries[i];
    }
    constexpr FunctionMapEntry &operator[](uint8_t i) {
        return entries[i];
    }
};

/**
 * Get the default function-to-channel mapping for the given model type.
 */
inline constexpr FunctionMapping defaultFunctionMap(ModelType model) {
    FunctionMapping m{};

    switch (model) {
    case ModelType::DumpTruck:
        m.entries[0] = {static_cast<uint8_t>(Function::Drive), 0};
        m.entries[1] = {static_cast<uint8_t>(Function::Steering), 1};
        m.entries[2] = {static_cast<uint8_t>(Function::DumpBed), 2};
        m.count      = 3;
        break;

    case ModelType::Excavator:
        m.entries[0] = {static_cast<uint8_t>(Function::TrackL), 0};
        m.entries[1] = {static_cast<uint8_t>(Function::TrackR), 1};
        m.entries[2] = {static_cast<uint8_t>(Function::BoomUD), 2};
        m.entries[3] = {static_cast<uint8_t>(Function::BoomLR), 3};
        m.entries[4] = {static_cast<uint8_t>(Function::Bucket), 4};
        m.entries[5] = {static_cast<uint8_t>(Function::Swing), 5};
        m.count      = 6;
        break;

    case ModelType::Tractor:
        m.entries[0] = {static_cast<uint8_t>(Function::Drive), 0};
        m.entries[1] = {static_cast<uint8_t>(Function::Steering), 1};
        m.entries[2] = {static_cast<uint8_t>(Function::Pto), 2};
        m.entries[3] = {static_cast<uint8_t>(Function::Hitch), 3};
        m.count      = 4;
        break;

    case ModelType::Crane:
        m.entries[0] = {static_cast<uint8_t>(Function::Drive), 0};
        m.entries[1] = {static_cast<uint8_t>(Function::Steering), 1};
        m.entries[2] = {static_cast<uint8_t>(Function::BoomUD), 2};
        m.entries[3] = {static_cast<uint8_t>(Function::Winch), 3};
        m.entries[4] = {static_cast<uint8_t>(Function::Swing), 4};
        m.count      = 5;
        break;

    case ModelType::Generic:
    default:
        m.entries[0] = {static_cast<uint8_t>(Function::Drive), 0};
        m.entries[1] = {static_cast<uint8_t>(Function::Steering), 1};
        m.entries[2] = {static_cast<uint8_t>(Function::Aux1), 2};
        m.entries[3] = {static_cast<uint8_t>(Function::Aux2), 3};
        m.entries[4] = {static_cast<uint8_t>(Function::Aux3), 4};
        m.entries[5] = {static_cast<uint8_t>(Function::Aux4), 5};
        m.count      = 6;
        break;
    }
    return m;
}

/**
 * Get the default function-to-channel mapping for the given model type (by raw uint8_t).
 */
inline constexpr FunctionMapping defaultFunctionMap(uint8_t model) {
    return defaultFunctionMap(static_cast<ModelType>(model));
}

// ── Mapping lookup helpers ──────────────────────────────────────────────

/**
 * Find the channel index for a given function in the mapping table.
 *
 * @return Channel index, or std::nullopt if not found.
 */
inline constexpr std::optional<uint8_t> functionToChannel(const FunctionMapping &map, Function func) {
    const auto funcRaw = static_cast<uint8_t>(func);
    for (uint8_t i = 0; i < map.count; i++) {
        if (map.entries[i].function == funcRaw) {
            return map.entries[i].channel;
        }
    }
    return std::nullopt;
}

/**
 * Find the function assigned to a given channel index.
 *
 * @return Function enum, or std::nullopt if not mapped.
 */
inline constexpr std::optional<Function> channelToFunction(const FunctionMapping &map, uint8_t channel) {
    for (uint8_t i = 0; i < map.count; i++) {
        if (map.entries[i].channel == channel) {
            return static_cast<Function>(map.entries[i].function);
        }
    }
    return std::nullopt;
}

// ── Legacy compatibility wrappers ───────────────────────────────────────
// These allow existing code to compile while transitioning to odh:: namespace.

inline const char *odh_function_name(uint8_t func) {
#ifdef ODH_HAS_STRING_VIEW
    auto sv = odh::functionName(func);
    // string_view from constexpr literal – always null-terminated
    return sv.data();
#else
    return odh::functionName(func);
#endif
}

inline const char *odh_model_name(uint8_t model) {
#ifdef ODH_HAS_STRING_VIEW
    auto sv = odh::modelName(model);
    return sv.data();
#else
    return odh::modelName(model);
#endif
}

inline uint8_t odh_default_function_map(uint8_t model, OdhFunctionMapEntry_t *entries) {
    auto map = odh::defaultFunctionMap(model);
    std::memcpy(entries, map.entries.data(), sizeof(OdhFunctionMapEntry_t) * map.count);
    return map.count;
}

inline uint8_t odh_function_to_channel(const OdhFunctionMapEntry_t *entries, uint8_t count, uint8_t function) {
    for (uint8_t i = 0; i < count; i++) {
        if (entries[i].function == function) {
            return entries[i].channel;
        }
    }
    return 0xFF;
}

inline uint8_t odh_channel_to_function(const OdhFunctionMapEntry_t *entries, uint8_t count, uint8_t channel) {
    for (uint8_t i = 0; i < count; i++) {
        if (entries[i].channel == channel) {
            return entries[i].function;
        }
    }
    return ODH_FUNC_NONE;
}

} // namespace odh

// Pull legacy functions into global scope
using odh::odh_channel_to_function;
using odh::odh_default_function_map;
using odh::odh_function_name;
using odh::odh_function_to_channel;
using odh::odh_model_name;
