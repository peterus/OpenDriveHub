/**
 * IRadioLink – abstract interface for ESP-NOW radio links.
 *
 * Both ReceiverRadioLink and TransmitterRadioLink implement this interface.
 * The interface allows mocking in tests and enables the sim layer to
 * provide alternative implementations.
 */

#pragma once

#include "Protocol.h"

#include <cstdint>
#include <functional>

namespace odh {

/// Callback type for received control packets (receiver side).
using ControlCallback = std::function<void(const ControlPacket &pkt)>;

/// Callback type for received telemetry packets (transmitter side).
using TelemetryCallback = std::function<void(const TelemetryPacket &pkt)>;

/// A vehicle discovered via announce broadcasts (transmitter side).
struct DiscoveredVehicle {
    uint8_t mac[6]{};
    uint8_t modelType = 0;
    char name[kVehicleNameMax]{};
    uint32_t lastSeenMs = 0; ///< millis() timestamp of last announce
    int8_t rssi         = 0; ///< RSSI of last announce packet (dBm)
    bool valid          = false;
};

} // namespace odh
