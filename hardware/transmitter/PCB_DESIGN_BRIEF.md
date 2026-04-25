# OpenDriveHub Transmitter — PCB Design Brief

Reference document for the parallel KiCad session. Pulls together the
component selection, mechanical envelope, signal architecture, and
layout positions that are already locked in by the case work.

Source of truth for **dimensions** is the OpenSCAD code in
`hardware/transmitter/parts/` and `hardware/transmitter/case/`. If a
number here disagrees with the .scad files, the .scad files win.

---

## 1. Project overview

DIY RC transmitter, "Mode 2" landscape format, ~286 × 136 mm panel.

- ESP32 controller (module choice TBD; ESP32-S3 WROOM-1 likely)
- 4.0" ST7796 IPS display (480×320) with FT6236 capacitive touch
- 2S LiPo, 2000 mAh nominal, USB-C charging via IP5389
- 7 sub-panel PCBs handling switches/buttons/encoders, all bridged to
  the main PCB via a shared I2C bus + PCF8574 I/O expanders
- 28 mm speaker (audio feedback / RX telemetry alarms)

The case shell is locked in (top + bottom shell, battery cover, battery
lid, USB-C panel-mount mount, etc.). The PCBs need to fit under the
existing panel cutouts and not collide with the corner bosses, side
walls, joystick bodies, or the battery compartment.

---

## 2. System architecture

```
                     ┌──────────────────────┐
                     │  USB-C female (ext.) │  AliExpress 17mm-pitch pigtail
                     └──────────┬───────────┘
                                │  (cable to main PCB)
                                ▼
   ┌─────────────────────────────────────────────────────────┐
   │  MAIN PCB                                               │
   │  ──────────────────────────────────────────────────     │
   │  · USB-C male plug socket (mates with extension)        │
   │  · IP5389 charger / boost controller (8.4V LiPo ↔ 5V)   │
   │  · 3.3V LDO/buck for ESP32 + I/O expanders              │
   │  · ESP32-S3 WROOM-1 module (or equivalent)              │
   │  · ST7796 display interface (SPI)                       │
   │  · FT6236 touch interface (I2C — shared bus)            │
   │  · Audio amp (PAM8403 or similar) → 28mm speaker        │
   │  · 2× joystick ADC headers (5-pin: V, GND, X, Y, SW)    │
   │  · 7× sub-PCB connectors (4-pin: V, GND, SDA, SCL)      │
   │  · LiPo + balance connector (JST-XH 3-pin for 2S)       │
   └────┬────────────────────────────────────────────────────┘
        │ I2C bus (SDA, SCL, 3.3V, GND)
        │
   ┌────┴────┬────────┬────────┬────────┬────────┬────────┐
   ▼         ▼        ▼        ▼        ▼        ▼        ▼
 toggle3  toggle3  illum3   illum3  encoder1 encoder1  nav3
 (-X top)(+X top)(-X bot)(+X bot) (-X bot) (+X bot)(centre bot)
```

All sub-PCBs carry a PCF8574 8-bit I/O expander, all on the same I2C
bus. Use **PCF8574A** if 8 expanders end up sharing the bus (the touch
controller eats one PCF8574 address; PCF8574A has a different address
block to avoid the collision).

---

## 3. PCB list

| # | PCB           | Qty | Purpose                                      | Approx. footprint  |
| - | ------------- | --- | -------------------------------------------- | ------------------ |
| 1 | `main`        | 1   | brain, charging, display, audio, I2C master  | ~80 × 60 × 1.6 mm  |
| 2 | `toggle3`     | 2   | 3 long-bat toggles + PCF8574                 | ~60 × 30 × 1.6 mm  |
| 3 | `illum3`      | 2   | 3 illuminated buttons + PCF8574              | ~60 × 30 × 1.6 mm  |
| 4 | `encoder1`    | 2   | 1 EC11 encoder + PCF8574                     | ~25 × 25 × 1.6 mm  |
| 5 | `nav3`        | 1   | 3 tact buttons + PCF8574                     | ~35 × 25 × 1.6 mm  |

Sub-PCB footprints are estimates — the real bound is "must fit under
the corresponding panel cutout cluster without poking outside the case
wall". Cluster spans (X axis) from `parts/layout_front.scad`:

- **toggle3**: 3 toggles 18 mm apart → 36 mm switch span; allow ~12 mm
  margin → ~60 mm wide.
