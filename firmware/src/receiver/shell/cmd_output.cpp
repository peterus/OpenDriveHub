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
 * cmd_output – ``mapping`` and ``failsafe`` command handlers for the receiver.
 */

#include "../ReceiverApp.h"
#include "ReceiverShellCmds.h"

#include <Config.h>
#include <FunctionMap.h>
#include <Protocol.h>
#include <Shell.h>
#include <ShellHelpers.h>
#include <cstdlib>
#include <cstring>

namespace odh {

// ── mapping ─────────────────────────────────────────────────────────────

int rxCmdMapping(Shell &shell, int argc, const char *const *argv, void *ctx) {
    auto &app = *static_cast<ReceiverApp *>(ctx);

    if (argc < 2) {
        shell.println("Usage: mapping <get|help|reset>");
        return 1;
    }

    if (strcmp(argv[1], "get") == 0) {
        const auto &map = app.output().functionMap();
        for (uint8_t ch = 0; ch < config::rx::kChannelCount; ++ch) {
            auto func = channelToFunction(map, ch);
            if (func.has_value()) {
                auto fn = functionName(*func);
#ifdef ODH_HAS_STRING_VIEW
                shell.println("  CH%u: %.*s", ch, static_cast<int>(fn.size()), fn.data());
#else
                shell.println("  CH%u: %s", ch, fn);
#endif
            } else {
                shell.println("  CH%u: (unmapped)", ch);
            }
        }
        return 0;
    }

    if (strcmp(argv[1], "help") == 0) {
        shellListFunctions(shell);
        return 0;
    }

    if (strcmp(argv[1], "reset") == 0) {
        app.output().setModelType(app.output().modelType());
        app.output().saveToNvs();
        shell.println("Function map reset to defaults");
        return 0;
    }

    shell.println("Unknown sub-command: %s", argv[1]);
    return 1;
}

// ── failsafe ────────────────────────────────────────────────────────────

int rxCmdFailsafe(Shell &shell, int argc, const char *const *argv, void *ctx) {
    auto &app = *static_cast<ReceiverApp *>(ctx);

    if (argc < 2) {
        shell.println("Usage: failsafe <get|set <ch> <us>>");
        return 1;
    }

    if (strcmp(argv[1], "get") == 0) {
        for (uint8_t i = 0; i < config::rx::kChannelCount; ++i) {
            shell.println("  CH%u: %u us", i, app.output().failsafeValue(i));
        }
        return 0;
    }

    if (strcmp(argv[1], "set") == 0) {
        if (argc < 4) {
            shell.println("Usage: failsafe set <ch> <us>");
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
        app.output().setFailsafeValue(static_cast<uint8_t>(ch), static_cast<uint16_t>(us));
        app.output().saveToNvs();
        shell.println("Failsafe CH%d = %d us", ch, us);
        return 0;
    }

    shell.println("Unknown sub-command: %s", argv[1]);
    return 1;
}

} // namespace odh
