# Transmitter Hardware

The OpenDriveHub transmitter is a hand-held controller built around an
**ESP32** development board.

---

## Block Diagram

```
                  ┌─────────────────────┐
         I²C     │    ESP32-DevKitC     │  SPI
  ┌──────────────┤ SDA=21  SCL=22      ├────────────┐
  │              │                      │            │
  │              │ GPIO34 (ADC) ← Batt  │    ┌───────┴───────┐
  │              │                      │    │  ILI9341 LCD  │
  │              │ MOSI=23 MISO=19      │    │  320×240      │
  │              │ SCK=18  CS=5         │    │  + XPT2046    │
  │              │ DC=27   RST=26       │    │    Touch      │
  │              │ BL=32   T_CS=4       │    └───────────────┘
  │              │ T_IRQ=33             │
  │              └─────────────────────┘
  │
  ├── TCA9548A (0x70)
  │     ├── CH0 → Slot 0
  │     ├── CH1 → Slot 1
  │     ├── …
  │     └── CH5 → Slot 5
  │
  └── (pull-ups 4k7 to 3V3)
```

---

## Bill of Materials

| Qty | Component | Notes |
|-----|-----------|-------|
| 1 | ESP32-DevKitC (or equivalent) | 38-pin recommended |
| 1 | ILI9341 SPI LCD 320×240 | 2.4″ or 2.8″ breakout |
| 1 | XPT2046 resistive touch controller | Usually integrated on LCD board |
| 1 | TCA9548A I²C multiplexer breakout | Adafruit 2717 or generic |
| 2 | 4.7 kΩ resistors | I²C pull-ups on SDA / SCL |
| 1 | Voltage divider (100 kΩ + 33 kΩ) | Battery ADC on GPIO34 |
| 1 | 2S LiPo battery (7.4 V) | Optional – can be USB-powered |
| 6 | 4-pin headers / connectors | Module slot connectors |

---

## GPIO Pinout

### SPI Display (ILI9341)

| GPIO | Function |
|------|----------|
| 23 | MOSI (SDI) |
| 19 | MISO (SDO) |
| 18 | SCK |
| 5 | CS (chip select) |
| 27 | DC (data / command) |
| 26 | RST (reset) |
| 32 | Backlight (active high) |

### Touch (XPT2046)

| GPIO | Function |
|------|----------|
| 4 | T_CS (touch chip select) |
| 33 | T_IRQ (touch interrupt) |

SPI bus is shared between LCD and touch.

### I²C Bus

| GPIO | Function |
|------|----------|
| 21 | SDA |
| 22 | SCL |

All modules and the TCA9548A mux share this bus at 400 kHz.

### Battery ADC

| GPIO | Function |
|------|----------|
| 34 | Battery voltage via divider |

Divider ratio: `odh::config::kBatteryDividerRatio` (default 4.03).

---

## Programming

```bash
cd firmware
pio run -e transmitter -t upload
pio run -e transmitter -t monitor   # 115200 baud
```

---

## WiFi Configuration

The transmitter starts a WiFi AP automatically at every boot:

- **SSID:** `OpenDriveHub-Config` (configurable in `odh::config::tx::kWebApSsid`)
- **Password:** `opendrv1` (configurable in `odh::config::tx::kWebApPass`)

Open `http://192.168.4.1` in a browser to access the web UI.

REST API endpoints:
- `GET /api/status` – link state, battery, RSSI
- `GET /api/modules` – detected modules per slot
- `GET /api/mapping` – current function→channel mapping
- `POST /api/mapping` – update mapping (JSON body)

---

## Module Slot Connector

Each slot provides a **4-pin header**:

| Pin | Signal |
|-----|--------|
| 1 | 3V3 |
| 2 | GND |
| 3 | SDAn |
| 4 | SCLn |

SDAn / SCLn are the TCA9548A downstream channel pins for that slot.

---

## Enclosure Considerations

- The LCD needs a rectangular cutout of approximately 50 × 35 mm (varies by
  breakout board).
- Leave openings or slots on the sides for module headers.
- The USB port should remain accessible for firmware updates.
- A power switch between the battery and the 3V3 regulator is recommended.
