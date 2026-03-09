/**
 * TelemetryData – telemetry snapshot received from the receiver.
 *
 * Separated from BatteryMonitor to keep concerns clean:
 *   - BatteryMonitor: local ADC battery measurement (used on both sides)
 *   - TelemetryData: received telemetry from the remote peer (TX side only)
 */

#pragma once

#include "Protocol.h"

#include <cstdint>

namespace odh {

class TelemetryData {
public:
    TelemetryData() = default;

    /**
     * Process an incoming telemetry packet from the receiver.
     * Called from the RadioLink receive callback.
     */
    void onPacketReceived(const TelemetryPacket &pkt);

    /// Reset all state (call on new connection).
    void reset();

    // ── Accessors ───────────────────────────────────────────────────

    /// Receiver battery voltage in millivolts.
    uint16_t rxBatteryMv() const {
        return _rxBatteryMv;
    }

    /// Last received RSSI in dBm (negative).
    int8_t rssi() const {
        return _rssi;
    }

    /// Link state reported by the receiver.
    LinkState linkState() const {
        return static_cast<LinkState>(_linkState);
    }

    /// Model flags from the receiver.
    uint8_t modelFlags() const {
        return _modelFlags;
    }

    /// Vehicle model type reported by the receiver.
    uint8_t rxModelType() const {
        return _rxModelType;
    }

    /// Sensor value by index (0 if out of range).
    uint16_t sensor(uint8_t index) const;

    /// Sensor count reported by the receiver.
    uint8_t sensorCount() const {
        return _sensorCount;
    }

    /// Milliseconds since the last telemetry packet.
    uint32_t msSinceLastPacket() const;

    /// True if at least one packet has been received.
    bool hasData() const {
        return _hasData;
    }

    /// Packets received since last reset().
    uint32_t packetCount() const {
        return _packetCount;
    }

    /// Connection uptime in milliseconds.
    uint32_t connectionUptimeMs() const;

    /// Average packets per second since connection start.
    float packetsPerSecond() const;

    // ── RX battery cell detection ───────────────────────────────────

    void setRxCells(uint8_t cells) {
        _rxCells = cells;
    }
    void autoDetectRxCells();
    uint8_t rxCells() const {
        return _rxCells;
    }
    uint16_t rxCellVoltageMv() const {
        return (_rxCells > 0) ? (_rxBatteryMv / _rxCells) : 0;
    }

private:
    uint16_t _rxBatteryMv          = 0;
    int8_t _rssi                   = 0;
    uint8_t _linkState             = 0;
    uint8_t _rxModelType           = 0;
    uint8_t _modelFlags            = 0;
    uint8_t _sensorCount           = 0;
    uint16_t _sensors[kMaxSensors] = {};

    bool _hasData               = false;
    uint32_t _lastPacketMs      = 0;
    uint32_t _packetCount       = 0;
    uint32_t _connectionStartMs = 0;

    uint8_t _rxCells = 0;

    static uint8_t detectCells(uint16_t totalMv);
};

} // namespace odh
