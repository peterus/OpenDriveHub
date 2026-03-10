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
 * OpenDriveHub Protocol – Modern C++ Definitions
 *
 * Shared packet structures, enums, and utilities used by both the
 * transmitter and receiver firmware.
 *
 * All multi-byte fields are little-endian.
 * Packet framing:
 *   [magic0][magic1][version][type][sequence_lo][sequence_hi][payload...][checksum]
 *
 * The checksum is XOR of all bytes from magic0 through the last payload byte.
 */

#pragma once

#include <cstdint>
#include <cstring>
#include <optional>

#ifdef __has_include
#if __has_include(<span>)
#include <span>
#define ODH_HAS_SPAN 1
#endif
#endif

namespace odh {

// ── Protocol constants ──────────────────────────────────────────────────

inline constexpr uint8_t kMagic0          = 0x4F; // 'O'
inline constexpr uint8_t kMagic1          = 0x44; // 'D'
inline constexpr uint8_t kProtocolVersion = 1;

inline constexpr uint8_t kMaxChannels    = 16;
inline constexpr uint16_t kChannelMin    = 1000;
inline constexpr uint16_t kChannelMid    = 1500;
inline constexpr uint16_t kChannelMax    = 2000;
inline constexpr uint8_t kMaxSensors     = 8;
inline constexpr uint8_t kMaxFunctions   = 16;
inline constexpr uint8_t kMaxDiscovered  = 8;
inline constexpr uint8_t kVehicleNameMax = 16;

// ── Packet types ────────────────────────────────────────────────────────

enum class PacketType : uint8_t {
    Control    = 0x01, ///< TX → RX: function values + trims
    Telemetry  = 0x02, ///< RX → TX: battery, RSSI, sensor data
    Bind       = 0x10, ///< Binding handshake (connect request / reply)
    Ack        = 0x20, ///< Generic acknowledgement
    Announce   = 0x30, ///< RX → broadcast: vehicle presence announcement
    Disconnect = 0x40, ///< TX → RX: disconnect / release link
};

// ── Vehicle model types ─────────────────────────────────────────────────

enum class ModelType : uint8_t {
    Generic   = 0x00,
    DumpTruck = 0x01,
    Excavator = 0x02,
    Tractor   = 0x03,
    Crane     = 0x04,
    Count     = 0x05,
};

// ── Vehicle functions ───────────────────────────────────────────────────

enum class Function : uint8_t {
    Drive    = 0x00,
    Steering = 0x01,
    Aux1     = 0x02,
    Aux2     = 0x03,
    Aux3     = 0x04,
    Aux4     = 0x05,
    BoomUD   = 0x06,
    BoomLR   = 0x07,
    Bucket   = 0x08,
    Swing    = 0x09,
    DumpBed  = 0x0A,
    Pto      = 0x0B,
    Hitch    = 0x0C,
    Winch    = 0x0D,
    TrackL   = 0x0E,
    TrackR   = 0x0F,
    None     = 0xFF,
};

// ── Link state ──────────────────────────────────────────────────────────

enum class LinkState : uint8_t {
    Disconnected = 0,
    Binding      = 1,
    Connected    = 2,
    Failsafe     = 3,
    Scanning     = 4,
};

// ── Packet structures ───────────────────────────────────────────────────

struct __attribute__((packed)) FunctionValue {
    uint8_t function; ///< odh::Function
    uint16_t value;   ///< RC pulse width in µs [kChannelMin, kChannelMax]
    int8_t trim;      ///< Trim offset ±100
};

struct __attribute__((packed)) ControlPacket {
    uint8_t magic[2];
    uint8_t version;
    uint8_t type; ///< PacketType::Control
    uint16_t sequence;
    uint8_t function_count;
    FunctionValue functions[kMaxFunctions];
    uint8_t flags;
    uint8_t checksum;
};

struct __attribute__((packed)) TelemetryPacket {
    uint8_t magic[2];
    uint8_t version;
    uint8_t type; ///< PacketType::Telemetry
    uint16_t sequence;
    uint16_t battery_mv;
    int8_t rssi;
    uint8_t link_state; ///< LinkState
    uint8_t model_type; ///< ModelType
    uint8_t model_flags;
    uint8_t sensor_count;
    uint16_t sensors[kMaxSensors];
    uint8_t checksum;
};

struct __attribute__((packed)) BindPacket {
    uint8_t magic[2];
    uint8_t version;
    uint8_t type; ///< PacketType::Bind or PacketType::Disconnect
    uint8_t mac[6];
    uint8_t checksum;
};

struct __attribute__((packed)) AnnouncePacket {
    uint8_t magic[2];
    uint8_t version;
    uint8_t type;       ///< PacketType::Announce
    uint8_t mac[6];
    uint8_t model_type; ///< ModelType
    char name[kVehicleNameMax];
    uint8_t checksum;
};

struct __attribute__((packed)) FunctionMapEntry {
    uint8_t function; ///< odh::Function
    uint8_t channel;  ///< Output channel index (0-based)
};

// ── Compile-time packet size verification ───────────────────────────────

static_assert(sizeof(ControlPacket) <= 250, "Control packet exceeds ESP-NOW limit");
static_assert(sizeof(TelemetryPacket) <= 250, "Telemetry packet exceeds ESP-NOW limit");
static_assert(sizeof(BindPacket) <= 250, "Bind packet exceeds ESP-NOW limit");
static_assert(sizeof(AnnouncePacket) <= 250, "Announce packet exceeds ESP-NOW limit");

// ── Utility: XOR checksum ───────────────────────────────────────────────

inline constexpr uint8_t checksum(const uint8_t *data, uint16_t len) {
    uint8_t cs = 0;
    for (uint16_t i = 0; i < len; i++) {
        cs ^= data[i];
    }
    return cs;
}

#ifdef ODH_HAS_SPAN
inline constexpr uint8_t checksum(std::span<const uint8_t> data) {
    return checksum(data.data(), static_cast<uint16_t>(data.size()));
}
#endif

// ── Packet header helpers ───────────────────────────────────────────────

/// Fill magic, version, and type in any packet struct.
template <typename Packet> inline void fillHeader(Packet &pkt, PacketType type) {
    pkt.magic[0] = kMagic0;
    pkt.magic[1] = kMagic1;
    pkt.version  = kProtocolVersion;
    pkt.type     = static_cast<uint8_t>(type);
}

/// Compute and set the checksum field of any packet struct.
template <typename Packet> inline void setChecksum(Packet &pkt) {
    const auto *raw = reinterpret_cast<const uint8_t *>(&pkt);
    pkt.checksum    = checksum(raw, sizeof(Packet) - 1);
}

/// Verify magic bytes and checksum of any raw packet buffer.
inline bool verifyPacket(const uint8_t *data, int len) {
    if (len < 4)
        return false;
    if (data[0] != kMagic0 || data[1] != kMagic1)
        return false;
    uint8_t expected = checksum(data, static_cast<uint16_t>(len - 1));
    return data[len - 1] == expected;
}

} // namespace odh

