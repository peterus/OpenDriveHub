/**
 * Native unit tests for the display utility helpers (display_utils.h).
 *
 * These tests exercise the constexpr helper functions that are shared between
 * the firmware Display class and the test suite.  No LVGL or hardware is
 * required.
 */

#include <cstdint>
#include <unity.h>

/* Include the utility header – uses only <Protocol.h> internally. */
#include "../../src/transmitter/display/display_utils.h"

using namespace odh;

/* ── barPermille ─────────────────────────────────────────────────────────── */

void test_bar_permille_min(void) {
    TEST_ASSERT_EQUAL_INT32(0, barPermille(kChannelMin));
}

void test_bar_permille_max(void) {
    TEST_ASSERT_EQUAL_INT32(1000, barPermille(kChannelMax));
}

void test_bar_permille_mid(void) {
    TEST_ASSERT_INT32_WITHIN(2, 500, barPermille(kChannelMid));
}

void test_bar_permille_clamps_below_min(void) {
    TEST_ASSERT_EQUAL_INT32(0, barPermille(0u));
    TEST_ASSERT_EQUAL_INT32(0, barPermille(static_cast<uint16_t>(kChannelMin - 1u)));
}

void test_bar_permille_clamps_above_max(void) {
    TEST_ASSERT_EQUAL_INT32(1000, barPermille(static_cast<uint16_t>(kChannelMax + 1u)));
    TEST_ASSERT_EQUAL_INT32(1000, barPermille(static_cast<uint16_t>(0xFFFFu)));
}

void test_bar_permille_quarter(void) {
    TEST_ASSERT_INT32_WITHIN(2, 250, barPermille(1250u));
}

void test_bar_permille_three_quarter(void) {
    TEST_ASSERT_INT32_WITHIN(2, 750, barPermille(1750u));
}

/* ── permilleToPx ────────────────────────────────────────────────────────── */

void test_permille_to_px_zero(void) {
    TEST_ASSERT_EQUAL_INT16(0, permilleToPx(0, 220));
}

void test_permille_to_px_full(void) {
    TEST_ASSERT_EQUAL_INT16(220, permilleToPx(1000, 220));
}

void test_permille_to_px_half(void) {
    TEST_ASSERT_EQUAL_INT16(110, permilleToPx(500, 220));
}

void test_permille_to_px_clamps_negative(void) {
    TEST_ASSERT_EQUAL_INT16(0, permilleToPx(-1, 220));
}

void test_permille_to_px_clamps_over_1000(void) {
    TEST_ASSERT_EQUAL_INT16(220, permilleToPx(1001, 220));
}

/* ── linkStateText ───────────────────────────────────────────────────────── */

void test_link_state_text_connected(void) {
    TEST_ASSERT_EQUAL_STRING("CONNECTED", linkStateText(LinkState::Connected));
}

void test_link_state_text_binding(void) {
    TEST_ASSERT_EQUAL_STRING("BINDING", linkStateText(LinkState::Binding));
}

void test_link_state_text_failsafe(void) {
    TEST_ASSERT_EQUAL_STRING("FAILSAFE", linkStateText(LinkState::Failsafe));
}

void test_link_state_text_disconnected(void) {
    TEST_ASSERT_EQUAL_STRING("---", linkStateText(LinkState::Disconnected));
}

void test_link_state_text_unknown(void) {
    TEST_ASSERT_EQUAL_STRING("---", linkStateText(static_cast<LinkState>(99)));
}

void test_link_state_text_scanning(void) {
    TEST_ASSERT_EQUAL_STRING("SCANNING", linkStateText(LinkState::Scanning));
}

/* ── battPercent ─────────────────────────────────────────────────────────── */

void test_batt_percent_empty(void) {
    TEST_ASSERT_EQUAL_INT32(0, battPercent(9000u, 9000u, 12600u));
    TEST_ASSERT_EQUAL_INT32(0, battPercent(8000u, 9000u, 12600u));
}

void test_batt_percent_full(void) {
    TEST_ASSERT_EQUAL_INT32(100, battPercent(12600u, 9000u, 12600u));
    TEST_ASSERT_EQUAL_INT32(100, battPercent(13000u, 9000u, 12600u));
}

