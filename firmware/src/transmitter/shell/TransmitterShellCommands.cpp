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

#include "TransmitterShellCommands.h"

#include "../TransmitterApp.h"

#include <Channel.h>
#include <Config.h>
#include <FunctionMap.h>
#include <NvsStore.h>
#include <Protocol.h>
#include <Shell.h>

#ifdef NATIVE_SIM
#include <sim_keyboard.h>
#endif

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
    auto &app = *static_cast<TransmitterApp *>(ctx);

    const char *linkStr = app.radio().isBound() ? "connected" : (app.radio().isScanning() ? "scanning" : "idle");
    shell.println("Link:      %s", linkStr);

    if (app.radio().isBound()) {
        shell.println("RSSI:      %d dBm", app.radio().lastRssi());
    }

    shell.println("TX Batt:   %u mV (%uS)", app.battery().voltageMv(), app.battery().cells());

    if (app.telemetry().hasData()) {
        shell.println("RX Batt:   %u mV (%uS)", app.telemetry().rxBatteryMv(), app.telemetry().rxCells());
        shell.println("RX RSSI:   %d dBm", app.telemetry().rssi());
        shell.println("Pkts/s:    %.1f", static_cast<double>(app.telemetry().packetsPerSecond()));
        shell.println("Uptime:    %lu ms", static_cast<unsigned long>(app.telemetry().connectionUptimeMs()));
    }

    return 0;
}

// ── bind ─────────────────────────────────────────────────────────────────

static int cmdBind(Shell &shell, int argc, const char *const *argv, void *ctx) {
    auto &app = *static_cast<TransmitterApp *>(ctx);

    if (argc < 2) {
        shell.println("Usage: bind <scan|list|connect <n>|help>");
        return 1;
    }

    if (strcmp(argv[1], "help") == 0) {
        shell.println("bind sub-commands:");
        shell.println("  scan        Start scanning for vehicles");
        shell.println("  list        List discovered vehicles");
        shell.println("  connect <n> Connect to vehicle by index");
        return 0;
    }

    if (strcmp(argv[1], "scan") == 0) {
        if (app.radio().isBound()) {
            shell.println("Already connected – disconnect first");
            return 1;
        }
        app.radio().startScanning();
        shell.println("Scanning for vehicles...");
        return 0;
    }

    if (strcmp(argv[1], "list") == 0) {
        uint8_t count = app.radio().discoveredCount();
        if (count == 0) {
            shell.println("No vehicles discovered");
            return 0;
        }
        for (uint8_t i = 0; i < count; ++i) {
            const auto *v = app.radio().discoveredVehicle(i);
            if (v && v->valid) {
                auto mn = modelName(v->modelType);
#ifdef ODH_HAS_STRING_VIEW
                shell.println("  [%u] %-15s  %.*s  RSSI %d", i, v->name, static_cast<int>(mn.size()), mn.data(), v->rssi);
#else
                shell.println("  [%u] %-15s  %s  RSSI %d", i, v->name, mn, v->rssi);
#endif
            }
        }
        return 0;
    }

    if (strcmp(argv[1], "connect") == 0) {
        if (argc < 3) {
            shell.println("Usage: bind connect <index>");
            return 1;
        }
        int idx = atoi(argv[2]);
        if (idx < 0 || idx >= app.radio().discoveredCount()) {
            shell.println("Index %d out of range (0-%d)", idx, app.radio().discoveredCount() - 1);
            return 1;
        }
        const auto *veh = app.radio().discoveredVehicle(static_cast<uint8_t>(idx));
        if (veh && veh->valid) {
            app.loadInputMapForModel(veh->modelType);
        }
        if (app.radio().connectTo(static_cast<uint8_t>(idx))) {
            shell.println("Connected to vehicle %d", idx);
            app.telemetry().reset();
        } else {
            shell.println("Connection failed");
        }
        return 0;
    }

    shell.println("Unknown sub-command: %s", argv[1]);
    return 1;
}

// ── disconnect ──────────────────────────────────────────────────────────

static int cmdDisconnect(Shell &shell, int, const char *const *, void *ctx) {
    auto &app = *static_cast<TransmitterApp *>(ctx);
    if (!app.radio().isBound()) {
        shell.println("Not connected");
        return 1;
    }
    app.radio().disconnect();
    shell.println("Disconnected – scanning");
    return 0;
}

// ── channel ─────────────────────────────────────────────────────────────

