# Copilot Instructions – OpenDriveHub

## Project Overview

OpenDriveHub is an open-source modular RC control platform for functional model
vehicles (dump trucks, excavators, tractors, cranes).  It consists of:

- **Transmitter** – hand-held controller with touch LCD and plug-in input modules
- **Receiver** – vehicle-mounted, drives servos/ESCs via PCA9685

Both run on **ESP32** (esp32dev) with Arduino framework.

## Language & Communication

- The developer communicates in **German** – respond in German.
- Code, comments, commit messages, and documentation are written in **English**.

## Repository Layout

```
firmware/                  # Single unified PlatformIO project
├── platformio.ini         # 5 environments (see below)
├── lib/                   # Shared libraries
│   ├── odh-protocol/      #   Protocol.h, FunctionMap.h
│   ├── odh-config/        #   Config.h, NvsStore.h
│   ├── odh-radio/         #   ReceiverRadioLink, TransmitterRadioLink
│   ├── odh-telemetry/     #   BatteryMonitor, TelemetryData
│   └── odh-web/           #   OdhWebServer, ApiHandler
├── src/
│   ├── receiver/          # ReceiverApp, OutputManager, Pca9685Output, ReceiverApi
│   └── transmitter/       # TransmitterApp, Display, Backplane, ModuleManager, TransmitterApi
├── data/
│   ├── receiver/          # Static HTML/CSS/JS for receiver web UI
│   └── transmitter/       # Static HTML/CSS/JS for transmitter web UI
├── include/               # lv_conf.h, User_Setup.h (TFT_eSPI pin config)
├── sim/                   # Simulation shim headers + sources for sim_rx / sim_tx
│   ├── include/           #   Arduino.h, Wire.h, esp_now.h, Preferences.h, etc.
│   ├── src/               #   sim_arduino.cpp, sim_espnow.cpp, sim_freertos.cpp, etc.
│   └── sim_build.py       #   PlatformIO extra_script
└── test/
    └── test_native/       # Unity tests (90 cases): test_protocol, test_function_map, test_display
docs/                      # GitHub Pages documentation (Jekyll / Cayman theme)
hardware/                  # Hardware design docs (transmitter, receiver, modules)
```

## Build Environments

| Environment    | Platform       | Purpose                          | Key Defines                        |
|----------------|----------------|----------------------------------|------------------------------------|
| `receiver`     | `espressif32`  | ESP32 receiver firmware          | `ODH_RECEIVER`                     |
| `transmitter`  | `espressif32`  | ESP32 transmitter firmware       | `ODH_TRANSMITTER`                  |
| `native`       | `native`       | Host unit tests (Unity)          | `NATIVE_TEST`                      |
| `sim_rx`       | `native`       | Receiver simulation (terminal)   | `NATIVE_SIM`, `ODH_RECEIVER`, `SIM_RX`  |
| `sim_tx`       | `native`       | Transmitter simulation (SDL2)    | `NATIVE_SIM`, `ODH_TRANSMITTER`, `SIM_TX` |

### Build Commands

```bash
cd firmware
pio run -e receiver              # Build receiver
pio run -e transmitter           # Build transmitter
pio test -e native               # Run 90 unit tests
pio run -e sim_rx -t exec        # Run receiver simulation
pio run -e sim_tx -t exec        # Run transmitter simulation (SDL2 window)
```

Always run builds from the `firmware/` directory – `platformio.ini` is there.

## C++ Style & Conventions

### Language Standard
- ESP32 targets: **`-std=gnu++2a`** (C++20 on GCC 8.4)
- Native/sim targets: **`-std=gnu++20`**

### Namespace
- All shared types under **`namespace odh`**
- Configuration constants: **`odh::config::`**, **`odh::config::rx::`**, **`odh::config::tx::`**
- Never pollute the global namespace (legacy aliases exist in Protocol.h but are deprecated)