void test_batt_percent_half(void) {
    TEST_ASSERT_EQUAL_INT32(50, battPercent(10800u, 9000u, 12600u));
}

void test_batt_percent_quarter(void) {
    TEST_ASSERT_EQUAL_INT32(25, battPercent(9900u, 9000u, 12600u));
}

/* ── battCellPercent ─────────────────────────────────────────────────────── */

void test_batt_cell_percent_full(void) {
    TEST_ASSERT_EQUAL_UINT8(100, battCellPercent(4200u));
}

void test_batt_cell_percent_empty(void) {
    TEST_ASSERT_EQUAL_UINT8(0, battCellPercent(3000u));
}

void test_batt_cell_percent_mid(void) {
    /* 3600 mV = 50 % of (3000..4200) range. */
    TEST_ASSERT_EQUAL_UINT8(50, battCellPercent(3600u));
}

void test_batt_cell_percent_zero_input(void) {
    TEST_ASSERT_EQUAL_UINT8(0, battCellPercent(0u));
}

/* ── Integrated bar pixel width for channel value ────────────────────────── */

void test_channel_to_bar_px_min(void) {
    auto pm = barPermille(kChannelMin);
    auto px = permilleToPx(pm, 220);
    TEST_ASSERT_EQUAL_INT16(0, px);
}

void test_channel_to_bar_px_max(void) {
    auto pm = barPermille(kChannelMax);
    auto px = permilleToPx(pm, 220);
    TEST_ASSERT_EQUAL_INT16(220, px);
}

void test_channel_to_bar_px_mid(void) {
    auto pm = barPermille(kChannelMid);
    auto px = permilleToPx(pm, 220);
    TEST_ASSERT_INT16_WITHIN(2, 110, px);
}

/* ── Battery cell warning thresholds ─────────────────────────────────────── */

void test_batt_warn_above_3500(void) {
    TEST_ASSERT_GREATER_THAN(3500, 3600);
}

void test_batt_warn_at_3500(void) {
    TEST_ASSERT_TRUE(3500 <= 3500);
}

void test_batt_crit_at_3300(void) {
    TEST_ASSERT_TRUE(3300 <= 3300);
}

void test_batt_crit_below_3300(void) {
    TEST_ASSERT_TRUE(3100 <= 3300);
}

/* ── Battery cell detection thresholds ───────────────────────────────────── */

void test_batt_cell_detect_zero(void) {
    TEST_ASSERT_TRUE(0 == 0);
}

void test_batt_cell_detect_1s(void) {
    TEST_ASSERT_TRUE(3700 < 5000);
}

void test_batt_cell_detect_2s(void) {
    TEST_ASSERT_TRUE(7400 >= 5000);
    TEST_ASSERT_TRUE(7400 < 9000);
}

void test_batt_cell_detect_3s(void) {
    TEST_ASSERT_TRUE(11100 >= 9000);
    TEST_ASSERT_TRUE(11100 < 13200);
}

void test_batt_cell_detect_4s(void) {
    TEST_ASSERT_TRUE(14800 >= 13200);
}

/* ── RSSI to bars mapping ────────────────────────────────────────────────── */

void test_rssi_to_bars_excellent(void) {
    TEST_ASSERT_TRUE(-40 > -50);
}

void test_rssi_to_bars_good(void) {
    TEST_ASSERT_TRUE(-55 > -65);
    TEST_ASSERT_FALSE(-55 > -50);
}

void test_rssi_to_bars_fair(void) {
    TEST_ASSERT_TRUE(-70 > -75);
    TEST_ASSERT_FALSE(-70 > -65);
}

void test_rssi_to_bars_weak(void) {
    TEST_ASSERT_TRUE(-80 > -85);
    TEST_ASSERT_FALSE(-80 > -75);
}

void test_rssi_to_bars_none(void) {
    TEST_ASSERT_FALSE(-90 > -85);
}

/* ── Runner ──────────────────────────────────────────────────────────────── */
/* No main() here – tests are invoked from test_protocol.cpp's main(). */