- **illum3**: 3 illum buttons 18 mm apart → 36 mm; same margin → 60 mm.
- **encoder1**: single encoder, ~12 × 12 mm body → 25 mm wide PCB.
- **nav3**: 3 tact buttons 10 mm apart → 20 mm switch span → 35 mm wide.

Y-axis (height) of each sub-PCB: aim for ~30 mm so the cluster fits
between panel and 27 mm of cavity (TOP_DEPTH 30 − PANEL_T 3 = 27 mm).
Sub-PCB sits 1-2 mm above its panel bushings (component bodies clear
the PCB) — so PCB Z position is roughly panel_interior − 16 to −20 mm
in case coords.

Connector on each sub-PCB: 4-pin 2.54 mm header (or JST-PH 4) with
**3.3V, GND, SDA, SCL**.

PCF8574 wiring on each sub-PCB:
- Address pins A0/A1/A2 set per PCB (jumper or hard-wired) so all 7
  share one bus
- 8 GPIO pins drive button/switch inputs to ground (with internal
  pull-ups). Encoder needs 2 inputs for A/B + 1 for switch = 3 pins.

---

## 4. Component dimensions (footprint inputs for KiCad)

### 4.1 JH-D400X-R4 joystick (×2)

`parts/joystick.scad`:

- Base plate: 38 × 38 mm, 2 mm thick — 4 corner mount holes
- Body cylinder: Ø30 × 28 mm below the base
- Pin block: 20 × 8 × 6 mm extending below the body
- Pins: 5 (VCC, GND, X-pot, Y-pot, switch)
- Boot height above base: 9 mm
- Shaft length: 22 mm; knob Ø16, height 16 mm

Mount holes — see `JS_MOUNT_HOLE_D` and `JS_MOUNT_PITCH` in
`parts/joystick.scad` (verify in code).

Joystick is **above** the panel — connects to main PCB via 5-pin cable
(direct ADC, NOT through PCF8574).

### 4.2 Toggle switches — long-bat (×6)

Generic mini long-lever toggles, 3-position likely (ON-OFF-ON):

- Bushing: M6 (6 mm OD threaded), 8 mm panel-thread length
- Body below panel: ~13 × 8 × 17 mm
- Pins: 3 (common + 2 throws); 0.1 inch pitch typical

Verify with calipers — `parts/toggle_switch.scad` has the case-side
cutout dimensions.

### 4.3 EC11 rotary encoder + push (×2)

`parts/encoder.scad`:

- Body: 12 × 12 × 7 mm (typical EC11 family)
- Bushing: ~7 mm flat-flat, M7 thread
- Shaft: Ø6 × 20 mm, D-cut (`EC_SHAFT_FLAT = 0.5`)
- Pins: 5 — 3 for encoder (A, B, common) + 2 for push switch

PCB footprint with mounting tabs — standard EC11 footprint in
KiCad libraries.

### 4.4 Illuminated buttons (×6)

Round momentary, panel-mount, with built-in LED:

- Bushing: M12 thread typical (verify), Ø16 cap
- Body length below panel: ~15 mm
- Pins: 4 (switch contacts ×2, LED anode, LED cathode)

`parts/button_illuminated.scad` for cutout dim.

### 4.5 Nav tact buttons (×3)

6 × 6 mm tact (12 mm centerline pitch on `nav3` PCB; 10 mm in case
layout, see open question below):

- Body: 6 × 6 × 5 mm SMD or THT
- Cap: Ø4-5 mm above panel
- Pins: 4 (2 pairs internally connected)

`parts/button_tact.scad`. Surface-mount preferred for low profile.

### 4.6 Display — 4.0" ST7796 IPS + FT6236 touch (×1)

- Module: ~99 × 64 × 5 mm including components
- Visible area: ~85 × 56 mm
- Interface: ST7796 = SPI (4-wire SPI + DC + RESET); FT6236 = I2C
- Mount: usually 4 corner holes M2 or M2.5

`parts/display.scad` for current footprint and panel cutout.

The display module typically comes pre-assembled with both controllers
on a single PCB; main PCB just provides FFC connector or wire harness.

### 4.7 USB-C female extension cable (×1)

`parts/usb_c_extension_cable.scad`:

- Pigtail: USB-C female (panel mount) on one end → USB-C male plug on
  the other, ~20 cm of cable in between
- Panel-mount housing: 22 × 9 × 5 mm
- 2 × M2 screw holes at **17 mm pitch** (built into the housing)
- USB 2.0 only (D+, D-, VBUS, GND, CC1, CC2)

