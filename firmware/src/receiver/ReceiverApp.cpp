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

#include "ReceiverApp.h"

#include <Arduino.h>

#include <Channel.h>
#include <ChannelScanner.h>
#include <Shell.h>
#include <cstring>

#ifndef NATIVE_SIM
#include <Wire.h>

#include "output/Pca9685Output.h"
#else
#include "output/LoggingOutput.h"
#endif

#include "shell/ReceiverShellCommands.h"

namespace odh {

ReceiverApp::ReceiverApp()
    : _battery(config::kBatteryAdcPin, config::kBatteryDividerRatio, config::kAdcVrefMv, config::kAdcResolutionBits),
      _webServer(config::rx::kWebHttpPort),
      _api(_webServer, _output, _radio, _battery) {}

void ReceiverApp::begin() {
    Serial.begin(115200);
    Serial.println(F("[ODH-RX] OpenDriveHub receiver starting..."));

    // ── Create output drivers ───────────────────────────────────────
#ifndef NATIVE_SIM
    for (uint8_t i = 0; i < config::rx::kChannelCount; i++) {
        _output.setDriver(i, std::make_unique<Pca9685Output>(i));
    }
#else
    for (uint8_t i = 0; i < config::rx::kChannelCount; i++) {
        _output.setDriver(i, std::make_unique<LoggingOutput>(i));
    }
#endif

    // ── Load NVS configuration ──────────────────────────────────────
    _output.loadFromNvs();
    loadVehicleName();

    Serial.print(F("[ODH-RX] Model type: "));
    {
        auto name = modelName(_output.modelType());
#ifdef ODH_HAS_STRING_VIEW
        Serial.println(name.data());
#else
        Serial.println(name);
#endif
    }

    // ── Battery ADC ─────────────────────────────────────────────────
    _battery.begin();
    _battery.sample();
    _battery.autoDetectCells();

    // ── I²C + PCA9685 ───────────────────────────────────────────────
#ifndef NATIVE_SIM
    Wire.begin(config::kI2cSdaPin, config::kI2cSclPin, config::kI2cFreqHz);
    Pca9685Output::initChip(config::rx::kPca9685Addr);
    Serial.println(F("[ODH-RX] PCA9685 initialised"));
#endif

    // ── Outputs ─────────────────────────────────────────────────────
    _output.begin();
    Serial.println(F("[ODH-RX] Outputs initialised"));

    // ── Mutex ───────────────────────────────────────────────────────
    _channelsMux = xSemaphoreCreateMutex();

    // ── Radio ───────────────────────────────────────────────────────
    if (!_radio.begin([this](const ControlPacket &pkt) { onControlReceived(pkt); })) {
        Serial.println(F("[ODH-RX] Radio init failed – halting"));
        while (true)
            delay(1000);
    }
    Serial.println(F("[ODH-RX] Radio (ESP-NOW) OK"));

    // Register channel migration callback
    _radio.onChannelMigration([this](uint8_t newChannel) { onChannelMigration(newChannel); });

    // Configure presence parameters (but don't start yet)
    _radio.configurePresence(_vehicleName, static_cast<uint8_t>(_output.modelType()));

    // Run channel discovery to find an active transmitter
    runChannelDiscovery();

    // Re-register DiscoveryResponse callback for ongoing TX activity tracking
    _radio.onDiscoveryResponse([this](uint8_t /*ch*/, int8_t /*rssi*/, uint8_t /*devCount*/) {
        _lastTransmitterActivityMs = millis();
        _discoveryComplete         = true;
    });

    Serial.print(F("[ODH-RX] Active on channel "));
    Serial.println(_currentChannel);
    Serial.print(F("[ODH-RX] Presence broadcast: "));
    Serial.println(_vehicleName);

    // ── Web server + API ────────────────────────────────────────────
    {
        char ssid[33];
        snprintf(ssid, sizeof(ssid), "ODH-%s", _vehicleName);
        _webServer.begin(ssid, config::rx::kWebApPass, config::rx::kWebHttpPort);
    }
    _api.begin();
#ifndef NATIVE_SIM
    Serial.print(F("[ODH-RX] Web config AP IP: "));
    Serial.println(WiFi.softAPIP());
#else
    Serial.println(F("[ODH-RX] Web server started (simulation)"));
#endif

    // ── FreeRTOS tasks ──────────────────────────────────────────────
    xTaskCreatePinnedToCore(taskOutputFn, "output", config::rx::kTaskOutputStackWords, this, config::rx::kTaskOutputPriority, nullptr, config::rx::kTaskOutputCore);

    xTaskCreatePinnedToCore(taskTelemetryFn, "telemetry", config::rx::kTaskTelemetryStackWords, this, config::rx::kTaskTelemetryPriority, nullptr, config::rx::kTaskTelemetryCore);

    // ── Shell console ───────────────────────────────────────────────
    registerReceiverShellCommands(_shell, *this);
    xTaskCreatePinnedToCore(taskShellFn, "shell", config::kShellTaskStackWords, this, config::kShellTaskPriority, nullptr, config::kShellTaskCore);

    Serial.println(F("[ODH-RX] Setup complete"));
}

// ── Control packet callback ─────────────────────────────────────────────

void ReceiverApp::onControlReceived(const ControlPacket &pkt) {
    if (xSemaphoreTake(_channelsMux, 0) == pdTRUE) {
        _output.applyControl(pkt);
        _failsafeActive = false;
        xSemaphoreGive(_channelsMux);
    }
}

// ── FreeRTOS task wrappers ──────────────────────────────────────────────

void ReceiverApp::taskOutputFn(void *param) {
    static_cast<ReceiverApp *>(param)->runOutputLoop();
}

void ReceiverApp::taskTelemetryFn(void *param) {
    static_cast<ReceiverApp *>(param)->runTelemetryLoop();
}

void ReceiverApp::taskWebFn(void *param) {
    static_cast<ReceiverApp *>(param)->runWebLoop();
}

void ReceiverApp::taskShellFn(void *param) {
    static_cast<ReceiverApp *>(param)->runShellLoop();
}

// ── Output loop (50 Hz) ─────────────────────────────────────────────────

void ReceiverApp::runOutputLoop() {
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        const bool failsafe = (_radio.msSinceLastControl() > config::kRadioFailsafeTimeoutMs);

        if (failsafe && !_failsafeActive) {
            _output.applyFailsafe();
            _failsafeActive = true;
        }
        // Normal channel updates happen directly in onControlReceived.

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(20)); // 50 Hz
    }
}

