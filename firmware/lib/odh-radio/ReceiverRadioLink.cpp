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

#include "ReceiverRadioLink.h"

#include <Arduino.h>
#include <WiFi.h>

#include <cstring>

#ifndef NATIVE_SIM
#include <esp_wifi.h>
#endif

namespace odh {

/* ── Promiscuous-mode RSSI capture ──────────────────────────────────────── */

static volatile int8_t sPromiscRssi = 0;

#ifndef NATIVE_SIM
// cppcheck-suppress constParameterCallback
static void promiscRxCb(void *buf, wifi_promiscuous_pkt_type_t /*type*/) {
    const auto *pkt = static_cast<const wifi_promiscuous_pkt_t *>(buf);
    sPromiscRssi    = pkt->rx_ctrl.rssi;
}
#endif

ReceiverRadioLink *ReceiverRadioLink::sInstance = nullptr;

static constexpr uint8_t kBroadcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ── Construction ────────────────────────────────────────────────────────

ReceiverRadioLink::ReceiverRadioLink() = default;

// ── Initialisation ──────────────────────────────────────────────────────

bool ReceiverRadioLink::begin(ControlCallback callback) {
    _controlCallback = std::move(callback);
    sInstance        = this;

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK) {
        return false;
    }

    esp_now_register_recv_cb(onReceive);
    esp_now_register_send_cb(onSent);

#ifndef NATIVE_SIM
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(promiscRxCb);
#endif

    _ready = true;
    return true;
}

// ── Channel management ──────────────────────────────────────────────────

bool ReceiverRadioLink::setChannel(uint8_t channel) {
    _wifiChannel = channel;
#ifdef NATIVE_SIM
    sim_set_wifi_channel(channel);
#else
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
#endif
    return true;
}

// ── Presence (replaces announcing) ──────────────────────────────────────

void ReceiverRadioLink::configurePresence(const char *name, uint8_t modelType) {
    _modelType = modelType;
    std::memset(_vehicleName, 0, sizeof(_vehicleName));
    std::strncpy(_vehicleName, name, kVehicleNameMax - 1);
}

void ReceiverRadioLink::beginPresence() {
    if (!_ready)
        return;

    addPeer(kBroadcastMac);
    _presencing     = true;
    _lastPresenceMs = 0;
}

void ReceiverRadioLink::tickPresence(uint32_t intervalMs) {
    if (!_presencing)
        return;

    const uint32_t now = millis();
    if ((now - _lastPresenceMs) >= intervalMs) {
        sendPresencePacket();
        _lastPresenceMs = now;
    }
}

void ReceiverRadioLink::sendPresencePacket() {
    ReceiverPresencePacket pkt{};
    fillHeader(pkt, PacketType::ReceiverPresence);
    pkt.model_type = _modelType;
    esp_read_mac(pkt.mac, ESP_MAC_WIFI_STA);
    std::strncpy(pkt.name, _vehicleName, kVehicleNameMax - 1);
    pkt.name[kVehicleNameMax - 1] = '\0';
    setChecksum(pkt);

    esp_now_send(kBroadcastMac, reinterpret_cast<uint8_t *>(&pkt), sizeof(pkt));
}

void ReceiverRadioLink::stopPresence() {
    _presencing = false;
    delPeer(kBroadcastMac);
}

void ReceiverRadioLink::resumePresence() {
    addPeer(kBroadcastMac);
    _presencing     = true;
    _lastPresenceMs = 0;
}

// ── Discovery ───────────────────────────────────────────────────────────

bool ReceiverRadioLink::sendDiscoveryRequest() {
    if (!_ready)
        return false;

    addPeer(kBroadcastMac);

    DiscoveryRequestPacket pkt{};
    fillHeader(pkt, PacketType::DiscoveryRequest);
    esp_read_mac(pkt.mac, ESP_MAC_WIFI_STA);
    pkt.role = static_cast<uint8_t>(DeviceRole::Receiver);
    setChecksum(pkt);

    auto rc = esp_now_send(kBroadcastMac, reinterpret_cast<uint8_t *>(&pkt), sizeof(pkt));
    return rc == ESP_OK;
}

void ReceiverRadioLink::onDiscoveryResponse(DiscoveryResponseCallback cb) {
    _discoveryResponseCallback = std::move(cb);
}

void ReceiverRadioLink::onChannelMigration(ChannelMigrationCallback cb) {
    _channelMigrationCallback = std::move(cb);
}

// ── Telemetry ───────────────────────────────────────────────────────────

