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
 * TelemetryData, Display, OdhWebServer, TransmitterApi.
 *
 * Creates three FreeRTOS tasks:
 *   taskControl  – core 1, 50 Hz: read modules, send control packets
 *   taskDisplay  – core 0, 4 Hz : LVGL refresh, touch events
 *   taskWeb      – core 0, low  : web config server
 */

#pragma once

#include "backplane/Backplane.h"
#include "display/Display.h"
#include "modules/InputMap.h"
#include "modules/ModuleManager.h"
#include "web/TransmitterApi.h"

#include <BatteryMonitor.h>
#include <ChannelScanner.h>
#include <Config.h>
#include <OdhWebServer.h>
#include <TelemetryData.h>
#include <TransmitterRadioLink.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace odh {

class TransmitterApp {
public:
    void begin();

    /// Rescan channels for other transmitters. Only callable when not bound.
    void rescan();

private:
    // Subsystems
    Backplane _backplane{config::tx::kI2cMuxAddr, config::tx::kModuleSlotCount};
    ModuleManager _modules{_backplane};
    TransmitterRadioLink _radio;
    BatteryMonitor _battery{config::kBatteryAdcPin, config::kBatteryDividerRatio, config::kAdcVrefMv, config::kAdcResolutionBits};
    TelemetryData _telemetry;
    Display _display;
    OdhWebServer _webServer;
    TransmitterApi _api{_webServer, _radio, _battery, _telemetry, _modules};

    // Input map (per current model)
    InputAssignment _inputMap[kMaxFunctions] = {};
    uint8_t _inputMapCount                   = 0;

    // Function values built each control cycle
    FunctionValue _funcValues[kMaxFunctions] = {};
    uint8_t _funcValueCount                  = 0;

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
    static void taskDisplay(void *param);
    static void taskWeb(void *param);
};

} // namespace odh
