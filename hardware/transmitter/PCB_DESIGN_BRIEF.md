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
- 7 sub-panel PCBs handling switches/buttons/encoders, fanned out via a
  TCA9548A I²C multiplexer on the main PCB so each sub-PCB sits on its
  own private downstream channel (all use the default PCF8574A address
  0x38 — no per-board jumpering, no collisions)
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
   │  · ST7796 display interface (SPI, direct from ESP32)    │
   │  · TCA9548A I²C multiplexer (1 upstream → 8 downstream) │
   │      ch 0..6: 7 sub-PCB headers (5-pin JST-XH each)     │
   │      ch 7   : FT6236 touch controller                   │
   │  · Audio amp (PAM8403 or similar) → 28mm speaker        │
   │  · 2× joystick ADC headers (5-pin: V, GND, X, Y, SW)    │
   │  · LiPo + balance connector (JST-XH 3-pin for 2S)       │
   └────┬────────────────────────────────────────────────────┘
        │ TCA9548A downstream channels (each is its own
        │ private I²C bus: 3.3V, GND, SDA, SCL + INT line)
        │
   ┌────┴────┬────────┬────────┬────────┬────────┬────────┐
   ▼         ▼        ▼        ▼        ▼        ▼        ▼
 toggle3  toggle3  illum3   illum3  encoder1 encoder1  nav3
 (-X top)(+X top)(-X bot)(+X bot) (-X bot) (+X bot)(centre bot)
