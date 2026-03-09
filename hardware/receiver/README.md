# Receiver Hardware

The OpenDriveHub receiver is mounted in a vehicle and drives servos / ESCs
via a **PCA9685** I²C PWM driver.  It runs on an **ESP32** development board.

---

## Block Diagram

```
                  ┌─────────────────────┐
         I²C     │    ESP32-DevKitC     │
  ┌──────────────┤ SDA=21  SCL=22      │
  │              │                      │
  │              │ GPIO34 (ADC) ← Batt  │
  │              │                      │
  │              │ GPIO0 ← Config btn   │
  │              └─────────────────────┘
  │
  ├── PCA9685 (0x40)
  │     ├── CH0 → Servo / ESC 0
  │     ├── CH1 → Servo / ESC 1
  │     ├── …
  │     └── CH7 → Servo / ESC 7
  │
  └── (pull-ups 4k7 to 3V3)
```

---

## Bill of Materials

| Qty | Component | Notes |
|-----|-----------|-------|
| 1 | ESP32-DevKitC (or equivalent) | 38-pin recommended |
| 1 | PCA9685 16-channel PWM driver | Adafruit 815 or generic breakout |
| 2 | 4.7 kΩ resistors | I²C pull-ups on SDA / SCL |
| 1 | Voltage divider (100 kΩ + 33 kΩ) | Battery ADC on GPIO34 |
| 1 | 5 V BEC / regulator | Servo power supply |
| n | Servos or ESCs | Up to 8 channels (default) |

---

## GPIO Pinout

### I²C Bus

| GPIO | Function |
|------|----------|
| 21 | SDA |
| 22 | SCL |

The PCA9685 connects to this bus at address **0x40**
(`odh::config::rx::kPca9685Addr`).

### Battery ADC

| GPIO | Function |
|------|----------|
| 34 | Battery voltage via divider |

### Config Button (optional)

| GPIO | Function |
|------|----------|
| 0 | Config mode trigger (active LOW, internal pull-up) |

> **Note:** The web configuration AP starts automatically at every boot.
> The config button is reserved for future use (e.g. factory reset).

---

## PCA9685 Wiring

The PCA9685 board has its own power input for the servo rail (usually
labelled **V+**).  This must be supplied by an appropriate BEC or regulator
(typically 5 V / 6 V depending on your servos).  The logic side is powered
by 3.3 V from the ESP32.

| PCA9685 Pin | Connects to |
|-------------|-------------|
| VCC | ESP32 3V3 |
| GND | ESP32 GND + servo GND |
| SDA | ESP32 GPIO21 |
| SCL | ESP32 GPIO22 |
| V+ | BEC 5 V (servo power) |
| OE | Leave unconnected or tie LOW (outputs enabled) |

Servo signal wires connect to the PCA9685 output headers (CH0–CH7).

---

## Programming

```bash
cd firmware
pio run -e receiver -t upload
pio run -e receiver -t monitor   # 115200 baud
```

---

## WiFi Configuration

The receiver starts a WiFi AP automatically at every boot:

- **SSID:** `ODH-Receiver-Config` (configurable in `odh::config::rx::kWebApSsid`)
- **Password:** `odhrecv1` (configurable in `odh::config::rx::kWebApPass`)

Open `http://192.168.4.1` in a browser to access the web UI.

REST API endpoints:
- `GET /api/status` – link state, battery, vehicle name
- `GET /api/config` – vehicle name, model type
- `POST /api/config` – update vehicle settings (JSON body)
- `GET /api/mapping` – current function→channel mapping
- `POST /api/mapping` – update mapping (JSON body)

---

## Binding (Discovery)

The receiver uses **receiver-initiated discovery**:

1. On power-up (and whenever disconnected), the receiver broadcasts an
   `odh::AnnouncePacket` every 500 ms containing its vehicle name and
   model type.
2. A transmitter in scanning mode receives the announcement and displays
   the vehicle on its LCD.
3. When the user selects this vehicle, the transmitter sends a
   `odh::BindPacket` to the receiver's MAC address.
4. The receiver transitions to **Connected** and begins accepting
   `ControlPacket` data.

---

## Failsafe

If no `ControlPacket` is received within `odh::config::kRadioFailsafeTimeoutMs`
(default **500 ms**):

- All output channels are set to `odh::config::kFailsafeChannelValue`
  (default **1500 µs** = center / neutral).
- The receiver reports `LinkState::Failsafe` in its telemetry.

If the link remains lost for `odh::config::kRadioLinkTimeoutMs`
(default **3000 ms**), the receiver returns to **Disconnected** and resumes
broadcasting announcements.

---

## Output Channels

The default channel count is **8** (`odh::config::rx::kChannelCount`).  The
PCA9685 hardware supports up to 16 channels – increase this constant and
connect additional servos as needed.

Each channel receives a PWM signal in the standard RC range:
- **1000 µs** = full reverse / minimum
- **1500 µs** = center / neutral
- **2000 µs** = full forward / maximum

The `OutputManager` maps incoming `odh::FunctionValue` entries to physical
PCA9685 channels using the function→channel mapping (configurable via the
web UI or `odh::defaultFunctionMap()`).

---

## Configuration Reference

All receiver constants are in `firmware/lib/odh-config/Config.h` under the
`odh::config::rx` namespace.
