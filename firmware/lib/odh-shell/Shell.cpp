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

#include "Shell.h"

#include <cstdarg>
#include <cstring>

// ── Tokeniser (always compiled – used by native unit tests) ─────────────

namespace odh {

int shellTokenize(char *line, const char **argv, int maxArgs) {
    int argc = 0;
    char *p  = line;

    while (*p && argc < maxArgs) {
        while (*p == ' ' || *p == '\t')
            ++p;
        if (*p == '\0')
            break;

        if (*p == '"') {
            ++p;
            argv[argc++] = p;
            while (*p && *p != '"')
                ++p;
            if (*p == '"')
                *p++ = '\0';
        } else {
            argv[argc++] = p;
            while (*p && *p != ' ' && *p != '\t')
                ++p;
            if (*p)
                *p++ = '\0';
        }
    }
    return argc;
}

} // namespace odh

// ── Full shell implementation (not for the native-test environment) ──────

#ifndef NATIVE_TEST

#include <Arduino.h>

namespace odh {

Shell::Shell() {
    registerCommand("help", "List available commands", cmdHelp, this);
#ifdef NATIVE_SIM
    registerCommand("exit", "Exit the simulation", cmdExit, nullptr);
#endif
}

bool Shell::registerCommand(const char *name, const char *help, CommandHandler handler, void *ctx) {
    if (_commandCount >= kMaxCommands)
        return false;
    _commands[_commandCount++] = {name, help, handler, ctx};
    return true;
}

// ── Polling ─────────────────────────────────────────────────────────────

void Shell::poll() {
    if (!_prompted) {
        showPrompt();
        _prompted = true;
    }

    while (Serial.available() > 0) {
        char c = static_cast<char>(Serial.read());
        processChar(c);
    }
}

void Shell::processChar(char c) {
    // ANSI escape sequence state machine (for arrow keys)
    if (_escState == EscState::Esc) {
        _escState = (c == '[') ? EscState::Bracket : EscState::None;
        return;
    }
    if (_escState == EscState::Bracket) {
        _escState = EscState::None;
        processEscapeChar(c);
        return;
    }
    if (c == '\x1B') {
        _escState = EscState::Esc;
        return;
    }

    if (c == '\n' || c == '\r') {
        Serial.println();
        if (_linePos > 0) {
            _lineBuf[_linePos] = '\0';
            historyPush(_lineBuf);
            execute(_lineBuf);
        }
        _linePos    = 0;
        _histBrowse = -1;
        _prompted   = false;
        return;
    }

    if (c == '\b' || c == 127) {
        if (_linePos > 0) {
            --_linePos;
            Serial.printf("\b \b");
        }
        return;
    }

    if (c < ' ')
        return;

    if (_linePos < kMaxLineLen - 1) {
        _lineBuf[_linePos++] = c;
        Serial.printf("%c", c);
    }
}

void Shell::processEscapeChar(char c) {
    if (c == 'A') {
        // Arrow Up – older history entry
        if (_histCount == 0)
            return;
        if (_histBrowse < 0) {
            // Save current line before browsing
            _lineBuf[_linePos] = '\0';
            strncpy(_histSavedLine, _lineBuf, kMaxLineLen);
            _histBrowse = 0;
        } else if (_histBrowse < static_cast<int16_t>(_histCount) - 1) {
            ++_histBrowse;
        } else {
            return; // already at oldest
        }
        replaceLine(historyAt(_histBrowse));
    } else if (c == 'B') {
        // Arrow Down – newer history entry
        if (_histBrowse < 0)
            return;
        --_histBrowse;
        if (_histBrowse < 0) {
            replaceLine(_histSavedLine);
        } else {
            replaceLine(historyAt(_histBrowse));
        }
    }
    // Ignore other escape sequences (C=right, D=left, etc.)
}

void Shell::clearLine() {
    while (_linePos > 0) {
        Serial.printf("\b \b");
        --_linePos;
    }
}

void Shell::replaceLine(const char *newLine) {
    clearLine();
    strncpy(_lineBuf, newLine, kMaxLineLen - 1);
    _lineBuf[kMaxLineLen - 1] = '\0';
    _linePos                  = static_cast<uint16_t>(strlen(_lineBuf));
    Serial.printf("%s", _lineBuf);
}

void Shell::historyPush(const char *line) {
    strncpy(_history[_histHead], line, kMaxLineLen - 1);
    _history[_histHead][kMaxLineLen - 1] = '\0';
    _histHead                            = (_histHead + 1) % kHistorySize;
    if (_histCount < kHistorySize)
        ++_histCount;
}

const char *Shell::historyAt(int16_t index) const {
    // index 0 = most recent, 1 = one before, ...
    int slot = (static_cast<int>(_histHead) - 1 - index + kHistorySize * 2) % kHistorySize;
    return _history[slot];
}

// ── Execute ─────────────────────────────────────────────────────────────

int Shell::execute(const char *line) {
    char buf[kMaxLineLen];
    strncpy(buf, line, sizeof(buf));
    buf[sizeof(buf) - 1] = '\0';

    const char *argv[kMaxArgs];
    int argc = shellTokenize(buf, argv, kMaxArgs);
    if (argc == 0)
        return 0;

    return dispatch(argc, argv);
}

int Shell::dispatch(int argc, const char *const *argv) {
    for (uint8_t i = 0; i < _commandCount; ++i) {
        if (strcmp(argv[0], _commands[i].name) == 0) {
            return _commands[i].handler(*this, argc, argv, _commands[i].context);
        }
    }
    println("Unknown command: %s  (type 'help')", argv[0]);
    return -1;
}

// ── Output helpers ──────────────────────────────────────────────────────

void Shell::print(const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    Serial.print(buf);
}

void Shell::println(const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    Serial.println(buf);
}

void Shell::println() {
    Serial.println();
}

void Shell::showPrompt() {
    Serial.print(kPrompt);
}

// ── Built-in: help ──────────────────────────────────────────────────────

int Shell::cmdHelp(Shell &shell, int, const char *const *, void *) {
    shell.println("Available commands:");
    for (uint8_t i = 0; i < shell._commandCount; ++i) {
        shell.println("  %-14s %s", shell._commands[i].name, shell._commands[i].help);
    }
    return 0;
}

// ── Built-in: exit (simulation only) ────────────────────────────────────

#ifdef NATIVE_SIM
int Shell::cmdExit(Shell &shell, int, const char *const *, void *) {
    shell.println("Bye.");
    std::exit(0);
    return 0;
}
#endif

} // namespace odh

#endif // NATIVE_TEST
