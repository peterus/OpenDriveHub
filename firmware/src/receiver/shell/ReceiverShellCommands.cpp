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

#include "ReceiverShellCommands.h"

#include "../ReceiverApp.h"

#include <Config.h>
#include <FunctionMap.h>
#include <NvsStore.h>
#include <Shell.h>
#include <cstdlib>
#include <cstring>

namespace odh {

// ── status ──────────────────────────────────────────────────────────────

static int cmdStatus(Shell &shell, int, const char *const *, void *ctx) {
    auto &app = *static_cast<ReceiverApp *>(ctx);

    const char *linkStr = app.radio().isBound() ? "connected" : "announcing";
    shell.println("Link:     %s", linkStr);

    if (app.radio().isBound()) {
        shell.println("RSSI:     %d dBm", app.radio().lastRssi());
        shell.println("Last pkt: %lu ms ago", static_cast<unsigned long>(app.radio().msSinceLastControl()));
    }

    shell.println("Battery:  %u mV (%uS)", app.battery().voltageMv(), app.battery().cells());

    auto mn = modelName(app.output().modelType());
#ifdef ODH_HAS_STRING_VIEW
    shell.println("Model:    %.*s", static_cast<int>(mn.size()), mn.data());
#else
    shell.println("Model:    %s", mn);
#endif

    shell.println("Vehicle:  %s", app.vehicleName());
    return 0;
}

// ── channel ─────────────────────────────────────────────────────────────

static int cmdChannel(Shell &shell, int argc, const char *const *argv, void *ctx) {
    auto &app = *static_cast<ReceiverApp *>(ctx);

    if (argc < 2) {
        shell.println("Usage: channel <list|set>");
        return 1;
    }

    if (strcmp(argv[1], "list") == 0) {
        const auto &vals = app.output().channelValues();
        for (uint8_t i = 0; i < config::rx::kChannelCount; ++i) {
            shell.println("  CH%u: %u us", i, vals[i]);
        }
        return 0;
    }

    if (strcmp(argv[1], "set") == 0) {
        if (argc < 4) {
            shell.println("Usage: channel set <ch> <us>");
            return 1;
        }
        int ch = atoi(argv[2]);
        int us = atoi(argv[3]);
        if (ch < 0 || ch >= config::rx::kChannelCount) {
            shell.println("Channel %d out of range (0-%d)", ch, config::rx::kChannelCount - 1);
            return 1;
        }
        if (us < kChannelMin || us > kChannelMax) {
            shell.println("Value %d out of range (%d-%d)", us, kChannelMin, kChannelMax);
            return 1;
        }
        app.output().setChannel(static_cast<uint8_t>(ch), static_cast<uint16_t>(us));
        shell.println("CH%d = %d us", ch, us);
        return 0;
    }

    shell.println("Unknown sub-command: %s", argv[1]);
    return 1;
}

// ── config ──────────────────────────────────────────────────────────────

static int cmdConfig(Shell &shell, int argc, const char *const *argv, void *ctx) {
    auto &app = *static_cast<ReceiverApp *>(ctx);

    if (argc < 2) {
        shell.println("Usage: config <get|set>");
        return 1;
    }

    if (strcmp(argv[1], "get") == 0) {
        NvsStore store("odh", true);
        shell.println("  radio_ch:  %u", store.getU8("radio_ch", config::kRadioWifiChannel));

        auto mn = modelName(app.output().modelType());
#ifdef ODH_HAS_STRING_VIEW
        shell.println("  model:     %.*s", static_cast<int>(mn.size()), mn.data());
#else
        shell.println("  model:     %s", mn);
#endif

        shell.println("  vehicle:   %s", app.vehicleName());
        shell.println("  batt_cell: %u", app.battery().cells());
        return 0;
    }

    if (strcmp(argv[1], "set") == 0) {
        if (argc < 4) {
            shell.println("Usage: config set <key> <value>");
            shell.println("  Keys: radio_ch, model, batt_cell");
            return 1;
        }

        if (strcmp(argv[2], "radio_ch") == 0) {
            int ch = atoi(argv[3]);
            if (ch < 1 || ch > 13) {
                shell.println("Channel must be 1-13");
                return 1;
            }
            NvsStore store("odh", false);
            store.putU8("radio_ch", static_cast<uint8_t>(ch));
            shell.println("radio_ch = %d (restart required)", ch);
            return 0;
        }

        if (strcmp(argv[2], "batt_cell") == 0) {
            int cells = atoi(argv[3]);
            if (cells < 0 || cells > 6) {
                shell.println("Cell count must be 0-6 (0 = auto)");
                return 1;
            }
            NvsStore store("odh", false);
            store.putU8("batt_cell", static_cast<uint8_t>(cells));
            shell.println("batt_cell = %d", cells);
            return 0;
        }

        shell.println("Unknown key: %s", argv[2]);
        return 1;
    }

    shell.println("Unknown sub-command: %s", argv[1]);
    return 1;
}

// ── vehicle ─────────────────────────────────────────────────────────────

static int cmdVehicle(Shell &shell, int argc, const char *const *argv, void *ctx) {
    auto &app = *static_cast<ReceiverApp *>(ctx);

    if (argc < 2) {
        shell.println("Vehicle: %s", app.vehicleName());
        return 0;
    }

    app.setVehicleName(argv[1]);
    shell.println("Vehicle name set to: %s", app.vehicleName());
    return 0;
}

// ── Registration ────────────────────────────────────────────────────────

void registerReceiverShellCommands(Shell &shell, ReceiverApp &app) {
    shell.registerCommand("status", "Show link, battery and vehicle info", cmdStatus, &app);
    shell.registerCommand("channel", "Channel ops: list, set <ch> <us>", cmdChannel, &app);
    shell.registerCommand("config", "Config ops: get, set <key> <val>", cmdConfig, &app);
    shell.registerCommand("vehicle", "Get/set vehicle name", cmdVehicle, &app);
}

} // namespace odh
