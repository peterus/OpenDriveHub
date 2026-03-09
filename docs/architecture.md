# Architecture

OpenDriveHub consists of a **transmitter** (hand-held controller) and a
**receiver** (vehicle-mounted).  They communicate over ESP-NOW, a
connectionless, low-latency IEEE 802.11 protocol.

Both devices run on ESP32 and share a common set of libraries.  The entire
firmware is built from a single PlatformIO project located in `firmware/`.

---

## High-Level Block Diagram

```
╭─────────────────────────────────────────────────────╮
│                   TRANSMITTER                       │
│                                                     │
│  ┌───────────┐  ┌────────────────┐  ┌───────────┐  │
│  │ Modules   │──│ ModuleManager  │──│ RadioLink │──┼──── ESP-NOW ────┐
│  │ (I²C via  │  └────────┬───────┘  └───────────┘  │                │
│  │  TCA9548A)│           │                          │                │
│  └───────────┘  ┌────────┴───────┐                  │                │
│                 │ TransmitterApp │                   │                │
│                 └────────┬───────┘                   │                │
│         ┌────────────────┼────────────────┐          │                │
│  ┌──────┴──────┐ ┌───────┴───────┐ ┌─────┴──────┐  │                │
│  │  Display    │ │ BatteryMonitor│ │ WebServer  │  │                │
│  │ (LVGL/LCD) │ │ (Telemetry)   │ │ + REST API │  │                │
│  └─────────────┘ └───────────────┘ └────────────┘  │                │
╰─────────────────────────────────────────────────────╯                │
                                                                       │
╭─────────────────────────────────────────────────────╮                │
│                    RECEIVER                         │                │
│                                                     │                │
│  ┌───────────┐  ┌────────────────┐  ┌───────────┐  │                │
│  │ PCA9685   │──│ OutputManager  │──│ RadioLink │──┼──── ESP-NOW ────┘
│  │ (I²C PWM) │  └────────┬───────┘  └───────────┘  │
│  └───────────┘           │                          │
│                 ┌────────┴───────┐                  │
│                 │  ReceiverApp   │                  │
│                 └────────┬───────┘                  │
│         ┌────────────────┼────────────────┐         │
│  ┌──────┴──────┐ ┌───────┴───────┐ ┌─────┴──────┐  │
│  │ Telemetry   │ │ BatteryMonitor│ │ WebServer  │  │
│  │ (announce / │ │               │ │ + REST API │  │
│  │  sensor)    │ └───────────────┘ └────────────┘  │
│  └─────────────┘                                    │
╰─────────────────────────────────────────────────────╯
```

---

## Directory Layout

```
firmware/
├── platformio.ini         # Unified build: receiver, transmitter, native, sim_rx, sim_tx
├── lib/                   # Shared libraries (used by both targets)
│   ├── odh-protocol/      #   Protocol.h, FunctionMap.h
│   ├── odh-config/        #   Config.h, NvsStore.h (compile-time + NVS)
│   ├── odh-radio/         #   ReceiverRadioLink, TransmitterRadioLink
│   ├── odh-telemetry/     #   BatteryMonitor, TelemetryData
│   └── odh-web/           #   OdhWebServer, ApiHandler
├── src/
│   ├── receiver/          # Receiver application
│   │   ├── ReceiverApp.h/cpp
│   │   ├── OutputManager.h/cpp
│   │   ├── output/        # IOutputDriver, Pca9685Output, LoggingOutput
│   │   ├── web/           # ReceiverApi
│   │   └── main.cpp
│   └── transmitter/       # Transmitter application
│       ├── TransmitterApp.h/cpp
│       ├── backplane/     # Backplane (TCA9548A mux)
│       ├── display/       # Display (LVGL), display_utils
│       ├── modules/       # IModule, ModuleManager, 4 module types
│       ├── web/           # TransmitterApi
│       └── main.cpp
├── data/
│   ├── receiver/          # Static web UI (HTML/CSS/JS) for receiver
│   └── transmitter/       # Static web UI for transmitter
├── include/               # Board-level overrides (lv_conf.h, User_Setup.h)
├── sim/                   # Simulation shim headers for sim_rx / sim_tx
└── test/
    └── test_native/       # Unity tests (90 cases)
```

