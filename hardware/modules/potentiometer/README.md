# Potentiometer Module

## Overview

The potentiometer module provides **4 analogue inputs** using an ADS1115 16-bit ADC.
Typical applications: joystick axes, throttle levers, trim knobs, faders.

---

## Hardware

| Component | Part | Notes |
|-----------|------|-------|
| 1× | ADS1115 (SSOP-10) or breakout board | 16-bit ADC, 0x48 |
| 1–4× | 10 kΩ linear potentiometer | B10K, mono |
| 1× | 100 nF capacitor | ADS1115 VDD bypass |
| 1× | 4-pin JST-SH connector | Matches transmitter slot connector |

**I²C address:** ADDR pin tied to GND → **0x48**

---

## Circuit

```
Slot connector:
  GND ─────── GND (ADS1115 GND, all pot wipers and low ends)
  3V3 ─────── VDD (ADS1115) ── 100 nF ── GND
  SDA ─────── SDA (ADS1115)
  SCL ─────── SCL (ADS1115)

  ADDR (pin 1) → GND   (address 0x48)

Potentiometer wiring (×4):
  3V3 ──────── pot high end
  GND ──────── pot low end
  AINx ─────── pot wiper (AIN0 for ch1, AIN1 for ch2, …)
```

---

## Firmware Behaviour

- ADS1115 is configured for **±4.096 V range, single-ended, 860 SPS**.
- Each channel is read sequentially in single-shot mode.
- Raw ADC value 0 → channel **1000 µs** (MIN)
- Raw ADC value 32767 → channel **2000 µs** (MAX)
- Values are linearly mapped; no deadband is applied by default.

---

## Notes

- The ESP32 supply is 3.3 V, so pots will not reach full ADS1115 input range
  (±4.096 V).  Maximum input ≈ 3.3 V → raw value ≈ 26428.
  The firmware maps the full 0–32767 range regardless; adjust `POT_ADC_MAX`
  in `PotModule.h` to match your supply if you need full-range calibration.
- ADS1115 is 5 V tolerant on inputs – a 5 V supply for pots is also acceptable.