```

All sub-PCBs carry a **PCF8574A** 8-bit I/O expander at the default
address **0x38** (A0=A1=A2 tied to GND on the sub-PCB silkscreen-and-
copper). Address conflicts are impossible because the TCA9548A on the
main PCB only enables one downstream channel at a time — every
sub-PCB lives in its own private bus segment. No per-board jumpering
required.

Each sub-PCB also exposes its PCF8574A's open-drain `/INT` pin to the
main PCB, so the firmware can sleep until input changes instead of
polling. Pull-up for `/INT` lives on the main PCB.

---

## 3. PCB list

| # | PCB           | Qty | Purpose                                      | Outline           | Status |
| - | ------------- | --- | -------------------------------------------- | ----------------- | ------ |
| 1 | `main`        | 1   | brain, charging, display, audio, I²C mux     | ~80 × 60 × 1.6 mm | TBD    |
| 2 | `toggle3`     | 2   | 3 long-bat toggles + PCF8574A                | ~60 × 30 × 1.6 mm | TBD    |
| 3 | `illum3`      | 2   | 3 illuminated buttons + PCF8574A             | ~60 × 30 × 1.6 mm | TBD    |
| 4 | `encoder1`    | 2   | 1 EC11 encoder + PCF8574A                    | ~30 × 30 × 1.6 mm | TBD    |
| 5 | `nav3`        | 1   | 3 tact buttons + PCF8574A                    | **32 × 30 × 1.6 mm** | **DONE** (`pcb/nav3/`) |

Sub-PCB footprints are estimates — the real bound is "must fit under
the corresponding panel cutout cluster, leave room for SOIC-16W IC
plus 4 corner mount holes plus a 5-pin JST-XH connector, and not poke
outside the case cavity". Cluster spans (X axis) from
`parts/layout_front.scad`:

- **toggle3**: 3 toggles 18 mm apart → 36 mm switch span; ~60 mm wide
- **illum3**: 3 illum buttons 18 mm apart → 36 mm span; ~60 mm wide
- **encoder1**: single encoder, ~12 × 12 mm body; ~30 mm both axes
- **nav3**: 3 tact buttons 10 mm apart → 20 mm switch span; 32 mm wide

**Sizing reality check from nav3** (the first board through KiCad):
the original 32 × 16 mm estimate was too short — the SOIC-16W body of
the PCF8574A on B.Cu plus 4× M2 corner mounts (3 mm inset) plus a
JST-XH 5-pin connector at one edge needed **32 × 30 mm**. Expect the
other sub-PCB outlines (especially encoder1 at "25 × 25") to grow
similarly when actually laid out.

Y-axis (height) of each sub-PCB: target ≤ 30 mm so the cluster fits
between panel and 27 mm of cavity (TOP_DEPTH 30 − PANEL_T 3 = 27 mm).
Sub-PCB sits 3-4 mm below the panel-mating face (the SUBPCB_Z_*
constants in `parts/layout_front.scad` are the source of truth);
component bodies hang below the PCB into the cavity.

**Connector** on each sub-PCB: 5-pin **JST-XH** 2.5 mm vertical header
(B5B-XH-A or equivalent), oriented so the cable exits toward the back
of the case. Pinout (matching nav3):

| Pin | Net      |
|-----|----------|
| 1   | 3V3      |
| 2   | GND      |
| 3   | I2C_SDA  |
| 4   | I2C_SCL  |
| 5   | /INT     |

**PCF8574A wiring** on each sub-PCB (default address 0x38):
- A0/A1/A2 hard-tied to GND
- 8 GPIO pins drive switch/button/encoder inputs to ground (internal
  weak pull-ups hold high at rest)
- /INT (open-drain) goes out on connector pin 5; pull-up lives on the
  main PCB
- 100 nF decoupling on VDD/VSS, 0805 metric (hand-solder friendly)

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

### 4.12 PCF8574A I/O expanders (×7)

- 8-bit I/O expander, I²C interface
- Default address **0x38** (A0=A1=A2 to GND on every sub-PCB — no
  per-board configuration; the TCA9548A on the main PCB isolates each
  sub-PCB on its own bus segment)
- SOIC-16W (`Package_SO:SOIC-16W_7.5x10.3mm_P1.27mm`) — large enough
  to hand-solder, large enough to drive sub-PCB minimum height
- Open-drain `/INT` pin out → connector pin 5 on every sub-PCB
- 100 nF X7R 0805 decoupling cap right next to VDD/VSS

### 4.13 TCA9548A I²C multiplexer (main PCB)

- 8-channel I²C switch
- Address 0x70 (configurable via A0/A1/A2 if needed)
- Each downstream channel gets its own 4.7 kΩ SDA/SCL pull-ups on the
  main PCB (PCF8574A pull-ups on the sub-PCBs are too weak)
- Drives 7 sub-PCB connectors + 1 channel for the FT6236 touch IC
- TSSOP-24 package, 3.3V supply

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

5-wire JST-XH 2.5 mm: **3.3V, GND, SDA, SCL, /INT** (in that pin
order, matching nav3 ref board).

Length: 100-200 mm typical (long enough to route around the joysticks
and battery without strain).

Recommended connector on main PCB: 7 × JST-XH 5-pin vertical headers
(B5B-XH-A or compatible), laid out so each cable goes to its nearest
sub-PCB cluster. Each header is wired to one downstream channel of
the TCA9548A.

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

I²C pull-ups: 4.7 kΩ on SDA and SCL on the **main PCB only**, one
pair per downstream TCA9548A channel (so 8 pairs in total). Sub-PCBs
add nothing — the PCF8574A's internal weak pull-ups drive the switch
inputs but the bus pull-ups stay on the main side.

`/INT` lines from each sub-PCB also need a pull-up on the main PCB
(open-drain). 10 kΩ is fine.

---

## 8. Open questions / TBD

- Joystick exact pinout & mounting hole pitch — measure when in hand
- Toggle switch exact body dimensions — measure
- Illuminated button exact bushing thread (M12 vs M16) — measure
- Display module: confirm SPI vs parallel RGB; confirm FT6236 vs another
  touch IC
- Tact button spacing on `nav3` PCB: confirmed 10 mm (matches the case
  layout) — use the same `Button_Switch_THT:SW_PUSH_6mm_H5mm` footprint
  for consistency on toggle3 substitutes if needed
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
├── parts/                       ← OpenSCAD vitamins + KiCad-exported PCBs
│   ├── battery.scad
│   ├── button_illuminated.scad
│   ├── button_tact.scad
│   ├── display.scad
│   ├── encoder.scad
│   ├── joystick.scad
│   ├── layout_front.scad        ← CENTRAL layout positions
│   ├── parameters.scad
│   ├── pcb_subpanel.scad        ← parametric placeholders + STL imports
│   ├── speaker.scad
│   ├── toggle_switch.scad
│   ├── usb_c_extension_cable.scad
│   ├── utils.scad
│   ├── nav3_actual.step         ← canonical KiCad STEP, tracked
│   ├── nav3_actual.stl          ← regenerated from STEP, gitignored
│   └── step_to_stl.py           ← FreeCAD headless conversion script
├── case/                        ← OpenSCAD case shell + printed parts
│   ├── parameters.scad
│   ├── top_shell.scad
│   ├── bottom_shell.scad
│   ├── battery_cover.scad
│   ├── battery_lid.scad
│   ├── nav_button_cap.scad
│   └── assembly_check.scad
├── pcb/                         ← KiCad projects, one folder per board
│   ├── nav3/                    ← reference template (see STATUS.md, BOM.md)
│   │   ├── nav3.kicad_pro       ← project settings — copy to new boards
│   │   ├── nav3.kicad_sch       ← schematic (user-owned content)
│   │   ├── nav3.kicad_pcb       ← layout (user-owned)
│   │   ├── nav3.kicad_dru       ← design rules (JLCPCB tier) — copy
│   │   ├── nav3_local.kicad_sym ← inlined symbols workaround — copy + rename
│   │   ├── sym-lib-table        ← project-local lib registration — copy
│   │   ├── BOM.md
│   │   ├── STATUS.md
│   │   ├── fix_kicad9_compat.py ← schematic-side workaround — copy
│   │   ├── fix_pcb_nets.py      ← schematic-side workaround — copy
│   │   └── pcb_sync_bypass.py   ← schematic-side workaround — copy
│   ├── toggle3/                 ← TODO
│   ├── illum3/                  ← TODO
│   ├── encoder1/                ← TODO
│   └── main/                    ← TODO
└── PCB_DESIGN_BRIEF.md          ← THIS FILE
```

