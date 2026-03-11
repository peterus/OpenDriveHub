---
layout: default
title: OpenDriveHub
---

# OpenDriveHub

**Open-source modular control platform for functional model vehicles**  
Excavators · Dump trucks · Tractors · Ground-based RC models

[![License: GPL-3.0](https://img.shields.io/badge/License-GPL%203.0-blue.svg)](https://github.com/peterus/OpenDriveHub/blob/main/LICENSE)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-red.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Framework: Arduino + FreeRTOS](https://img.shields.io/badge/Framework-Arduino%20%2B%20FreeRTOS-green.svg)](https://platformio.org/)

---

## What is OpenDriveHub?

OpenDriveHub is a hobby-grade RC transmitter and receiver platform designed
specifically for **functional model vehicles** – not aircraft.  It focuses on
modularity, simplicity, and always-on telemetry.

The firmware is written in **modern C++20** with a unified `odh::` namespace,
`enum class` types, `constexpr` configuration, and shared libraries.

---

## Key Features

| Feature | Details |
|---------|---------|
| **Modular transmitter** | Plug-in front slots for switches, buttons, potentiometers, encoders |
| **I²C backplane** | TCA9548A mux – identical I²C devices in every slot, no address conflicts |
| **Bidirectional telemetry** | Battery voltage, RSSI, link state, and custom sensor values – always active |
| **ESP-NOW radio link** | No router or WiFi infrastructure needed; low latency |
| **ILI9341 touch-screen display** | 320×240 colour LCD with XPT2046 resistive touch; LVGL widget UI |
| **PCA9685 servo driver** | I²C-based 16-channel PWM output on the receiver |
| **Receiver-initiated discovery** | Receivers announce their presence; operator selects a vehicle on the touch screen |
| **Vehicle model presets** | Built-in function maps for Generic, Dump Truck, Excavator, Tractor, and Crane |
| **FreeRTOS firmware** | Dedicated tasks for control (50 Hz), display (4 Hz), and web config |
| **Always-on web configuration** | WiFi AP + REST API (ESPAsyncWebServer + ArduinoJson v7) starts automatically at boot on both TX and RX; settings stored in ESP32 NVS |
| **Linux simulation** | Full TX + RX simulation on a PC – SDL2 display window, keyboard channel control, UDP radio loopback; no hardware needed |
| **Modern C++20** | Unified `odh::` namespace, `enum class`, `constexpr`, `std::optional`, shared libraries |
| **Open hardware** | Standard, widely available components; no proprietary chips |
| **GPL-3.0** | Fully open firmware and hardware under GPL-3.0 |

---

## System Architecture

```
┌──────────────────────────────────┐       ESP-NOW        ┌────────────────────────┐
│         TRANSMITTER              │◄────────────────────►│      RECEIVER          │
│  ┌─────────────────────────┐     │    Control @ 50 Hz   │                        │
│  │ ESP32 (FreeRTOS)        │     │   Telemetry @ 10 Hz  │  ESP32 (FreeRTOS)      │
│  │  taskControl  (core 1)  │     │                      │  taskOutput  (core 1)  │
│  │  taskDisplay  (core 0)  │     │                      │  taskTelemetry (core 0)│
│  │  taskWebConfig (core 0) │     │                      │                        │
│  └─────────────────────────┘     │                      │  PCA9685 PWM outputs   │
│  ┌──────────────────────────┐    │                      │  Battery monitor       │
│  │ TCA9548A I²C Mux         │    │                      │  Failsafe on link loss │
│  │  Slot 0 … Slot 5         │    │                      └────────────────────────┘
│  │  [SW][BTN][POT][ENC][ ][ ]│   │
│  └──────────────────────────┘    │
│  SPI LCD (ILI9341 + XPT2046 touch, LVGL)  │
│  Battery ADC                              │
└──────────────────────────────────────────────┘
```

---

## Module Types

| Module | I²C Device | Channels | Application |
|--------|-----------|---------|-------------|
| **Switch** | PCF8574 @ 0x20 | 8 | Toggle switches for lights, attachments |
| **Button** | PCF8574 @ 0x21 | 8 | Momentary buttons for horn, reset, etc. |
| **Potentiometer** | ADS1115 @ 0x48 | 4 | Joystick axes, throttle levers, trim knobs |
| **Encoder** | AS5600 @ 0x36 | 1 | Absolute rotary knobs |

Modules are detected automatically at boot.  No configuration is needed – just
plug in and power on.

---

## Hardware

The transmitter and receiver are built from **standard, widely available components**:

- ESP32-DevKitC development board
- TCA9548A I²C multiplexer breakout
- ILI9341 320×240 SPI LCD with XPT2046 resistive touch (transmitter only)
- PCA9685 16-channel PWM driver (receiver only)
- PCF8574 GPIO expanders (switch/button modules)
- ADS1115 ADC (potentiometer module)
- AS5600 magnetic encoder (encoder module)

Full BOMs and wiring diagrams:
- [Transmitter Hardware](hardware/transmitter/)
- [Receiver Hardware](hardware/receiver/)
- [Switch Module](hardware/modules/switch/)
- [Button Module](hardware/modules/button/)
- [Potentiometer Module](hardware/modules/potentiometer/)
- [Encoder Module](hardware/modules/encoder/)

---

## Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/) (VS Code plugin or CLI)
- ESP32 development board (× 2: one for transmitter, one for receiver)

### Quick start

```bash
git clone https://github.com/peterus/OpenDriveHub.git
cd OpenDriveHub/firmware

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

See the **[Simulation Guide](https://github.com/peterus/OpenDriveHub/blob/main/firmware/sim/README.md)** for full details.

See **[Getting Started Guide](getting-started)** for full step-by-step instructions.

---

## Radio Protocol

OpenDriveHub uses a simple, compact binary protocol over ESP-NOW.
All types live in `namespace odh` (defined in `firmware/lib/odh-protocol/Protocol.h`):

| Direction | Packet | Rate | Size |
|-----------|--------|------|------|
| TX → RX | `odh::ControlPacket` | 50 Hz | 73 bytes |
| RX → TX | `odh::TelemetryPacket` | 10 Hz | 30 bytes |
| TX → RX | `odh::BindPacket` | once | 11 bytes |
| RX → broadcast | `odh::AnnouncePacket` | 2 Hz | 28 bytes |
| TX → RX | Disconnect (`odh::BindPacket` with type 0x40) | once | 11 bytes |

All packets use an XOR checksum.  See the [Protocol Reference](protocol) for details.

---

## Documentation

- [Getting Started](getting-started)
- [System Architecture](architecture)
- [Backplane & Modules](backplane)
- [Protocol Reference](protocol)
- [Simulation Guide](https://github.com/peterus/OpenDriveHub/blob/main/firmware/sim/README.md)

---

## License

OpenDriveHub is released under the **GNU General Public License v3.0**.  
You are free to use, modify, and distribute this project under the terms of that licence.

[View on GitHub](https://github.com/peterus/OpenDriveHub) ·
[Report an Issue](https://github.com/peterus/OpenDriveHub/issues) ·
[GPL-3.0 Licence](https://github.com/peterus/OpenDriveHub/blob/main/LICENSE)