// ── Legacy compatibility aliases (for gradual migration) ────────────────
// These allow existing code to compile while transitioning to odh:: namespace.

using OdhFunctionValue_t    = odh::FunctionValue;
using OdhControlPacket_t    = odh::ControlPacket;
using OdhTelemetryPacket_t  = odh::TelemetryPacket;
using OdhBindPacket_t       = odh::BindPacket;
using OdhAnnouncePacket_t   = odh::AnnouncePacket;
using OdhFunctionMapEntry_t = odh::FunctionMapEntry;

// Legacy enum values as constexpr
inline constexpr uint8_t ODH_MAGIC_0          = odh::kMagic0;
inline constexpr uint8_t ODH_MAGIC_1          = odh::kMagic1;
inline constexpr uint8_t ODH_PROTOCOL_VERSION = odh::kProtocolVersion;
inline constexpr uint8_t ODH_MAX_CHANNELS     = odh::kMaxChannels;
inline constexpr uint16_t ODH_CHANNEL_MIN     = odh::kChannelMin;
inline constexpr uint16_t ODH_CHANNEL_MID     = odh::kChannelMid;
inline constexpr uint16_t ODH_CHANNEL_MAX     = odh::kChannelMax;
inline constexpr uint8_t ODH_MAX_SENSORS      = odh::kMaxSensors;
inline constexpr uint8_t ODH_MAX_FUNCTIONS    = odh::kMaxFunctions;
inline constexpr uint8_t ODH_MAX_DISCOVERED   = odh::kMaxDiscovered;
inline constexpr uint8_t ODH_VEHICLE_NAME_MAX = odh::kVehicleNameMax;

