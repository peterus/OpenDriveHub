#include "TelemetryData.h"

#include <Arduino.h>

namespace odh {

void TelemetryData::onPacketReceived(const TelemetryPacket &pkt) {
    _rxBatteryMv = pkt.battery_mv;
    _rssi        = pkt.rssi;
    _linkState   = pkt.link_state;
    _rxModelType = pkt.model_type;
    _modelFlags  = pkt.model_flags;
    _sensorCount = (pkt.sensor_count < kMaxSensors) ? pkt.sensor_count : kMaxSensors;

    for (uint8_t i = 0; i < _sensorCount; i++) {
        _sensors[i] = pkt.sensors[i];
    }

    _lastPacketMs = millis();
    _hasData      = true;
    _packetCount++;
}

void TelemetryData::reset() {
    _hasData           = false;
    _lastPacketMs      = 0;
    _packetCount       = 0;
    _connectionStartMs = millis();
    _rxBatteryMv       = 0;
    _rssi              = 0;
    _linkState         = static_cast<uint8_t>(LinkState::Disconnected);
}

uint16_t TelemetryData::sensor(uint8_t index) const {
    if (index >= kMaxSensors)
        return 0;
    return _sensors[index];
}

uint32_t TelemetryData::msSinceLastPacket() const {
    if (!_hasData)
        return UINT32_MAX;
    return millis() - _lastPacketMs;
}

uint32_t TelemetryData::connectionUptimeMs() const {
    return millis() - _connectionStartMs;
}

float TelemetryData::packetsPerSecond() const {
    const uint32_t uptime = connectionUptimeMs();
    if (uptime < 500 || _packetCount == 0)
        return 0.0f;
    return static_cast<float>(_packetCount) * 1000.0f / static_cast<float>(uptime);
}

void TelemetryData::autoDetectRxCells() {
    _rxCells = detectCells(_rxBatteryMv);
}

uint8_t TelemetryData::detectCells(uint16_t totalMv) {
    if (totalMv == 0)
        return 0;
    if (totalMv < 5000)
        return 1;
    if (totalMv < 9000)
        return 2;
    if (totalMv < 13200)
        return 3;
    return 4;
}

} // namespace odh
