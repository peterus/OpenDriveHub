# Encoder Module

## Overview

The encoder module provides **one absolute rotary encoder input** using an AS5600
12-bit contactless magnetic position sensor.  The encoder reports a true absolute
angle (0–4095 per revolution) with no count accumulation or overflow.

---

## Hardware

| Component | Part | Notes |
|-----------|------|-------|
| 1× | AS5600 (SOIC-8) or breakout board | Magnetic encoder, fixed address 0x36 |
| 1× | Diametrically magnetised disc magnet | 6 mm × 2.5 mm, N/S through diameter |
| 1× | Rotary shaft / knob | Must hold the magnet centred above AS5600 |
| 1× | 100 nF capacitor | AS5600 VDD bypass |
| 1× | 4-pin JST-SH connector | Matches transmitter slot connector |

**I²C address:** Fixed at **0x36** (not configurable).

---

## Circuit

```
Slot connector:
  GND ─────── GND (AS5600)
  3V3 ─────── VDD (AS5600) ── 100 nF ── GND
  SDA ─────── SDA (AS5600)
  SCL ─────── SCL (AS5600)

  DIR pin → GND   (clockwise = increasing angle)
```

The AS5600 has no address-select pin.  Because the backplane mux gives each slot its
own I²C channel, address conflicts between multiple encoder modules in different slots
are automatically avoided.

---

## Magnet Placement

Mount the diametrically magnetised disc magnet on the encoder shaft, centred directly
above the AS5600 IC, with a gap of **0.5–3 mm**.  The magnet must be parallel to the
IC surface.  An alignment LED on the AS5600 breakout indicates correct field strength.

---

## Firmware Behaviour

- Reads the **ANGLE** register (0x0C/0x0D, 12-bit) in every `update()` call.
- Raw angle 0 → channel **1000 µs** (MIN)
- Raw angle 4095 → channel **2000 µs** (MAX)
- Linear mapping, no filtering applied by default.
