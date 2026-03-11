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
 * TransmitterApp – implementation.
 */

#include "TransmitterApp.h"

#include <Arduino.h>
#include <Preferences.h>
#include <Wire.h>

#include <Config.h>
#include <FunctionMap.h>
#include <NvsStore.h>
#include <Protocol.h>
#include <Shell.h>

#ifdef NATIVE_SIM
#include <WiFi.h>

#include <sim_keyboard.h>
#endif

#include <freertos/task.h>
#ifndef ODH_HEADLESS
#include <lvgl.h>
#endif

#include "shell/TransmitterShellCommands.h"

namespace odh {

/* ── NVS helpers ─────────────────────────────────────────────────────────── */

void TransmitterApp::loadInputMap(uint8_t model) {
    NvsStore nvs("odh", true);
    char ck[16], ek[16];
    snprintf(ck, sizeof(ck), "imapc_%u", model);
    snprintf(ek, sizeof(ek), "imape_%u", model);

    _inputMapCount = nvs.getU8(ck, 0);
    if (_inputMapCount > 0 && _inputMapCount <= kMaxFunctions) {
        nvs.getBytes(ek, _inputMap, _inputMapCount * sizeof(InputAssignment));
    } else {
        // Build default from model's function list
        auto fmap      = defaultFunctionMap(static_cast<ModelType>(model));
        _inputMapCount = fmap.count;
        uint8_t slot = 0, inp = 0;
        for (uint8_t i = 0; i < _inputMapCount; ++i) {
            _inputMap[i].function   = fmap.entries[i].function;
            _inputMap[i].slot       = slot;
            _inputMap[i].inputIndex = inp;
            _inputMap[i].trim       = 0;
            if (++inp >= 4) {
                inp = 0;
                ++slot;
            }
        }
    }
}

/* ── begin() ─────────────────────────────────────────────────────────────── */

void TransmitterApp::begin() {
    Serial.begin(115200);
    Serial.println(F("[ODH] OpenDriveHub transmitter starting..."));

    _i2cMutex = xSemaphoreCreateMutex();
    _funcMux  = xSemaphoreCreateMutex();

    // Load NVS config
    uint8_t model, txCells, rxCells;
    {
        NvsStore nvs("odh", true);
        model   = nvs.getU8("model_type", static_cast<uint8_t>(ModelType::Generic));
        txCells = nvs.getU8("tx_cells", 0);
        rxCells = nvs.getU8("rx_cells", 0);
    }
    loadInputMap(model);

    if (txCells > 0 && txCells <= 4)
        _battery.setCells(txCells);
    if (rxCells > 0 && rxCells <= 4)
        _telemetry.setRxCells(rxCells);

    // I²C + Backplane
    Wire.begin(config::kI2cSdaPin, config::kI2cSclPin);
    Wire.setClock(config::kI2cFreqHz);

    // Display
#ifndef ODH_HEADLESS
    if (_display.begin())
        Serial.println(F("[ODH] Display + touch OK"));
    else
        Serial.println(F("[ODH] Display not found – continuing without it"));
#else
    Serial.println(F("[ODH] Headless mode – display disabled"));
#endif

    // Backplane
    if (_backplane.begin())
        Serial.println(F("[ODH] Backplane mux OK"));
    else
        Serial.println(F("[ODH] Backplane mux not found – check wiring"));

    // Modules
    _modules.scanAndInit();
    Serial.print(F("[ODH] Modules detected: "));
    for (uint8_t s = 0; s < _modules.slotCount(); ++s) {
        switch (_modules.typeAt(s)) {
        case ModuleType::Switch:
            Serial.print('S');
            break;
        case ModuleType::Button:
            Serial.print('B');
            break;
        case ModuleType::Potentiometer:
            Serial.print('P');
            break;
        case ModuleType::Encoder:
            Serial.print('E');
            break;
        default:
            Serial.print('.');
            break;
        }
    }
    Serial.println();

    // Battery initial sample
    _battery.sample();
    if (_battery.cells() == 0) {
        _battery.autoDetectCells();
        Serial.printf("[ODH] TX battery auto-detected: %uS\n", _battery.cells());
    }

    // Radio
    auto telCb = [this](const TelemetryPacket &pkt) { _telemetry.onPacketReceived(pkt); };
    if (_radio.begin(config::kRadioWifiChannel, telCb)) {
        Serial.println(F("[ODH] Radio (ESP-NOW) OK"));
        _radio.startScanning();
        Serial.println(F("[ODH] Scanning for vehicles..."));
    } else {
        Serial.println(F("[ODH] Radio init failed"));
    }

    // Web config
    _api.begin();
    {
        NvsStore nvs("odh", true);
        String devName = nvs.getString("dev_name", "TX");
        char ssid[33];
        snprintf(ssid, sizeof(ssid), "ODH-%s", devName.c_str());
        _webServer.begin(ssid, config::tx::kWebApPass, config::tx::kWebHttpPort);
    }
#ifndef NATIVE_SIM
    Serial.print(F("[ODH] Web config AP IP: "));
    Serial.println(WiFi.softAPIP());
#endif

    // Start FreeRTOS tasks
    xTaskCreatePinnedToCore(taskControl, "control", config::tx::kTaskControlStackWords, this, config::tx::kTaskControlPriority, nullptr, config::tx::kTaskControlCore);

#ifndef ODH_HEADLESS
    xTaskCreatePinnedToCore(taskDisplay, "display", config::tx::kTaskDisplayStackWords, this, config::tx::kTaskDisplayPriority, nullptr, config::tx::kTaskDisplayCore);
#endif

    xTaskCreatePinnedToCore(taskWeb, "webconfig", config::tx::kTaskWebConfigStackWords, this, config::tx::kTaskWebConfigPriority, nullptr, config::tx::kTaskWebConfigCore);

    // Shell console
    registerTransmitterShellCommands(_shell, *this);
    xTaskCreatePinnedToCore(taskShell, "shell", config::kShellTaskStackWords, this, config::kShellTaskPriority, nullptr, config::kShellTaskCore);

    Serial.println(F("[ODH] Setup complete"));
}

/* ── taskControl – 50 Hz ─────────────────────────────────────────────────── */

void TransmitterApp::taskControl(void *param) {
    auto &app           = *static_cast<TransmitterApp *>(param);
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        // 1. Read modules
        if (xSemaphoreTake(app._i2cMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
            app._modules.update();
            xSemaphoreGive(app._i2cMutex);
        }

#ifdef NATIVE_SIM
        uint16_t kbValues[16];
        sim_apply_keyboard_inputs(kbValues, 16);
#endif

        // 2. Sample battery
        app._battery.sample();

        // 3. Failsafe flag
        uint8_t flags = 0;
        if (app._telemetry.msSinceLastPacket() > config::kRadioFailsafeTimeoutMs) {
            flags |= 0x01u;
        }

        // 4. Build function values from InputMap
        FunctionValue localFuncs[kMaxFunctions];
        uint8_t localCount = app._inputMapCount;
        for (uint8_t i = 0; i < localCount; ++i) {
            const auto &a          = app._inputMap[i];
            localFuncs[i].function = a.function;
            localFuncs[i].trim     = a.trim;

            if (a.isAssigned()) {
                localFuncs[i].value = app._modules.readInput(a.slot, a.inputIndex);
            } else {
                localFuncs[i].value = kChannelMid;
            }

#ifdef NATIVE_SIM
            if (i < 16)
                localFuncs[i].value = kbValues[i];
#endif
        }

        // 5. Copy under lock and send
        if (xSemaphoreTake(app._funcMux, pdMS_TO_TICKS(2)) == pdTRUE) {
            app._funcValueCount = localCount;
            memcpy(app._funcValues, localFuncs, localCount * sizeof(FunctionValue));
            xSemaphoreGive(app._funcMux);
        }

        if (app._radio.isBound()) {
            app._radio.sendControl(localFuncs, localCount, flags);
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(config::tx::kControlLoopIntervalMs));
    }
}

/* ── taskDisplay – 4 Hz ──────────────────────────────────────────────────── */

#ifndef ODH_HEADLESS

void TransmitterApp::taskDisplay(void *param) {
    auto &app               = *static_cast<TransmitterApp *>(param);
    TickType_t lastTickTime = xTaskGetTickCount();

    for (;;) {
        // Advance LVGL tick counter
        TickType_t now   = xTaskGetTickCount();
        uint32_t elapsed = static_cast<uint32_t>(pdTICKS_TO_MS(now - lastTickTime));
        lv_tick_inc(elapsed);
        lastTickTime = now;

        // Process touch events
        uint8_t evtData  = 0;
        DisplayEvent evt = app._display.consumeEvent(&evtData);

        if (evt == DisplayEvent::Vehicle && app._radio.isScanning()) {
            if (evtData < app._radio.discoveredCount()) {
                Serial.printf("[ODH] Connecting to vehicle %u\n", evtData);
                const auto *veh = app._radio.discoveredVehicle(evtData);
                if (veh && veh->valid) {
                    app.loadInputMap(veh->modelType);
                }
                if (app._radio.connectTo(evtData)) {
                    Serial.println(F("[ODH] Connected OK"));
                    app._telemetry.reset();
                } else {
                    Serial.println(F("[ODH] Connect failed"));
                }
            }
        } else if (evt == DisplayEvent::Disconnect && app._radio.isBound()) {
            Serial.println(F("[ODH] Disconnecting..."));
            app._radio.disconnect();
            Serial.println(F("[ODH] Disconnected – scanning"));
        } else if ((evt == DisplayEvent::TrimUp || evt == DisplayEvent::TrimDown) && evtData < app._inputMapCount) {
            int8_t delta = (evt == DisplayEvent::TrimUp) ? 1 : -1;
            if (xSemaphoreTake(app._funcMux, pdMS_TO_TICKS(5)) == pdTRUE) {
                int16_t newTrim = app._inputMap[evtData].trim + delta;
                if (newTrim > 100)
                    newTrim = 100;
                if (newTrim < -100)
                    newTrim = -100;
                app._inputMap[evtData].trim = static_cast<int8_t>(newTrim);
                if (evtData < app._funcValueCount)
                    app._funcValues[evtData].trim = app._inputMap[evtData].trim;
                xSemaphoreGive(app._funcMux);
            }
            // Persist trim
            {
                NvsStore nvs("odh", false);
                NvsStore nvsr("odh", true);
                uint8_t m = nvsr.getU8("model_type", static_cast<uint8_t>(ModelType::Generic));
                char ek[16];
                snprintf(ek, sizeof(ek), "imape_%u", m);
                nvs.putBytes(ek, app._inputMap, app._inputMapCount * sizeof(InputAssignment));
            }
        }

        // Telemetry timeout → auto-disconnect
        if (app._radio.isBound() && app._telemetry.hasData() && app._telemetry.msSinceLastPacket() > config::kRadioLinkTimeoutMs) {
            Serial.println(F("[ODH] Telemetry timeout – disconnecting"));
            app._radio.disconnect();
        }

        // Auto-detect RX cells on first telemetry packet
        static bool rxCellsDetected = false;
        if (app._radio.isBound() && app._telemetry.hasData() && !rxCellsDetected) {
            if (app._telemetry.rxCells() == 0) {
                app._telemetry.autoDetectRxCells();
            }
            rxCellsDetected = true;
        }
        if (!app._radio.isBound())
            rxCellsDetected = false;

        // Display update
        if (app._radio.isScanning()) {
            app._radio.pruneStaleVehicles(config::tx::kVehicleDiscoveryTimeoutMs);
            app._display.refreshScan(app._radio.discoveredVehicle(0), app._radio.discoveredCount(), 0);
        } else {
            FunctionValue snap[kMaxFunctions];
            uint8_t snapCount = 0;
            if (xSemaphoreTake(app._funcMux, pdMS_TO_TICKS(5)) == pdTRUE) {
                snapCount = app._funcValueCount;
                memcpy(snap, app._funcValues, snapCount * sizeof(FunctionValue));
                xSemaphoreGive(app._funcMux);
            }
            app._display.refresh(app._telemetry, app._battery, app._modules, snap, snapCount);
        }

        vTaskDelay(pdMS_TO_TICKS(config::tx::kDisplayRefreshIntervalMs));
    }
}

#endif // ODH_HEADLESS

/* ── taskShell ───────────────────────────────────────────────────────────── */

void TransmitterApp::taskShell(void *param) {
    auto &app = *static_cast<TransmitterApp *>(param);

#ifdef ODH_HEADLESS
    // In headless mode the shell task also handles telemetry timeout and
    // RX cell auto-detection that normally live in taskDisplay.
    bool rxCellsDetected = false;
#endif

    for (;;) {
        app._shell.poll();

#ifdef ODH_HEADLESS
        // Telemetry timeout → auto-disconnect
        if (app._radio.isBound() && app._telemetry.hasData() && app._telemetry.msSinceLastPacket() > config::kRadioLinkTimeoutMs) {
            Serial.println(F("[ODH] Telemetry timeout – disconnecting"));
            app._radio.disconnect();
        }

        // Auto-detect RX cells
        if (app._radio.isBound() && app._telemetry.hasData() && !rxCellsDetected) {
            if (app._telemetry.rxCells() == 0) {
                app._telemetry.autoDetectRxCells();
            }
            rxCellsDetected = true;
        }
        if (!app._radio.isBound())
            rxCellsDetected = false;

        // Prune stale vehicles while scanning
        if (app._radio.isScanning()) {
            app._radio.pruneStaleVehicles(config::tx::kVehicleDiscoveryTimeoutMs);
        }
#endif

        vTaskDelay(pdMS_TO_TICKS(config::kShellPollIntervalMs));
    }
}

/* ── Accessors ───────────────────────────────────────────────────────────── */

std::pair<const FunctionValue *, uint8_t> TransmitterApp::snapshotFuncValues() {
    uint8_t count = 0;
    if (xSemaphoreTake(_funcMux, pdMS_TO_TICKS(10)) == pdTRUE) {
        count = _funcValueCount;
        memcpy(_snapBuf, _funcValues, count * sizeof(FunctionValue));
        xSemaphoreGive(_funcMux);
    }
    return {_snapBuf, count};
}

bool TransmitterApp::setTrim(uint8_t idx, int8_t value) {
    if (idx >= _inputMapCount)
        return false;

    if (xSemaphoreTake(_funcMux, pdMS_TO_TICKS(10)) == pdTRUE) {
        _inputMap[idx].trim = value;
        if (idx < _funcValueCount)
            _funcValues[idx].trim = value;
        xSemaphoreGive(_funcMux);
    }

    // Persist
    NvsStore nvs("odh", false);
    NvsStore nvsr("odh", true);
    uint8_t m = nvsr.getU8("model_type", static_cast<uint8_t>(ModelType::Generic));
    char ek[16];
    snprintf(ek, sizeof(ek), "imape_%u", m);
    nvs.putBytes(ek, _inputMap, _inputMapCount * sizeof(InputAssignment));
    return true;
}

/* ── taskWeb ─────────────────────────────────────────────────────────────── */

void TransmitterApp::taskWeb(void *param) {
    (void)param;
    // ESPAsyncWebServer is event-driven; task just keeps the task alive
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

} // namespace odh
