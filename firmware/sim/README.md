# Simulation

OpenDriveHub includes two simulation environments that build the full
firmware as **Linux executables**.  No ESP32 hardware is required.

---

## Quick Start

### Prerequisites

```bash
# SDL2 is needed only for the transmitter simulation (display window)
sudo apt install pkg-config libsdl2-dev
```

### Running

```bash
cd firmware

# Receiver simulation (terminal-only, no display)
pio run -e sim_rx -t exec

# Transmitter simulation (opens an SDL2 window)
pio run -e sim_tx -t exec
```

Open both in separate terminals to simulate a complete TX → RX link.

---

## What Is Simulated

The sim layer replaces every hardware-specific API with a software shim.
Source files live in `firmware/sim/`.

| Real Hardware | Shim Header | Shim Source | Approach |
|---------------|-------------|-------------|----------|
| ESP-NOW radio | `esp_now.h` | `sim_espnow.cpp` | UDP multicast on localhost |
| FreeRTOS | `freertos/FreeRTOS.h`, `task.h`, `semphr.h` | `sim_freertos.cpp` | pthreads, BSD timers |
| I²C (Wire) | `Wire.h` | `sim_wire.cpp` | In-memory virtual bus |
| ADS1115 | `Adafruit_ADS1X15.h` | (header-only stub) | Returns constant mid-value |
| PCF8574 | `Adafruit_PCF8574.h` | (header-only stub) | Returns 0xFF (all high) |
| Preferences (NVS) | `Preferences.h` | `sim_preferences.cpp` | In-memory key-value map |
| ILI9341 + LVGL | `TFT_eSPI.h` | (linked SDL2) | SDL2 window (sim_tx only) |
| WiFi AP | `WiFi.h` | `sim_wifi.cpp` | No-op (AP name printed) |
| ESPAsyncWebServer | `ESPAsyncWebServer.h` | `sim_webserver.cpp` | Localhost HTTP |
| LittleFS | `LittleFS.h` | (header-only stub) | No-op |
| Arduino core | `Arduino.h` | `sim_arduino.cpp` | `millis()`, `delay()`, `Serial` |
| Keyboard input | `sim_keyboard.h` | `sim_keyboard.cpp` | SDL2 key events (sim_tx) |

---

## Connection Flow (in simulation)

1. Start `sim_rx` — it immediately begins broadcasting `AnnouncePacket` via
   UDP.
2. Start `sim_tx` — an SDL2 window appears showing the scan screen.
3. After a few seconds the receiver's vehicle name appears on screen.
4. Click the vehicle button in the SDL2 window to bind.
5. Both sides enter **Connected** state.  Control packets flow TX → RX;
   telemetry flows RX → TX.

---

## Keyboard Shortcuts (sim_tx)

The transmitter simulation accepts keyboard input to control function values
instead of physical modules.

| Key | Action |
|-----|--------|
| `1` – `8` | Select active channel |
| `↑` / `↓` | Increase / decrease value by 10 µs |
| `Page Up` / `Page Down` | Increase / decrease value by 100 µs |
| `Home` | Center active channel (1500 µs) |
| `Space` | Center ALL channels |
| `Esc` / close window | Quit |

---

## Web UI in Simulation

Both environments start a local HTTP server:

| Environment | URL | Port |
|-------------|-----|------|
| sim_tx | http://localhost:8080 | 8080 |
| sim_rx | http://localhost:8081 | 8081 |

> **Note:** Static files (HTML/CSS/JS) are **not** served in simulation
> because LittleFS is stubbed.  Only the REST API endpoints (`/api/…`) are
> functional.

---

## Configuration Constants in Simulation

All compile-time constants come from `firmware/lib/odh-config/Config.h` and
apply identically in simulation.  Key values:

| Constant | Namespace | Default |
|----------|-----------|---------|
| `kRadioFailsafeTimeoutMs` | `odh::config` | 500 ms |
| `kRadioLinkTimeoutMs` | `odh::config` | 3000 ms |
| `kVehicleDiscoveryTimeoutMs` | `odh::config::tx` | 5000 ms |
| `kRadioWifiChannel` | `odh::config` | 1 |
| `kControlLoopIntervalMs` | `odh::config::tx` | 20 ms (50 Hz) |
| `kAnnounceIntervalMs` | `odh::config::rx` | 500 ms |

---

## Build System

The simulation environments are defined in `firmware/platformio.ini` and
extend the `[esp32_base]` configuration with `platform = native`.  A custom
build script (`firmware/sim/sim_build.py`) adds:

- `-I firmware/sim/include` (shim headers)
- Linking of shim `.cpp` sources
- SDL2 flags via `pkg-config` (sim_tx only)
- `-DNATIVE_SIM` preprocessor define

---

## Known Limitations

- **No real WiFi** – the WiFi AP is a no-op; only localhost HTTP works.
- **No persistent storage** – `Preferences` (NVS) data is lost on exit.
- **No I²C devices** – Wire reads return 0; module stubs return default
  values (mid-range for pots, 0xFF for digital I/O).
- **Static files not served** – LittleFS is stubbed; REST API endpoints
  work but the HTML/CSS/JS web UI does not load.
- **Single machine only** – UDP radio uses `127.0.0.1`; cross-machine
  simulation is not supported.
