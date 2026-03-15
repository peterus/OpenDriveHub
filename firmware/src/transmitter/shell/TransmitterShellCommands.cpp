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
 * TransmitterShellCommands – registration façade.
 *
 * The actual command handlers live in separate translation units:
 *   cmd_status.cpp   – status
 *   cmd_bind.cpp     – bind, disconnect
 *   cmd_channel.cpp  – channel
 *   cmd_io.cpp       – trim, module, input, rescan
 *   cmd_config.cpp   – config
 *
 * The shared ``reboot`` handler comes from ShellHelpers (lib/odh-shell).
 */

#include "TransmitterShellCommands.h"

#include "TransmitterShellCmds.h"

#include <Shell.h>
#include <ShellHelpers.h>

namespace odh {

void registerTransmitterShellCommands(Shell &shell, TransmitterApp &app) {
    shell.registerCommand("status", "Show link, battery and telemetry", txCmdStatus, &app);
    shell.registerCommand("bind", "Bind: scan, list, connect <n>, help", txCmdBind, &app);
    shell.registerCommand("disconnect", "Disconnect from vehicle", txCmdDisconnect, &app);
    shell.registerCommand("channel", "Channel: get, set <idx> <us>", txCmdChannel, &app);
    shell.registerCommand("trim", "Trim: get, set <idx> <val>", txCmdTrim, &app);
    shell.registerCommand("module", "Module: list", txCmdModule, &app);
    shell.registerCommand("input", "Input map: get, help, reset", txCmdInput, &app);
    shell.registerCommand("config", "Config: get, set, help, reset", txCmdConfig, &app);
    shell.registerCommand("rescan", "Rescan WiFi channels", txCmdRescan, &app);
    shell.registerCommand("reboot", "Restart the device", cmdReboot, nullptr);
}

} // namespace odh