The MAIN PCB has a regular USB-C female connector that the male end
of this pigtail plugs into. KiCad library entry: standard USB-C
receptacle (SMD, 16-pin or 24-pin depending on USB-PD support
requirements).

### 4.8 Speaker

- 28 mm round, ~5 mm thick incl. magnet
- 8 Ω, 0.5-1 W
- Wire connection (solder pads on speaker)

Mount to back panel of bottom shell (TBD position — deferred).

### 4.9 LiPo battery (×1)

`parts/parameters.scad` `BATT_BODY = [80, 35, 15]`:

- 2S, 2000 mAh nominal
- Body: 80 × 35 × 15 mm pouch
- Power leads: 18 AWG silicone (red/black) at 6 mm centerline
- Balance connector: JST-XH 3-pin (2S) at 10 mm Y-offset

Connectors on main PCB:
- 2-pin power (XT30 or solder pads)
- 3-pin balance (JST-XH)

### 4.10 IP5389 charging IC

- USB-C input, supports up to 18W USB-PD charging
- 2-cell LiPo charge management
- 5V boost output (for any 5V loads on main PCB)
- QFN package, multiple support components (inductor, MOSFETs,
  bypass caps) — see Injoinic IP5389 datasheet/reference design

### 4.11 ESP32

ESP32-S3-WROOM-1 (recommended) or ESP32-WROOM-32E:

- 18 × 25 mm module footprint
- Castellated edge pads
- Built-in PCB antenna or U.FL connector for external

S3 has native USB so the USB-C port can also be used for serial /
firmware upload directly to the ESP32 without an external USB-UART.

### 4.12 PCF8574 / PCF8574A I/O expanders (×7)

- 8-bit I/O expander, I2C interface
- Address: 3 select pins → 8 unique addresses
- PCF8574: 0x20-0x27
- PCF8574A: 0x38-0x3F
- TSSOP-20 or SOIC-20 package
- Built-in pull-ups not super strong, may need external 4.7 kΩ on SDA/SCL

7 expanders fit in 8 addresses of one part. Use PCF8574 for 7 sub-PCBs;
keep PCF8574A range free in case the FT6236 touch controller's address
collides (FT6236 default = 0x38).

---

## 5. Layout positions (from `parts/layout_front.scad`)

Coordinates in panel-mating-face-centred system, units mm, +X right,
+Y up:

| Component       | Center positions                              |
| --------------- | --------------------------------------------- |
| Display         | (0, 12)                                       |
| Joystick L      | (-85, 12)                                     |
| Joystick R      | (+85, 12)                                     |
| Toggle row Y    | 45                                            |
| Toggle L (×3)   | (-120, 45) (-102, 45) (-84, 45)               |
| Toggle R (×3)   | (+84, 45) (+102, 45) (+120, 45)               |
| Illum row Y     | -32                                           |
| Illum L (×3)    | (-103, -32) (-85, -32) (-67, -32)             |
| Illum R (×3)    | (+67, -32) (+85, -32) (+103, -32)             |
| Encoder L       | (-30, -42)                                    |
| Encoder R       | (+30, -42)                                    |
| Nav buttons (×3)| (-10, -42) (0, -42) (+10, -42)                |
| Battery centre  | (0, -25)  (in bottom shell)                   |
| USB-C centre    | (80, ?, on -Y wall)                           |

Sub-PCB mounting positions (TBD — depends on PCB outline) should mirror
the cluster centres.

Outer panel: **280 × 130 mm** (PANEL_W × PANEL_H).
Outer case: **286 × 136 mm**.
Available cavity inside top shell: ≈ 268 × 122 × 27 mm.

---

## 6. Cabling between PCBs

### 6.1 Sub-PCB to main PCB (×7)

4-wire ribbon or JST-PH 4-pin: **3.3V, GND, SDA, SCL**.

Length: 100-200 mm typical (long enough to route around the joysticks
and battery without strain).

Recommended connector on main PCB: 7 × JST-PH 4 (4-pin, 2.0 mm pitch),
laid out so each cable goes to its nearest cluster.

### 6.2 Joystick to main PCB (×2)

5-wire to a JST-PH 5 (or solder direct):
**3.3V, GND, X-pot wiper, Y-pot wiper, push-switch**.

Joysticks read directly via ESP32 ADC (NO I/O expander) for low latency
and full 12-bit resolution.

### 6.3 USB-C extension

