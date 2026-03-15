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
 * cmd_channel – ``channel`` and ``vehicle`` command handlers for the receiver.
 */

#include "../ReceiverApp.h"
#include "ReceiverShellCmds.h"

#include <Config.h>
#include <Protocol.h>
#include <Shell.h>
#include <cstdlib>
#include <cstring>

namespace odh {

// ── channel ─────────────────────────────────────────────────────────────

int rxCmdChannel(Shell &shell, int argc, const char *const *argv, void *ctx) {
    auto &app = *static_cast<ReceiverApp *>(ctx);

    if (argc < 2) {
        shell.println("Usage: channel <get|set>");
        return 1;
    }

    if (strcmp(argv[1], "get") == 0) {
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

// ── vehicle ─────────────────────────────────────────────────────────────

int rxCmdVehicle(Shell &shell, int argc, const char *const *argv, void *ctx) {
    auto &app = *static_cast<ReceiverApp *>(ctx);

    if (argc < 2) {
        shell.println("Vehicle: %s", app.vehicleName());
        return 0;
    }

    app.setVehicleName(argv[1]);
    shell.println("Vehicle name set to: %s", app.vehicleName());
    return 0;
}

} // namespace odh
