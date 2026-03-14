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
 * ReceiverShellCmds.h – Internal declarations for receiver shell command
 * handler functions.  Each handler lives in its own translation unit
 * (cmd_status.cpp, cmd_channel.cpp, cmd_config.cpp, cmd_output.cpp).
 *
 * This header is NOT part of the public API – only
 * ReceiverShellCommands.cpp includes it.
 */

#pragma once

#include <Shell.h>

namespace odh {

// cmd_status.cpp
int rxCmdStatus(Shell &shell, int argc, const char *const *argv, void *ctx);

// cmd_channel.cpp
int rxCmdChannel(Shell &shell, int argc, const char *const *argv, void *ctx);
int rxCmdVehicle(Shell &shell, int argc, const char *const *argv, void *ctx);

// cmd_config.cpp
int rxCmdConfig(Shell &shell, int argc, const char *const *argv, void *ctx);

// cmd_output.cpp
int rxCmdMapping(Shell &shell, int argc, const char *const *argv, void *ctx);
int rxCmdFailsafe(Shell &shell, int argc, const char *const *argv, void *ctx);

} // namespace odh
