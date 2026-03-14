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
 * ReceiverShellCommands – registration façade.
 *
 * The actual command handlers live in separate translation units:
 *   cmd_status.cpp   – status
 *   cmd_channel.cpp  – channel, vehicle
 *   cmd_config.cpp   – config
 *   cmd_output.cpp   – mapping, failsafe
 *
 * The shared ``reboot`` handler comes from ShellHelpers (lib/odh-shell).
 */

#include "ReceiverShellCommands.h"

#include "ReceiverShellCmds.h"

#include <Shell.h>
#include <ShellHelpers.h>

namespace odh {

void registerReceiverShellCommands(Shell &shell, ReceiverApp &app) {
    shell.registerCommand("status", "Show link, battery and vehicle info", rxCmdStatus, &app);
    shell.registerCommand("channel", "Channel ops: get, set <ch> <us>", rxCmdChannel, &app);
    shell.registerCommand("config", "Config ops: get, set, help, reset", rxCmdConfig, &app);
    shell.registerCommand("vehicle", "Get/set vehicle name", rxCmdVehicle, &app);
    shell.registerCommand("mapping", "Function map: get, help, reset", rxCmdMapping, &app);
    shell.registerCommand("failsafe", "Failsafe: get, set <ch> <us>", rxCmdFailsafe, &app);
    shell.registerCommand("reboot", "Restart the device", cmdReboot, nullptr);
}

} // namespace odh