// ── Telemetry loop (10 Hz) ──────────────────────────────────────────────

void ReceiverApp::runTelemetryLoop() {
    for (;;) {
        if (_radio.isBound()) {
            if (_radio.checkLinkTimeout(config::kRadioLinkTimeoutMs)) {
                Serial.println(F("[ODH-RX] Link timeout – resuming presence"));
            }

            _battery.sample();

            const uint8_t linkState = (_radio.msSinceLastControl() > config::kRadioFailsafeTimeoutMs) ? static_cast<uint8_t>(LinkState::Failsafe) : static_cast<uint8_t>(LinkState::Connected);

            _radio.sendTelemetry(_battery.voltageMv(), _radio.lastRssi(), linkState, static_cast<uint8_t>(_output.modelType()), 0, nullptr, 0);
        } else {
            _radio.tickPresence(channel::kPresenceIntervalMs);
            checkTransmitterLoss();
        }

        vTaskDelay(pdMS_TO_TICKS(config::rx::kTelemetrySendIntervalMs));
    }
}

// ── Web loop (ESPAsyncWebServer – actually event-driven, no poll needed)

void ReceiverApp::runWebLoop() {
    // With ESPAsyncWebServer, the server runs on the lwIP task.
    // This task is only needed for periodic web-related maintenance.
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// ── Channel discovery ───────────────────────────────────────────────────

void ReceiverApp::runChannelDiscovery() {
    // Wire up the ChannelScanner callbacks to our radio layer
    ChannelScanner scanner([this](uint8_t ch) -> bool { return _radio.setChannel(ch); }, [this](uint8_t /*ch*/) -> bool { return _radio.sendDiscoveryRequest(); }, [](uint32_t ms) { delay(ms); });

    // Forward DiscoveryResponse to the scanner
    _radio.onDiscoveryResponse([&scanner](uint8_t ch, int8_t rssi, uint8_t devCount) { scanner.onDiscoveryResponse(ch, rssi, devCount); });

    // Step 1: Try last known channel from NVS
    uint8_t lastCh = loadChannel();
    if (channel::isValidChannel(lastCh)) {
        Serial.printf("[ODH-RX] Trying last known channel %u...\n", lastCh);
        ScanResult result = scanner.scanChannel(lastCh);
        if (result.foundTransmitter) {
            _currentChannel = lastCh;
            _radio.setChannel(lastCh);
            _lastTransmitterActivityMs = millis();
            _radio.beginPresence();
            _discoveryComplete = true;
            Serial.printf("[ODH-RX] Transmitter found on saved channel %u\n", lastCh);
            return;
        }
    }

    // Step 2: Scan all candidate channels
    Serial.println(F("[ODH-RX] Scanning channels 1, 6, 11..."));
    ScanResult results[channel::kCandidateChannelCount];
    scanner.scanAllChannels(results);

    for (uint8_t i = 0; i < channel::kCandidateChannelCount; ++i) {
        if (results[i].foundTransmitter) {
            _currentChannel = results[i].channel;
            _radio.setChannel(results[i].channel);
            saveChannel(results[i].channel);
            _lastTransmitterActivityMs = millis();
            _radio.beginPresence();
            _discoveryComplete = true;
            Serial.printf("[ODH-RX] Transmitter found on channel %u\n", results[i].channel);
            return;
        }
    }

    // Step 3: No transmitter found – stay on default channel and keep trying
    _currentChannel = channel::kDefaultChannel;
    _radio.setChannel(channel::kDefaultChannel);
    _radio.beginPresence();
    _discoveryComplete = false;
    Serial.println(F("[ODH-RX] No transmitter found – will retry"));
}

uint8_t ReceiverApp::loadChannel() {
    NvsStore store("odh", true);
    uint8_t ch = store.getU8("radio_ch", 0);
    if (channel::isValidChannel(ch))
        return ch;
    return channel::kDefaultChannel;
}

void ReceiverApp::saveChannel(uint8_t ch) {
    if (!channel::isValidChannel(ch))
        return;
    NvsStore store("odh", false);
    store.putU8("radio_ch", ch);
}

// ── Channel migration callback ─────────────────────────────────────────

void ReceiverApp::onChannelMigration(uint8_t newChannel) {
    if (!channel::isValidChannel(newChannel))
        return;
    Serial.printf("[ODH-RX] Channel migration → %u\n", newChannel);
    _radio.setChannel(newChannel);
    _currentChannel = newChannel;
    saveChannel(newChannel);
    _lastTransmitterActivityMs = millis();
}

// ── Transmitter loss detection ──────────────────────────────────────────

void ReceiverApp::checkTransmitterLoss() {
    // Only check if we've completed initial discovery and are not bound
    if (_radio.isBound()) {
        _lastTransmitterActivityMs = millis();
        return;
    }

    // If we haven't heard from any transmitter in a while, re-enter discovery
    if (_discoveryComplete && (millis() - _lastTransmitterActivityMs) > channel::kTransmitterLossTimeoutMs) {
        Serial.println(F("[ODH-RX] Transmitter lost – re-entering discovery"));
        _discoveryComplete = false;
    }

    // If not discovery complete, periodically retry discovery
    if (!_discoveryComplete) {
        runChannelDiscovery();
    }
}

// ── Shell loop ──────────────────────────────────────────────────────────

void ReceiverApp::runShellLoop() {
    for (;;) {
        _shell.poll();
        vTaskDelay(pdMS_TO_TICKS(config::kShellPollIntervalMs));
    }
}

// ── NVS helpers ─────────────────────────────────────────────────────────

void ReceiverApp::setVehicleName(const char *name) {
    std::strncpy(_vehicleName, name, kVehicleNameMax - 1);
    _vehicleName[kVehicleNameMax - 1] = '\0';
    saveVehicleName();
}

void ReceiverApp::loadVehicleName() {
    NvsStore store("odh", true);
    String name = store.getString("veh_name", config::rx::kVehicleName);
    std::strncpy(_vehicleName, name.c_str(), kVehicleNameMax - 1);
    _vehicleName[kVehicleNameMax - 1] = '\0';
}

void ReceiverApp::saveVehicleName() {
    NvsStore store("odh", false);
    store.putString("veh_name", _vehicleName);
}

bool ReceiverApp::isConfigButtonPressed() const {
    return digitalRead(config::rx::kConfigButtonPin) == LOW;
}

} // namespace odh
