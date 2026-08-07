# toggle3 — Bill of Materials

3-toggle I²C sub-PCB. A PCF8574A reads 3 panel-mount long-bat toggle switches and
reports their state to the main MCU over I²C, with a hardware interrupt line so the
host can sleep until a switch moves. Built **2×** (left and right toggle cluster).

I²C address: **0x38** (A0=A1=A2 → GND, hard-wired). Every sub-PCB uses this same
address; the TCA9548A on the main PCB gives each board its own private downstream
channel, so no per-board jumpering and no collisions.

Pull-ups (4.7 kΩ on SDA/SCL) and the `/INT` pull-up (10 kΩ) live on the **main PCB**,
exactly once per downstream channel — nothing here.
PCF8574A internal quasi-bidirectional pull-ups (~100 µA) hold the switch inputs at
VCC; a closed contact pulls the GPIO to GND.

## Parts

| Ref | Qty | Value | KiCad Symbol | KiCad Footprint | Source | Notes |
|-----|-----|-------|--------------|------------------|--------|-------|
| U101 | 1 | PCF8574A | `toggle3_local:PCF8574AT` | `Package_SO:SOIC-16W_7.5x10.3mm_P1.27mm` | project-local sym | **TI `PCF8574ADWR`** (DW = SOIC wide, body 10.30 × 7.50 mm per datasheet, 1.27 mm pitch). I²C 8-bit I/O expander. Flattened (non-`extends`) symbol copy so the library loads standalone |
| SW101, SW102, SW103 | 3 | SPDT ON-OFF-ON | `Switch:SW_SPDT` | **TBD** | std lib symbol, footprint hand-rolled | Mini long-bat toggle, M6 bushing, 3 solder lugs in a row. Footprint deferred — see "Open: switch footprint" |
| H101, H102, H103, H104 | 4 | MountingHole | `Mechanical:MountingHole` | `MountingHole:MountingHole_2.2mm_M2` | std lib | 4-corner mount to case (M2 screws) |
| C101 | 1 | 100 nF | `Device:C` | `Capacitor_SMD:C_0805_2012Metric` | std lib | Decoupling on U101 VDD, X7R, 0805 for hand-soldering |
| J101 | 1 | B5B-XH-A | `Connector_Generic:Conn_01x05` | `Connector_JST:JST_XH_B5B-XH-A_1x05_P2.50mm_Vertical` | std lib | JST-XH 5-pin (3V3, GND, SDA, SCL, INT), white locking crimp connector |
| LOGO101, LOGO102 | 2 | OSHW logo | `Graphic:Logo_Open_Hardware_Small` | `Symbol:OSHW-Logo_5.7x6mm_SilkScreen` | std lib | Silkscreen-only, F.Cu (LOGO101) and B.Cu mirrored (LOGO102, rot=180). Reference must NOT start with `#` or PCB sync skips it |

Total: **12 footprints** (10 BOM-relevant + 2 silk-only graphics). All from KiCad
standard libraries except the toggle footprint, which does not exist yet.

## Package choice — why SOIC-16W

Order **`PCF8574ADWR`**, not `PCF8574DWR`. The **A** variant has address base
`0b0111xxx`, so A0=A1=A2=GND gives **0x38** — the address this whole design is
built around. The non-A part is `0b0100xxx` → 0x20 and would silently break the
documented address.

Of what Mouser.at stocks for this part, measured as KiCad courtyard area (the
board area actually consumed, leads and clearance included):

| Package | Pins | Courtyard | Pitch |
|---------|------|-----------|-------|
| VQFN-16 | 16 | ~18 mm² | 0.50 mm, no leads |
| VQFN-20 | 20 | ~27 mm² | 0.50 mm, no leads |
| TVSOP-20 | 20 | 42.4 mm² | 0.40 mm |
| TSSOP-20 | 20 | 53.9 mm² | 0.65 mm |
| **SOIC-16W** | **16** | **128.1 mm²** | **1.27 mm** |
| PDIP-16 | 16 | 202.6 mm² | 2.54 mm, THT |

