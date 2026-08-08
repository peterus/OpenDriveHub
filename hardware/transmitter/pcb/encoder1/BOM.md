# encoder1 — Bill of Materials

Single-encoder I²C sub-PCB. A PCF8574A reads one EC11 rotary encoder (quadrature
A/B plus push switch) and reports state to the main MCU over I²C, with a hardware
interrupt line so the host can sleep until the knob moves. Built **2×** (left and
right encoder position).

I²C address: **0x38** (A0=A1=A2 → GND, hard-wired). Same address on every
sub-PCB; the TCA9548A on the main PCB gives each board its own private downstream
channel, so no jumpering and no collisions.

Bus pull-ups (4.7 kΩ SDA/SCL) and the `/INT` pull-up (10 kΩ) live on the **main
PCB**, once per downstream channel — nothing here.

## Parts

| Ref | Qty | Value | KiCad Symbol | KiCad Footprint | Source | Notes |
|-----|-----|-------|--------------|------------------|--------|-------|
| U101 | 1 | PCF8574A | `encoder1_local:PCF8574AT` | `Package_SO:SOIC-16W_7.5x10.3mm_P1.27mm` | project-local sym | **TI `PCF8574ADWR`** (DW = SOIC wide, body 10.30 × 7.50 mm, 1.27 mm pitch). The **A** variant is required — plain `PCF8574DWR` sits at 0x20, not 0x38 |
| SW101 | 1 | EC11 | `Device:RotaryEncoder_Switch_MP` | `Rotary_Encoder:RotaryEncoder_Alps_EC11E-Switch_Vertical_H20mm` | std lib | Alps EC11E, vertical, 20 mm shaft, with push switch and mounting posts. Symbol pins A/B/C/S1/S2/MP map 1:1 onto the footprint pads — no hand-rolling |
| R101, R102 | 2 | 10 kΩ | `Device:R` | `Resistor_SMD:R_0805_2012Metric` | std lib | Pull-ups to 3V3 on encoder A and B — see "Debounce filter" |
| C102, C103 | 2 | 10 nF | `Device:C` | `Capacitor_SMD:C_0805_2012Metric` | std lib | Filter caps to GND on encoder A and B, X7R |
| C101 | 1 | 100 nF | `Device:C` | `Capacitor_SMD:C_0805_2012Metric` | std lib | Decoupling on U101 VDD, X7R |
| H101, H102, H103, H104 | 4 | MountingHole | `Mechanical:MountingHole` | `MountingHole:MountingHole_2.2mm_M2` | std lib | 4-corner mount to case (M2 screws) |
| J101 | 1 | B5B-XH-A | `Connector_Generic:Conn_01x05` | `Connector_JST:JST_XH_B5B-XH-A_1x05_P2.50mm_Vertical` | std lib | JST-XH 5-pin (3V3, GND, SDA, SCL, INT) |
| LOGO101, LOGO102 | 2 | OSHW logo | `Graphic:Logo_Open_Hardware_Small` | `Symbol:OSHW-Logo_5.7x6mm_SilkScreen` | std lib | Silkscreen-only, F.Cu and B.Cu mirrored. Reference must NOT start with `#` or PCB sync skips it |

Total: **13 footprints** (11 BOM-relevant + 2 silk-only graphics), all from KiCad
standard libraries. Unlike `toggle3`, nothing here needs a hand-built footprint.

## Debounce filter — why it is on the board

EC11 contacts bounce for 1–5 ms with edges roughly 100 µs apart. A brisk turn is
about 3 rev/s; at 20 detents/rev that is 60 detents/s, and since one detent is a
full quadrature cycle (4 transitions), real state changes arrive about every
4 ms. Bounce is therefore ~40× faster than the signal, and an RC low-pass
separates them cleanly.

    3V3
     |
    R101 10k
     |
  A -+---------- P0        τ = R·C = 100 µs
     |                     corner ≈ 1.6 kHz
   C102 10n
     |
    GND

Same network for B → P1. The encoder common (pin C) goes to GND, so a closed
contact pulls the node low through 10 kΩ (0.33 mA).

