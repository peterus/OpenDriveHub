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

#include "ShellHelpers.h"

#ifndef NATIVE_TEST

#include <Arduino.h>

#include <FunctionMap.h>
#include <Shell.h>
#include <cstring>

namespace odh {

void shellPrintName(Shell &shell, const char *prefix, NameView name) {
#ifdef ODH_HAS_STRING_VIEW
    shell.println("%s%.*s", prefix, static_cast<int>(name.size()), name.data());
#else
    shell.println("%s%s", prefix, name);
#endif
}

bool shellParseModel(const char *str, ModelType &out) {
    for (uint8_t m = 0; m < static_cast<uint8_t>(ModelType::Count); ++m) {
        auto mn = modelName(static_cast<ModelType>(m));
#ifdef ODH_HAS_STRING_VIEW
        if (mn.size() > 0) {
            bool match    = true;
            const char *s = str;
            for (size_t i = 0; i < mn.size(); ++i) {
                if (*s == '\0' || (*s | 0x20) != (mn[i] | 0x20)) {
                    match = false;
                    break;
                }
                ++s;
            }
            if (match && *s == '\0') {
                out = static_cast<ModelType>(m);
                return true;
            }
        }
#else
        if (strcasecmp(str, mn) == 0) {
            out = static_cast<ModelType>(m);
            return true;
        }
#endif
    }
    return false;
}

void shellListModels(Shell &shell) {
    shell.println("Available models:");
    for (uint8_t m = 0; m < static_cast<uint8_t>(ModelType::Count); ++m) {
        shellPrintName(shell, "  ", modelName(static_cast<ModelType>(m)));
    }
}

void shellListFunctions(Shell &shell) {
    shell.println("Available functions:");
    for (uint8_t f = 0; f <= static_cast<uint8_t>(Function::TrackR); ++f) {
        shellPrintName(shell, "  ", functionName(f));
    }
}

// cppcheck-suppress constParameterCallback
int cmdReboot(Shell &shell, int, const char *const *, void *) {
#ifdef NATIVE_SIM
    shell.println("Reboot not available in simulation");
    return 1;
#else
    shell.println("Rebooting...");
    delay(100);
    ESP.restart();
    return 0; // unreachable
#endif
}

} // namespace odh

#endif // NATIVE_TEST
