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
 * Native unit tests for the ODH protocol utilities.
 *
 * Tests run on the host using PlatformIO's Unity framework.
 * No hardware is required.
 */

#include <cstdint>
#include <cstring>
#include <unity.h>

/* Include the modernised protocol header. */
#include <Protocol.h>

using namespace odh;

/* ── checksum ────────────────────────────────────────────────────────────── */

void test_checksum_empty(void) {
    uint8_t data[] = {0}; // dummy element, length 0
    TEST_ASSERT_EQUAL_UINT8(0, checksum(data, 0));
}

void test_checksum_single_byte(void) {
    uint8_t data[] = {0xAB};
    TEST_ASSERT_EQUAL_UINT8(0xAB, checksum(data, 1));
}

void test_checksum_xor_properties(void) {
    uint8_t data[] = {0x42, 0x42};
    TEST_ASSERT_EQUAL_UINT8(0x00, checksum(data, 2));
}

void test_checksum_known_vector(void) {
    uint8_t data[]   = {kMagic0, kMagic1, kProtocolVersion, static_cast<uint8_t>(PacketType::Control)};
    uint8_t expected = kMagic0 ^ kMagic1 ^ kProtocolVersion ^ static_cast<uint8_t>(PacketType::Control);
    TEST_ASSERT_EQUAL_UINT8(expected, checksum(data, 4));
}

/* ── Control packet round-trip ───────────────────────────────────────────── */

void test_control_packet_magic(void) {
    ControlPacket pkt{};
    pkt.magic[0] = kMagic0;
    pkt.magic[1] = kMagic1;
    TEST_ASSERT_EQUAL_UINT8(kMagic0, pkt.magic[0]);
    TEST_ASSERT_EQUAL_UINT8(kMagic1, pkt.magic[1]);
}

void test_control_packet_channel_bounds(void) {
    TEST_ASSERT_LESS_THAN(kChannelMax, kChannelMid);
    TEST_ASSERT_GREATER_THAN(kChannelMin, kChannelMid);
}

void test_control_packet_checksum_verify(void) {
    ControlPacket pkt{};
    fillHeader(pkt, PacketType::Control);
    pkt.sequence       = 42;
    pkt.function_count = 4;
    for (uint8_t i = 0; i < 4; i++) {
        pkt.functions[i].function = i;
        pkt.functions[i].value    = kChannelMid;
        pkt.functions[i].trim     = 0;
    }
    pkt.flags = 0;
    setChecksum(pkt);

    auto *raw      = reinterpret_cast<uint8_t *>(&pkt);
    uint8_t recomp = checksum(raw, sizeof(pkt) - 1);
    TEST_ASSERT_EQUAL_UINT8(pkt.checksum, recomp);
}

void test_control_packet_checksum_detects_corruption(void) {
    ControlPacket pkt{};
    fillHeader(pkt, PacketType::Control);
    setChecksum(pkt);

    auto *raw = reinterpret_cast<uint8_t *>(&pkt);
    raw[4] ^= 0x01; // flip a bit

    uint8_t recomp = checksum(raw, sizeof(pkt) - 1);
    TEST_ASSERT_NOT_EQUAL(pkt.checksum, recomp);
}

/* ── Packet sizes ────────────────────────────────────────────────────────── */

void test_telemetry_packet_size(void) {
    TEST_ASSERT_LESS_OR_EQUAL(250, static_cast<int>(sizeof(TelemetryPacket)));
}

void test_control_packet_size(void) {
    TEST_ASSERT_LESS_OR_EQUAL(250, static_cast<int>(sizeof(ControlPacket)));
}

void test_bind_packet_size(void) {
    TEST_ASSERT_LESS_OR_EQUAL(250, static_cast<int>(sizeof(BindPacket)));
}

void test_announce_packet_size(void) {
    TEST_ASSERT_LESS_OR_EQUAL(250, static_cast<int>(sizeof(ReceiverPresencePacket)));
}

/* ── ReceiverPresence packet ─────────────────────────────────────────────── */