### Naming Conventions
- Constants: `kCamelCase` (`kMaxChannels`, `kRadioFailsafeTimeoutMs`)
- Types: `PascalCase` (`ControlPacket`, `FunctionValue`, `IModule`)
- Functions/methods: `camelCase` (`selectSlot()`, `defaultFunctionMap()`)
- Private members: `_camelCase` (`_radio`, `_battery`)
- Enums: `enum class` with `PascalCase` values (`PacketType::Control`, `LinkState::Connected`)
- Interfaces: prefix `I` (`IModule`, `IOutputDriver`)

### Modern C++ Patterns (use these consistently)
- `inline constexpr` for compile-time constants (no `#define`)
- `enum class` for all enumerations (no plain enums)
- `std::optional` for nullable returns (e.g. NVS lookups)
- `std::unique_ptr` for owned polymorphic objects (modules, output drivers)
- `virtual ~Interface() = default` for interface destructors
- `static_assert` for compile-time checks (packet sizes)
- RAII wrappers (e.g. `NvsStore` for `Preferences`)
- `__attribute__((packed))` for wire-format structs

### Formatting (clang-format)
- Style: LLVM base, **4-space indent**, **500-column line limit**
- Braces: Attach (K&R)
- Pointer alignment: right (`uint8_t *ptr`)
- Include order: Arduino/ESP-IDF → third-party libs → project headers
- Consecutive assignments are aligned
- Run: `clang-format -i --style=file <file>` (version **19**)
- CI enforces formatting – always format before committing

### Static Analysis (cppcheck)
- CI runs cppcheck with `--enable=warning,style,performance,portability`
- Suppressed globally: `missingIncludeSystem`, `missingInclude`
- Use `// cppcheck-suppress <id>` for inline suppressions when needed (e.g. `constParameterCallback` for ESP-IDF callbacks)

## Key Architecture Details

### Protocol (`odh-protocol`)
- Packets: `ControlPacket` (73B), `TelemetryPacket` (30B), `BindPacket` (11B), `AnnouncePacket` (28B)
- All packets share 6-byte header: `[magic 'O','D'][version][type][sequence_le]`
- Last byte is XOR checksum via `odh::checksum()`
- Max ESP-NOW payload: 250 bytes (enforced by `static_assert`)
- Functions: `odh::Function` enum (Drive, Steering, Aux1–4, BoomUD/LR, Bucket, Swing, etc.)
- `FunctionValue` = `{function, value_us, trim}` (4 bytes packed)

### Transmitter
- FreeRTOS tasks: Control (50Hz, core1), Display (4Hz, core0), Web (async, core0)
- `Backplane` – TCA9548A I²C mux at 0x70, 6 slots
- `ModuleManager` – detects modules by probing I²C addresses per slot
- Input modules: `PotModule` (ADS1115@0x48), `SwitchModule` (PCF8574@0x20), `ButtonModule` (PCF8574@0x21), `EncoderModule` (AS5600@0x36)
- Display: ILI9341 320×240 SPI + XPT2046 touch, LVGL v8

### Receiver
- FreeRTOS tasks: Output (core1), Telemetry (10Hz, core0), Web (async, core0)
- `OutputManager` → `IOutputDriver` → `Pca9685Output` (I²C PWM at 0x40)
- Receiver-initiated discovery: broadcasts `AnnouncePacket` when disconnected
- Failsafe: `kRadioFailsafeTimeoutMs` (500ms) → center outputs; `kRadioLinkTimeoutMs` (3s) → disconnect

### Web Stack
- `OdhWebServer` wraps ESPAsyncWebServer + LittleFS static file serving
- `ApiHandler` provides `sendJson()`, `sendError()`, `sendOk()`, `parseBody()` helpers
- REST API: `/api/status`, `/api/config`, `/api/mapping`, `/api/modules` (JSON via ArduinoJson v7)
- Static files served from LittleFS: `/receiver/` and `/transmitter/` directories

### Simulation
- Shim headers in `firmware/sim/include/` replace hardware APIs
- ESP-NOW → UDP multicast on localhost
- FreeRTOS → pthreads
- I²C (Wire) → in-memory stubs
- Preferences → in-memory key-value store
- LCD+LVGL → SDL2 (sim_tx only)
- `sim_build.py` adds shim includes and sources automatically

