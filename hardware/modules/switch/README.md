# Switch Module

## Overview

The switch module provides **8 independent toggle switch inputs** via a single
PCF8574 I²C GPIO expander.  It plugs into any transmitter slot on the backplane.

Because the backplane multiplexer isolates each slot to its own I²C channel,
every switch module uses the same default PCF8574 address (0x20) without conflict.

---

## Hardware

| Component | Part | Notes |
|-----------|------|-------|
| 1× | PCF8574AP (DIP-16) or PCF8574T (SO-16) | I²C GPIO expander, 0x20 |
| 8× | SPDT toggle switch | Or SPST – one terminal to GND, other to GPIO pin |
| 8× | 10 kΩ resistor (optional) | External pull-up (PCF8574 has weak internal pull-ups ~100 µA) |
| 1× | 100 nF capacitor | Bypass on VCC pin |
| 1× | 4-pin JST-SH connector | Matches transmitter slot connector |

**I²C address:** A0–A2 all tied to GND → **0x20**

---

## Circuit

```
Slot connector:
  GND ─────── GND (PCF8574 pin 8, all switch commons)
  3V3 ─────── VCC (PCF8574 pin 16) ── 100 nF ── GND
  SDA ─────── SDA (PCF8574 pin 15)
  SCL ─────── SCL (PCF8574 pin 1)

Each input:
  PCF8574 Px pin ── Switch ── GND
  (switch open → Px high via internal pull-up; switch closed → Px low)
```

Address pins:
```
  A0 (pin 1) → GND
  A1 (pin 2) → GND
  A2 (pin 3) → GND
```

---

## Firmware Behaviour

- Bit = **0** in the PCF8574 byte: switch **closed** → channel **2000 µs** (MAX)
- Bit = **1** in the PCF8574 byte: switch **open** → channel **1000 µs** (MIN)

Each switch maps to one logical channel in the order P0–P7.