void test_announce_packet_fields(void) {
    ReceiverPresencePacket pkt{};
    fillHeader(pkt, PacketType::ReceiverPresence);
    pkt.model_type = static_cast<uint8_t>(ModelType::Excavator);

    const char *name = "TestVehicle";
    std::memcpy(pkt.name, name, std::strlen(name) + 1);

    TEST_ASSERT_EQUAL_UINT8(kMagic0, pkt.magic[0]);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PacketType::ReceiverPresence), pkt.type);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(ModelType::Excavator), pkt.model_type);
    TEST_ASSERT_EQUAL_STRING("TestVehicle", pkt.name);
}

void test_announce_packet_checksum(void) {
    ReceiverPresencePacket pkt{};
    fillHeader(pkt, PacketType::ReceiverPresence);
    pkt.model_type = static_cast<uint8_t>(ModelType::Crane);
    std::strncpy(pkt.name, "MyCrane", kVehicleNameMax - 1);
    pkt.mac[0] = 0xAA;
    pkt.mac[5] = 0xBB;
    setChecksum(pkt);

    auto *raw      = reinterpret_cast<uint8_t *>(&pkt);
    uint8_t recomp = checksum(raw, sizeof(pkt) - 1);
    TEST_ASSERT_EQUAL_UINT8(pkt.checksum, recomp);
}

void test_announce_packet_checksum_detects_corruption(void) {
    ReceiverPresencePacket pkt{};
    fillHeader(pkt, PacketType::ReceiverPresence);
    pkt.model_type = static_cast<uint8_t>(ModelType::Generic);
    setChecksum(pkt);

    auto *raw = reinterpret_cast<uint8_t *>(&pkt);
    raw[12] ^= 0x01;
    uint8_t recomp = checksum(raw, sizeof(pkt) - 1);
    TEST_ASSERT_NOT_EQUAL(pkt.checksum, recomp);
}

void test_announce_name_max_length(void) {
    TEST_ASSERT_EQUAL(16, kVehicleNameMax);
}

/* ── Disconnect packet type ──────────────────────────────────────────────── */

void test_disconnect_packet_type(void) {
    TEST_ASSERT_NOT_EQUAL(static_cast<uint8_t>(PacketType::Bind), static_cast<uint8_t>(PacketType::Disconnect));
    TEST_ASSERT_EQUAL_UINT8(0x40, static_cast<uint8_t>(PacketType::Disconnect));
}

void test_disconnect_uses_bind_struct(void) {
    BindPacket pkt{};
    fillHeader(pkt, PacketType::Disconnect);
    pkt.mac[0] = 0x11;
    setChecksum(pkt);

    auto *raw      = reinterpret_cast<uint8_t *>(&pkt);
    uint8_t recomp = checksum(raw, sizeof(pkt) - 1);
    TEST_ASSERT_EQUAL_UINT8(pkt.checksum, recomp);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PacketType::Disconnect), pkt.type);
}

/* ── Scanning link state ─────────────────────────────────────────────────── */

void test_link_state_scanning_value(void) {
    TEST_ASSERT_EQUAL(4, static_cast<uint8_t>(LinkState::Scanning));
}

void test_link_state_scanning_distinct(void) {
    TEST_ASSERT_NOT_EQUAL(static_cast<uint8_t>(LinkState::Disconnected), static_cast<uint8_t>(LinkState::Scanning));
    TEST_ASSERT_NOT_EQUAL(static_cast<uint8_t>(LinkState::Binding), static_cast<uint8_t>(LinkState::Scanning));
    TEST_ASSERT_NOT_EQUAL(static_cast<uint8_t>(LinkState::Connected), static_cast<uint8_t>(LinkState::Scanning));
    TEST_ASSERT_NOT_EQUAL(static_cast<uint8_t>(LinkState::Failsafe), static_cast<uint8_t>(LinkState::Scanning));
}

void test_max_discovered_constant(void) {
    TEST_ASSERT_EQUAL(8, kMaxDiscovered);
}

/* ── Channel value mapping helpers ──────────────────────────────────────── */