SOIC-16W is the largest sensible option and that is fine: the board is ~1400 mm²,
so the IC costs 9 % of it. Board size is set by the 36 mm switch span, the four
M2 corner holes and the JST-XH connector — nav3 grew from 16 to 30 mm for exactly
those reasons, not because of its IC. A VQFN would save ~110 mm² and shrink the
outline by zero.

What SOIC-16W buys instead: 1.27 mm pitch, twice as coarse as anything else on
the list, on a board that is hand-soldered (the 0805 passives are chosen on the
same grounds). VQFN has no accessible leads at all and needs stencil + reflow.

It also keeps the **16-pin pinout**, so the existing symbol and the net table
below stay valid. The 20-pin packages carry four NC pins and renumber everything
(VDD 16→5, VSS 8→15, P0 4→10) — they need `Interface_Expansion:PCF8574ATS` and a
full relabel at U101.

Note: `TSSOP-16`, which the built nav3 board carries, is **not orderable at
Mouser.at**. If more nav3 boards get built, that BOM needs the same change.

## Switch wiring — 2 GPIO per switch

Each toggle is wired as a full SPDT: common to GND, **both** throws to their own
GPIO. This costs 6 of 8 GPIOs but is a superset — the same board reads a
3-position ON-OFF-ON switch *and* a 2-position ON-ON switch with no layout change.

`Switch:SW_SPDT` names its pins **A / B / C**, and the middle one is the common —
verified by pin geometry, pin 2 sits alone on the opposite side from pins 1 and 3.
Net suffixes below deliberately reuse the symbol's own pin letters so the mapping
cannot be misread:

| SW pin | Pin name | Role | Net (SW101 / SW102 / SW103) |
|--------|----------|------|------------------------------|
| 1 | A | throw | `TOG1_A` / `TOG2_A` / `TOG3_A` |
| 2 | B | **common** | GND |
| 3 | C | throw | `TOG1_C` / `TOG2_C` / `TOG3_C` |

State decoding (pull-ups mean "low = contact closed"):

| Lever | `TOGn_A` | `TOGn_C` | Meaning |
|-------|----------|----------|---------|
| toward A | low | high | position A |
| centre | high | high | OFF (3-position switch only) |
| toward C | high | low | position C |
| — | low | low | impossible — use as fault/short detection in firmware |

With a 2-position ON-ON switch the centre row never occurs; firmware needs no
variant knowledge.

`A` / `C` are the symbol's pin letters, **not** lever directions. Which lug closes
when the bat points up depends on the switch and must be confirmed on the real
part — firmware can swap the mapping without a board respin.

## Net list

| U101 pin | Net | Connection |
|----------|-----|------------|
| 1 (A0) | GND | address bit 0 = 0 |
| 2 (A1) | GND | address bit 1 = 0 |
| 3 (A2) | GND | address bit 2 = 0 |
| 4 (P0) | TOG1_A | SW101 pin 1 (throw A) |
| 5 (P1) | TOG1_C | SW101 pin 3 (throw C) |
| 6 (P2) | TOG2_A | SW102 pin 1 (throw A) |
| 7 (P3) | TOG2_C | SW102 pin 3 (throw C) |
| 8 (VSS) | GND | + C101 −, J101 pin 2, all 3 switch commons (pin 2 / B) |
| 9 (P4) | TOG3_A | SW103 pin 1 (throw A) |
| 10 (P5) | TOG3_C | SW103 pin 3 (throw C) |
| 11 (P6) | — | NC (free GPIO) |
| 12 (P7) | — | NC (free GPIO) |
| 13 (/INT) | IO_INT | J101 pin 5 — open-drain, MCU watches for changes (no polling) |
| 14 (SCL) | I2C_SCL | J101 pin 4 |
| 15 (SDA) | I2C_SDA | J101 pin 3 |
| 16 (VDD) | 3V3 | + C101 +, J101 pin 1 |