// Legacy packet type values
inline constexpr uint8_t ODH_PKT_CONTROL    = static_cast<uint8_t>(odh::PacketType::Control);
inline constexpr uint8_t ODH_PKT_TELEMETRY  = static_cast<uint8_t>(odh::PacketType::Telemetry);
inline constexpr uint8_t ODH_PKT_BIND       = static_cast<uint8_t>(odh::PacketType::Bind);
inline constexpr uint8_t ODH_PKT_ACK        = static_cast<uint8_t>(odh::PacketType::Ack);
inline constexpr uint8_t ODH_PKT_ANNOUNCE   = static_cast<uint8_t>(odh::PacketType::Announce);
inline constexpr uint8_t ODH_PKT_DISCONNECT = static_cast<uint8_t>(odh::PacketType::Disconnect);

// Legacy model type values
inline constexpr uint8_t ODH_MODEL_GENERIC    = static_cast<uint8_t>(odh::ModelType::Generic);
inline constexpr uint8_t ODH_MODEL_DUMP_TRUCK = static_cast<uint8_t>(odh::ModelType::DumpTruck);
inline constexpr uint8_t ODH_MODEL_EXCAVATOR  = static_cast<uint8_t>(odh::ModelType::Excavator);
inline constexpr uint8_t ODH_MODEL_TRACTOR    = static_cast<uint8_t>(odh::ModelType::Tractor);
inline constexpr uint8_t ODH_MODEL_CRANE      = static_cast<uint8_t>(odh::ModelType::Crane);
inline constexpr uint8_t ODH_MODEL_COUNT      = static_cast<uint8_t>(odh::ModelType::Count);

// Legacy function values
inline constexpr uint8_t ODH_FUNC_DRIVE    = static_cast<uint8_t>(odh::Function::Drive);
inline constexpr uint8_t ODH_FUNC_STEERING = static_cast<uint8_t>(odh::Function::Steering);
inline constexpr uint8_t ODH_FUNC_AUX1     = static_cast<uint8_t>(odh::Function::Aux1);
inline constexpr uint8_t ODH_FUNC_AUX2     = static_cast<uint8_t>(odh::Function::Aux2);
inline constexpr uint8_t ODH_FUNC_AUX3     = static_cast<uint8_t>(odh::Function::Aux3);
inline constexpr uint8_t ODH_FUNC_AUX4     = static_cast<uint8_t>(odh::Function::Aux4);
inline constexpr uint8_t ODH_FUNC_BOOM_UD  = static_cast<uint8_t>(odh::Function::BoomUD);
inline constexpr uint8_t ODH_FUNC_BOOM_LR  = static_cast<uint8_t>(odh::Function::BoomLR);
inline constexpr uint8_t ODH_FUNC_BUCKET   = static_cast<uint8_t>(odh::Function::Bucket);
inline constexpr uint8_t ODH_FUNC_SWING    = static_cast<uint8_t>(odh::Function::Swing);
inline constexpr uint8_t ODH_FUNC_DUMP_BED = static_cast<uint8_t>(odh::Function::DumpBed);
inline constexpr uint8_t ODH_FUNC_PTO      = static_cast<uint8_t>(odh::Function::Pto);
inline constexpr uint8_t ODH_FUNC_HITCH    = static_cast<uint8_t>(odh::Function::Hitch);
inline constexpr uint8_t ODH_FUNC_WINCH    = static_cast<uint8_t>(odh::Function::Winch);
inline constexpr uint8_t ODH_FUNC_TRACK_L  = static_cast<uint8_t>(odh::Function::TrackL);
inline constexpr uint8_t ODH_FUNC_TRACK_R  = static_cast<uint8_t>(odh::Function::TrackR);
inline constexpr uint8_t ODH_FUNC_NONE     = static_cast<uint8_t>(odh::Function::None);

// Legacy link state values
inline constexpr uint8_t ODH_LINK_DISCONNECTED = static_cast<uint8_t>(odh::LinkState::Disconnected);
inline constexpr uint8_t ODH_LINK_BINDING      = static_cast<uint8_t>(odh::LinkState::Binding);
inline constexpr uint8_t ODH_LINK_CONNECTED    = static_cast<uint8_t>(odh::LinkState::Connected);
inline constexpr uint8_t ODH_LINK_FAILSAFE     = static_cast<uint8_t>(odh::LinkState::Failsafe);
inline constexpr uint8_t ODH_LINK_SCANNING     = static_cast<uint8_t>(odh::LinkState::Scanning);

// Legacy checksum function
inline uint8_t odh_checksum(const uint8_t *data, uint16_t len) {
    return odh::checksum(data, len);
}