static int cmdChannel(Shell &shell, int argc, const char *const *argv, void *ctx) {
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

// ── trim ─────────────────────────────────────────────────────────────────

static int cmdTrim(Shell &shell, int argc, const char *const *argv, void *ctx) {
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

// cppcheck-suppress constParameterCallback
static int cmdModule(Shell &shell, int argc, const char *const *argv, void *ctx) {
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

static int cmdInput(Shell &shell, int argc, const char *const *argv, void *ctx) {
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
        listFunctions(shell);
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

// ── config ──────────────────────────────────────────────────────────────

static int cmdConfig(Shell &shell, int argc, const char *const *argv, void *ctx) {
    if (argc < 2) {
        shell.println("Usage: config <get|set|help|reset>");
        return 1;
    }

    // ── config get ──
    if (strcmp(argv[1], "get") == 0) {
        NvsStore nvs("odh", true);
        shell.println("  radio_ch:  %u", nvs.getU8("radio_ch", channel::kDefaultChannel));

        uint8_t modelRaw = nvs.getU8("model_type", static_cast<uint8_t>(ModelType::Generic));
        printName(shell, "  model:     ", modelName(modelRaw));

        shell.println("  tx_cells:  %u", nvs.getU8("tx_cells", 0));
        shell.println("  rx_cells:  %u", nvs.getU8("rx_cells", 0));
        String dn = nvs.getString("dev_name", "TX");
        shell.println("  dev_name:  %s", dn.c_str());
        return 0;
    }

    // ── config help ──
    if (strcmp(argv[1], "help") == 0) {
        shell.println("Available config keys:");
        shell.println("  radio_ch   WiFi channel (1, 6, 11)");
        shell.println("  model      Model type:");
        for (uint8_t m = 0; m < static_cast<uint8_t>(ModelType::Count); ++m) {
            printName(shell, "               ", modelName(static_cast<ModelType>(m)));
        }
        shell.println("  tx_cells   TX battery cells (0-6, 0=auto)");
        shell.println("  rx_cells   RX battery cells (0-6, 0=auto)");
        shell.println("  dev_name   Device name (max %u chars)", kVehicleNameMax - 1);
        return 0;
    }

    // ── config reset ──
    if (strcmp(argv[1], "reset") == 0) {
        NvsStore nvs("odh", false);
        nvs.raw().clear();
        shell.println("Config reset – restart required");
        return 0;
    }

    // ── config set ──
    if (strcmp(argv[1], "set") == 0) {
        if (argc < 4) {
            shell.println("Usage: config set <key> <value>");
            shell.println("  Use 'config help' to see available keys");
            return 1;
        }

        NvsStore nvs("odh", false);

        if (strcmp(argv[2], "radio_ch") == 0) {
            int ch = atoi(argv[3]);
            if (!channel::isValidChannel(static_cast<uint8_t>(ch))) {
                shell.println("Invalid channel (valid: 1, 6, 11)");
                return 1;
            }
            nvs.putU8("radio_ch", static_cast<uint8_t>(ch));
            shell.println("radio_ch = %d (restart required)", ch);
            return 0;
        }

        if (strcmp(argv[2], "model") == 0) {
            ModelType model;
            if (!parseModel(argv[3], model)) {
                shell.println("Unknown model: %s", argv[3]);
                listModels(shell);
                return 1;
            }
            nvs.putU8("model_type", static_cast<uint8_t>(model));
            auto &app = *static_cast<TransmitterApp *>(ctx);
            app.loadInputMapForModel(static_cast<uint8_t>(model));
            printName(shell, "model = ", modelName(model));
            return 0;
        }

        if (strcmp(argv[2], "tx_cells") == 0) {
            int c = atoi(argv[3]);
            if (c < 0 || c > 6) {
                shell.println("Must be 0-6 (0 = auto)");
                return 1;
            }
            nvs.putU8("tx_cells", static_cast<uint8_t>(c));
            shell.println("tx_cells = %d", c);
            return 0;
        }

        if (strcmp(argv[2], "rx_cells") == 0) {
            int c = atoi(argv[3]);
            if (c < 0 || c > 6) {
                shell.println("Must be 0-6 (0 = auto)");
                return 1;
            }
            nvs.putU8("rx_cells", static_cast<uint8_t>(c));
            shell.println("rx_cells = %d", c);
            return 0;
        }

        if (strcmp(argv[2], "dev_name") == 0) {
            nvs.putString("dev_name", argv[3]);
            shell.println("dev_name = %s", argv[3]);
            return 0;
        }

        shell.println("Unknown key: %s", argv[2]);
        shell.println("  Use 'config help' to see available keys");
        return 1;
    }

    shell.println("Unknown sub-command: %s", argv[1]);
    return 1;
}

// ── rescan ──────────────────────────────────────────────────────────────

// cppcheck-suppress constParameterCallback
static int cmdRescan(Shell &shell, int, const char *const *, void *ctx) {
    auto &app = *static_cast<TransmitterApp *>(ctx);
    if (app.radio().isBound()) {
        shell.println("Cannot rescan while connected – disconnect first");
        return 1;
    }
    app.rescan();
    shell.println("Channel rescan initiated");
    return 0;
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

void registerTransmitterShellCommands(Shell &shell, TransmitterApp &app) {
    shell.registerCommand("status", "Show link, battery and telemetry", cmdStatus, &app);
    shell.registerCommand("bind", "Bind: scan, list, connect <n>, help", cmdBind, &app);
    shell.registerCommand("disconnect", "Disconnect from vehicle", cmdDisconnect, &app);
    shell.registerCommand("channel", "Channel: get, set <idx> <us>", cmdChannel, &app);
    shell.registerCommand("trim", "Trim: get, set <idx> <val>", cmdTrim, &app);
    shell.registerCommand("module", "Module: list", cmdModule, &app);
    shell.registerCommand("input", "Input map: get, help, reset", cmdInput, &app);
    shell.registerCommand("config", "Config: get, set, help, reset", cmdConfig, &app);
    shell.registerCommand("rescan", "Rescan WiFi channels", cmdRescan, &app);
    shell.registerCommand("reboot", "Restart the device", cmdReboot, nullptr);
}

} // namespace odh
