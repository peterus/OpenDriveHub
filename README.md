# OpenDriveHub

Open-source modular control platform for functional model vehicles (excavators,
dump trucks, tractors, and other ground-based RC models).

## Features

- **Modular transmitter** – plug-in front-panel slots for switches, buttons,
  potentiometers, and encoders; detected automatically at boot
- **I²C backplane** with TCA9548A multiplexer – identical standard I²C devices
  in every slot, no address conflicts, no MCU needed on modules
- **Always-on bidirectional telemetry** – battery voltage, RSSI, link state,
  and custom sensor values returned from the model
- **ESP32-based** firmware using PlatformIO + Arduino + FreeRTOS
- **Modern C++20** – `namespace odh`, `enum class`, `constexpr`, `std::optional`,
  RAII wrappers, shared libraries
- **ESP-NOW radio link** – low-latency, no router required
- **ILI9341 touch-screen display** – 320×240 colour LCD with XPT2046 resistive
  touch, LVGL widget UI for vehicle selection and live channel monitoring
- **PCA9685 servo driver** – I²C-based 16-channel PWM output on the receiver
- **Receiver-initiated discovery** – receivers broadcast their presence;
  the operator selects a vehicle on the transmitter touch screen
- **Vehicle model presets** – built-in function-to-channel maps for Generic,
  Dump Truck, Excavator, Tractor, and Crane models
- **Always-on web configuration** – WiFi AP + REST API (ESPAsyncWebServer +
  ArduinoJson v7) on both transmitter and receiver; starts automatically at
  boot, no button required; settings stored in ESP32 NVS
- **Linux simulation** – full transmitter + receiver simulation on a PC; SDL2
  display window, keyboard channel control, UDP radio loopback
- **Open hardware + firmware** under GPL-3.0

## Repository Structure

```
OpenDriveHub/
├── firmware/                   # Unified PlatformIO project (C++20, namespace odh::)
│   ├── platformio.ini          # Build config for all 5 environments
│   ├── lib/                    # Shared libraries
│   │   ├── odh-protocol/      # Packet definitions & function mapping
│   │   ├── odh-config/        # Compile-time configuration & NVS store
│   │   ├── odh-radio/         # ESP-NOW radio link (TX + RX)
│   │   ├── odh-telemetry/     # Battery monitor & telemetry data
│   │   └── odh-web/           # ESPAsyncWebServer + REST API helpers
│   ├── src/
│   │   ├── receiver/          # Receiver app (OutputManager, PCA9685, ReceiverApi)
│   │   └── transmitter/       # Transmitter app (Modules, Display, TransmitterApi)
│   ├── test/test_native/      # Native unit tests (90 tests, no hardware needed)
│   ├── data/                   # LittleFS web UI (receiver/ and transmitter/)
│   ├── include/                # LVGL & TFT_eSPI build-config headers
│   └── sim/                    # Linux simulation shims (SDL2, UDP, pthreads)
│       ├── include/            # Replacement headers (Arduino.h, esp_now.h, …)
│       └── src/                # Shim implementations
├── hardware/
│   ├── transmitter/            # Transmitter BOM and wiring reference
│   ├── receiver/               # Receiver BOM and wiring reference
│   └── modules/                # Per-module hardware documentation
│       ├── switch/
│       ├── button/
│       ├── potentiometer/
│       └── encoder/
└── docs/
    ├── index.md                # GitHub Pages one-pager
    ├── getting-started.md
    ├── architecture.md
    ├── backplane.md
    └── protocol.md
```

## Quick Start

See **[docs/getting-started.md](docs/getting-started.md)** for full build and
flash instructions.

```bash
cd firmware

# Flash transmitter
pio run -e transmitter -t upload

# Flash receiver
pio run -e receiver -t upload

# Run host-side unit tests (90 tests)
pio test -e native
```

### Run the simulation (no hardware needed)

```bash
cd firmware

# Terminal 1 – receiver
pio run -e sim_rx -t exec

# Terminal 2 – transmitter (headless terminal simulation)
pio run -e sim_tx -t exec
```

See **[firmware/sim/README.md](firmware/sim/README.md)** for full simulation docs.

## Supported Module Types

| Module | I²C Device | Channels |
|--------|------------|----------|
| Switch (toggle) | PCF8574 @ 0x20 | 8 |
| Button (momentary) | PCF8574 @ 0x21 | 8 |
| Potentiometer / fader | ADS1115 @ 0x48 | 4 |
| Encoder (absolute) | AS5600 @ 0x36 | 1 |

## Documentation

- [Getting Started](docs/getting-started.md)
- [System Architecture](docs/architecture.md)
- [Backplane & Modules](docs/backplane.md)
- [Radio Protocol](docs/protocol.md)
- [Transmitter Hardware](hardware/transmitter/README.md)
- [Receiver Hardware](hardware/receiver/README.md)
- [Simulation](firmware/sim/README.md)

## License

GPL-3.0 – see [LICENSE](LICENSE).