**The PCF8574A's internal pull-up is not enough on its own.** It is a weak
quasi-bidirectional source of roughly 100 µA, equivalent to 30–50 kΩ. Paired with
a filter cap large enough to matter it would give τ ≈ 3–5 ms, which smears the
real 4 ms transitions instead of the bounce. The explicit 10 kΩ sets the time
constant where it belongs.

**The push switch gets no RC.** Its state is a level, not a sequence — a bounce
costs a few spurious `/INT` assertions and the host reads the settled level.
Firmware debounce handles that with no parts. Encoder A/B are different: the
information is in the *order* of transitions, and a lost edge is a permanently
lost count.

Designed for the stricter of the two intended uses (proportional channel /
trim), so it also covers plain menu navigation.

### Known limit

The RC filter fixes contact bounce. It does **not** remove the architectural
constraint that A/B are sampled by the host over I²C after an `/INT`, rather than
latched in hardware. Every other sub-PCB reads switch *levels*, where a missed
intermediate state is harmless because the final read is still correct. Quadrature
is the one case where a missed transition is lost for good.

At human knob speeds this has margin: a PCF8574A read plus the TCA9548A channel
select is a few hundred µs at 400 kHz, against ~4 ms between transitions.
Firmware should still validate the state sequence and ignore illegal jumps rather
than trusting every sample.

## Net list

| U101 pin | Net | Connection |
|----------|-----|------------|
| 1 (A0) | GND | address bit 0 = 0 |
| 2 (A1) | GND | address bit 1 = 0 |
| 3 (A2) | GND | address bit 2 = 0 |
| 4 (P0) | ENC_A | SW101 pin A, via R101/C102 filter |
| 5 (P1) | ENC_B | SW101 pin B, via R102/C103 filter |
| 6 (P2) | ENC_SW | SW101 pin S1 (push switch) |
| 7 (P3) | — | NC (free GPIO) |
| 8 (VSS) | GND | + C101 −, C102 −, C103 −, J101 pin 2, SW101 pins C / S2 / MP |
| 9 (P4) | — | NC (free GPIO) |
| 10 (P5) | — | NC (free GPIO) |
| 11 (P6) | — | NC (free GPIO) |
| 12 (P7) | — | NC (free GPIO) |
| 13 (/INT) | IO_INT | J101 pin 5 — open-drain, MCU watches for changes (no polling) |
| 14 (SCL) | I2C_SCL | J101 pin 4 |
| 15 (SDA) | I2C_SDA | J101 pin 3 |
| 16 (VDD) | 3V3 | + C101 +, R101/R102 top, J101 pin 1 |

| SW101 pin | Net | Notes |
|-----------|-----|-------|
| A | ENC_A | quadrature channel A |
| B | ENC_B | quadrature channel B |
| C | GND | encoder common |
| S1 | ENC_SW | push switch |
| S2 | GND | push switch other side |
| MP | GND | mounting posts — mechanical anchor, tied to GND |

| J101 pin | Net |
|----------|-----|
| 1 | 3V3 |
| 2 | GND |
| 3 | I2C_SDA |
| 4 | I2C_SCL |
| 5 | IO_INT |

Identical connector pinout to `nav3` and `toggle3` — one cable type for all seven
sub-PCBs.

## Mechanical envelope

- Encoder position: (−30, −42) and (+30, −42) — `ENCODER_X` / `ENCODER_Y` in
  `parts/layout_front.scad`
- Encoder body below panel: 12.5 × 13.4 × 6.5 mm; pins in two rows 2.5 mm apart,
  2.5 mm pitch, 3.5 mm long (the `EC_*` constants in `parts/encoder.scad`)
- Board sits at `SUBPCB_Z_ENCODER = −6.5`, i.e. at the encoder body's bottom face
  — the pins pass **through** the PCB, the encoder is board-mounted
- F.Cu (panel side): only SW101 (and one OSHW logo)
- B.Cu (case-interior side): U101, C101–C103, R101/R102, J101, second OSHW logo
- B.Cu carries a solid GND pour over the full board
- Mount holes 3 mm inset from each corner (4× M2)
- Board outline: **36 × 30 mm**, agreed 2026-08-08. Mount holes at (±15, ±12),
  3 mm inset. Still to be written into `ENCODER_PCB_SIZE` in
  `parts/pcb_subpanel.scad`, which currently estimates 30 × 22 mm

