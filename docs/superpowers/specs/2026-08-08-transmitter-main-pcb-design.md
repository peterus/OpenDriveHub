# OpenDriveHub Transmitter — `main` PCB Design Spec

**Date:** 2026-08-08
**Status:** Design locked, pending schematic capture
**Scope:** The `main` (brain) PCB of the OpenDriveHub transmitter. One
ERC-clean hierarchical KiCad schematic + BOM + net/footprint list. PCB
layout is the user's job (see §11 Workflow split).

This spec supersedes the open questions in
`hardware/transmitter/PCB_DESIGN_BRIEF.md` §8 for the `main` board. Where
this document and the brief disagree, **this document wins** for `main`;
the brief remains canonical for mechanical envelope and the sub-PCBs.

---

## 1. Summary of decisions

| Topic | Decision | Rationale |
|-------|----------|-----------|
| MCU module | **ESP32-S3-WROOM-1U-N16R2** | Native USB (flash/console/JTAG over USB-C), external antenna (U.FL) for range, **Quad**-PSRAM keeps GPIO35–37 free (Octal R8 would consume them) |
| Charger | **TI BQ25798** (buck-boost, I²C) | 2S charge from 5 V USB (boost), integrated 16-bit ADC → battery/charge telemetry over I²C, Mouser-stocked, QFN reflow-friendly (hotplate + stencil) |
| Power rails | **Single 3.3 V rail** | Audio went I²S/3.3 V, so no 5 V load remains → no boost, no 5 V buck. Simpler. |
| 3.3 V regulator | Buck from charger SYS (≈V_bat) → 3.3 V, ~2–3 A | Efficient; handles ESP32 WiFi peaks + backlight + audio |
| Display | **ST7796 (SPI)**, 2.54 mm pin-header harness | Module-agnostic, robust for first build |
| Touch | **FT6236 (I²C)** on the **internal** I²C bus | On the display module; wired via display harness |
| Audio | **MAX98357A** (I²S, 3.3 V, mono ~3 W) → speaker | No analog DAC, no 5 V rail; clean digital path from ESP32 I²S |
| I²C topology | **Two buses** (bus isolation, see §4) | A misbehaving module can only wedge the external bus; internal peripherals stay alive |
| Module expansion | **8 module sockets** (JST-XH 5-pin), each with **direct /INT** to ESP32 | TCA9548A Ch0–7 all used for modules (touch moved off mux) |
| Battery telemetry | via BQ25798 I²C ADC (no discrete divider) | Divider redundant with charger ADC |
| Extras | RTC (DS3231SN + CR1220), microSD, power switch (ship-mode), reverse-polarity P-FET | All requested |
| Debug | **USB-C** (USB-Serial-JTAG: console + JTAG) **+ physical UART0 header** | Redundant debug paths |

---

## 2. System block diagram

```
                 USB-C receptacle (16-pin, USB 2.0)
                   │ VBUS(5V)      │ D+/D-      │ CC1/CC2 (5.1k → GND, sink)
                   ▼               ▼            
        ┌──────────────────┐   (native USB → ESP32 GPIO19/20:
        │  BQ25798          │    console + JTAG + flashing)
        │  buck-boost 2S    │
        │  charger + I²C ADC │◄───── 2S LiPo (XT30 power + JST-XH balance)
        └───────┬───────────┘        ▲ reverse-polarity P-FET
                │ SYS (≈V_bat)        │ ship-mode button → /QON (soft power switch)
                ▼
        ┌──────────────────┐
        │ 3.3 V buck (~3A) │────────────────── 3V3 rail ───────────────┐
        └──────────────────┘                                            │
                                                                        │
   ┌─────────────────────────── ESP32-S3-WROOM-1U-N16R2 ───────────────┤
   │                                                                    │
   │  I²C0 (MODULE bus, external)        I²C1 (INTERNAL bus, protected) │
   │      └─ TCA9548A 0x70                   ├─ FT6236 touch  0x38      │
   │           ├ Ch0..7 → 8× module            ├─ BQ25798     0x6B      │
   │           │   sockets (JST-XH 5p)         └─ DS3231 RTC  0x68      │
   │           │   (each: own 4.7k pull-ups)                            │
   │           └ 8× /INT → direct GPIO                                  │
   │                                                                    │
   │  SPI2 (shared): ST7796 display + microSD (separate CS)             │
   │  I²S: MAX98357A → speaker (JST-PH 2p)                              │
   │  ADC1: 2× joystick (X,Y,SW) via JST-PH 5p (direct, no expander)    │
   │  UART0 → debug header                                              │
   └────────────────────────────────────────────────────────────────────┘
```

