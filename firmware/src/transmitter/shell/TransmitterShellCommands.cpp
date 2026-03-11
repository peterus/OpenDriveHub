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
        shell.println("Usage: bind <scan|list|connect <n>>");
        return 1;
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
        shell.println("Usage: channel <list|set <idx> <us>>");
        return 1;
    }

    if (strcmp(argv[1], "list") == 0) {
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
        shell.println("Usage: trim <list|set <idx> <val>>");
        return 1;
    }

    if (strcmp(argv[1], "list") == 0) {
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
static int cmdModule(Shell &shell, int, const char *const *, void *ctx) {
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

// ── config ──────────────────────────────────────────────────────────────

static int cmdConfig(Shell &shell, int argc, const char *const *argv, void *ctx) {
    (void)ctx;

    if (argc < 2) {
        shell.println("Usage: config <get|set>");
        return 1;
    }

    if (strcmp(argv[1], "get") == 0) {
        NvsStore nvs("odh", true);
        shell.println("  radio_ch:  %u", nvs.getU8("radio_ch", config::kRadioWifiChannel));
        shell.println("  model:     %u", nvs.getU8("model_type", static_cast<uint8_t>(ModelType::Generic)));
        shell.println("  tx_cells:  %u", nvs.getU8("tx_cells", 0));
        shell.println("  rx_cells:  %u", nvs.getU8("rx_cells", 0));
        String dn = nvs.getString("dev_name", "TX");
        shell.println("  dev_name:  %s", dn.c_str());
        return 0;
    }

    if (strcmp(argv[1], "set") == 0) {
        if (argc < 4) {
            shell.println("Usage: config set <key> <value>");
            shell.println("  Keys: radio_ch, tx_cells, rx_cells, dev_name");
            return 1;
        }

        NvsStore nvs("odh", false);

        if (strcmp(argv[2], "radio_ch") == 0) {
            int ch = atoi(argv[3]);
            if (ch < 1 || ch > 13) {
                shell.println("Channel must be 1-13");
                return 1;
            }
            nvs.putU8("radio_ch", static_cast<uint8_t>(ch));
            shell.println("radio_ch = %d (restart required)", ch);
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
        return 1;
    }

    shell.println("Unknown sub-command: %s", argv[1]);
    return 1;
}

// ── Registration ────────────────────────────────────────────────────────

void registerTransmitterShellCommands(Shell &shell, TransmitterApp &app) {
    shell.registerCommand("status", "Show link, battery and telemetry", cmdStatus, &app);
    shell.registerCommand("bind", "Bind ops: scan, list, connect <n>", cmdBind, &app);
    shell.registerCommand("disconnect", "Disconnect from vehicle", cmdDisconnect, &app);
    shell.registerCommand("channel", "Channel ops: list, set <idx> <us>", cmdChannel, &app);
    shell.registerCommand("trim", "Trim ops: list, set <idx> <val>", cmdTrim, &app);
    shell.registerCommand("module", "List detected input modules", cmdModule, &app);
    shell.registerCommand("config", "Config ops: get, set <key> <val>", cmdConfig, &app);
}

} // namespace odh
