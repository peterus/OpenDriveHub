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
 * sim_espnow.cpp – ESP-NOW over UDP multicast sockets.
 *
 * Each WiFi channel maps to a UDP multicast group and port:
 *   channel 1  → 239.0.0.1:7001
 *   channel 6  → 239.0.0.6:7006
 *   channel 11 → 239.0.0.11:7011
 *
 * Using multicast ensures that every process on the same channel receives
 * every packet (fan-out), unlike SO_REUSEPORT unicast which only delivers
 * to one socket.  The receive thread is stopped and restarted whenever the
 * channel changes so the socket is never accessed concurrently with rebind.
 *
 * Packets include a 6-byte source MAC header prepended to the original
 * payload so the receive callback gets the sender's address.
 * Self-packets are filtered by comparing the source MAC.
 */

#include "Arduino.h"
#include "Channel.h"
#include "WiFi.h"
#include "esp_now.h"

#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

/* ── Internal state ─────────────────────────────────────────────────────── */

static int s_sock                 = -1;
static esp_now_recv_cb_t s_recvCb = nullptr;
static esp_now_send_cb_t s_sendCb = nullptr;
static pthread_t s_recvThread;
static bool s_running = false;

struct PeerEntry {
    uint8_t mac[6];
    uint8_t channel;
};
static std::vector<PeerEntry> s_peers;
static std::mutex s_peerMutex;

static uint8_t s_localMac[6]    = {};
static uint8_t s_currentChannel = 1;

/// Per-channel simulated RSSI values (indexed by channel number).
/// Default is -50 dBm for all channels.
static int8_t s_channelRssi[16] = {-50, -50, -50, -50, -50, -50, -50, -50, -50, -50, -50, -50, -50, -50, -50, -50};

static wifi_promiscuous_cb_t s_promiscCb = nullptr;
static bool s_promiscEnabled             = false;

/* ── Multicast helper ───────────────────────────────────────────────────── */

/// Returns the multicast group IPv4 address for the given channel in host
/// byte order: channel ch → 239.0.0.ch.
/// Valid input channels are 1, 6, and 11 (see channel::kCandidateChannels).
static inline uint32_t channelMulticastIp(uint8_t ch) {
    return (239u << 24) | static_cast<uint32_t>(ch);
}

/* ── Socket helper ──────────────────────────────────────────────────────── */

