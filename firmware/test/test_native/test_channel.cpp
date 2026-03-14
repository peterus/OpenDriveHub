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
 * Native unit tests for the ODH channel / discovery module.
 *
 * Covers channel validation, timing constants, discovery packet
 * construction, and the ChannelScanner logic.
 */

#include <Channel.h>
#include <ChannelScanner.h>
#include <Protocol.h>
#include <cstdint>
#include <cstring>
#include <unity.h>

using namespace odh;

/* ── Channel validation ─────────────────────────────────────────────────── */

void test_channel_valid_1(void) {
    TEST_ASSERT_TRUE(channel::isValidChannel(1));
}

void test_channel_valid_6(void) {
    TEST_ASSERT_TRUE(channel::isValidChannel(6));
}

void test_channel_valid_11(void) {
    TEST_ASSERT_TRUE(channel::isValidChannel(11));
}

void test_channel_invalid_0(void) {
    TEST_ASSERT_FALSE(channel::isValidChannel(0));
}

void test_channel_invalid_2(void) {
    TEST_ASSERT_FALSE(channel::isValidChannel(2));
}

void test_channel_invalid_5(void) {
    TEST_ASSERT_FALSE(channel::isValidChannel(5));
}

void test_channel_invalid_7(void) {
    TEST_ASSERT_FALSE(channel::isValidChannel(7));
}

void test_channel_invalid_12(void) {
    TEST_ASSERT_FALSE(channel::isValidChannel(12));
}

void test_channel_invalid_13(void) {
    TEST_ASSERT_FALSE(channel::isValidChannel(13));
}

void test_channel_invalid_255(void) {
    TEST_ASSERT_FALSE(channel::isValidChannel(255));
}

/* ── Channel constants ──────────────────────────────────────────────────── */

void test_candidate_channel_count(void) {
    TEST_ASSERT_EQUAL_UINT8(3, channel::kCandidateChannelCount);
}

void test_candidate_channel_0_is_1(void) {
    TEST_ASSERT_EQUAL_UINT8(1, channel::kCandidateChannels[0]);
}

void test_candidate_channel_1_is_6(void) {
    TEST_ASSERT_EQUAL_UINT8(6, channel::kCandidateChannels[1]);
}

void test_candidate_channel_2_is_11(void) {
    TEST_ASSERT_EQUAL_UINT8(11, channel::kCandidateChannels[2]);
}

void test_default_channel(void) {
    TEST_ASSERT_EQUAL_UINT8(1, channel::kDefaultChannel);
}

/* ── Sim port mapping ───────────────────────────────────────────────────── */

void test_sim_port_channel_1(void) {
    TEST_ASSERT_EQUAL_UINT16(7001, channel::channelToSimPort(1));
}

void test_sim_port_channel_6(void) {
    TEST_ASSERT_EQUAL_UINT16(7006, channel::channelToSimPort(6));
}

void test_sim_port_channel_11(void) {
    TEST_ASSERT_EQUAL_UINT16(7011, channel::channelToSimPort(11));
}

void test_sim_port_invalid_fallback(void) {
    TEST_ASSERT_EQUAL_UINT16(7001, channel::channelToSimPort(3));
}

/* ── Timing constants sanity checks ─────────────────────────────────────── */

void test_channel_settle_positive(void) {
    TEST_ASSERT_GREATER_THAN(0u, channel::kChannelSettleMs);
}

void test_discovery_wait_positive(void) {
    TEST_ASSERT_GREATER_THAN(0u, channel::kDiscoveryWaitMs);
}

void test_discovery_retries_at_least_one(void) {
    TEST_ASSERT_GREATER_OR_EQUAL(1, channel::kDiscoveryRetries);
}

void test_presence_interval_positive(void) {
    TEST_ASSERT_GREATER_THAN(0u, channel::kPresenceIntervalMs);
}

void test_transmitter_loss_exceeds_presence(void) {
    TEST_ASSERT_GREATER_THAN(channel::kPresenceIntervalMs, channel::kTransmitterLossTimeoutMs);
}

void test_founding_backoff_max_exceeds_min(void) {
    TEST_ASSERT_GREATER_THAN(channel::kFoundingBackoffMinMs, channel::kFoundingBackoffMaxMs);
}

/* ── Discovery packet sizes ─────────────────────────────────────────────── */

void test_discovery_request_packet_size(void) {
    TEST_ASSERT_LESS_OR_EQUAL(250, static_cast<int>(sizeof(DiscoveryRequestPacket)));
}

void test_discovery_response_packet_size(void) {
    TEST_ASSERT_LESS_OR_EQUAL(250, static_cast<int>(sizeof(DiscoveryResponsePacket)));
}

void test_receiver_presence_packet_size(void) {
    TEST_ASSERT_LESS_OR_EQUAL(250, static_cast<int>(sizeof(ReceiverPresencePacket)));
}

void test_channel_migration_packet_size(void) {
    TEST_ASSERT_LESS_OR_EQUAL(250, static_cast<int>(sizeof(ChannelMigrationPacket)));
}

/* ── Discovery packet construction & checksum ───────────────────────────── */

