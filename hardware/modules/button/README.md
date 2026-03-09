# Button Module

## Overview

The button module provides **8 independent momentary push-button inputs** via a
PCF8574 I²C GPIO expander.  It is electrically identical to the switch module
but uses I²C address **0x21** (A0 tied HIGH), allowing it to coexist with a switch
module on the same PCB or be distinguished during slot scanning.

---

## Hardware

| Component | Part | Notes |
|-----------|------|-------|
| 1× | PCF8574AP (DIP-16) or PCF8574T (SO-16) | I²C GPIO expander, 0x21 |
| 8× | Momentary tactile push-button | SPST-NO, close to GND |
| 1× | 100 nF capacitor | Bypass on VCC pin |
| 1× | 4-pin JST-SH connector | Matches transmitter slot connector |

**I²C address:** A0 tied to 3V3, A1–A2 tied to GND → **0x21**

---

## Circuit

```
Slot connector:
  GND ─────── GND (PCF8574 pin 8, all button commons)
  3V3 ─────── VCC (PCF8574 pin 16) ── 100 nF ── GND
  SDA ─────── SDA (PCF8574 pin 15)
  SCL ─────── SCL (PCF8574 pin 1)

Address pins:
  A0 (pin 1) → 3V3
  A1 (pin 2) → GND
  A2 (pin 3) → GND

Each input:
  PCF8574 Px pin ── Button ── GND
```

---

## Firmware Behaviour

- Button **pressed** → Px low → channel **2000 µs** (MAX)
- Button **released** → Px high → channel **1000 µs** (MIN)
