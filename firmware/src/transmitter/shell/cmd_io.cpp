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
 * cmd_io – ``trim``, ``module``, ``input`` and ``rescan`` command handlers
 * for the transmitter.
 */

#include "../TransmitterApp.h"
#include "TransmitterShellCmds.h"

#include <FunctionMap.h>
#include <NvsStore.h>
#include <Protocol.h>
#include <Shell.h>
#include <ShellHelpers.h>
#include <cstdlib>
#include <cstring>

namespace odh {

// ── trim ─────────────────────────────────────────────────────────────────

int txCmdTrim(Shell &shell, int argc, const char *const *argv, void *ctx) {
    auto &app = *static_cast<TransmitterApp *>(ctx);

    if (argc < 2) {
        shell.println("Usage: trim <get|set <idx> <val>>");
        return 1;
    }

    if (strcmp(argv[1], "get") == 0) {
        auto snap = app.snapshotFuncValues();
        for (uint8_t i = 0; i < snap.second; ++i) {
            auto fn = functionName(snap.first[i].function);
#ifdef ODH_HAS_STRING_VIEW
            shell.println("  [%u] %-10.*s trim %+d", i, static_cast<int>(fn.size()), fn.data(), snap.first[i].trim);
#else
            shell.println("  [%u] %-10s trim %+d", i, fn, snap.first[i].trim);
#endif
        }
        return 0;
    }

    if (strcmp(argv[1], "set") == 0) {
        if (argc < 4) {
            shell.println("Usage: trim set <idx> <value>");
            return 1;
        }
        int idx = atoi(argv[2]);
        int val = atoi(argv[3]);
        if (val < -100 || val > 100) {
            shell.println("Trim must be -100 to +100");
            return 1;
        }
        if (!app.setTrim(static_cast<uint8_t>(idx), static_cast<int8_t>(val))) {
            shell.println("Index %d out of range", idx);
            return 1;
        }
        shell.println("Trim[%d] = %+d", idx, val);
        return 0;
    }

    shell.println("Unknown sub-command: %s", argv[1]);
    return 1;
}

// ── module ──────────────────────────────────────────────────────────────

// cppcheck-suppress constParameterPointer
int txCmdModule(Shell &shell, int argc, const char *const *argv, void *ctx) {
    if (argc < 2) {
        shell.println("Usage: module <list>");
        return 1;
    }

    if (strcmp(argv[1], "list") == 0) {
        const auto &app = *static_cast<const TransmitterApp *>(ctx);
        for (uint8_t s = 0; s < app.modules().slotCount(); ++s) {
            const char *typeStr = "empty";
            switch (app.modules().typeAt(s)) {
            case ModuleType::Switch:
                typeStr = "Switch";
                break;
            case ModuleType::Button:
                typeStr = "Button";
                break;
            case ModuleType::Potentiometer:
                typeStr = "Pot";
                break;
            case ModuleType::Encoder:
                typeStr = "Encoder";
                break;
            default:
                break;
            }
            shell.println("  Slot %u: %s (%u inputs)", s, typeStr, app.modules().inputCount(s));
        }
        return 0;
    }

    shell.println("Unknown sub-command: %s", argv[1]);
    return 1;
}

// ── input ───────────────────────────────────────────────────────────────

int txCmdInput(Shell &shell, int argc, const char *const *argv, void *ctx) {
    auto &app = *static_cast<TransmitterApp *>(ctx);

    if (argc < 2) {
        shell.println("Usage: input <get|help|reset>");
        return 1;
    }

    if (strcmp(argv[1], "get") == 0) {
        auto snap = app.snapshotFuncValues();
        if (snap.second == 0) {
            shell.println("No input map configured");
            return 0;
        }
        for (uint8_t i = 0; i < snap.second; ++i) {
            auto fn = functionName(snap.first[i].function);
#ifdef ODH_HAS_STRING_VIEW
            shell.println("  [%u] %.*s  value %u us  trim %+d", i, static_cast<int>(fn.size()), fn.data(), snap.first[i].value, snap.first[i].trim);
#else
            shell.println("  [%u] %s  value %u us  trim %+d", i, fn, snap.first[i].value, snap.first[i].trim);
#endif
        }
        return 0;
    }

    if (strcmp(argv[1], "help") == 0) {
        shellListFunctions(shell);
        return 0;
    }

    if (strcmp(argv[1], "reset") == 0) {
        NvsStore nvs("odh", true);
        uint8_t model = nvs.getU8("model_type", static_cast<uint8_t>(ModelType::Generic));
        app.loadInputMapForModel(model);
        shell.println("Input map reset to defaults");
        return 0;
    }

    shell.println("Unknown sub-command: %s", argv[1]);
    return 1;
}

// ── rescan ──────────────────────────────────────────────────────────────

// cppcheck-suppress constParameterCallback
int txCmdRescan(Shell &shell, int, const char *const *, void *ctx) {
    auto &app = *static_cast<TransmitterApp *>(ctx);
    if (app.radio().isBound()) {
        shell.println("Cannot rescan while connected – disconnect first");
        return 1;
    }
    app.rescan();
    shell.println("Channel rescan initiated");
    return 0;
}

} // namespace odh
