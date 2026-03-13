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
 * cmd_channel – ``channel`` command handler for the transmitter.
 */

#include "../TransmitterApp.h"
#include "TransmitterShellCmds.h"

#include <FunctionMap.h>
#include <Protocol.h>
#include <Shell.h>

#ifdef NATIVE_SIM
#include <sim_keyboard.h>
#endif

#include <cstdlib>
#include <cstring>

namespace odh {

int txCmdChannel(Shell &shell, int argc, const char *const *argv, void *ctx) {
    auto &app = *static_cast<TransmitterApp *>(ctx);

    if (argc < 2) {
        shell.println("Usage: channel <get|set <idx> <us>>");
        return 1;
    }

    if (strcmp(argv[1], "get") == 0) {
        auto snap = app.snapshotFuncValues();
        for (uint8_t i = 0; i < snap.second; ++i) {
            auto fn = functionName(snap.first[i].function);
#ifdef ODH_HAS_STRING_VIEW
            shell.println("  [%u] %-10.*s %4u us  trim %+d", i, static_cast<int>(fn.size()), fn.data(), snap.first[i].value, snap.first[i].trim);
#else
            shell.println("  [%u] %-10s %4u us  trim %+d", i, fn, snap.first[i].value, snap.first[i].trim);
#endif
        }
        return 0;
    }

    if (strcmp(argv[1], "set") == 0) {
#ifdef NATIVE_SIM
        if (argc < 4) {
            shell.println("Usage: channel set <idx> <us>");
            return 1;
        }
        int idx = atoi(argv[2]);
        int us  = atoi(argv[3]);
        if (idx < 0 || idx >= 16) {
            shell.println("Index must be 0-15");
            return 1;
        }
        if (us < kChannelMin || us > kChannelMax) {
            shell.println("Value must be %d-%d", kChannelMin, kChannelMax);
            return 1;
        }
        {
            std::lock_guard<std::mutex> lock(g_simKeyboard.mtx);
            g_simKeyboard.channels[idx] = static_cast<uint16_t>(us);
        }
        shell.println("Channel %d = %d us", idx, us);
        return 0;
#else
        shell.println("channel set is only available in simulation");
        return 1;
#endif
    }

    shell.println("Unknown sub-command: %s", argv[1]);
    return 1;
}

} // namespace odh