When a board is finalised:
1. KiCad → `kicad-cli pcb export step <board>.kicad_pcb -o <board>.step`
2. Copy STEP → `parts/<board>_actual.step` (commit this)
3. `STEP_IN=parts/<board>_actual.step STL_OUT=parts/<board>_actual.stl
   freecadcmd parts/step_to_stl.py` (STL is gitignored — regenerate
   locally from the committed STEP)
4. Update `parts/pcb_subpanel.scad` to import the new STL (mirror the
   `subpanel_pcb_nav3()` pattern: `USE_REAL_PCB` toggle, `<NAME>_STL_OFFSET`
   computed from the KiCad board outline, `render(convexity=10) import(...)`
   wrap so F6 in OpenSCAD shows the imported geometry correctly)
5. Adjust the layout offset in `parts/layout_front.scad` if the KiCad
   board centroid doesn't match the panel cutout cluster centre (nav3
   needed a `-5` mm Y shift because switches sit "north" of centroid)

---

## 9.5 Workflow split (AI ↔ user)

Empirical result from the nav3 board: scripted PCB layout was a dead
end (kicad-mcp-pro v2.4.x cannot reliably place + route on KiCad
9.0.7 from a clean schematic). The pragmatic division of labour is:

- **AI does**: schematic capture as a *starting template*, project
  scaffolding, design-rule files, BOM draft, STEP export + STL
  conversion + OpenSCAD case-fit verification.
- **User does**: schematic review and rework (the AI's draft is a
  starting point — pin assignments, decoupling, footprint choice, etc.
  may all need adjustment), and **the PCB layout in full**: footprint
  placement, routing, copper pours, silkscreen, mechanical keep-outs.
  Done in the KiCad GUI, not via scripts.

The AI hands off after `kicad-cli sch erc` is clean and a paper-tape
of nets + footprints exists. The user picks up there, owns the board
file from that point on, and emits a STEP when the layout is complete.
The AI then takes the STEP back into the OpenSCAD case-fit check.

## 9.6 Starting a new sub-PCB (template procedure)

`pcb/nav3/` is the canonical template — it carries project settings,
design rules, schematic-side workaround scripts, and an inlined symbol
library that all transfer directly to other sub-PCBs. To bootstrap a
new board (e.g. `toggle3`):