---

## 3. Power subsystem

### 3.1 Input & charging
- **USB-C receptacle** (16-pin USB 2.0): VBUS → BQ25798 VBUS; CC1/CC2 each
  5.1 kΩ to GND (sink, fixed 5 V, no PD); D+/D- → ESP32-S3 GPIO20/19 (native
  USB). No BC1.2 detection needed — input current limit is set over I²C.
- **BQ25798** buck-boost charger, 2S configuration (VREG = 8.4 V). Charges
  2S from 5 V USB via boost. External per TI reference design: single power
  inductor, ACFET/BATFET (external), input/charge sense resistors, VBUS/SYS/
  BAT caps, bootstrap caps, REGN cap, ILIM_HIZ / PROG resistors. Provides SYS
  power-path output.
- **I²C ADC** in BQ25798 reports VBUS, VBAT, IBUS, IBAT, TS, TDIE → firmware
  reads battery %, charge current, USB presence over **I²C1**.

### 3.2 Protection & switching
- **Reverse-polarity protection:** P-channel MOSFET ideal-diode in the
  battery+ path (XT30 is also mechanically keyed as a second line of defense).
- **Soft power switch:** momentary button on BQ25798 **/QON** toggles ship
  mode (BATFET disconnect) — real on/off without a bulky slide switch.

### 3.3 3.3 V rail
- Buck regulator from **SYS** (≈V_bat, 6.0–8.4 V) → **3.3 V**, ≥2 A
  (target 3 A headroom for ESP32 WiFi + backlight + audio peaks).
- Candidate: **TI TPS62933** (3.8–30 V in, 3 A, SOT-583) or LMR33630. Input
  rating must exceed 8.4 V. Final part TBD in BOM lock.
- Bulk + decoupling per datasheet; ferrite/pi optional for the ESP32 rail.

---

## 4. I²C topology (bus isolation)

The ESP32-S3 has **two I²C controllers**. We use both to physically separate
"things a user can poke" from "things that must never die":

**I²C0 — MODULE bus (external-facing):**
- Only device on the trunk: **TCA9548A** mux (0x70).
- Ch0–Ch7 → **8 module sockets**. Each downstream channel is its own private
  segment with its **own 4.7 kΩ SDA/SCL pull-ups** on the main PCB.
- All sub-PCBs use PCF8574A @ 0x38 — no conflict, each is isolated on its
  channel.
- A shorted/wedged module can only take down I²C0; firmware recovers it via
  the **MUX_RST** line (TCA9548A /RESET on GPIO3, **external 10 kΩ pull-up** so
  the mux is out of reset by default even before GPIO init) and/or a bus-reset
  sequence.