| J101 pin | Net |
|----------|-----|
| 1 | 3V3 |
| 2 | GND |
| 3 | I2C_SDA |
| 4 | I2C_SCL |
| 5 | IO_INT |

Identical pinout to nav3 and to every other sub-PCB — one cable type for all seven.

## Mechanical envelope

- Switch pitch: **18 mm**, 3 in a row → 36 mm switch span
  (`TOGGLE_SPACING_X` in `parts/layout_front.scad`)
- Switch body below panel: 13 × 8 × 13 mm; solder lugs 5 mm below that
  (`SW_BODY`, `SW_TERM_L` in `parts/toggle_switch.scad`)
- Board sits at `SUBPCB_Z_TOGGLE = -13`, i.e. at the switch body's bottom face —
  the solder lugs pass **through** the PCB, the switches are board-mounted
- Board outline: **TBD** — set during layout, then written back here and into
  `TOGGLE_PCB_SIZE` in `parts/pcb_subpanel.scad` (current estimate there: 60 × 24 mm)
- F.Cu (panel side): only SW101/SW102/SW103 (and one OSHW logo)
- B.Cu (case-interior side): U101, C101, J101, second OSHW logo
- B.Cu carries a solid GND pour over the full board
- Mount holes 3 mm inset from each corner (4× M2)
- Must clear the case corner bosses at (±129, ±56) and stay inside the 27 mm cavity

## Open: switch footprint

No switch on hand to measure. The two sources disagree:

- `PCB_DESIGN_BRIEF.md` §4.2: "0.1 inch pitch typical" → 2.54 mm
- `parts/toggle_switch.scad` `SW_TERM_PITCH`: 4 mm, with a "verify" comment

Nothing downstream of the schematic depends on this — symbol and nets are
pitch-independent. The footprint gets built once a real switch is measured
(lug pitch, lug width/thickness, body outline, bushing Ø). Until then the board
cannot go to fab.

## Validation status (schematic, 2026-08-07)

`kicad-cli-10 sch erc` — **0 violations**. Netlist verified against the table
above via `kicad-cli sch export netlist`: all 6 switch nets, both I²C lines,
`/INT`, `+3V3` and a 9-node `GND` land exactly as specified, and the netlist is
byte-identical before and after the SOIC-16W footprint change.

Advisory findings from `schematic_design_rule_check`, all **dismissed by design**:

| Finding | Why it is not a defect |
|---------|------------------------|
| `i2c_pullups` on `I2C_SDA` | Pull-ups live on the main PCB, once per TCA9548A downstream channel. A pull-up here would put 7 pairs on one bus |
| `i2c_pullups` on `I2C_SCL` | same |
| `external_port_protection` on `IO_INT` | J101 is not an external port — it is a ~10 cm JST-XH cable inside the sealed case to the main PCB. No ESD exposure |

Open, blocking fab (not the schematic):

- **Switch footprint does not exist** — see "Open: switch footprint" above

Resolved in the KiCad 10 GUI:

- `lib_symbol_mismatch` ×3 on `SW_SPDT` — cleared by *Tools → Update Symbols
  from Library*. Cause was the KiCad 9 / KiCad 10 library split documented in
  `.claude/skills/kicad-hardware/references/kicad-environment.md`
- Annotation errors on export — cleared by *Tools → Annotate Schematic*. The
  three server-placed power symbols now read `#PWRb9ee01`, `#PWRe733`,
  `#PWRef9c01`; cosmetically odd but non-electrical and never synced to the PCB

## Source references

- Design brief: `hardware/transmitter/PCB_DESIGN_BRIEF.md` §3 (PCB list), §4.2 (toggles)
- Panel layout: `hardware/transmitter/parts/layout_front.scad` `TOGGLE_*` constants
- Switch model: `hardware/transmitter/parts/toggle_switch.scad`
- Sub-PCB OpenSCAD model: `hardware/transmitter/parts/pcb_subpanel.scad`
- Template board: `hardware/transmitter/pcb/nav3/`
