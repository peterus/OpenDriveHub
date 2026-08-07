# nav3 — Bill of Materials

3-button I²C navigation sub-PCB. PCF8574A reads 3 momentary tact switches and reports state to the main MCU over I²C, with a hardware interrupt line so the host can sleep until input changes.

I²C address: **0x38** (A0=A1=A2 → GND, hard-wired). The transmitter has two separate I²C buses; this PCB sits on the sub-PCB bus, isolated from the FT6236 touch controller (which lives on the display bus, also at 0x38). No conflict.

Pull-ups (4.7 kΩ on SDA/SCL) live on the **main PCB**, not here — exactly once per sub-PCB bus.
PCF8574A internal pull-ups (~100 µA quasi-bidirectional) drive the switch inputs to VCC at rest; pressing the switch pulls the GPIO to GND. The /INT pin is open-drain — needs a pull-up on the main PCB too.

## Parts

| Ref | Qty | Value | KiCad Symbol | KiCad Footprint | Source | Notes |
|-----|-----|-------|--------------|------------------|--------|-------|
| U101 | 1 | PCF8574AT | `nav3_local:PCF8574AT` | `Package_SO:SOIC-16W_7.5x10.3mm_P1.27mm` | project-local sym | I²C 8-bit I/O expander. Kept as a flattened (non-`extends`) copy in the project-local library so it loads standalone |
| SW101, SW102, SW103 | 3 | TS-A6PV-130 / equiv | `Switch:SW_Push` | `Button_Switch_THT:SW_PUSH_6mm_H5mm` | std lib | 6×6 mm THT tact, 5 mm body height, 4-pin |
| H101, H102, H103, H104 | 4 | MountingHole | `Mechanical:MountingHole` | `MountingHole:MountingHole_2.2mm_M2` | std lib | 4-corner mount to case (M2 screws) |
| C101 | 1 | 100 nF | `Device:C` | `Capacitor_SMD:C_0805_2012Metric` | std lib | Decoupling on U1 VCC, X7R, 0805 chosen for hand-soldering |
| J101 | 1 | B5B-XH-A | `Connector_Generic:Conn_01x05` | `Connector_JST:JST_XH_B5B-XH-A_1x05_P2.50mm_Vertical` | std lib | JST-XH 5-pin (3V3, GND, SDA, SCL, INT), white locking crimp connector |
| LOGO101, LOGO102 | 2 | OSHW logo | `Graphic:Logo_Open_Hardware_Small` | `Symbol:OSHW-Logo_5.7x6mm_SilkScreen` | std lib | Silkscreen-only OSHW logo on F.Cu (LOGO101) and B.Cu mirrored (LOGO102, rot=180). Reference must NOT start with `#` or PCB sync skips it |

Total: **12 footprints** (10 BOM-relevant + 2 silk-only graphics), all from KiCad standard libraries — no project-local footprint libs needed.

## Net list

| U101 pin | Net | Connection |
|----------|-----|------------|
| 1 (A0) | GND | Tied to GND → address bit 0 = 0 |
| 2 (A1) | GND | Tied to GND → address bit 1 = 0 |
| 3 (A2) | GND | Tied to GND → address bit 2 = 0 |
| 4 (P0) | NAV_LEFT | SW101.1 → GND when pressed |
| 5 (P1) | NAV_MID | SW102.1 → GND when pressed |
| 6 (P2) | NAV_RIGHT | SW103.1 → GND when pressed |
| 7 (P3) | — | NC (free GPIO, leave NC) |
| 8 (VSS) | GND | + C101 −, J101 pin 2 |
| 9 (P4) | — | NC |
| 10 (P5) | — | NC |
| 11 (P6) | — | NC |
| 12 (P7) | — | NC |
| 13 (/INT) | IO_INT | J101 pin 5 — open-drain, MCU watches for input changes (no polling) |
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

## Mechanical envelope

- Board outline: **32 × 30 mm** (matches updated `NAV_PCB_SIZE` in `hardware/transmitter/parts/pcb_subpanel.scad`)
- F.Cu (panel side): **only** SW101/SW102/SW103 (and one OSHW logo)
- B.Cu (case-interior side): U101, C101, J101, second OSHW logo
- B.Cu carries a solid GND copper pour covering the full board
- Mount holes 3 mm inset from each corner (4× M2)
- J101 vertical THT exits toward the case interior; cable is 5-wire JST-XH

## Source references

- Design brief: `hardware/transmitter/PCB_DESIGN_BRIEF.md` §3 (PCB list), §4.5 (tact buttons)
- Panel layout: `hardware/transmitter/parts/layout_front.scad` `NAV_BTN_*` constants
- Sub-PCB OpenSCAD model: `hardware/transmitter/parts/pcb_subpanel.scad`
- Tact switch model: `hardware/transmitter/parts/button_tact.scad`