static uint16_t pot_map(int32_t raw, int32_t adcMax) {
    if (raw < 0)
        raw = 0;
    if (raw > adcMax)
        raw = adcMax;
    uint32_t mapped = kChannelMin + (static_cast<uint32_t>(raw) * (kChannelMax - kChannelMin)) / static_cast<uint32_t>(adcMax);
    return static_cast<uint16_t>(mapped);
}

void test_pot_map_min(void) {
    TEST_ASSERT_EQUAL_UINT16(kChannelMin, pot_map(0, 32767));
}

void test_pot_map_max(void) {
    TEST_ASSERT_EQUAL_UINT16(kChannelMax, pot_map(32767, 32767));
}

void test_pot_map_mid(void) {
    uint16_t result = pot_map(16383, 32767);
    TEST_ASSERT_UINT16_WITHIN(2, kChannelMid, result);
}

void test_pot_map_clamps_negative(void) {
    TEST_ASSERT_EQUAL_UINT16(kChannelMin, pot_map(-100, 32767));
}

void test_pot_map_clamps_overflow(void) {
    TEST_ASSERT_EQUAL_UINT16(kChannelMax, pot_map(32768, 32767));
}

static uint16_t enc_map(uint16_t raw) {
    if (raw > 4095)
        raw = 4095;
    uint32_t mapped = kChannelMin + (static_cast<uint32_t>(raw) * (kChannelMax - kChannelMin)) / 4095u;
    return static_cast<uint16_t>(mapped);
}

void test_encoder_map_min(void) {
    TEST_ASSERT_EQUAL_UINT16(kChannelMin, enc_map(0));
}

void test_encoder_map_max(void) {
    TEST_ASSERT_EQUAL_UINT16(kChannelMax, enc_map(4095));
}

/* ── Switch / button channel logic ──────────────────────────────────────── */

void test_switch_active_low(void) {
    uint8_t raw = 0xFE;
    uint16_t ch = ((raw & 0x01) == 0) ? kChannelMax : kChannelMin;
    TEST_ASSERT_EQUAL_UINT16(kChannelMax, ch);
}

void test_switch_open(void) {
    uint8_t raw = 0xFF;
    uint16_t ch = ((raw & 0x01) == 0) ? kChannelMax : kChannelMin;
    TEST_ASSERT_EQUAL_UINT16(kChannelMin, ch);
}

/* ── Runner ──────────────────────────────────────────────────────────────── */

/* Forward-declarations – display utility tests (test_display.cpp). */
extern void test_bar_permille_min(void);
extern void test_bar_permille_max(void);
extern void test_bar_permille_mid(void);
extern void test_bar_permille_clamps_below_min(void);
extern void test_bar_permille_clamps_above_max(void);
extern void test_bar_permille_quarter(void);
extern void test_bar_permille_three_quarter(void);
extern void test_permille_to_px_zero(void);
extern void test_permille_to_px_full(void);
extern void test_permille_to_px_half(void);
extern void test_permille_to_px_clamps_negative(void);
extern void test_permille_to_px_clamps_over_1000(void);
extern void test_link_state_text_connected(void);
extern void test_link_state_text_binding(void);
extern void test_link_state_text_failsafe(void);
extern void test_link_state_text_disconnected(void);
extern void test_link_state_text_unknown(void);
extern void test_link_state_text_scanning(void);
extern void test_batt_percent_empty(void);
extern void test_batt_percent_full(void);
extern void test_batt_percent_half(void);
extern void test_batt_percent_quarter(void);
extern void test_batt_cell_percent_full(void);
extern void test_batt_cell_percent_empty(void);
extern void test_batt_cell_percent_mid(void);
extern void test_batt_cell_percent_zero_input(void);
extern void test_channel_to_bar_px_min(void);
extern void test_channel_to_bar_px_max(void);
extern void test_channel_to_bar_px_mid(void);
extern void test_batt_warn_above_3500(void);
extern void test_batt_warn_at_3500(void);
extern void test_batt_crit_at_3300(void);
extern void test_batt_crit_below_3300(void);
extern void test_batt_cell_detect_zero(void);
extern void test_batt_cell_detect_1s(void);
extern void test_batt_cell_detect_2s(void);
extern void test_batt_cell_detect_3s(void);
extern void test_batt_cell_detect_4s(void);
extern void test_rssi_to_bars_excellent(void);
extern void test_rssi_to_bars_good(void);
extern void test_rssi_to_bars_fair(void);
extern void test_rssi_to_bars_weak(void);
extern void test_rssi_to_bars_none(void);