### Configuration (`odh-config`)
- All constants are `inline constexpr` in `Config.h`
- `odh::config::` – shared (radio channel, failsafe, I²C pins, battery ADC)
- `odh::config::rx::` – receiver (PCA9685 addr, channel count, announce interval, web AP)
- `odh::config::tx::` – transmitter (mux addr, slot count, LCD pins, touch pins, web AP)
- `NvsStore` – RAII wrapper around `Preferences` for persistent key-value storage

## CI / GitHub Actions

The CI workflow (`.github/workflows/ci.yml`) runs on push/PR to main/master:

| Job               | What it does                                |
|--------------------|---------------------------------------------|
| `native-tests`     | `pio test -e native` (90 Unity tests)       |
| `build-transmitter`| `pio run -e transmitter`                    |
| `build-receiver`   | `pio run -e receiver`                       |
| `build-sim-rx`     | `pio run -e sim_rx`                         |
| `build-sim-tx`     | `pio run -e sim_tx` (needs `libsdl2-dev`)   |
| `clang-format`     | Formatting check (clang-format-19)          |
| `cppcheck`         | Static analysis                             |

After ANY code change, verify at minimum:
1. `pio test -e native` – all 90 tests pass
2. `pio run -e receiver` – compiles
3. `pio run -e transmitter` – compiles
4. **clang-format** – all changed files must be formatted before committing
5. **cppcheck** – no new warnings allowed

### clang-format (mandatory)

CI **rejects** any PR with formatting violations. Always run clang-format **19**
on every touched `.cpp` / `.h` / `.c` file before committing:

```bash
# Format a single file
clang-format -i --style=file firmware/src/receiver/web/ReceiverApi.cpp

# Format all project sources
find firmware/src firmware/lib \
  \( -name '*.cpp' -o -name '*.h' -o -name '*.c' \) \
  ! -path '*/.*' \
  -exec clang-format -i --style=file {} +
```

The `.clang-format` file in the repo root defines the style. Do **not** override
it with `--style=…` other than `--style=file`.

### cppcheck (mandatory)

CI **rejects** any PR that introduces new cppcheck warnings. Run locally:

```bash
cppcheck \
  --error-exitcode=1 \
  --enable=warning,style,performance,portability \
  --suppress=missingIncludeSystem \
  --suppress=missingInclude \
  --suppress=unmatchedSuppression \
  --check-level=exhaustive \
  --inline-suppr \
  -I firmware/src/receiver \
  -I firmware/src/transmitter \
  -I firmware/lib/odh-protocol \
  -I firmware/lib/odh-config \
  -I firmware/lib/odh-radio \
  -I firmware/lib/odh-telemetry \
  -I firmware/lib/odh-web \
  firmware/src \
  firmware/lib
```

If a warning is a false positive, suppress it inline:
```cpp
// cppcheck-suppress constParameterCallback
void onData(uint8_t *mac, uint8_t *data, int len) { … }
```

## Common Pitfalls

- **Working directory**: Always `cd firmware` before running PlatformIO commands.
- **ESP-NOW 250B limit**: Any new packet struct must have a `static_assert(sizeof(…) <= 250)`.
- **`#ifndef NATIVE_SIM` guards**: Hardware-specific code (WiFi, ESP-NOW, Wire, FreeRTOS tasks) must be guarded for simulation builds.
- **clang-format version**: CI uses clang-format **19**. Different versions may produce different output – always format locally with version 19 before pushing.
- **Callback signatures**: ESP-IDF/Arduino callback types have fixed signatures – don't add `const` to parameters that match a library-defined callback typedef. Use `// cppcheck-suppress constParameterCallback` if needed.
- **Include paths**: Shared library headers are included as `<Protocol.h>`, `<Config.h>`, etc. (not relative paths). Application headers use `"relative/path.h"`.
- **`build_src_filter`**: Receiver builds only `+<receiver/>`, transmitter only `+<transmitter/>`. Shared code goes into `lib/`.
