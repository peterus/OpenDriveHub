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
 * cmd_config – ``config`` command handler for the receiver.
 */

#include "../ReceiverApp.h"
#include "ReceiverShellCmds.h"

#include <Channel.h>
#include <FunctionMap.h>
#include <NvsStore.h>
#include <Protocol.h>
#include <Shell.h>
#include <ShellHelpers.h>
#include <cstdlib>
#include <cstring>

namespace odh {

int rxCmdConfig(Shell &shell, int argc, const char *const *argv, void *ctx) {
    auto &app = *static_cast<ReceiverApp *>(ctx);

    if (argc < 2) {
        shell.println("Usage: config <get|set|help|reset>");
        return 1;
    }

    // ── config get ──
    if (strcmp(argv[1], "get") == 0) {
        NvsStore store("odh", true);
        shell.println("  radio_ch:  %u", store.getU8("radio_ch", channel::kDefaultChannel));

        shellPrintName(shell, "  model:     ", modelName(app.output().modelType()));

        shell.println("  vehicle:   %s", app.vehicleName());
        shell.println("  batt_cell: %u", app.battery().cells());
        return 0;
    }

    // ── config help ──
    if (strcmp(argv[1], "help") == 0) {
        shell.println("Available config keys:");
        shell.println("  radio_ch   WiFi channel (1, 6, 11)");
        shell.println("  batt_cell  Battery cells (0-6, 0=auto)");
        shell.println("  model      Model type:");
        for (uint8_t m = 0; m < static_cast<uint8_t>(ModelType::Count); ++m) {
            shellPrintName(shell, "               ", modelName(static_cast<ModelType>(m)));
        }
        shell.println("  vehicle    Vehicle name (max %u chars)", kVehicleNameMax - 1);
        return 0;
    }

    // ── config reset ──
    if (strcmp(argv[1], "reset") == 0) {
        app.output().setModelType(app.output().modelType());
        app.output().saveToNvs();
        shell.println("Config reset to defaults");
        return 0;
    }

    // ── config set ──
    if (strcmp(argv[1], "set") == 0) {
        if (argc < 4) {
            shell.println("Usage: config set <key> <value>");
            shell.println("  Use 'config help' to see available keys");
            return 1;
        }

        if (strcmp(argv[2], "radio_ch") == 0) {
            int ch = atoi(argv[3]);
            if (!channel::isValidChannel(static_cast<uint8_t>(ch))) {
                shell.println("Invalid channel (valid: 1, 6, 11)");
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

        if (strcmp(argv[2], "model") == 0) {
            ModelType model;
            if (!shellParseModel(argv[3], model)) {
                shell.println("Unknown model: %s", argv[3]);
                shellListModels(shell);
                return 1;
            }
            app.output().setModelType(model);
            app.output().saveToNvs();
            shellPrintName(shell, "model = ", modelName(model));
            return 0;
        }

        if (strcmp(argv[2], "vehicle") == 0) {
            app.setVehicleName(argv[3]);
            shell.println("vehicle = %s", app.vehicleName());
            return 0;
        }

        shell.println("Unknown key: %s", argv[2]);
        shell.println("  Use 'config help' to see available keys");
        return 1;
    }

    shell.println("Unknown sub-command: %s", argv[1]);
    return 1;
}

} // namespace odh
