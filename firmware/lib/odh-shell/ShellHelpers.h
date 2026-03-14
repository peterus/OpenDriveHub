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
 * ShellHelpers.h – Shared helper functions for shell command handlers.
 *
 * These utilities are used by both receiver and transmitter shell commands
 * and are therefore kept in the shared odh-shell library.
 */

#pragma once

#ifndef NATIVE_TEST

#include <FunctionMap.h>

namespace odh {

class Shell;

/// Print a NameView through the shell with an optional prefix.
void shellPrintName(Shell &shell, const char *prefix, NameView name);

/// Parse a model name string (case-insensitive) into a ModelType.
/// Returns true on success.
bool shellParseModel(const char *str, ModelType &out);

/// Print the list of available model names.
void shellListModels(Shell &shell);

/// Print the list of available function names.
void shellListFunctions(Shell &shell);

/// Common ``reboot`` command handler (shared by RX and TX).
int cmdReboot(Shell &shell, int argc, const char *const *argv, void *ctx);

} // namespace odh

#endif // NATIVE_TEST
