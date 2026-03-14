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
 * cmd_bind – ``bind`` and ``disconnect`` command handlers for the transmitter.
 */

#include "../TransmitterApp.h"
#include "TransmitterShellCmds.h"

#include <FunctionMap.h>
#include <Shell.h>
#include <ShellHelpers.h>
#include <cstdlib>
#include <cstring>

namespace odh {

// ── bind ─────────────────────────────────────────────────────────────────

int txCmdBind(Shell &shell, int argc, const char *const *argv, void *ctx) {
    auto &app = *static_cast<TransmitterApp *>(ctx);

    if (argc < 2) {
        shell.println("Usage: bind <scan|list|connect <n>|help>");
        return 1;
    }

    if (strcmp(argv[1], "help") == 0) {
        shell.println("bind sub-commands:");
        shell.println("  scan        Start scanning for vehicles");
        shell.println("  list        List discovered vehicles");
        shell.println("  connect <n> Connect to vehicle by index");
        return 0;
    }

    if (strcmp(argv[1], "scan") == 0) {
        if (app.radio().isBound()) {
            shell.println("Already connected – disconnect first");
            return 1;
        }
        app.radio().startScanning();
        shell.println("Scanning for vehicles...");
        return 0;
    }

    if (strcmp(argv[1], "list") == 0) {
        uint8_t count = app.radio().discoveredCount();
        if (count == 0) {
            shell.println("No vehicles discovered");
            return 0;
        }
        for (uint8_t i = 0; i < count; ++i) {
            const auto *v = app.radio().discoveredVehicle(i);
            if (v && v->valid) {
                auto mn = modelName(v->modelType);
#ifdef ODH_HAS_STRING_VIEW
                shell.println("  [%u] %-15s  %.*s  RSSI %d", i, v->name, static_cast<int>(mn.size()), mn.data(), v->rssi);
#else
                shell.println("  [%u] %-15s  %s  RSSI %d", i, v->name, mn, v->rssi);
#endif
            }
        }
        return 0;
    }

    if (strcmp(argv[1], "connect") == 0) {
        if (argc < 3) {
            shell.println("Usage: bind connect <index>");
            return 1;
        }
        int idx = atoi(argv[2]);
        if (idx < 0 || idx >= app.radio().discoveredCount()) {
            shell.println("Index %d out of range (0-%d)", idx, app.radio().discoveredCount() - 1);
            return 1;
        }
        const auto *veh = app.radio().discoveredVehicle(static_cast<uint8_t>(idx));
        if (veh && veh->valid) {
            app.loadInputMapForModel(veh->modelType);
        }
        if (app.radio().connectTo(static_cast<uint8_t>(idx))) {
            shell.println("Connected to vehicle %d", idx);
            app.telemetry().reset();
        } else {
            shell.println("Connection failed");
        }
        return 0;
    }

    shell.println("Unknown sub-command: %s", argv[1]);
    return 1;
}

// ── disconnect ──────────────────────────────────────────────────────────

int txCmdDisconnect(Shell &shell, int, const char *const *, void *ctx) {
    auto &app = *static_cast<TransmitterApp *>(ctx);
    if (!app.radio().isBound()) {
        shell.println("Not connected");
        return 1;
    }
    app.radio().disconnect();
    shell.println("Disconnected – scanning");
    return 0;
}

} // namespace odh