void test_discovery_request_checksum(void) {
    DiscoveryRequestPacket pkt{};
    fillHeader(pkt, PacketType::DiscoveryRequest);
    pkt.mac[0] = 0xAA;
    pkt.mac[5] = 0xBB;
    pkt.role   = static_cast<uint8_t>(DeviceRole::Receiver);
    setChecksum(pkt);

    auto *raw      = reinterpret_cast<uint8_t *>(&pkt);
    uint8_t recomp = checksum(raw, sizeof(pkt) - 1);
    TEST_ASSERT_EQUAL_UINT8(pkt.checksum, recomp);
}

void test_discovery_response_checksum(void) {
    DiscoveryResponsePacket pkt{};
    fillHeader(pkt, PacketType::DiscoveryResponse);
    pkt.mac[0]       = 0x11;
    pkt.mac[5]       = 0x22;
    pkt.channel      = 6;
    pkt.device_count = 3;
    setChecksum(pkt);

    auto *raw      = reinterpret_cast<uint8_t *>(&pkt);
    uint8_t recomp = checksum(raw, sizeof(pkt) - 1);
    TEST_ASSERT_EQUAL_UINT8(pkt.checksum, recomp);
}

void test_receiver_presence_checksum(void) {
    ReceiverPresencePacket pkt{};
    fillHeader(pkt, PacketType::ReceiverPresence);
    pkt.mac[0]     = 0xCC;
    pkt.model_type = static_cast<uint8_t>(ModelType::DumpTruck);
    std::strncpy(pkt.name, "TestRx", kVehicleNameMax - 1);
    setChecksum(pkt);

    auto *raw      = reinterpret_cast<uint8_t *>(&pkt);
    uint8_t recomp = checksum(raw, sizeof(pkt) - 1);
    TEST_ASSERT_EQUAL_UINT8(pkt.checksum, recomp);
}

void test_channel_migration_checksum(void) {
    ChannelMigrationPacket pkt{};
    fillHeader(pkt, PacketType::ChannelMigration);
    pkt.mac[0]      = 0xDD;
    pkt.mac[5]      = 0xEE;
    pkt.new_channel = 11;
    setChecksum(pkt);

    auto *raw      = reinterpret_cast<uint8_t *>(&pkt);
    uint8_t recomp = checksum(raw, sizeof(pkt) - 1);
    TEST_ASSERT_EQUAL_UINT8(pkt.checksum, recomp);
}

/* ── ChannelScanner basic tests ─────────────────────────────────────────── */

void test_scanner_no_response(void) {
    // Mock callbacks: setChannel and sendDiscovery succeed, delay is a no-op
    auto setChannel    = [](uint8_t) { return true; };
    auto sendDiscovery = [](uint8_t) { return true; };
    auto delayMs       = [](uint32_t) {};

    ChannelScanner scanner(setChannel, sendDiscovery, delayMs);
    ScanResult result = scanner.scanChannel(1);

    TEST_ASSERT_EQUAL_UINT8(1, result.channel);
    TEST_ASSERT_FALSE(result.foundTransmitter);
}

void test_scanner_with_response(void) {
    ChannelScanner *scannerPtr = nullptr;

    auto setChannel    = [](uint8_t) { return true; };
    auto sendDiscovery = [&scannerPtr](uint8_t ch) {
        // Simulate receiving a discovery response during the scan
        if (scannerPtr) {
            scannerPtr->onDiscoveryResponse(ch, -45, 2);
        }
        return true;
    };
    auto delayMs = [](uint32_t) {};

    ChannelScanner scanner(setChannel, sendDiscovery, delayMs);
    scannerPtr = &scanner;

    ScanResult result = scanner.scanChannel(6);

    TEST_ASSERT_EQUAL_UINT8(6, result.channel);
    TEST_ASSERT_TRUE(result.foundTransmitter);
    TEST_ASSERT_EQUAL_INT8(-45, result.rssi);
    TEST_ASSERT_EQUAL_UINT8(2, result.deviceCount);
}

void test_scanner_best_channel_with_transmitter(void) {
    ScanResult results[channel::kCandidateChannelCount];

    results[0].channel          = 1;
    results[0].foundTransmitter = false;
    results[0].rssi             = -127;

    results[1].channel          = 6;
    results[1].foundTransmitter = true;
    results[1].rssi             = -50;

    results[2].channel          = 11;
    results[2].foundTransmitter = true;
    results[2].rssi             = -30;

    // Should pick channel 11 (best RSSI among transmitter channels)
    TEST_ASSERT_EQUAL_UINT8(11, ChannelScanner::bestChannel(results));
}

void test_scanner_best_channel_no_transmitter(void) {
    ScanResult results[channel::kCandidateChannelCount];

    results[0].channel          = 1;
    results[0].foundTransmitter = false;

    results[1].channel          = 6;
    results[1].foundTransmitter = false;

    results[2].channel          = 11;
    results[2].foundTransmitter = false;

    // No transmitter → returns first candidate channel
    TEST_ASSERT_EQUAL_UINT8(1, ChannelScanner::bestChannel(results));
}