/* Forward-declarations – function map tests (test_function_map.cpp). */
extern void test_function_map_entry_size(void);
extern void test_telemetry_packet_has_model_type(void);
extern void test_generic_model_has_drive_and_steering(void);
extern void test_dump_truck_has_dump_bed(void);
extern void test_excavator_has_six_functions(void);
extern void test_tractor_has_four_functions(void);
extern void test_crane_has_five_functions(void);
extern void test_unknown_model_falls_back_to_generic(void);
extern void test_function_to_channel_found(void);
extern void test_function_to_channel_not_found(void);
extern void test_channel_to_function_found(void);
extern void test_channel_to_function_not_found(void);
extern void test_function_name_drive(void);
extern void test_function_name_unknown(void);
extern void test_model_name_excavator(void);
extern void test_model_name_unknown(void);
extern void test_channel_assignments_unique(void);

/* Forward-declarations – channel / discovery tests (test_channel.cpp). */
extern void test_channel_valid_1(void);
extern void test_channel_valid_6(void);
extern void test_channel_valid_11(void);
extern void test_channel_invalid_0(void);
extern void test_channel_invalid_2(void);
extern void test_channel_invalid_5(void);
extern void test_channel_invalid_7(void);
extern void test_channel_invalid_12(void);
extern void test_channel_invalid_13(void);
extern void test_channel_invalid_255(void);
extern void test_candidate_channel_count(void);
extern void test_candidate_channel_0_is_1(void);
extern void test_candidate_channel_1_is_6(void);
extern void test_candidate_channel_2_is_11(void);
extern void test_default_channel(void);
extern void test_sim_port_channel_1(void);
extern void test_sim_port_channel_6(void);
extern void test_sim_port_channel_11(void);
extern void test_sim_port_invalid_fallback(void);
extern void test_channel_settle_positive(void);
extern void test_discovery_wait_positive(void);
extern void test_discovery_retries_at_least_one(void);
extern void test_presence_interval_positive(void);
extern void test_transmitter_loss_exceeds_presence(void);
extern void test_founding_backoff_max_exceeds_min(void);
extern void test_discovery_request_packet_size(void);
extern void test_discovery_response_packet_size(void);
extern void test_receiver_presence_packet_size(void);
extern void test_channel_migration_packet_size(void);
extern void test_discovery_request_checksum(void);
extern void test_discovery_response_checksum(void);
extern void test_receiver_presence_checksum(void);
extern void test_channel_migration_checksum(void);
extern void test_scanner_no_response(void);
extern void test_scanner_with_response(void);
extern void test_scanner_best_channel_with_transmitter(void);
extern void test_scanner_best_channel_no_transmitter(void);
/* Shell tokenizer tests (test_shell.cpp) */
extern void test_tokenize_single_word(void);
extern void test_tokenize_two_words(void);
extern void test_tokenize_three_args(void);
extern void test_tokenize_leading_spaces(void);
extern void test_tokenize_trailing_spaces(void);
extern void test_tokenize_multiple_spaces(void);
extern void test_tokenize_tabs(void);
extern void test_tokenize_empty_string(void);
extern void test_tokenize_only_spaces(void);
extern void test_tokenize_max_args_limit(void);
extern void test_tokenize_quoted_string(void);
extern void test_tokenize_quoted_with_extra_args(void);
extern void test_tokenize_empty_quotes(void);

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    UNITY_BEGIN();

    /* Protocol tests */
    RUN_TEST(test_checksum_empty);
    RUN_TEST(test_checksum_single_byte);
    RUN_TEST(test_checksum_xor_properties);
    RUN_TEST(test_checksum_known_vector);

    RUN_TEST(test_control_packet_magic);
    RUN_TEST(test_control_packet_channel_bounds);
    RUN_TEST(test_control_packet_checksum_verify);
    RUN_TEST(test_control_packet_checksum_detects_corruption);

    RUN_TEST(test_telemetry_packet_size);
    RUN_TEST(test_control_packet_size);
    RUN_TEST(test_bind_packet_size);
    RUN_TEST(test_announce_packet_size);
    RUN_TEST(test_announce_packet_fields);
    RUN_TEST(test_announce_packet_checksum);
    RUN_TEST(test_announce_packet_checksum_detects_corruption);
    RUN_TEST(test_announce_name_max_length);
    RUN_TEST(test_disconnect_packet_type);
    RUN_TEST(test_disconnect_uses_bind_struct);
    RUN_TEST(test_link_state_scanning_value);
    RUN_TEST(test_link_state_scanning_distinct);
    RUN_TEST(test_max_discovered_constant);

    RUN_TEST(test_pot_map_min);
    RUN_TEST(test_pot_map_max);
    RUN_TEST(test_pot_map_mid);
    RUN_TEST(test_pot_map_clamps_negative);
    RUN_TEST(test_pot_map_clamps_overflow);

    RUN_TEST(test_encoder_map_min);
    RUN_TEST(test_encoder_map_max);

    RUN_TEST(test_switch_active_low);
    RUN_TEST(test_switch_open);

    /* Display utility tests */
    RUN_TEST(test_bar_permille_min);
    RUN_TEST(test_bar_permille_max);
    RUN_TEST(test_bar_permille_mid);
    RUN_TEST(test_bar_permille_clamps_below_min);
    RUN_TEST(test_bar_permille_clamps_above_max);
    RUN_TEST(test_bar_permille_quarter);
    RUN_TEST(test_bar_permille_three_quarter);

    RUN_TEST(test_permille_to_px_zero);
    RUN_TEST(test_permille_to_px_full);
    RUN_TEST(test_permille_to_px_half);
    RUN_TEST(test_permille_to_px_clamps_negative);
    RUN_TEST(test_permille_to_px_clamps_over_1000);

    RUN_TEST(test_link_state_text_connected);
    RUN_TEST(test_link_state_text_binding);
    RUN_TEST(test_link_state_text_failsafe);
    RUN_TEST(test_link_state_text_disconnected);
    RUN_TEST(test_link_state_text_unknown);
    RUN_TEST(test_link_state_text_scanning);

    RUN_TEST(test_batt_percent_empty);
    RUN_TEST(test_batt_percent_full);
    RUN_TEST(test_batt_percent_half);
    RUN_TEST(test_batt_percent_quarter);

    RUN_TEST(test_batt_cell_percent_full);
    RUN_TEST(test_batt_cell_percent_empty);
    RUN_TEST(test_batt_cell_percent_mid);
    RUN_TEST(test_batt_cell_percent_zero_input);

    RUN_TEST(test_channel_to_bar_px_min);
    RUN_TEST(test_channel_to_bar_px_max);
    RUN_TEST(test_channel_to_bar_px_mid);

    /* Battery threshold tests */
    RUN_TEST(test_batt_warn_above_3500);
    RUN_TEST(test_batt_warn_at_3500);
    RUN_TEST(test_batt_crit_at_3300);
    RUN_TEST(test_batt_crit_below_3300);
    RUN_TEST(test_batt_cell_detect_zero);
    RUN_TEST(test_batt_cell_detect_1s);
    RUN_TEST(test_batt_cell_detect_2s);
    RUN_TEST(test_batt_cell_detect_3s);
    RUN_TEST(test_batt_cell_detect_4s);

    /* RSSI bar mapping tests */
    RUN_TEST(test_rssi_to_bars_excellent);
    RUN_TEST(test_rssi_to_bars_good);
    RUN_TEST(test_rssi_to_bars_fair);
    RUN_TEST(test_rssi_to_bars_weak);
    RUN_TEST(test_rssi_to_bars_none);

    /* Function map tests */
    RUN_TEST(test_function_map_entry_size);
    RUN_TEST(test_telemetry_packet_has_model_type);
    RUN_TEST(test_generic_model_has_drive_and_steering);
    RUN_TEST(test_dump_truck_has_dump_bed);
    RUN_TEST(test_excavator_has_six_functions);
    RUN_TEST(test_tractor_has_four_functions);
    RUN_TEST(test_crane_has_five_functions);
    RUN_TEST(test_unknown_model_falls_back_to_generic);
    RUN_TEST(test_function_to_channel_found);
    RUN_TEST(test_function_to_channel_not_found);
    RUN_TEST(test_channel_to_function_found);
    RUN_TEST(test_channel_to_function_not_found);
    RUN_TEST(test_function_name_drive);
    RUN_TEST(test_function_name_unknown);
    RUN_TEST(test_model_name_excavator);
    RUN_TEST(test_model_name_unknown);
    RUN_TEST(test_channel_assignments_unique);

    /* Channel / discovery tests */
    RUN_TEST(test_channel_valid_1);
    RUN_TEST(test_channel_valid_6);
    RUN_TEST(test_channel_valid_11);
    RUN_TEST(test_channel_invalid_0);
    RUN_TEST(test_channel_invalid_2);
    RUN_TEST(test_channel_invalid_5);
    RUN_TEST(test_channel_invalid_7);
    RUN_TEST(test_channel_invalid_12);
    RUN_TEST(test_channel_invalid_13);
    RUN_TEST(test_channel_invalid_255);

    RUN_TEST(test_candidate_channel_count);
    RUN_TEST(test_candidate_channel_0_is_1);
    RUN_TEST(test_candidate_channel_1_is_6);
    RUN_TEST(test_candidate_channel_2_is_11);
    RUN_TEST(test_default_channel);

    RUN_TEST(test_sim_port_channel_1);
    RUN_TEST(test_sim_port_channel_6);
    RUN_TEST(test_sim_port_channel_11);
    RUN_TEST(test_sim_port_invalid_fallback);

    RUN_TEST(test_channel_settle_positive);
    RUN_TEST(test_discovery_wait_positive);
    RUN_TEST(test_discovery_retries_at_least_one);
    RUN_TEST(test_presence_interval_positive);
    RUN_TEST(test_transmitter_loss_exceeds_presence);
    RUN_TEST(test_founding_backoff_max_exceeds_min);

    RUN_TEST(test_discovery_request_packet_size);
    RUN_TEST(test_discovery_response_packet_size);
    RUN_TEST(test_receiver_presence_packet_size);
    RUN_TEST(test_channel_migration_packet_size);

    RUN_TEST(test_discovery_request_checksum);
    RUN_TEST(test_discovery_response_checksum);
    RUN_TEST(test_receiver_presence_checksum);
    RUN_TEST(test_channel_migration_checksum);

    RUN_TEST(test_scanner_no_response);
    RUN_TEST(test_scanner_with_response);
    RUN_TEST(test_scanner_best_channel_with_transmitter);
    RUN_TEST(test_scanner_best_channel_no_transmitter);
    /* Shell tokenizer tests */
    RUN_TEST(test_tokenize_single_word);
    RUN_TEST(test_tokenize_two_words);
    RUN_TEST(test_tokenize_three_args);
    RUN_TEST(test_tokenize_leading_spaces);
    RUN_TEST(test_tokenize_trailing_spaces);
    RUN_TEST(test_tokenize_multiple_spaces);
    RUN_TEST(test_tokenize_tabs);
    RUN_TEST(test_tokenize_empty_string);
    RUN_TEST(test_tokenize_only_spaces);
    RUN_TEST(test_tokenize_max_args_limit);
    RUN_TEST(test_tokenize_quoted_string);
    RUN_TEST(test_tokenize_quoted_with_extra_args);
    RUN_TEST(test_tokenize_empty_quotes);

    return UNITY_END();
}