### Why 36 × 30, and the two offsets it needs

The parts cannot stack in Y: EC11 (14.2) + SOIC-16W (10.8) + JST-XH (6.2) is
**31.2 mm** of bodies before any margin, against the brief's 30 mm ceiling on
sub-PCB height. The IC and the connector therefore sit side by side, which needs
11.9 + 14.6 = 26.5 mm of clear width; with 3 mm hole insets that puts the board
at **36 mm** wide.

`nav3` got away with 32 × 30 because a 6 mm tact over a TSSOP-16 is a far shorter
stack. The EC11 alone is as tall as nav3's switch and IC combined.

**The board is offset 3 mm outboard of the encoder axis.** Centred on the encoder
at x = −30 a 36 mm board would span −48…−12, and `nav3` reaches to −16 with its
leftmost switch at x = −13.25 — those through-hole switch pins would sit directly
over this board's edge with only 1.4 mm of clearance (3 mm Z spacing less the
1.6 mm board). Shifting the board centre to x = −33 moves its edge to −15 and
removes the conflict; the cavity extends to ±134 and the corner boss sits at
−129, so there is room outboard.

Two offsets therefore go into `parts/layout_front.scad`, the same pattern nav3
already uses for its −5 mm Y shift (`PCB_DESIGN_BRIEF.md` §9.7):

| Axis | Offset | Reason |
|------|--------|--------|
| X | 3 mm outboard | clear `nav3`'s switch pins |
| Y | ~6 mm | encoder sits above the IC/connector row |

Mirror the X sign for the right-hand board.

## Validation status (schematic, 2026-08-07)

`kicad-cli-10 sch erc` — **0 violations**. Netlist exported and checked against
the table above:

```
ENC_A   C102.1 R101.2 SW101.A U101.4 (P0)     ← RC node
ENC_B   C103.1 R102.2 SW101.B U101.5 (P1)     ← RC node
ENC_SW  SW101.S1 U101.6 (P2)
+3V3    C101.1 J101.1 R101.1 R102.1 U101.16
GND     11 nodes, incl. SW101 C / S2 / MP and both filter caps
```

Advisory findings from `schematic_design_rule_check`, all **dismissed by design**
— identical to `toggle3` and for the same reasons:

| Finding | Why it is not a defect |
|---------|------------------------|
| `i2c_pullups` on `I2C_SDA` / `I2C_SCL` | Pull-ups live on the main PCB, once per TCA9548A downstream channel |
| `external_port_protection` on `IO_INT` | J101 is not an external port — it is a short JST-XH cable inside the sealed case |

Open, blocking fab (not the schematic):

- **Board outline** — set during layout, then written back here and into
  `ENCODER_PCB_SIZE` in `parts/pcb_subpanel.scad`
- **Three power symbols carry hash references** instead of `#PWR01xx`, which
  makes `kicad-cli` report annotation errors on export. Clear with *Tools →
  Annotate Schematic* → *Keep existing annotations*. Known `sch_add_power_symbol`
  defect, documented in
  `.claude/skills/kicad-hardware/references/kicad-environment.md`
- **`lib_symbol_mismatch`** may appear on the newly placed symbols for the same
  KiCad 9/10 library reason. Clear with *Tools → Update Symbols from Library*

Unlike `toggle3`, **no footprint is missing** — every part here comes from a
KiCad standard library, so nothing blocks fab once the layout is done.

## Source references

- Design brief: `hardware/transmitter/PCB_DESIGN_BRIEF.md` §3 (PCB list), §4.3 (EC11)
- Panel layout: `hardware/transmitter/parts/layout_front.scad` `ENCODER_*` constants
- Encoder model: `hardware/transmitter/parts/encoder.scad`
- Sub-PCB OpenSCAD model: `hardware/transmitter/parts/pcb_subpanel.scad`
- Template board: `hardware/transmitter/pcb/nav3/`; package rationale in
  `hardware/transmitter/pcb/toggle3/BOM.md`
