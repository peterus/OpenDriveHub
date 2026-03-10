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

#include <cstring>

#ifndef NATIVE_SIM
#include <Wire.h>

#include "output/Pca9685Output.h"
#else
#include "output/LoggingOutput.h"
#endif

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
    {
        NvsStore store("odh", true);
        const uint8_t wifiCh = store.getU8("radio_ch", config::kRadioWifiChannel);

        if (!_radio.begin(wifiCh, [this](const ControlPacket &pkt) { onControlReceived(pkt); })) {
            Serial.println(F("[ODH-RX] Radio init failed – halting"));
            while (true)
                delay(1000);
        }
    }
    Serial.println(F("[ODH-RX] Radio (ESP-NOW) OK"));

    _radio.beginAnnouncing(_vehicleName, static_cast<uint8_t>(_output.modelType()));
    Serial.print(F("[ODH-RX] Announcing vehicle: "));
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
                Serial.println(F("[ODH-RX] Link timeout – resuming announce"));
            }

            _battery.sample();

            const uint8_t linkState = (_radio.msSinceLastControl() > config::kRadioFailsafeTimeoutMs) ? static_cast<uint8_t>(LinkState::Failsafe) : static_cast<uint8_t>(LinkState::Connected);

            _radio.sendTelemetry(_battery.voltageMv(), _radio.lastRssi(), linkState, static_cast<uint8_t>(_output.modelType()), 0, nullptr, 0);
        } else {
            _radio.tickAnnounce(config::rx::kAnnounceIntervalMs);
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

// ── NVS helpers ─────────────────────────────────────────────────────────

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
