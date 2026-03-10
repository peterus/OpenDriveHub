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
 * OpenDriveHub – Unified Compile-Time Configuration
 *
 * All #define macros from the old receiver/config.h and transmitter/config.h
 * are consolidated here as inline constexpr in proper namespaces.
 *
 * Board-specific pin assignments live under odh::config::rx and
 * odh::config::tx.  Shared settings (radio channel, failsafe timeout, etc.)
 * live directly under odh::config.
 */

#pragma once

#include <cstdint>

namespace odh::config {

// ── Shared settings ─────────────────────────────────────────────────────

/// ESP-NOW WiFi channel (1-13). Must match on TX and RX.
inline constexpr uint8_t kRadioWifiChannel = 1;

/// Failsafe timeout (ms) – if no control packet arrives within this window,
/// the receiver applies failsafe values.
inline constexpr uint32_t kRadioFailsafeTimeoutMs = 500;

/// Link timeout (ms) – if no packet is received from the peer within this
/// window, the link is considered lost.
inline constexpr uint32_t kRadioLinkTimeoutMs = 3000;

/// Default channel position for failsafe (µs).
inline constexpr uint16_t kFailsafeChannelValue = 1500;

/// I²C bus speed (Hz) – 400 kHz fast-mode.
inline constexpr uint32_t kI2cFreqHz = 400000;

/// I²C pins (same on both boards).
inline constexpr uint8_t kI2cSdaPin = 21;
inline constexpr uint8_t kI2cSclPin = 22;

/// Battery ADC settings (shared divider circuit).
inline constexpr uint8_t kBatteryAdcPin     = 34;
inline constexpr float kBatteryDividerRatio = 4.03f;
inline constexpr uint16_t kAdcVrefMv        = 3300;
inline constexpr uint8_t kAdcResolutionBits = 12;

// ── Receiver-specific settings ──────────────────────────────────────────

namespace rx {

/// Number of output channels.
inline constexpr uint8_t kChannelCount = 8;

/// Default vehicle name for broadcast announcements.
inline constexpr const char *kVehicleName = "Vehicle";

/// Announce broadcast interval (ms).
inline constexpr uint32_t kAnnounceIntervalMs = 500;

/// Telemetry send interval (ms).
inline constexpr uint32_t kTelemetrySendIntervalMs = 100;

/// PCA9685 I²C PWM driver.
inline constexpr uint8_t kPca9685Addr = 0x40;

/// GPIO for config-mode entry (hold LOW during power-on).
inline constexpr uint8_t kConfigButtonPin = 0;

/// WiFi AP for receiver configuration.
inline constexpr const char *kWebApSsid = "ODH-Receiver-Config";
inline constexpr const char *kWebApPass = "odhrecv1";
inline constexpr uint16_t kWebHttpPort  = 80;

/// FreeRTOS task stack sizes (in 4-byte words).
inline constexpr uint32_t kTaskOutputStackWords    = 3072;
inline constexpr uint32_t kTaskTelemetryStackWords = 3072;
inline constexpr uint32_t kTaskWebConfigStackWords = 8192;

/// FreeRTOS task priorities.
inline constexpr uint8_t kTaskOutputPriority    = 3;
inline constexpr uint8_t kTaskTelemetryPriority = 1;
inline constexpr uint8_t kTaskWebConfigPriority = 1;

/// FreeRTOS core assignments.
inline constexpr uint8_t kTaskOutputCore    = 1;
inline constexpr uint8_t kTaskTelemetryCore = 0;
inline constexpr uint8_t kTaskWebConfigCore = 0;

} // namespace rx

// ── Transmitter-specific settings ───────────────────────────────────────

namespace tx {

/// TCA9548A I²C multiplexer.
inline constexpr uint8_t kI2cMuxAddr      = 0x70;
inline constexpr uint8_t kModuleSlotCount = 6;

/// SPI LCD (ILI9341) pins.
inline constexpr uint8_t kLcdMosiPin   = 23;
inline constexpr uint8_t kLcdMisoPin   = 19;
inline constexpr uint8_t kLcdSckPin    = 18;
inline constexpr uint8_t kLcdCsPin     = 5;
inline constexpr uint8_t kLcdDcPin     = 27;
inline constexpr uint8_t kLcdRstPin    = 26;
inline constexpr uint8_t kLcdBlPin     = 32;
inline constexpr bool kLcdBlActiveHigh = true;

/// LCD panel dimensions.
inline constexpr uint16_t kLcdPanelWidth  = 240;
inline constexpr uint16_t kLcdPanelHeight = 320;
inline constexpr uint32_t kLcdSpiFreqHz   = 40000000;

/// Logical display size after 90° rotation (landscape).
inline constexpr uint16_t kDisplayWidth  = 320;
inline constexpr uint16_t kDisplayHeight = 240;

/// Touch screen (XPT2046).
inline constexpr uint8_t kTouchCsPin  = 4;
inline constexpr uint8_t kTouchIrqPin = 33;

/// Control loop interval (ms) – 20ms = 50Hz.
inline constexpr uint32_t kControlLoopIntervalMs = 20;

/// Display refresh interval (ms).
inline constexpr uint32_t kDisplayRefreshIntervalMs = 250;

/// Telemetry poll interval (ms).
inline constexpr uint32_t kTelemetryPollIntervalMs = 100;

/// Timeout for removing stale discovered vehicles (ms).
inline constexpr uint32_t kVehicleDiscoveryTimeoutMs = 5000;

/// WiFi AP for transmitter configuration.
inline constexpr const char *kWebApSsid = "OpenDriveHub-Config";
inline constexpr const char *kWebApPass = "opendrv1";
inline constexpr uint16_t kWebHttpPort  = 80;

/// FreeRTOS task stack sizes (in 4-byte words).
inline constexpr uint32_t kTaskControlStackWords   = 4096;
inline constexpr uint32_t kTaskDisplayStackWords   = 8192;
inline constexpr uint32_t kTaskWebConfigStackWords = 8192;

/// FreeRTOS task priorities.
inline constexpr uint8_t kTaskControlPriority   = 3;
inline constexpr uint8_t kTaskDisplayPriority   = 1;
inline constexpr uint8_t kTaskWebConfigPriority = 1;

/// FreeRTOS core assignments.
inline constexpr uint8_t kTaskControlCore   = 1;
inline constexpr uint8_t kTaskDisplayCore   = 0;
inline constexpr uint8_t kTaskWebConfigCore = 0;

} // namespace tx

} // namespace odh::config
