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
 * cmd_config – ``config`` command handler for the transmitter.
 */

#include "../TransmitterApp.h"
#include "TransmitterShellCmds.h"

#include <Channel.h>
#include <Config.h>
#include <FunctionMap.h>
#include <NvsStore.h>
#include <Protocol.h>
#include <Shell.h>
#include <ShellHelpers.h>
#include <cstdlib>
#include <cstring>

namespace odh {

int txCmdConfig(Shell &shell, int argc, const char *const *argv, void *ctx) {
    if (argc < 2) {
        shell.println("Usage: config <get|set|help|reset>");
        return 1;
    }

    // ── config get ──
    if (strcmp(argv[1], "get") == 0) {
        NvsStore nvs("odh", true);
        shell.println("  radio_ch:  %u", nvs.getU8("radio_ch", channel::kDefaultChannel));

        uint8_t modelRaw = nvs.getU8("model_type", static_cast<uint8_t>(ModelType::Generic));
        shellPrintName(shell, "  model:     ", modelName(modelRaw));

        shell.println("  tx_cells:  %u", nvs.getU8("tx_cells", 0));
        shell.println("  rx_cells:  %u", nvs.getU8("rx_cells", 0));
        String dn = nvs.getString("dev_name", "TX");
        shell.println("  dev_name:  %s", dn.c_str());
        return 0;
    }

    // ── config help ──
    if (strcmp(argv[1], "help") == 0) {
        shell.println("Available config keys:");
        shell.println("  radio_ch   WiFi channel (1, 6, 11)");
        shell.println("  model      Model type:");
        for (uint8_t m = 0; m < static_cast<uint8_t>(ModelType::Count); ++m) {
            shellPrintName(shell, "               ", modelName(static_cast<ModelType>(m)));
        }
        shell.println("  tx_cells   TX battery cells (0-6, 0=auto)");
        shell.println("  rx_cells   RX battery cells (0-6, 0=auto)");
        shell.println("  dev_name   Device name (max %u chars)", kVehicleNameMax - 1);
        return 0;
    }

    // ── config reset ──
    if (strcmp(argv[1], "reset") == 0) {
        NvsStore nvs("odh", false);
        nvs.raw().clear();
        shell.println("Config reset – restart required");
        return 0;
    }

    // ── config set ──
    if (strcmp(argv[1], "set") == 0) {
        if (argc < 4) {
            shell.println("Usage: config set <key> <value>");
            shell.println("  Use 'config help' to see available keys");
            return 1;
        }

        NvsStore nvs("odh", false);

        if (strcmp(argv[2], "radio_ch") == 0) {
            int ch = atoi(argv[3]);
            if (!channel::isValidChannel(static_cast<uint8_t>(ch))) {
                shell.println("Invalid channel (valid: 1, 6, 11)");
                return 1;
            }
            nvs.putU8("radio_ch", static_cast<uint8_t>(ch));
            shell.println("radio_ch = %d (restart required)", ch);
            return 0;
        }

        if (strcmp(argv[2], "model") == 0) {
            ModelType model;
            if (!shellParseModel(argv[3], model)) {
                shell.println("Unknown model: %s", argv[3]);
                shellListModels(shell);
                return 1;
            }
            nvs.putU8("model_type", static_cast<uint8_t>(model));
            auto &app = *static_cast<TransmitterApp *>(ctx);
            app.loadInputMapForModel(static_cast<uint8_t>(model));
            shellPrintName(shell, "model = ", modelName(model));
            return 0;
        }

        if (strcmp(argv[2], "tx_cells") == 0) {
            int c = atoi(argv[3]);
            if (c < 0 || c > 6) {
                shell.println("Must be 0-6 (0 = auto)");
                return 1;
            }
            nvs.putU8("tx_cells", static_cast<uint8_t>(c));
            shell.println("tx_cells = %d", c);
            return 0;
        }

        if (strcmp(argv[2], "rx_cells") == 0) {
            int c = atoi(argv[3]);
            if (c < 0 || c > 6) {
                shell.println("Must be 0-6 (0 = auto)");
                return 1;
            }
            nvs.putU8("rx_cells", static_cast<uint8_t>(c));
            shell.println("rx_cells = %d", c);
            return 0;
        }

        if (strcmp(argv[2], "dev_name") == 0) {
            nvs.putString("dev_name", argv[3]);
            shell.println("dev_name = %s", argv[3]);
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