bool ReceiverRadioLink::sendTelemetry(uint16_t batteryMv, int8_t rssi, uint8_t linkState, uint8_t modelType, uint8_t modelFlags, const uint16_t *sensors, uint8_t sensorCount) {
    if (!_ready || !_bound)
        return false;

    TelemetryPacket pkt{};
    fillHeader(pkt, PacketType::Telemetry);
    pkt.sequence     = _txSequence++;
    pkt.battery_mv   = batteryMv;
    pkt.rssi         = rssi;
    pkt.link_state   = linkState;
    pkt.model_type   = modelType;
    pkt.model_flags  = modelFlags;
    pkt.sensor_count = (sensorCount < kMaxSensors) ? sensorCount : kMaxSensors;

    if (sensors) {
        for (uint8_t i = 0; i < pkt.sensor_count; i++) {
            pkt.sensors[i] = sensors[i];
        }
    }
    setChecksum(pkt);

    return esp_now_send(_txMac, reinterpret_cast<uint8_t *>(&pkt), sizeof(pkt)) == ESP_OK;
}

uint32_t ReceiverRadioLink::msSinceLastControl() const {
    if (_lastControlMs == 0)
        return UINT32_MAX;
    return millis() - _lastControlMs;
}

bool ReceiverRadioLink::checkLinkTimeout(uint32_t timeoutMs) {
    if (!_bound || _lastControlMs == 0)
        return false;

    if ((millis() - _lastControlMs) > timeoutMs) {
        delPeer(_txMac);
        std::memset(_txMac, 0, sizeof(_txMac));
        _bound         = false;
        _lastControlMs = 0;
        resumePresence();
        return true;
    }
    return false;
}

// ── Static callbacks ────────────────────────────────────────────────────

void ReceiverRadioLink::onReceive(const uint8_t *mac, const uint8_t *data, int len) {
    if (sInstance)
        sInstance->handleReceive(mac, data, len);
}

void ReceiverRadioLink::onSent(const uint8_t * /*mac*/, esp_now_send_status_t /*status*/) {
    // No action needed on send confirmation for receiver.
}

// ── Packet handling ─────────────────────────────────────────────────────

void ReceiverRadioLink::handleReceive(const uint8_t * /*mac*/, const uint8_t *data, int len) {
    if (!verifyPacket(data, len))
        return;
    if (len < 4)
        return;

    const auto type = static_cast<PacketType>(data[3]);

    // ── Bind request from transmitter (while presencing) ────────────
    if (type == PacketType::Bind && !_bound) {
        if (len < static_cast<int>(sizeof(BindPacket)))
            return;

        const auto *bp = reinterpret_cast<const BindPacket *>(data);

        std::memcpy(_txMac, bp->mac, 6);
        addPeer(_txMac);

        // Reply with our own bind packet.
        BindPacket reply{};
        fillHeader(reply, PacketType::Bind);
        esp_read_mac(reply.mac, ESP_MAC_WIFI_STA);
        setChecksum(reply);
        esp_now_send(_txMac, reinterpret_cast<uint8_t *>(&reply), sizeof(reply));

        _bound = true;
        stopPresence();
        return;
    }

    // ── Disconnect from transmitter ─────────────────────────────────
    if (type == PacketType::Disconnect && _bound) {
        if (len < static_cast<int>(sizeof(BindPacket)))
            return;

        delPeer(_txMac);
        std::memset(_txMac, 0, sizeof(_txMac));
        _bound         = false;
        _lastControlMs = 0;
        resumePresence();
        return;
    }

    // ── Control packet (while connected) ────────────────────────────
    if (type == PacketType::Control && _bound) {
        if (len < static_cast<int>(sizeof(ControlPacket)))
            return;

        const auto *cp = reinterpret_cast<const ControlPacket *>(data);

        _lastControlMs = millis();
        _lastRssi      = sPromiscRssi;

        if (_controlCallback) {
            _controlCallback(*cp);
        }
        return;
    }

    // ── DiscoveryResponse (during channel scanning) ────────────
    if (type == PacketType::DiscoveryResponse) {
        if (len < static_cast<int>(sizeof(DiscoveryResponsePacket)))
            return;
        const auto *dp = reinterpret_cast<const DiscoveryResponsePacket *>(data);
        if (_discoveryResponseCallback) {
            _discoveryResponseCallback(dp->channel, sPromiscRssi, dp->device_count);
        }
        return;
    }

    // ── ChannelMigration (transmitter is switching channels) ───
    if (type == PacketType::ChannelMigration) {
        if (len < static_cast<int>(sizeof(ChannelMigrationPacket)))
            return;
        const auto *mp = reinterpret_cast<const ChannelMigrationPacket *>(data);
        if (_channelMigrationCallback) {
            _channelMigrationCallback(mp->new_channel);
        }
        return;
    }
}

// ── Peer management ─────────────────────────────────────────────────────

bool ReceiverRadioLink::addPeer(const uint8_t mac[6]) {
    if (esp_now_is_peer_exist(mac))
        return true;

    esp_now_peer_info_t peer{};
    std::memcpy(peer.peer_addr, mac, 6);
    peer.channel = _wifiChannel;
    peer.encrypt = false;
    return esp_now_add_peer(&peer) == ESP_OK;
}

void ReceiverRadioLink::delPeer(const uint8_t mac[6]) {
    if (esp_now_is_peer_exist(mac)) {
        esp_now_del_peer(mac);
    }
}

} // namespace odh