Between USB-C panel-mount female (in bottom shell wall) and USB-C male
plug end → goes into a USB-C female receptacle on the main PCB. Just
one short pigtail; main PCB sees a standard USB-C signal.

### 6.4 Battery

- 2-pin XT30 male socket on main PCB ↔ XT30 female on battery leads,
  OR solder pads with screw terminal
- 3-pin JST-XH balance header on main PCB ↔ JST-XH on battery balance
  lead

### 6.5 Display

FFC ribbon (typically 18-wire for ST7796 + FT6236 combined), or 2
separate harnesses (display SPI + touch I2C). Length ≤ 50 mm to keep
SPI signal integrity.

### 6.6 Speaker

2 wires to solder pads on main PCB. Audio amp drives directly.

---

## 7. Power & charging

```
USB-C 5V/3A   ─→ IP5389 ─→ 2S charging  (8.4V max)
                    │
                    └─→ 5V boost output (for amp, optional)
                    
2S battery 7.4V ─→ Buck regulator ─→ 3.3V (ESP32, expanders, displays)
                                    
                  Direct 5V (boost) ─→ PAM8403 audio amp
```

Open: ESP32-S3 needs 3.3V; the WROOM-1 module includes its own LDO so
the input can be either 3.3V or 5V — pick the cleaner rail.

I2C pull-ups: 4.7 kΩ on SDA and SCL on the **main PCB only** — the
sub-PCBs have a single PCF8574 each and shouldn't add their own.

---

## 8. Open questions / TBD

- Joystick exact pinout & mounting hole pitch — measure when in hand
- Toggle switch exact body dimensions — measure
- Illuminated button exact bushing thread (M12 vs M16) — measure
- Display module: confirm SPI vs parallel RGB; confirm FT6236 vs another
  touch IC
- Tact button spacing on `nav3` PCB: case layout uses 10 mm but verify
  the chosen part has compatible pad pitch
- Audio amp choice: PAM8403 (class-D, 3W stereo) vs simpler
- Speaker connector: solder pads vs JST
- ESP32 variant: S3 (native USB, more I/O) vs classic (cheaper, more
  mature) — recommendation: S3 for the USB story
- Counter-MOSFET / power-switch topology between battery and 3.3V
  regulator (soft-start, polarity protection)
- Whether to add a real-time clock (RTC) for telemetry timestamping
- SD card slot for log files? (separate header into main PCB)

---

## 9. Repository layout

```
hardware/transmitter/
├── parts/                   ← OpenSCAD vitamins (source of truth for dims)
│   ├── battery.scad
│   ├── button_illuminated.scad
│   ├── button_tact.scad
│   ├── display.scad
│   ├── encoder.scad
│   ├── joystick.scad
│   ├── layout_front.scad    ← CENTRAL layout positions
│   ├── parameters.scad
│   ├── speaker.scad
│   ├── toggle_switch.scad
│   ├── usb_c_extension_cable.scad
│   └── utils.scad
├── case/                    ← OpenSCAD case shell
│   ├── parameters.scad
│   ├── top_shell.scad
│   ├── bottom_shell.scad
│   ├── battery_cover.scad
│   ├── battery_lid.scad
│   └── assembly_check.scad
└── PCB_DESIGN_BRIEF.md      ← THIS FILE
```

KiCad project should live alongside, e.g. `hardware/transmitter/pcb/`
with a sub-directory per board (`pcb/main/`, `pcb/toggle3/`, etc.).

When PCB outlines & mounting hole positions are finalised, mirror them
back into `parts/<pcb_name>.scad` so the case can model the standoffs
that hold each PCB.

---

## 10. Constraints from the case

- Top shell internal cavity: 268 × 122 × 27 mm (W × D × H above panel
  interior, with taper toward 280 × 130 at mating rim)
- Case-corner bosses at (±129, ±56) — ~6 mm OD pillars from panel to
  mating rim with stiffener fins, do NOT route PCBs into these corners
- 4 toggle bushings + 6 illum buttons + 2 encoder bushings + 3 nav
  buttons + 2 joystick mounting plates eat into the top-shell cavity
- Battery occupies (0, -25) in the bottom shell, 80 × 35 × 15 mm + 1 mm
  wall clearance
- USB-C adapter at (80, on -Y wall) — sub-PCBs near this corner must
  leave clearance for the cable bend radius

If a sub-PCB collides with a case feature, the case takes precedence —
the sub-PCB has to shrink or move, since the panel cutouts are fixed by
ergonomics.

---