---

## Shared Libraries (firmware/lib/)

Each library is self-contained and has no dependencies on application code.

| Library | Purpose | Key Types |
|---------|---------|-----------|
| **odh-protocol** | Radio protocol structs, enums, checksum utilities, function mapping | `odh::ControlPacket`, `odh::TelemetryPacket`, `odh::BindPacket`, `odh::AnnouncePacket`, `odh::FunctionValue`, `odh::Function`, `odh::PacketType`, `odh::ModelType`, `odh::LinkState` |
| **odh-config** | Compile-time constants and NVS persistence | `odh::config::*`, `odh::config::rx::*`, `odh::config::tx::*`, `odh::NvsStore` |
| **odh-radio** | ESP-NOW radio link abstraction | `odh::ReceiverRadioLink`, `odh::TransmitterRadioLink` |
| **odh-telemetry** | Battery monitoring and telemetry data | `odh::BatteryMonitor`, `odh::TelemetryData` |
| **odh-web** | HTTP server and REST API helpers | `odh::OdhWebServer`, `odh::ApiHandler` |

---

## Transmitter Software Layers

### FreeRTOS Tasks

| Task | Core | Priority | Rate | Purpose |
|------|------|----------|------|---------|
| Control | 1 | 3 | 50 Hz | Module scan → packet build → radio TX |
| Display | 0 | 1 | ~4 Hz | LVGL tick → screen repaint (scan / control) |
| Web | 0 | 1 | async | ESPAsyncWebServer – REST API + static files |

### Application Flow

1. **`main.cpp`** constructs a `TransmitterApp` and creates FreeRTOS tasks.
2. **`TransmitterApp`** owns `ModuleManager`, `Display`, `TransmitterRadioLink`,
   `BatteryMonitor`, `OdhWebServer`, and `TransmitterApi`.
3. **`ModuleManager`** scans each slot through the **`Backplane`** (TCA9548A mux)
   and instantiates the detected **`IModule`** implementation:
   - `PotModule` (ADS1115 – 16-bit analog, 0x48)
   - `SwitchModule` (PCF8574 – 8-bit I/O expander, 0x20)
   - `ButtonModule` (PCF8574 – 8-bit I/O expander, 0x21)
   - `EncoderModule` (AS5600 – magnetic rotary encoder, 0x36)
4. Each module produces a `FunctionValue` array that is packed into
   an `odh::ControlPacket` and dispatched to the `TransmitterRadioLink`.
5. **`Display`** drives the ILI9341 LCD via LVGL.  It has two screens:
   - *Scan screen* – shows discovered vehicles; tap to bind
   - *Control screen* – channel bars, trim values, telemetry (battery, RSSI)
6. **`TransmitterApi`** (via `OdhWebServer` + `ApiHandler`) exposes:
   - `GET /api/status` – link state, battery, RSSI
   - `GET /api/modules` – detected module list
   - `GET /api/mapping` – function→channel map
   - `POST /api/mapping` – update function map
   - Static files from LittleFS `/transmitter/`

### Module Detection

When a slot is selected via `Backplane::selectSlot(n)`, the `ModuleManager`
probes known I²C addresses:

| Address | Chip | Module |
|---------|------|--------|
| 0x48 | ADS1115 | `PotModule` |
| 0x20 | PCF8574 | `SwitchModule` |
| 0x21 | PCF8574 | `ButtonModule` |
| 0x36 | AS5600 | `EncoderModule` |

If no known device responds, the slot is marked empty.

---

## Receiver Software Layers

### FreeRTOS Tasks

| Task | Core | Priority | Rate | Purpose |
|------|------|----------|------|---------|
| Output | 1 | 3 | continuous | Apply latest function values to PCA9685 |
| Telemetry | 0 | 1 | 10 Hz | Build + send `TelemetryPacket` back to TX |
| Web | 0 | 1 | async | ESPAsyncWebServer – REST API + static files |

### Application Flow

1. **`main.cpp`** constructs a `ReceiverApp` and creates FreeRTOS tasks.
2. **`ReceiverApp`** owns `OutputManager`, `ReceiverRadioLink`,
   `BatteryMonitor`, `TelemetryData`, `OdhWebServer`, and `ReceiverApi`.
