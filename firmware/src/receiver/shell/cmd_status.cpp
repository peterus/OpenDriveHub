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
 * cmd_status – ``status`` command handler for the receiver.
 */

#include "../ReceiverApp.h"
#include "ReceiverShellCmds.h"

#include <FunctionMap.h>
#include <Shell.h>
#include <ShellHelpers.h>

namespace odh {

int rxCmdStatus(Shell &shell, int, const char *const *, void *ctx) {
    auto &app = *static_cast<ReceiverApp *>(ctx);

    const char *linkStr = app.radio().isBound() ? "connected" : "announcing";
    shell.println("Link:     %s", linkStr);

    if (app.radio().isBound()) {
        shell.println("RSSI:     %d dBm", app.radio().lastRssi());
        shell.println("Last pkt: %lu ms ago", static_cast<unsigned long>(app.radio().msSinceLastControl()));
    }

    shell.println("Battery:  %u mV (%uS)", app.battery().voltageMv(), app.battery().cells());

    shellPrintName(shell, "Model:    ", modelName(app.output().modelType()));

    shell.println("Vehicle:  %s", app.vehicleName());
    return 0;
}

} // namespace odh
