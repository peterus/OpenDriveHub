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

#include "ReceiverShellCommands.h"

#include "../ReceiverApp.h"

#include <Channel.h>
#include <Config.h>
#include <FunctionMap.h>
#include <NvsStore.h>
#include <Protocol.h>
#include <Shell.h>
#include <cstdlib>
#include <cstring>

namespace odh {

// ── Helper: print a NameView via shell ──────────────────────────────────

static void printName(Shell &shell, const char *prefix, NameView name) {
#ifdef ODH_HAS_STRING_VIEW
    shell.println("%s%.*s", prefix, static_cast<int>(name.size()), name.data());
#else
    shell.println("%s%s", prefix, name);
#endif
}

// ── Helper: parse model name to ModelType ───────────────────────────────

static bool parseModel(const char *str, ModelType &out) {
    for (uint8_t m = 0; m < static_cast<uint8_t>(ModelType::Count); ++m) {
        auto mn = modelName(static_cast<ModelType>(m));
#ifdef ODH_HAS_STRING_VIEW
        // Case-insensitive compare
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

// ── Helper: list available model names ──────────────────────────────────

static void listModels(Shell &shell) {
    shell.println("Available models:");
    for (uint8_t m = 0; m < static_cast<uint8_t>(ModelType::Count); ++m) {
        printName(shell, "  ", modelName(static_cast<ModelType>(m)));
    }
}

// ── Helper: list available function names ───────────────────────────────

static void listFunctions(Shell &shell) {
    shell.println("Available functions:");
    for (uint8_t f = 0; f <= static_cast<uint8_t>(Function::TrackR); ++f) {
        printName(shell, "  ", functionName(f));
    }
}

// ── status ──────────────────────────────────────────────────────────────

static int cmdStatus(Shell &shell, int, const char *const *, void *ctx) {
    auto &app = *static_cast<ReceiverApp *>(ctx);

    const char *linkStr = app.radio().isBound() ? "connected" : "announcing";
    shell.println("Link:     %s", linkStr);

    if (app.radio().isBound()) {
        shell.println("RSSI:     %d dBm", app.radio().lastRssi());
        shell.println("Last pkt: %lu ms ago", static_cast<unsigned long>(app.radio().msSinceLastControl()));
    }

    shell.println("Battery:  %u mV (%uS)", app.battery().voltageMv(), app.battery().cells());

    printName(shell, "Model:    ", modelName(app.output().modelType()));

    shell.println("Vehicle:  %s", app.vehicleName());
    return 0;
}

// ── channel ─────────────────────────────────────────────────────────────

static int cmdChannel(Shell &shell, int argc, const char *const *argv, void *ctx) {
    auto &app = *static_cast<ReceiverApp *>(ctx);

    if (argc < 2) {
        shell.println("Usage: channel <get|set>");
        return 1;
    }

    if (strcmp(argv[1], "get") == 0) {
        const auto &vals = app.output().channelValues();
        for (uint8_t i = 0; i < config::rx::kChannelCount; ++i) {
            shell.println("  CH%u: %u us", i, vals[i]);
        }
        return 0;
    }

    if (strcmp(argv[1], "set") == 0) {
        if (argc < 4) {
            shell.println("Usage: channel set <ch> <us>");
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
        app.output().setChannel(static_cast<uint8_t>(ch), static_cast<uint16_t>(us));
        shell.println("CH%d = %d us", ch, us);
        return 0;
    }

    shell.println("Unknown sub-command: %s", argv[1]);
    return 1;
}

// ── config ──────────────────────────────────────────────────────────────

static int cmdConfig(Shell &shell, int argc, const char *const *argv, void *ctx) {
    auto &app = *static_cast<ReceiverApp *>(ctx);

    if (argc < 2) {
        shell.println("Usage: config <get|set|help|reset>");
        return 1;
    }

    // ── config get ──
    if (strcmp(argv[1], "get") == 0) {
        NvsStore store("odh", true);
        shell.println("  radio_ch:  %u", store.getU8("radio_ch", channel::kDefaultChannel));

        printName(shell, "  model:     ", modelName(app.output().modelType()));

        shell.println("  vehicle:   %s", app.vehicleName());
        shell.println("  batt_cell: %u", app.battery().cells());
        return 0;
    }

    // ── config help ──
    if (strcmp(argv[1], "help") == 0) {
        shell.println("Available config keys:");
        shell.println("  radio_ch   WiFi channel (1, 6, 11)");
        shell.println("  batt_cell  Battery cells (0-6, 0=auto)");
        shell.println("  model      Model type:");
        for (uint8_t m = 0; m < static_cast<uint8_t>(ModelType::Count); ++m) {
            printName(shell, "               ", modelName(static_cast<ModelType>(m)));
        }
        shell.println("  vehicle    Vehicle name (max %u chars)", kVehicleNameMax - 1);
        return 0;
    }

    // ── config reset ──
    if (strcmp(argv[1], "reset") == 0) {
        app.output().setModelType(app.output().modelType());
        app.output().saveToNvs();
        shell.println("Config reset to defaults");
        return 0;
    }

    // ── config set ──
    if (strcmp(argv[1], "set") == 0) {
        if (argc < 4) {
            shell.println("Usage: config set <key> <value>");
            shell.println("  Use 'config help' to see available keys");
            return 1;
        }

        if (strcmp(argv[2], "radio_ch") == 0) {
            int ch = atoi(argv[3]);
            if (!channel::isValidChannel(static_cast<uint8_t>(ch))) {
                shell.println("Invalid channel (valid: 1, 6, 11)");
                return 1;
            }
            NvsStore store("odh", false);
            store.putU8("radio_ch", static_cast<uint8_t>(ch));
            shell.println("radio_ch = %d (restart required)", ch);
            return 0;
        }

        if (strcmp(argv[2], "batt_cell") == 0) {
            int cells = atoi(argv[3]);
            if (cells < 0 || cells > 6) {
                shell.println("Cell count must be 0-6 (0 = auto)");
                return 1;
            }
            NvsStore store("odh", false);
            store.putU8("batt_cell", static_cast<uint8_t>(cells));
            shell.println("batt_cell = %d", cells);
            return 0;
        }

        if (strcmp(argv[2], "model") == 0) {
            ModelType model;
            if (!parseModel(argv[3], model)) {
                shell.println("Unknown model: %s", argv[3]);
                listModels(shell);
                return 1;
            }
            app.output().setModelType(model);
            app.output().saveToNvs();
            printName(shell, "model = ", modelName(model));
            return 0;
        }

        if (strcmp(argv[2], "vehicle") == 0) {
            app.setVehicleName(argv[3]);
            shell.println("vehicle = %s", app.vehicleName());
            return 0;
        }

        shell.println("Unknown key: %s", argv[2]);
        shell.println("  Use 'config help' to see available keys");
        return 1;
    }

    shell.println("Unknown sub-command: %s", argv[1]);
    return 1;
}

// ── vehicle ─────────────────────────────────────────────────────────────

static int cmdVehicle(Shell &shell, int argc, const char *const *argv, void *ctx) {
    auto &app = *static_cast<ReceiverApp *>(ctx);

    if (argc < 2) {
        shell.println("Vehicle: %s", app.vehicleName());
        return 0;
    }

    app.setVehicleName(argv[1]);
    shell.println("Vehicle name set to: %s", app.vehicleName());
    return 0;
}

// ── mapping ─────────────────────────────────────────────────────────────

static int cmdMapping(Shell &shell, int argc, const char *const *argv, void *ctx) {
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
        listFunctions(shell);
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

static int cmdFailsafe(Shell &shell, int argc, const char *const *argv, void *ctx) {
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

// ── reboot ──────────────────────────────────────────────────────────────

// cppcheck-suppress constParameterCallback
static int cmdReboot(Shell &shell, int, const char *const *, void *) {
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

// ── Registration ────────────────────────────────────────────────────────

void registerReceiverShellCommands(Shell &shell, ReceiverApp &app) {
    shell.registerCommand("status", "Show link, battery and vehicle info", cmdStatus, &app);
    shell.registerCommand("channel", "Channel ops: get, set <ch> <us>", cmdChannel, &app);
    shell.registerCommand("config", "Config ops: get, set, help, reset", cmdConfig, &app);
    shell.registerCommand("vehicle", "Get/set vehicle name", cmdVehicle, &app);
    shell.registerCommand("mapping", "Function map: get, help, reset", cmdMapping, &app);
    shell.registerCommand("failsafe", "Failsafe: get, set <ch> <us>", cmdFailsafe, &app);
    shell.registerCommand("reboot", "Restart the device", cmdReboot, nullptr);
}

} // namespace odh
