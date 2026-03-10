#include "TransmitterRadioLink.h"

#include <Arduino.h>
#include <WiFi.h>

#include <algorithm>
#include <cstring>

namespace odh {

TransmitterRadioLink *TransmitterRadioLink::sInstance = nullptr;

static constexpr uint8_t kBroadcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ── Construction ────────────────────────────────────────────────────────

TransmitterRadioLink::TransmitterRadioLink() = default;

// ── Initialisation ──────────────────────────────────────────────────────

bool TransmitterRadioLink::begin(uint8_t wifiChannel, TelemetryCallback callback) {
    _wifiChannel       = wifiChannel;
    _telemetryCallback = std::move(callback);
    sInstance          = this;

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK) {
        return false;
    }

    esp_now_register_recv_cb(onReceive);
    esp_now_register_send_cb(onSent);

    _ready = true;
    return true;
}

// ── Scanning ────────────────────────────────────────────────────────────

void TransmitterRadioLink::startScanning() {
    if (!_ready)
        return;

    _bound           = false;
    _scanning        = true;
    _discoveredCount = 0;
    std::fill(std::begin(_discovered), std::end(_discovered), DiscoveredVehicle{});
}

const DiscoveredVehicle *TransmitterRadioLink::discoveredVehicle(uint8_t index) const {
    if (index >= _discoveredCount)
        return nullptr;
    return &_discovered[index];
}

void TransmitterRadioLink::pruneStaleVehicles(uint32_t timeoutMs) {
    const uint32_t now = millis();
    for (uint8_t i = 0; i < _discoveredCount;) {
        if (_discovered[i].valid && (now - _discovered[i].lastSeenMs) > timeoutMs) {
            // Shift remaining entries down.
            for (uint8_t j = i; j + 1 < _discoveredCount; j++) {
                _discovered[j] = _discovered[j + 1];
            }
            _discoveredCount--;
            _discovered[_discoveredCount] = DiscoveredVehicle{};
        } else {
            i++;
        }
    }
}

// ── Connect / Disconnect ────────────────────────────────────────────────

bool TransmitterRadioLink::connectTo(uint8_t index, uint32_t timeoutMs) {
    if (!_ready || !_scanning || index >= _discoveredCount)
        return false;

    const auto &v = _discovered[index];

    addPeer(v.mac);
    std::memcpy(_peerMac, v.mac, 6);

    // Send a bind packet with our MAC.
    BindPacket pkt{};
    fillHeader(pkt, PacketType::Bind);
    esp_read_mac(pkt.mac, ESP_MAC_WIFI_STA);
    setChecksum(pkt);

    esp_now_send(_peerMac, reinterpret_cast<uint8_t *>(&pkt), sizeof(pkt));

    // Wait for bind reply (sets _bound via receive callback).
    const uint32_t start = millis();
    while (!_bound && (millis() - start) < timeoutMs) {
        delay(10);
    }

    if (_bound) {
        _scanning = false;
    } else {
        delPeer(_peerMac);
        std::memset(_peerMac, 0, 6);
    }
    return _bound;
}

void TransmitterRadioLink::disconnect() {
    if (!_ready || !_bound)
        return;

    BindPacket pkt{};
    fillHeader(pkt, PacketType::Disconnect);
    esp_read_mac(pkt.mac, ESP_MAC_WIFI_STA);
    setChecksum(pkt);

    esp_now_send(_peerMac, reinterpret_cast<uint8_t *>(&pkt), sizeof(pkt));

    delPeer(_peerMac);
    std::memset(_peerMac, 0, 6);
    _bound = false;

    startScanning();
}

// ── Send control ────────────────────────────────────────────────────────

bool TransmitterRadioLink::sendControl(const FunctionValue *functions, uint8_t count, uint8_t flags) {
    if (!_ready || !_bound)
        return false;

    ControlPacket pkt{};
    fillHeader(pkt, PacketType::Control);
    pkt.sequence       = _txSequence++;
    pkt.function_count = (count < kMaxFunctions) ? count : kMaxFunctions;
    pkt.flags          = flags;

    for (uint8_t i = 0; i < pkt.function_count; i++) {
        pkt.functions[i] = functions[i];
    }
    setChecksum(pkt);

    return esp_now_send(_peerMac, reinterpret_cast<uint8_t *>(&pkt), sizeof(pkt)) == ESP_OK;
}

