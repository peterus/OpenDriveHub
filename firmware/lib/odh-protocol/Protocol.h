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
    Control           = 0x01, ///< TX → RX: function values + trims
    Telemetry         = 0x02, ///< RX → TX: battery, RSSI, sensor data
    Bind              = 0x10, ///< Binding handshake (connect request / reply)
    Disconnect        = 0x40, ///< TX → RX: disconnect / release link
    DiscoveryRequest  = 0x50, ///< Sent by any device searching for a transmitter
    DiscoveryResponse = 0x51, ///< TX → broadcast: confirms active transmitter
    ReceiverPresence  = 0x52, ///< RX → broadcast: periodic presence announcement
    ChannelMigration  = 0x53, ///< TX → broadcast: notifies channel change
};

// ── Device roles ────────────────────────────────────────────────────────

enum class DeviceRole : uint8_t {
    Transmitter = 0x01,
    Receiver    = 0x02,
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

struct __attribute__((packed)) FunctionMapEntry {
    uint8_t function; ///< odh::Function
    uint8_t channel;  ///< Output channel index (0-based)
};

/// Discovery request – sent by any device searching for an active transmitter.
struct __attribute__((packed)) DiscoveryRequestPacket {
    uint8_t magic[2];
    uint8_t version;
    uint8_t type;   ///< PacketType::DiscoveryRequest
    uint8_t mac[6]; ///< Sender's MAC address
    uint8_t role;   ///< DeviceRole of the sender
    uint8_t checksum;
};

/// Discovery response – sent by an active transmitter answering a request.
struct __attribute__((packed)) DiscoveryResponsePacket {
    uint8_t magic[2];
    uint8_t version;
    uint8_t type;         ///< PacketType::DiscoveryResponse
    uint8_t mac[6];       ///< Transmitter's MAC address
    uint8_t channel;      ///< Current WiFi channel (1, 6, or 11)
    uint8_t device_count; ///< Number of known receivers on this channel
    uint8_t checksum;
};

/// Receiver presence – periodic broadcast by a receiver on the active channel.
struct __attribute__((packed)) ReceiverPresencePacket {
    uint8_t magic[2];
    uint8_t version;
    uint8_t type;       ///< PacketType::ReceiverPresence
    uint8_t mac[6];
    uint8_t model_type; ///< ModelType
    char name[kVehicleNameMax];
    uint8_t checksum;
};

/// Channel migration – sent by a transmitter before switching channels.
struct __attribute__((packed)) ChannelMigrationPacket {
    uint8_t magic[2];
    uint8_t version;
    uint8_t type;        ///< PacketType::ChannelMigration
    uint8_t mac[6];      ///< Transmitter's MAC address
    uint8_t new_channel; ///< Target WiFi channel (1, 6, or 11)
    uint8_t checksum;
};

// ── Compile-time packet size verification ───────────────────────────────

static_assert(sizeof(ControlPacket) <= 250, "Control packet exceeds ESP-NOW limit");
static_assert(sizeof(TelemetryPacket) <= 250, "Telemetry packet exceeds ESP-NOW limit");
static_assert(sizeof(BindPacket) <= 250, "Bind packet exceeds ESP-NOW limit");
static_assert(sizeof(DiscoveryRequestPacket) <= 250, "DiscoveryRequest packet exceeds ESP-NOW limit");
static_assert(sizeof(DiscoveryResponsePacket) <= 250, "DiscoveryResponse packet exceeds ESP-NOW limit");
static_assert(sizeof(ReceiverPresencePacket) <= 250, "ReceiverPresence packet exceeds ESP-NOW limit");
static_assert(sizeof(ChannelMigrationPacket) <= 250, "ChannelMigration packet exceeds ESP-NOW limit");

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

// ── Discovery helper types ──────────────────────────────────────────────

/// A vehicle discovered via presence broadcasts (transmitter side).
struct DiscoveredVehicle {
    uint8_t mac[6]{};
    uint8_t modelType = 0;
    char name[kVehicleNameMax]{};
    uint32_t lastSeenMs = 0; ///< millis() timestamp of last presence
    int8_t rssi         = 0; ///< RSSI of last presence packet (dBm)
    uint8_t channel     = 0; ///< WiFi channel where this vehicle was seen
    bool valid          = false;
};

} // namespace odh