3. **`ReceiverRadioLink`**:
   - In **Disconnected** state, periodically broadcasts an `AnnouncePacket`
     with the vehicle name and model type so that transmitters can discover it.
   - On receiving a `BindPacket`, transitions to **Connected** and starts
     accepting `ControlPacket` data.
   - On timeout (`kRadioFailsafeTimeoutMs`), enters **Failsafe** and
     applies safe channel values.
4. **`OutputManager`** routes incoming `FunctionValue[]` to the appropriate
   **`IOutputDriver`**:
   - `Pca9685Output` – writes PWM pulse widths to a PCA9685 I²C PWM driver
   - `LoggingOutput` – debug fallback that logs values to serial
5. **`ReceiverApi`** (via `OdhWebServer` + `ApiHandler`) exposes:
   - `GET /api/status` – link state, battery, vehicle name
   - `GET /api/mapping` – function→channel map
   - `POST /api/mapping` – update function map
   - `GET /api/config` – vehicle name, model type
   - `POST /api/config` – update vehicle settings
   - Static files from LittleFS `/receiver/`

---

## Protocol Summary

See `docs/protocol.md` for the full specification.

| Packet | Size | Direction | Rate |
|--------|------|-----------|------|
| `ControlPacket` | 73 B | TX → RX | 50 Hz |
| `TelemetryPacket` | 30 B | RX → TX | 10 Hz |
| `BindPacket` | 11 B | TX → RX | on bind/disconnect |
| `AnnouncePacket` | 28 B | RX → broadcast | 2 Hz |

All packets share a common 6-byte header:

```
[magic0 'O'][magic1 'D'][version][type][sequence_lo][sequence_hi]
```

The last byte of every packet is an XOR checksum of all preceding bytes
(computed by `odh::checksum()`).

---

## Backplane

See `docs/backplane.md` for details on the TCA9548A-based I²C multiplexer
bus that connects input modules to the transmitter.

---

## Simulation

Two simulation environments (`sim_rx`, `sim_tx`) build the full firmware as
a Linux executable.  Hardware peripherals are replaced by software shims:

| Hardware | Simulation Shim |
|----------|----------------|
| ESP-NOW | UDP multicast on 127.0.0.1 |
| FreeRTOS | pthreads / BSD timers |
| I²C (Wire) | In-memory virtual bus |
| Preferences (NVS) | In-memory key-value store |
| ILI9341 LCD + LVGL | SDL2 window (sim_tx only) |
| PCA9685 / ADS1115 | Console logging |
| WiFi / WebServer | Localhost HTTP |

```bash
cd firmware
pio run -e sim_rx -t exec   # receiver simulation
pio run -e sim_tx -t exec   # transmitter simulation (opens SDL2 window)
```

See `firmware/sim/README.md` for keyboard shortcuts and known limitations.

---

## Build Environments

The unified `firmwave/platformio.ini` defines five environments:

| Environment | Target | Board | Framework |
|-------------|--------|-------|-----------|
| `receiver` | ESP32 | esp32dev | Arduino |
| `transmitter` | ESP32 | esp32dev | Arduino |
| `native` | Host | native | — |
| `sim_rx` | Host (Linux) | native | — |
| `sim_tx` | Host (Linux) | native | — |

Build commands:

```bash
cd firmware
pio run -e receiver          # build receiver
pio run -e transmitter       # build transmitter
pio test -e native           # run 90 unit tests
pio run -e sim_rx -t exec    # run receiver sim
pio run -e sim_tx -t exec    # run transmitter sim
```

---

## C++20 Style

The codebase uses Modern C++ (ESP32 supports `gnu++2a`, native uses `gnu++20`):

- **`enum class`** for all protocol enums (`PacketType`, `ModelType`, `Function`, `LinkState`)
- **`inline constexpr`** for all compile-time constants (replaces `#define`)
- **`std::optional`** for nullable returns (e.g. NVS lookups)
- **`std::unique_ptr`** for owned polymorphic objects (modules, output drivers)
- **Interfaces** via pure virtual classes with `virtual ~I…() = default`
- **RAII wrappers** (e.g. `NvsStore` for `Preferences`)
- **`namespace odh`** used consistently throughout; no global pollution
- **`static_assert`** for compile-time packet size checks