```bash
cd hardware/transmitter/pcb
cp -r nav3 toggle3
cd toggle3
# Rename the project files
for f in nav3.*; do mv "$f" "${f/nav3/toggle3}"; done
mv nav3_local.kicad_sym toggle3_local.kicad_sym
# Inside toggle3.kicad_pro / .kicad_sch / .kicad_pcb / sym-lib-table /
# any of the .py scripts: substitute the string "nav3" → "toggle3"
sed -i 's/nav3/toggle3/g' *.kicad_pro *.kicad_pcb sym-lib-table *.py
# Rip out the nav3 schematic content (sheet contents, NOT the file
# header) so you start from an empty schematic with the same project
# settings, DRC rules, and symbol library
# (best done in the KiCad GUI: open the schematic, select all, delete)
# Drop the failed PCB layout scripts that didn't work for nav3 either
rm build_pcb_layout.py place_footprints.py
# Drop the gitignored junk so they don't carry over
rm -rf nav3-backups output .kicad-mcp ~*.lck
```

Once the new project boots cleanly:

1. AI generates the schematic from the brief (parts list + nets).
2. Run `python3 fix_kicad9_compat.py` and `python3 fix_pcb_nets.py`
   to apply the nav3-discovered workarounds.
3. AI exports the schematic to PDF for user review.
4. **Hand off to user.** User reviews/reworks the schematic and does
   the entire PCB layout in the KiCad GUI.
5. After user-side DRC passes, AI runs `kicad-cli pcb export step` →
   `parts/<board>_actual.step` → `step_to_stl.py` → import into
   `pcb_subpanel.scad`.
6. Case-fit check via OpenSCAD assembly_check — tweak the
   `<NAME>_STL_OFFSET` and the `layout_front.scad` shift until the
   imported board's panel-side components line up with the case
   cutouts.

## 9.7 Gotchas inherited from nav3

These bit us during nav3 and will bite again unless avoided:

**Net mapping bug**: kicad-mcp-pro v2.4.x assigns every PCB pad to
`+3V3` after schematic-derived sync. Always re-run `fix_pcb_nets.py`
before checking the ratsnest. (Only relevant if any scripted PCB-side
edits happen at all — for a fully GUI-driven layout, this never fires.)

**Schematic file format**: kicad-mcp-pro emits KiCad-10 (date stamp
20250316), KiCad 9.0.7 wants KiCad-9 (20240920). `fix_kicad9_compat.py`
downgrades the format and inlines `extends`-based symbols (KiCad 9
can't load PCF8574AT-extends-PCF8574; the script flattens it into a
standalone symbol in `<board>_local.kicad_sym`).

**Reference designators must NOT start with `#`** (MCP skips those
when syncing footprints). Non-electrical decorations like OSHW logos
need a real-looking ref like `LOGO101`.

**STEP-to-STL gotcha**: KiCad's STEP export carries the PCB body, IC
bodies, connector body, etc. as separate solids that share faces.
OpenSCAD's CGAL backend (F6) rejects this as non-manifold and silently
drops geometry. `step_to_stl.py` already handles this by FreeCAD-fusing
all solids before tessellation, and `pcb_subpanel.scad` wraps the
import in `render()` as belt-and-braces. **Keep both.** If you change
the FreeCAD pipeline, run `admesh <board>_actual.stl` and confirm that
"Number of parts" stays in the single digits — high counts mean the
fuse failed.

**KiCad screen-Y vs STEP-Y**: KiCad PCB editor uses screen-style Y
(positive = down), STEP/STL use right-handed Y (positive = up). The
`<NAME>_STL_OFFSET` in `pcb_subpanel.scad` corrects for this; if a
new board appears mirrored after import, double-check the sign of
the Y components in the offset.

**Switch / connector centroid != PCB centroid**: nav3's switches are
5 mm "north" of the PCB centre because the connector takes the bottom
edge. The fix lives in `parts/layout_front.scad` (`translate([0,
NAV_BTN_Y - 5, SUBPCB_Z_NAV])`). Each new sub-PCB will need its own
offset measured once and hard-coded.

**Outline grew from estimate**: nav3 ended up at 32 × 30 mm vs the
original 32 × 16 mm guess. The driver was the SOIC-16W IC body on
B.Cu plus 4 corner mounts plus the JST-XH 5-pin connector. Plan for
similar growth on the other sub-PCBs — the design brief estimates
are minimums, not hard limits.

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
