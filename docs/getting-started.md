# Getting Started with OpenDriveHub

This guide walks you through building and flashing the transmitter firmware for
the first time.

---

## Prerequisites

### Software

- [PlatformIO](https://platformio.org/) IDE plugin for VS Code, or PlatformIO Core CLI
- Git

### Hardware (minimum)

- ESP32 development board (ESP32-DevKitC recommended)
- TCA9548A I²C multiplexer breakout
- ILI9341 320×240 SPI LCD with XPT2046 resistive touch controller
- At least one input module (switch, button, potentiometer, or encoder)
- USB cable for programming

---

## Step 1: Clone the Repository

```bash
git clone https://github.com/peterus/OpenDriveHub.git
cd OpenDriveHub
```

---

## Step 2: Wire the Transmitter Hardware

Follow the wiring diagram in `hardware/transmitter/README.md`.

**Minimum connections:**

| ESP32 Pin | Connects to |
|-----------|-------------|
| GPIO21 (SDA) | TCA9548A SDA |
| GPIO22 (SCL) | TCA9548A SCL |
| GPIO23 (MOSI) | ILI9341 SDI / XPT2046 DIN |
| GPIO19 (MISO) | ILI9341 SDO / XPT2046 DOUT |
| GPIO18 (SCK) | ILI9341 SCK / XPT2046 CLK |
| GPIO5 | ILI9341 CS |
| GPIO27 | ILI9341 DC |
| GPIO26 | ILI9341 RST |
| GPIO32 | ILI9341 Backlight |
| GPIO4 | XPT2046 CS |
| GPIO2 | XPT2046 IRQ |
| 3V3 | TCA9548A VCC, ILI9341 VCC, XPT2046 VCC |
| GND | TCA9548A GND, ILI9341 GND, XPT2046 GND |

Add 4.7 kΩ pull-up resistors from 3V3 to both SDA and SCL lines.

---

## Step 3: Plug In a Module

Connect a module to **Slot 0** (TCA9548A channel 0 – SD0/SC0 pins).
See `hardware/modules/*/README.md` for individual module wiring.

---

## Step 4: Configure (optional)

Edit `firmware/lib/odh-config/Config.h` if your hardware differs from
the defaults (e.g. different GPIO pins, mux address, or slot count).

Configuration is organised as `inline constexpr` values in the
`odh::config` namespace:
- `odh::config::tx::` – transmitter-specific settings (LCD pins, slot count, etc.)
- `odh::config::rx::` – receiver-specific settings (PCA9685 address, channel count, etc.)
- `odh::config::` – shared settings (radio channel, failsafe timeout, battery ADC, etc.)

---

## Step 5: Build and Flash

```bash
cd firmware
pio run -e transmitter -t upload
```

Open the serial monitor at 115 200 baud to see startup messages:

```
[ODH] OpenDriveHub transmitter starting...
[ODH] Display OK
[ODH] Backplane mux OK
[ODH] Modules detected: P.....
[ODH] Radio (ESP-NOW) OK
[ODH] Scanning for vehicles...
```

---

## Step 6: Connect to a Receiver

OpenDriveHub uses **receiver-initiated discovery**.  Each receiver periodically
broadcasts an announcement containing its vehicle name and model type.  The
transmitter scans for these announcements and shows discovered vehicles on the
touch screen.

1. Power on the receiver.  It begins broadcasting its presence automatically.
2. Power on the transmitter.  The display enters scan mode and lists
   discovered vehicles as they appear.
3. Tap the vehicle you want to control on the touch screen.
4. The transmitter sends a bind packet and the link is established within a
   few seconds.
5. The display switches to control mode (channel bars + telemetry).

To disconnect, tap the **Disconnect** button on the control screen.  The
transmitter returns to scan mode and the receiver resumes announcing.

---

## Step 7: Web Configuration

Both the transmitter and receiver start a WiFi configuration interface
**automatically at every boot** – no button press required.

### Transmitter

1. The transmitter starts a WiFi Access Point:
   - SSID: `ODH-<DeviceName>` (default `ODH-TX`)
   - Password: `opendrv1`
2. Connect your phone or laptop to this network.
3. Open `http://192.168.4.1` in a browser.
4. Adjust settings and click **Save**.

### Receiver

1. The receiver starts a WiFi Access Point:
   - SSID: `ODH-Receiver-Config`
   - Password: `odhrecv1`
2. Connect to this network and open `http://192.168.4.1`.
3. Configure the vehicle name, model type, and function-to-channel mapping.

> **Note:** The transmitter and receiver each run their own independent AP.
> You can only connect to one at a time from a single device.

---

## Step 8: Running the Simulation (no hardware needed)

The simulation builds the full firmware for Linux, replacing hardware
peripherals with software shims (SDL2 display, UDP radio, pthreads).

### Prerequisites

```bash
sudo apt install pkg-config libsdl2-dev
```

### Run

Open two terminals:

```bash
cd firmware

# Terminal 1 – receiver
pio run -e sim_rx -t exec

# Terminal 2 – transmitter (headless terminal simulation)
pio run -e sim_tx -t exec

# Optional: transmitter with SDL2 GUI (requires libsdl2-dev)
pio run -e sim_tx_gui -t exec
```

The receiver starts announcing immediately.  The headless transmitter simulation
runs in the terminal.  For a graphical display, use `sim_tx_gui` which opens an
SDL2 window showing the scan screen.  After a few seconds the vehicle appears as
a button – click it to connect.

The web config UI is also available in simulation:
- Transmitter: `http://localhost:8080`
- Receiver: `http://localhost:8081`

See **[firmware/sim/README.md](../firmware/sim/README.md)** for the full keyboard
reference, connection flow, and known limitations.

---

## Running the Native Tests

Tests for the protocol utilities, function mapping, and display helpers run on
your host machine (no hardware needed):

```bash
cd firmware
pio test -e native
```

Expected output:
```
test/test_native/test_protocol.cpp:…
test/test_native/test_function_map.cpp:…
test/test_native/test_display.cpp:…
…
90 test cases: 90 succeeded
```

---

## Next Steps

- Add more modules – plug one into each slot and reboot; they are detected
  automatically.
- Read `docs/architecture.md` for an overview of the firmware design.
- Read `docs/protocol.md` for the radio protocol specification.
- Read `docs/backplane.md` for backplane and module slot details.