void TransmitterRadioLink::boundMac(uint8_t out[6]) const {
    std::memcpy(out, _peerMac, 6);
}

// ── Static callbacks ────────────────────────────────────────────────────

void TransmitterRadioLink::onReceive(const uint8_t *mac, const uint8_t *data, int len) {
    if (sInstance)
        sInstance->handleReceive(mac, data, len);
}

void TransmitterRadioLink::onSent(const uint8_t * /*mac*/, esp_now_send_status_t status) {
    if (sInstance)
        sInstance->handleSent(status);
}

// ── Packet handling ─────────────────────────────────────────────────────

void TransmitterRadioLink::handleReceive(const uint8_t * /*mac*/, const uint8_t *data, int len) {
    if (!verifyPacket(data, len))
        return;
    if (len < 4)
        return;

    const auto type = static_cast<PacketType>(data[3]);

    // ── Announce packets (while scanning) ───────────────────────────
    if (type == PacketType::Announce && _scanning) {
        if (len < static_cast<int>(sizeof(AnnouncePacket)))
            return;

        const auto *ap = reinterpret_cast<const AnnouncePacket *>(data);

        // Update existing entry or add new one.
        for (uint8_t i = 0; i < _discoveredCount; i++) {
            if (std::memcmp(_discovered[i].mac, ap->mac, 6) == 0) {
                _discovered[i].modelType  = ap->model_type;
                _discovered[i].lastSeenMs = millis();
                _discovered[i].rssi       = _lastRssi;
                std::memset(_discovered[i].name, 0, kVehicleNameMax);
                std::strncpy(_discovered[i].name, ap->name, kVehicleNameMax - 1);
                return;
            }
        }

        if (_discoveredCount < kMaxDiscovered) {
            auto &v = _discovered[_discoveredCount];
            std::memcpy(v.mac, ap->mac, 6);
            v.modelType  = ap->model_type;
            v.lastSeenMs = millis();
            v.rssi       = _lastRssi;
            v.valid      = true;
            std::memset(v.name, 0, kVehicleNameMax);
            std::strncpy(v.name, ap->name, kVehicleNameMax - 1);
            _discoveredCount++;
        }
        return;
    }

    // ── Bind reply (connection handshake) ───────────────────────────
    if (type == PacketType::Bind && !_bound) {
        if (len < static_cast<int>(sizeof(BindPacket)))
            return;

        const auto *bp = reinterpret_cast<const BindPacket *>(data);
        std::memcpy(_peerMac, bp->mac, 6);
        addPeer(_peerMac);
        _bound = true;
        return;
    }

    // ── Telemetry (while connected) ─────────────────────────────────
    if (type == PacketType::Telemetry) {
        if (len < static_cast<int>(sizeof(TelemetryPacket)))
            return;

        const auto *tp = reinterpret_cast<const TelemetryPacket *>(data);
        if (_telemetryCallback) {
            _telemetryCallback(*tp);
        }
    }
}

void TransmitterRadioLink::handleSent(esp_now_send_status_t status) {
    _lastSendOk = (status == ESP_NOW_SEND_SUCCESS);
}

// ── Peer management ─────────────────────────────────────────────────────

bool TransmitterRadioLink::addPeer(const uint8_t mac[6]) {
    if (esp_now_is_peer_exist(mac))
        return true;

    esp_now_peer_info_t peer{};
    std::memcpy(peer.peer_addr, mac, 6);
    peer.channel = _wifiChannel;
    peer.encrypt = false;
    return esp_now_add_peer(&peer) == ESP_OK;
}

void TransmitterRadioLink::delPeer(const uint8_t mac[6]) {
    if (esp_now_is_peer_exist(mac)) {
        esp_now_del_peer(mac);
    }
}

} // namespace odh
