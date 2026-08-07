# FDM Tolerances & Standard Part Dimensions

Numbers below assume FDM printing with ~0.4mm nozzle. Adjust for resin/SLS.

## Print tolerances (printer-dependent; tune per printer)

| Fit | Offset (radial) | Use |
|---|---|---|
| Free / clearance | +0.3 to +0.4 mm | screw pass-through, part that must slide |
| Slip fit | +0.15 to +0.2 mm | fits together by hand, stays together |
| Press fit | -0.05 to -0.1 mm | needs force, permanent |
| Heat-set insert | -0.1 mm below insert OD | insert melts its way in |

Add these to the **radius** (i.e. diameter += 2× offset).

## Wall & feature minimums

- **Min wall thickness**: 1.2mm (3 perimeters @ 0.4mm nozzle). Below that → weak or under-extruded.
- **Min hole diameter (vertical)**: 2mm; smaller shrinks closed.
- **Min feature**: 0.8mm (lines too thin to print).
- **Overhang angle**: up to ~45° from vertical unprinted. >45° needs support or bridging.
- **Bridge distance**: reliably up to ~10mm; sag beyond that.

## Layer-adhesion direction

Z-axis is the weak axis. Orient load-bearing features so stress is in XY, not pulling layers apart. Screw bosses take tension → print upright (boss axis = Z) is BAD for pull-out strength; consider inserts instead.

## Screws (ISO metric, common in electronics/RC)

| Size | Clearance hole Ø | Tap hole Ø (self-tap into print) | Head Ø (cap) | Head Ø (pan) | Head H (cap) |
|---|---|---|---|---|---|
| M2 | 2.4 | 1.6 | 3.8 | 4.0 | 2.0 |
| M2.5 | 2.9 | 2.1 | 4.5 | 5.0 | 2.5 |
| M3 | 3.4 | 2.5 | 5.5 | 6.0 | 3.0 |
| M4 | 4.5 | 3.3 | 7.0 | 8.0 | 4.0 |
| M5 | 5.5 | 4.2 | 8.5 | 10.0 | 5.0 |

Prefer NopSCADlib's `screw()` and `screw_hole_radius()` over these numbers.

## Heat-set inserts (common voron/3dp sizes)

| Thread | Insert OD | Insert length | Pocket Ø (nominal) | Pocket depth |
|---|---|---|---|---|
| M2 | 3.5 | 4.0 | 3.4 | 4.5 |
| M2.5 | 4.0 | 4.0 | 3.9 | 4.5 |
| M3 | 4.0 | 4.0 | 3.9 | 5.0 |
| M3 (long) | 4.0 | 5.7 | 3.9 | 6.2 |
| M4 | 5.6 | 5.0 | 5.5 | 5.5 |

Pocket Ø = insert OD − 0.1mm so heat+press forms a tight bond. Pocket depth = insert length + 0.5mm so shavings have somewhere to go.

Use `NopSCADlib/vitamins/insert.scad` which models these correctly.

## Common PCB / component hole patterns

| Part | Mounting | Notes |
|---|---|---|
| Raspberry Pi 4 | M2.5, 49 × 58 mm | 4 holes |
| Raspberry Pi Pico | M2, 11.4 × 47 mm | 4 holes |
| ESP32 DevKit v1 | no std. mounting — usually clip or glue |
| Arduino Uno | M3, mixed pattern | see NopSCADlib |
| SSD1306 0.96" OLED | M2 or M2.5, 23 × 23.5 mm | check specific board |
| JH-D400X-R4 joystick | M3, 30 × 30 mm (typical clone) | 4 holes, verify per unit |

Treat these as starting points — always verify against the physical part or its datasheet.

## Shrinkage / offsetting

Use `offset=0` at first. If fits are consistently tight/loose, measure a calibration part (Ø10mm hole, Ø10mm shaft) and set a single `FIT_TOL` constant in `parameters.scad` — don't sprinkle per-feature fudge factors.