**I²C1 — INTERNAL bus (protected):**
- **FT6236** touch (0x38), **BQ25798** charger (0x6B), **DS3231** RTC (0x68).
- All addresses distinct; physically separate from the module chaos. The
  0x38 collision (touch vs. PCF8574A) is avoided by construction, not by
  address remapping (FT6236's 0x38 is fixed — no address pin).
- Pull-ups: 4.7 kΩ SDA/SCL on the trunk.

**INT lines:** all **8 module /INT** lines are open-drain, each with a 10 kΩ
pull-up on the main PCB, routed **directly** to individual ESP32 GPIOs
(GPIO38–42, 47, 48, 15) — firmware identifies the source immediately, no
channel scan. Touch /INT → GPIO12 (10 kΩ pull-up).

---

## 5. Compute

- **ESP32-S3-WROOM-1U-N16R2**. External antenna via **U.FL/IPEX** connector.
- **EN** pin: RC reset (10 kΩ / 1 µF) + RESET button.
- **GPIO0**: BOOT button (weak pull-up default → SPI boot).
- Native **USB** (GPIO19 = D-, GPIO20 = D+) → USB-C: console (USB-Serial-JTAG),
  JTAG hardware debug, and firmware flashing — no external USB-UART needed.
- Full decoupling on 3V3 (bulk + per-pin 100 nF).
- **Module variant constraint:** must be **N16R2 (Quad-PSRAM) or a non-Octal
  variant**. An `…R8` (Octal-PSRAM) module consumes GPIO35–37 and breaks the
  pin budget.

---

## 6. Pin assignment (locked)

ESP32-S3-WROOM-1U — verified against Espressif datasheet v1.8, Table 3-1.
Available module GPIO: `0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,21,
35,36,37,38,39,40,41,42,43,44,45,46,47,48` (34 usable + USB 19/20). ADC1 =
GPIO1–10 (mandatory for analog; ADC2 unusable with WiFi active).

| GPIO | Net | I/O | Block | Notes |
|------|-----|-----|-------|-------|
| 0 | BOOT_BTN | in | System | Strapping (weak PU) |
| EN | RESET_BTN | in | System | RC 10k/1µF (not a GPIO) |
| 19 | USB_D− | — | USB | fixed (console+JTAG) |
| 20 | USB_D+ | — | USB | fixed |
| 1 | JOY_LX | ADC1_CH0 | Joystick | |
| 2 | JOY_LY | ADC1_CH1 | Joystick | |
| 4 | JOY_RX | ADC1_CH3 | Joystick | |
| 5 | JOY_RY | ADC1_CH4 | Joystick | |
| 6 | JOY_LSW | in | Joystick | |
| 7 | JOY_RSW | in | Joystick | |
| 8 | I2C1_SDA | bidir | Internal bus | Touch/Charger/RTC |
| 9 | I2C1_SCL | bidir | Internal bus | |
| 10 | I2C0_SDA | bidir | Module bus | → TCA9548A |
| 11 | I2C0_SCL | bidir | Module bus | |
| 12 | TOUCH_INT | in | Display | FT6236 /INT, 10k PU |
| 13 | DISP_DC | out | Display | |
| 14 | DISP_RST | out | Display | shared touch reset |
| 15 | MOD_INT7 | in | Module 7 | 10k PU (moved off GPIO46 — strap) |
| 16 | SPI2_SCK | out | Display+SD (shared) | |
| 17 | SPI2_MOSI | out | shared | |
| 18 | SPI2_MISO | in | shared | needed for SD |
| 21 | DISP_CS | out | Display | |
| 45 | SD_CS | out | microSD | free on N16R2 (VDD_SPI eFuse-fixed) |
| 35 | I2S_BCLK | out | Audio | |
| 36 | I2S_LRCLK | out | Audio | |
| 37 | I2S_DIN | out | Audio | data → amp |
| 38 | MOD_INT0 | in | Module 0 | 10k PU |
| 39 | MOD_INT1 | in | Module 1 | MTCK free (JTAG via USB) |
| 40 | MOD_INT2 | in | Module 2 | MTDO free |
| 41 | MOD_INT3 | in | Module 3 | MTDI free |
| 42 | MOD_INT4 | in | Module 4 | MTMS free |
| 47 | MOD_INT5 | in | Module 5 | |
| 48 | MOD_INT6 | in | Module 6 | |
| 46 | DISP_BL | out | Display | backlight enable; **external pull-down** (BL off @ boot + keeps BOOT-button download working: GPIO46 must read 0) |
| 3 | MUX_RST | out | Module bus | TCA9548A /RESET recovery |
| 43 | UART0_TX | out | Debug header | |
| 44 | UART0_RX | in | Debug header | |

**Budget:** 34/34 pins used. No hard spare (GPIO0 hosts the BOOT button by
choice). Cut vs. maximal wishlist: display read-back (ST7796 driven
write-only; the shared MISO on GPIO18 serves the SD card) and a dedicated
charger /INT pin (charger polled over I²C1) — neither is a functional loss.

**Strapping-pin handling (all four accounted for):**
- **GPIO0** — BOOT button, internal weak pull-up → SPI boot by default.
- **GPIO3** — MUX_RST output; JTAG-source strap is ignored with the default
  USB-JTAG eFuse config. External pull-up on the /RESET net.
- **GPIO45** — SD_CS; on N16R2 VDD_SPI is eFuse-fixed, so GPIO45's level is
  ignored at boot (no constraint). No external pull-up/down needed.
- **GPIO46** — DISP_BL output with an **external pull-down**: internal default
  is already weak-pull-down (=0), so backlight is off at boot and the manual
  BOOT-button download path stays valid (needs GPIO46=0). No external pull-up
  anywhere on this net.

---

## 7. Peripheral details

### 7.1 Display + touch (harness via 2.54 mm pin header)
- ST7796 SPI: SCK, MOSI, MISO (shared bus), DISP_CS, DISP_DC, DISP_RST,
  DISP_BL (+3V3, GND).
- FT6236 touch: I2C1_SDA, I2C1_SCL, TOUCH_INT, shared RST.
- Header pin count finalized against the actual module (est. ~1×14).

### 7.2 microSD (shared SPI2)
- Push-push socket, 3.3 V. Shares SCK/MOSI/MISO with the display; own SD_CS
  (GPIO45). Series resistors optional; card-detect omitted (pin budget).
- SD access and display refresh are mutually exclusive on the bus — fine for
  occasional logging.

### 7.3 Audio (I²S)
- MAX98357A: BCLK/LRCLK/DIN from ESP32, SD_MODE strap (gain/channel) via
  resistor, 3.3 V, bulk cap. Output → **JST-PH 2-pin** → 28 mm 8 Ω speaker.

### 7.4 Joysticks (×2, direct ADC)
- **JST-PH 5-pin** each: 3V3, GND, X→ADC1, Y→ADC1, SW→GPIO. Optional RC
  filter on the wiper lines. No I/O expander (low latency, 12-bit).

### 7.5 RTC
- DS3231SN on I²C1 (0x68). CR1220 coin-cell backup + holder. SQW/INT
  unused (pin budget).

---

## 8. Connector inventory

| Ref (draft) | Connector | Qty | Purpose |
|-------------|-----------|-----|---------|
| J_USB | USB-C receptacle, 16-pin USB 2.0 | 1 | Power in + native USB |
| J_BATT | XT30 (power) | 1 | 2S LiPo main leads |
| J_BAL | JST-XH 3-pin (B3B-XH-A) | 1 | 2S balance |
| J_MOD0..7 | JST-XH 5-pin (B5B-XH-A) | 8 | Module sockets (3V3,GND,SDA,SCL,/INT) |
| J_JOYL/R | JST-PH 5-pin (B5B-PH-K-S) | 2 | Joysticks (3V3,GND,X,Y,SW) |
| J_SPK | JST-PH 2-pin (B2B-PH-K-S) | 1 | Speaker |
| J_DISP | 2.54 mm pin header (~1×14) | 1 | Display + touch harness |
| J_SD | microSD push-push socket | 1 | Log storage |
| J_DBG | 2.54 mm 1×4 header | 1 | 3V3, TX(43), RX(44), GND |
| J_ANT | U.FL/IPEX | 1 | External 2.4 GHz antenna |
| SW_BOOT / SW_RST | tact 6 mm | 2 | Boot (GPIO0) / Reset (EN) |
| SW_PWR | tact 6 mm | 1 | Ship-mode / soft power (→ /QON) |

Module socket pinout (matches nav3): 1=3V3, 2=GND, 3=SDA, 4=SCL, 5=/INT.

---

## 9. BOM draft (Mouser-focused; MPNs to verify at BOM lock)

| Ref | Part | MPN (candidate) | Package | Source |
|-----|------|-----------------|---------|--------|
| U1 | ESP32-S3-WROOM-1U-N16R2 | ESP32-S3-WROOM-1U-N16R2 | module | Mouser |
| U2 | Buck-boost 2S charger, I²C | TI **BQ25798RQMR** | QFN-29 | Mouser |
| U3 | 3.3 V buck ~3 A | TI **TPS62933** (or LMR33630) | SOT-583 | Mouser |
| U4 | I²C 8-ch mux | TI **TCA9548APWR** | TSSOP-24 | Mouser |
| U5 | I²S class-D amp | **MAX98357AETE+T** | QFN-16 | Mouser |
| U6 | RTC + TCXO | **DS3231SN** | SOIC-16W | Mouser |
| Q1 | P-FET reverse-polarity | (e.g. DMP3017SFG-class) | SOT-23 | Mouser |
| L1 | Charger power inductor | per BQ25798 ref | — | Mouser |
| — | U.FL connector + 2.4 GHz antenna | — | — | Mouser |
| — | microSD push-push socket | Hirose DM3AT-SF-PEJM5 | — | Mouser |
| — | USB-C 16-pin receptacle | GCT USB4085-class | — | Mouser |
| — | Passives | 4.7k (I²C ×2 trunks + 8 ch pairs), 10k (8× INT + touch), 5.1k CC ×2, 100 nF decoupling, bulk caps, EN RC 10k/1µF, sense R's, CR1220 + holder | 0805 pref | Mouser |

FT6236 touch controller lives on the display module — **not** a main-PCB BOM
line; only its I²C/INT/RST route through J_DISP.

---

## 10. Schematic structure (hierarchical)

Root sheet + child sheets, project/DRC settings + conventions inherited from
the `nav3` template (`pcb/nav3/*.kicad_pro`, `sym-lib-table`, flattened
symbol lib pattern). Reference designators must NOT start with `#`.

- `01_power` — USB-C, BQ25798 + externals, reverse P-FET, 3.3 V buck, ship-mode
- `02_mcu` — ESP32-S3-WROOM-1U, EN/boot, USB, U.FL, decoupling
- `03_display_touch` — J_DISP header, backlight, touch INT/RST nets
- `04_i2c` — TCA9548A + 8 module sockets + per-channel pull-ups + MUX_RST;
  internal-bus routing (charger/RTC/touch) + trunk pull-ups
- `05_audio` — MAX98357A + speaker
- `06_joystick_io` — 2× joystick headers, ADC filters
- `07_connectors_misc` — battery (XT30 + balance), microSD, debug header, buttons

Project scaffolding: `cp -r pcb/nav3 pcb/main`, rename, `sed nav3→main`, strip
nav3 sheet content, then build the hierarchy (per brief §9.6).

---

## 11. Workflow split (AI ↔ user)

Per brief §9.5 and the `kicad-hardware` skill:

- **AI (me):** schematic capture as a starting draft, project scaffolding,
  design-rule/BOM draft, ERC to clean, net + footprint list. Validation gate:
  `kicad-cli sch erc` clean + schematic SVG rendered and visually inspected.
- **User:** schematic review/rework (draft is a starting point, not a proposal
  to defend), and the **entire PCB layout** in the GUI (placement, routing,
  pours, silk, keep-outs).
- **Handoff:** when ERC is clean and the net/footprint list exists. Then the
  board file is the user's. AI re-enters for STEP export → OpenSCAD case-fit.

Outline target ~80 × 60 mm (brief §3) — final size driven by connector count
(8 module + 2 joystick + battery + USB-C + display + SD) and the case cavity;
expect growth like nav3 did. Layout must clear the corner bosses at (±129,±56)
and the battery compartment.

---

## 12. Open items to resolve during schematic capture

- BQ25798 external network: finalize inductor value, ACFET/BATFET parts,
  sense-resistor values against TI reference design (SLUSD90).
- 3.3 V buck: confirm final part + feedback divider for 3.3 V, verify current
  headroom vs. measured loads.
- Display header pinout: finalize against the actual ST7796 module in hand.
- P-FET reverse-protection part + gate network.
- MAX98357A SD_MODE resistor value (gain + mono-mix selection).
- Confirm ESP32-S3-WROOM-1U-N16R2 availability at Mouser (else nearest
  non-Octal variant).
```