static void rebindSocket() {
    if (s_sock >= 0) {
        close(s_sock);
        s_sock = -1;
    }

    s_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (s_sock < 0) {
        perror("[SIM] socket");
        return;
    }

    int opt = 1;
    setsockopt(s_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    uint16_t port = odh::channel::channelToSimPort(s_currentChannel);

    // Bind to INADDR_ANY so multicast packets addressed to the group are received.
    struct sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s_sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        perror("[SIM] bind");
        close(s_sock);
        s_sock = -1;
        return;
    }

    // Join the multicast group for this channel on the loopback interface.
    struct ip_mreq mreq{};
    mreq.imr_multiaddr.s_addr = htonl(channelMulticastIp(s_currentChannel));
    mreq.imr_interface.s_addr = htonl(INADDR_LOOPBACK);
    if (setsockopt(s_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("[SIM] IP_ADD_MEMBERSHIP");
    }

    // Ensure multicast packets are sent via the loopback interface.
    struct in_addr iface{};
    iface.s_addr = htonl(INADDR_LOOPBACK);
    setsockopt(s_sock, IPPROTO_IP, IP_MULTICAST_IF, &iface, sizeof(iface));

    struct timeval tv{.tv_sec = 0, .tv_usec = 100000};
    setsockopt(s_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    printf("[SIM] Channel %u → UDP multicast 239.0.0.%u:%u\n", s_currentChannel, s_currentChannel, port);
}

/* ── Listener thread ────────────────────────────────────────────────────── */

static void *recvLoop(void *) {
    uint8_t buf[6 + ESP_NOW_MAX_DATA_LEN]; /* [mac(6)] [payload] */
    while (s_running) {
        ssize_t n = recvfrom(s_sock, buf, sizeof(buf), 0, nullptr, nullptr);
        if (n > 6 && s_recvCb) {
            if (memcmp(buf, s_localMac, 6) == 0)
                continue;
            // Fire the promiscuous callback before the ESP-NOW callback,
            // exactly mirroring real ESP32 behaviour.  This lets the
            // radio layer read RSSI via the normal promiscRxCb path.
            if (s_promiscEnabled && s_promiscCb) {
                wifi_promiscuous_pkt_t pkt{};
                pkt.rx_ctrl.rssi = s_channelRssi[s_currentChannel];
                s_promiscCb(&pkt, WIFI_PKT_DATA);
            }
            s_recvCb(buf, buf + 6, static_cast<int>(n - 6));
        }
    }
    return nullptr;
}

/* ── ESP-NOW API ────────────────────────────────────────────────────────── */

int esp_now_init() {
    esp_read_mac(s_localMac, ESP_MAC_WIFI_STA);

    rebindSocket();
    if (s_sock < 0)
        return ESP_FAIL;

    /* Start the listener thread. */
    s_running = true;
    pthread_create(&s_recvThread, nullptr, recvLoop, nullptr);

    return ESP_OK;
}

int esp_now_deinit() {
    s_running = false;
    pthread_join(s_recvThread, nullptr);
    if (s_sock >= 0) {
        close(s_sock);
        s_sock = -1;
    }
    return ESP_OK;
}

int esp_now_register_recv_cb(esp_now_recv_cb_t cb) {
    s_recvCb = cb;
    return ESP_OK;
}

int esp_now_register_send_cb(esp_now_send_cb_t cb) {
    s_sendCb = cb;
    return ESP_OK;
}

int esp_now_send(const uint8_t *peer_addr, const uint8_t *data, int len) {
    if (s_sock < 0 || len <= 0)
        return ESP_FAIL;

    /* Prepend our MAC address to the payload. */
    uint8_t buf[6 + ESP_NOW_MAX_DATA_LEN];
    memcpy(buf, s_localMac, 6);
    int copyLen = len < ESP_NOW_MAX_DATA_LEN ? len : ESP_NOW_MAX_DATA_LEN;
    memcpy(buf + 6, data, copyLen);

    // Send to the multicast group for the current channel.
    struct sockaddr_in dest{};
    dest.sin_family      = AF_INET;
    dest.sin_port        = htons(odh::channel::channelToSimPort(s_currentChannel));
    dest.sin_addr.s_addr = htonl(channelMulticastIp(s_currentChannel));

    ssize_t sent = sendto(s_sock, buf, 6 + copyLen, 0, reinterpret_cast<sockaddr *>(&dest), sizeof(dest));

    esp_now_send_status_t status = (sent > 0) ? ESP_NOW_SEND_SUCCESS : ESP_NOW_SEND_FAIL;

    if (s_sendCb) {
        s_sendCb(peer_addr, status);
    }

    return (sent > 0) ? ESP_OK : ESP_FAIL;
}

int esp_now_add_peer(const esp_now_peer_info_t *peer) {
    if (!peer)
        return ESP_FAIL;
    std::lock_guard<std::mutex> lock(s_peerMutex);
    /* Don't add duplicates. */
    for (auto &p : s_peers) {
        if (memcmp(p.mac, peer->peer_addr, 6) == 0)
            return ESP_OK;
    }
    PeerEntry e;
    memcpy(e.mac, peer->peer_addr, 6);
    e.channel = peer->channel;
    s_peers.push_back(e);
    return ESP_OK;
}

int esp_now_del_peer(const uint8_t *peer_addr) {
    std::lock_guard<std::mutex> lock(s_peerMutex);
    for (auto it = s_peers.begin(); it != s_peers.end(); ++it) {
        if (memcmp(it->mac, peer_addr, 6) == 0) {
            s_peers.erase(it);
            return ESP_OK;
        }
    }
    return ESP_FAIL;
}

bool esp_now_is_peer_exist(const uint8_t *peer_addr) {
    std::lock_guard<std::mutex> lock(s_peerMutex);
    for (auto &p : s_peers) {
        if (memcmp(p.mac, peer_addr, 6) == 0)
            return true;
    }
    return false;
}

/* ── Channel switching ──────────────────────────────────────────────────── */

void sim_set_wifi_channel(uint8_t channel) {
    if (s_currentChannel == channel)
        return;

    // If the recv thread is running, stop it before touching the socket.
    // pthread_join guarantees the thread has exited before we rebind.
    const bool wasRunning = s_running;
    if (wasRunning) {
        s_running = false;
        if (pthread_join(s_recvThread, nullptr) != 0) {
            perror("[SIM] pthread_join failed during channel switch");
        }
    }

    s_currentChannel = channel;

    if (wasRunning) {
        rebindSocket();
        if (s_sock >= 0) {
            s_running = true;
            pthread_create(&s_recvThread, nullptr, recvLoop, nullptr);
        } else {
            fprintf(stderr, "[SIM] rebindSocket failed during channel switch to %u\n", channel);
        }
    }
}

uint8_t sim_get_wifi_channel() {
    return s_currentChannel;
}

void sim_set_channel_rssi(uint8_t channel, int8_t rssi) {
    if (channel < 16) {
        s_channelRssi[channel] = rssi;
    }
}

/* ── esp_wifi API shim ───────────────────────────────────────────────────── */

void esp_wifi_set_channel(uint8_t channel, wifi_second_chan_t /*secondary*/) {
    sim_set_wifi_channel(channel);
}

void esp_wifi_set_promiscuous(bool enable) {
    s_promiscEnabled = enable;
}

void esp_wifi_set_promiscuous_rx_cb(wifi_promiscuous_cb_t cb) {
    s_promiscCb = cb;
}
