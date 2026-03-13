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
 * cmd_status – ``status`` command handler for the transmitter.
 */

#include "../TransmitterApp.h"
#include "TransmitterShellCmds.h"

#include <Shell.h>

namespace odh {

int txCmdStatus(Shell &shell, int, const char *const *, void *ctx) {
    auto &app = *static_cast<TransmitterApp *>(ctx);

    const char *linkStr = app.radio().isBound() ? "connected" : (app.radio().isScanning() ? "scanning" : "idle");
    shell.println("Link:      %s", linkStr);

    if (app.radio().isBound()) {
        shell.println("RSSI:      %d dBm", app.radio().lastRssi());
    }

    shell.println("TX Batt:   %u mV (%uS)", app.battery().voltageMv(), app.battery().cells());

    if (app.telemetry().hasData()) {
        shell.println("RX Batt:   %u mV (%uS)", app.telemetry().rxBatteryMv(), app.telemetry().rxCells());
        shell.println("RX RSSI:   %d dBm", app.telemetry().rssi());
        shell.println("Pkts/s:    %.1f", static_cast<double>(app.telemetry().packetsPerSecond()));
        shell.println("Uptime:    %lu ms", static_cast<unsigned long>(app.telemetry().connectionUptimeMs()));
    }

    return 0;
}

} // namespace odh
