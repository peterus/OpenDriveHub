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
 * TransmitterApp – top-level application class for the transmitter.
 *
 * Owns all subsystems: Backplane, ModuleManager, RadioLink, BatteryMonitor,
 * TelemetryData, Display, OdhWebServer, TransmitterApi, Shell.
 *
 * Creates FreeRTOS tasks:
 *   taskControl  – core 1, 50 Hz: read modules, send control packets
 *   taskDisplay  – core 0, 4 Hz : LVGL refresh, touch events (not in headless)
 *   taskShell    – core 0       : interactive console
 *   taskWeb      – core 0, low  : web config server
 */

#pragma once

#include "backplane/Backplane.h"
#ifndef ODH_HEADLESS
#include "display/Display.h"
#endif
#include "modules/InputMap.h"
#include "modules/ModuleManager.h"
#include "web/TransmitterApi.h"

#include <BatteryMonitor.h>
#include <ChannelScanner.h>
#include <Config.h>
#include <OdhWebServer.h>
#include <Shell.h>
#include <TelemetryData.h>
#include <TransmitterRadioLink.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <utility>

namespace odh {

class TransmitterApp {
public:
    void begin();

    /// Rescan channels for other transmitters. Only callable when not bound.
    void rescan();

    // ── Accessors for shell commands ────────────────────────────────

    TransmitterRadioLink &radio() {
        return _radio;
    }
    const TransmitterRadioLink &radio() const {
        return _radio;
    }
    const BatteryMonitor &battery() const {
        return _battery;
    }
    TelemetryData &telemetry() {
        return _telemetry;
    }
    const TelemetryData &telemetry() const {
        return _telemetry;
    }
    const ModuleManager &modules() const {
        return _modules;
    }

    /// Snapshot of current function values (thread-safe copy).
    std::pair<const FunctionValue *, uint8_t> snapshotFuncValues();

    /// Set trim for a function index.  Returns false if out of range.
    bool setTrim(uint8_t idx, int8_t value);

    /// Load input map for a model type (used by shell bind command).
    void loadInputMapForModel(uint8_t model) {
        loadInputMap(model);
    }

private:
    // Subsystems
    Backplane _backplane{config::tx::kI2cMuxAddr, config::tx::kModuleSlotCount};
    ModuleManager _modules{_backplane};
    TransmitterRadioLink _radio;
    BatteryMonitor _battery{config::kBatteryAdcPin, config::kBatteryDividerRatio, config::kAdcVrefMv, config::kAdcResolutionBits};
    TelemetryData _telemetry;
#ifndef ODH_HEADLESS
    Display _display;
#endif
    OdhWebServer _webServer;
    TransmitterApi _api{_webServer, _radio, _battery, _telemetry, _modules};
    Shell _shell;

    // Input map (per current model)
    InputAssignment _inputMap[kMaxFunctions] = {};
    uint8_t _inputMapCount                   = 0;

    // Function values built each control cycle
    FunctionValue _funcValues[kMaxFunctions] = {};
    uint8_t _funcValueCount                  = 0;
    FunctionValue _snapBuf[kMaxFunctions]    = {};

    SemaphoreHandle_t _i2cMutex = nullptr;
    SemaphoreHandle_t _funcMux  = nullptr;

    // Channel state
    uint8_t _currentChannel = 0;
    bool _channelActive     = false;

    // NVS helpers
    void loadInputMap(uint8_t model);

    // Channel discovery
    void runChannelAcquisition();
    void setupActiveTransmitter();
    void saveChannel(uint8_t channel);
    uint8_t loadChannel();

    // FreeRTOS task bodies
    static void taskControl(void *param);
#ifndef ODH_HEADLESS
    static void taskDisplay(void *param);
#endif
    static void taskShell(void *param);
    static void taskWeb(void *param);
};

} // namespace odh
