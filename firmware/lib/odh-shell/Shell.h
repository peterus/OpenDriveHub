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
 * Shell.h – Lightweight interactive console inspired by the Zephyr shell.
 *
 * Provides command registration, line editing, tokenisation and dispatch.
 * On ESP32 the shell reads from UART via Serial; in the Linux simulation it
 * reads from stdin (see sim_arduino.cpp for the stdin reader thread).
 */

#pragma once

#include <cstdint>

namespace odh {

// ── Tokeniser (always available, including native unit tests) ───────────

/// Tokenise a command line into an argc/argv pair.
/// Supports double-quoted strings.  Modifies @p line in-place by inserting
/// NUL terminators.  Returns the number of tokens written to @p argv.
int shellTokenize(char *line, const char **argv, int maxArgs);

// ── Shell class (not available in the pure native-test environment) ─────

#ifndef NATIVE_TEST

#ifdef __GNUC__
#define ODH_PRINTF_FMT(fmtIdx, argIdx) __attribute__((format(printf, fmtIdx, argIdx)))
#else
#define ODH_PRINTF_FMT(fmtIdx, argIdx)
#endif

/// Function pointer type for shell command handlers.
/// @return 0 on success, non-zero on error.
using CommandHandler = int (*)(class Shell &shell, int argc, const char *const *argv, void *ctx);

/// A single registered command.
struct ShellCommand {
    const char *name;
    const char *help;
    CommandHandler handler;
    void *context;
};

class Shell {
public:
    Shell();

    /// Register a command.  @p name and @p help must have static lifetime.
    /// Returns false if the command table is full.
    bool registerCommand(const char *name, const char *help, CommandHandler handler, void *ctx = nullptr);

    /// Non-blocking poll: read available Serial input and process complete
    /// lines.  Call this periodically from a FreeRTOS task.
    void poll();

    /// Execute a command line string directly (useful for programmatic
    /// injection).  Returns the handler return value or -1 if not found.
    int execute(const char *line);

    /// Formatted output helpers (delegate to Serial).
    void print(const char *fmt, ...) ODH_PRINTF_FMT(2, 3);
    void println(const char *fmt, ...) ODH_PRINTF_FMT(2, 3);
    void println();

private:
    static constexpr uint8_t kMaxCommands = 32;
    static constexpr uint16_t kMaxLineLen = 128;
    static constexpr uint8_t kMaxArgs     = 16;
    static constexpr uint8_t kHistorySize = 16;
    static constexpr const char *kPrompt  = "odh> ";

    // Command table
    ShellCommand _commands[kMaxCommands] = {};
    uint8_t _commandCount                = 0;

    // Line editing
    char _lineBuf[kMaxLineLen] = {};
    uint16_t _linePos          = 0;
    bool _prompted             = false;

    // Command history (ring buffer)
    char _history[kHistorySize][kMaxLineLen] = {};
    uint8_t _histHead                        = 0;  // next write slot
    uint8_t _histCount                       = 0;  // entries stored
    int16_t _histBrowse                      = -1; // browse index (-1 = not browsing)
    char _histSavedLine[kMaxLineLen]         = {}; // saved current line when browsing

    // ANSI escape sequence state machine
    enum class EscState : uint8_t { None, Esc, Bracket };
    EscState _escState = EscState::None;

    void processChar(char c);
    void processEscapeChar(char c);
    void clearLine();
    void replaceLine(const char *newLine);
    void historyPush(const char *line);
    const char *historyAt(int16_t index) const;
    int dispatch(int argc, const char *const *argv);
    void showPrompt();

    static int cmdHelp(Shell &shell, int argc, const char *const *argv, void *ctx);
#ifdef NATIVE_SIM
    static int cmdExit(Shell &shell, int argc, const char *const *argv, void *ctx);
#endif
};

#undef ODH_PRINTF_FMT

#endif // NATIVE_TEST

} // namespace odh
