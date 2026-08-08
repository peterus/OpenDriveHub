# Transmitter `main` PCB — Schematic Capture Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.
>
> **REQUIRED SKILL:** `kicad-hardware` + `kicad-schematic-design` govern every task. Read them before starting.

**Goal:** Produce an ERC-clean hierarchical KiCad schematic + locked BOM +
net/footprint handoff list for the transmitter's `main` (brain) PCB.

**Architecture:** One KiCad project (`hardware/transmitter/pcb/main/`) scaffolded
from the `nav3` template, built as a hierarchical schematic with 7 functional
sheets under a root. Components placed and connected with the kicad-mcp-pro
`sch_*` tools; validated after every sheet with `kicad-cli` ERC + SVG render.
**PCB layout is NOT in scope** — handoff to the user after ERC is clean (brief §9.5).

**Tech Stack:** KiCad 10.0.5, kicad-cli-10, kicad-mcp-pro v3.30.1 (`sch_*`,
`lib_*`, `run_erc`, `schematic_design_rule_check`), S-expression project files.

## The KiCad validation cycle (replaces TDD)

Schematic work has no unit tests. Each task's "test" is the **non-negotiable
validation gate** from the kicad-hardware skill, run as the last steps of the task:

1. `kicad-cli sch erc main.kicad_sch --output erc.json --format json --severity-all --exit-code-violations` → read JSON, zero violations OR each explained.
2. `kicad-cli sch export svg main.kicad_sch -o sch.svg` (or per-sheet PDF) → **`Read` the image and describe what you see**. "Looks fine" without a Read call is a lie.
3. Only then commit.

A task is "done" only when ERC is clean (or every remaining violation is
explicitly justified) AND a render has been visually inspected.

## Global Constraints

- **No hand-editing of `.kicad_sch`/`.kicad_pcb` S-expressions.** Use `sch_*`
  MCP tools; `kicad-cli` for ERC/render; hand-edit only `.kicad_pro` metadata,
  `sym-lib-table`/`fp-lib-table`, title-block text. (kicad-hardware file-edit policy)
- **MCU module:** ESP32-S3-WROOM-1U-**N16R2** (Quad-PSRAM; an `…R8` Octal part
  breaks the pin budget by consuming GPIO35–37).
- **Reference designators must NOT start with `#`** (MCP skips them on sync).
- **Connect by name, not geometry:** short pin stub + same-named label, not
  long wires across pins (geometry-merge = silent shorts). Add junctions after.
- **PWR_FLAG on every source net** (3V3, SYS, VBAT, VBUS) or ERC screams.
- **Pin map is frozen** — see spec §6. Every ESP32 net name below matches it verbatim.
- **Two I²C buses:** `I2C0_*` = module bus (mux only), `I2C1_*` = internal bus
  (touch/charger/RTC). Never merge them.
- Spec of record: `docs/superpowers/specs/2026-08-08-transmitter-main-pcb-design.md`.

---

## File structure

```
hardware/transmitter/pcb/main/
  main.kicad_pro          # project + DRC/ERC settings (from nav3, renamed)
  main.kicad_sch          # ROOT sheet (sheet symbols → 7 children)
  main_01_power.kicad_sch
  main_02_mcu.kicad_sch
  main_03_display_touch.kicad_sch
  main_04_i2c.kicad_sch
  main_05_audio.kicad_sch
  main_06_joystick_io.kicad_sch
  main_07_connectors_misc.kicad_sch
  main_local.kicad_sym    # project-local flattened symbols (from nav3 + new)
  main.pretty/            # project-local footprints (only if a part needs one)
  sym-lib-table           # registers main_local (from nav3, renamed)
  fp-lib-table            # registers main.pretty (if used)
  BOM.md                  # locked part list with MPNs + symbol/footprint
  fab/                    # generated exports, .gitignored
```

Each sheet = one responsibility (matches spec §10). A sheet is the smallest unit
that carries its own ERC+render gate, so each is its own task.

---

## Task 0: Toolchain preflight + project scaffold

**Files:**
- Create: whole `hardware/transmitter/pcb/main/` (copied from `nav3/`)
- Modify: `main.kicad_pro`, `sym-lib-table` (rename strings)

**Interfaces:**
- Produces: a loadable `main` project with an empty root schematic, confirmed
  `sch_*` toolchain, and `kicad_set_project` pointing at it.

- [ ] **Step 1: Preflight the tools.** Call `kicad_get_version()`. Confirm CLI
  version == IPC version and note whether IPC is connected. Then confirm the
  specific tools this plan needs are actually callable (registry over-reports):
  `sch_add_component`, `sch_add_label`, `sch_find_free_placement`,
  `sch_check_power_flags`, `run_erc`, `schematic_design_rule_check`,
  `sch_render_png`, `lib_create_custom_symbol`, `lib_assign_footprint`.
  Expected: all resolve. If `sch_*` live-edit tools are gated by missing IPC
  and cannot be made to work → **STOP**, report to the user, and hand the spec
  to GUI capture instead of hand-editing S-expressions (Global Constraints).

- [ ] **Step 2: Scaffold from nav3.**

```bash
cd hardware/transmitter/pcb
cp -r nav3 main
cd main
for f in nav3.*; do mv "$f" "${f/nav3/main}"; done
mv nav3_local.kicad_sym main_local.kicad_sym
sed -i 's/nav3/main/g' *.kicad_pro *.kicad_pcb sym-lib-table
rm -rf nav3-backups output ~*.lck main.kicad_pcb
```

- [ ] **Step 3: Strip nav3 schematic content** to an empty root. In the KiCad
  GUI (or via `sch_get_symbols` then delete), remove all nav3 symbols/wires from
  `main.kicad_sch`, keeping the file header, project settings, and title block.

- [ ] **Step 4: Set project + design intent.**

```
kicad_set_project(project_dir=".../pcb/main", sch_file="main.kicad_sch")
```
Then `project_set_design_intent` with: 2-layer→(user may raise), 3.3 V logic,
handheld, hand-assembly-friendly 0805 passives.

- [ ] **Step 5: ERC smoke test.**

```bash
kicad-cli sch erc main.kicad_sch --output fab/erc.json --format json --severity-all
```
Expected: loads without "Failed to load schematic"; empty sheet → no violations.

- [ ] **Step 6: Commit.**

```bash
git add hardware/transmitter/pcb/main
git commit -m "main pcb: scaffold KiCad project from nav3 template"
```

---

## Task 1: Lock the BOM + project-local symbols

**Do this before wiring** (kicad-hardware: "no wires before the BOM exists").

**Files:**
- Create: `hardware/transmitter/pcb/main/BOM.md`
- Modify: `main_local.kicad_sym` (add flattened symbols for non-standard parts)

**Interfaces:**
- Produces: for every part — a decided KiCad symbol + footprint + MPN. Later
  tasks reference these exact symbol names.

- [ ] **Step 1: Resolve standard-library parts** with `lib_search_symbols` /
  `lib_search_footprints`. Confirm availability for: `Device:C`, `Device:R`,
  `Device:L`, `Device:Q_PMOS_GSD`, `Connector:USB_C_Receptacle_USB2.0`,
  `Connector_Generic:Conn_01x05` (JST-XH/PH), `Connector:Conn_01x02`,
  `Connector:Conn_01x03`, `Connector:Conn_01x04`, `Switch:SW_Push`,
  `Regulator_Switching:TPS62933` (or nearest), `Interface_Expansion:TCA9548APWR`,
  `RF_Module:ESP32-S3-WROOM-1` (base; verify -1U/N16R2 pin compatibility).

- [ ] **Step 2: Create project-local symbols** for parts not in std libs, as
  flattened (non-`extends`) copies via `lib_create_custom_symbol` or
  `lib_generate_symbol_from_pintable`: **BQ25798** (QFN-29), **MAX98357A**
  (QFN-16), **DS3231SN** (SOIC-16W). Cross-check every pin against the datasheet.

- [ ] **Step 3: Write `BOM.md`** in the nav3 format (Ref | Qty | Value | Symbol
  | Footprint | Source | Notes), covering spec §9 parts + all passives from
  spec §4/§7 (pull-ups: I2C0 4.7k×2, I2C1 4.7k×2, 8× channel pairs 4.7k, 8×
  INT 10k, touch 10k; CC 5.1k×2; MUX_RST 10k; EN 10k/1µF; decoupling 100nF per
  IC pin; bulk caps; charger inductor + sense R; CR1220 + holder). MPNs
  Mouser-preferred.

- [ ] **Step 4: Footprint sanity.** For BQ25798/MAX98357A/DS3231 verify pad
  count + pin mapping with `lib_get_footprint_info` against the datasheet.

- [ ] **Step 5: Commit.**

```bash
git add hardware/transmitter/pcb/main/BOM.md hardware/transmitter/pcb/main/main_local.kicad_sym
git commit -m "main pcb: lock BOM and project-local symbols"
```

---

## Task 2: Power sheet (`main_01_power`)

**Files:** Create `main_01_power.kicad_sch`; Modify root `main.kicad_sch` (add sheet symbol).

**Interfaces:**
- Produces global/hierarchical nets: `VBUS`, `SYS`, `3V3`, `VBAT`, `GND`,
  `PWR_QON`, `I2C1_SDA`, `I2C1_SCL` (BQ25798 sits on internal bus).

- [ ] **Step 1: Add the sheet** to root and open it. Place power symbols first
  (`power:GND`, `power:+3V3`, plus local labels `VBUS`/`SYS`/`VBAT`).

- [ ] **Step 2: Place + wire USB-C input.** `J_USB` (USB_C_Receptacle_USB2.0):
  VBUS→`VBUS`, GND→`GND`, CC1/CC2 each 5.1k→`GND`, SBU NC, D+→label `USB_DP`,
  D−→label `USB_DM` (consumed by MCU sheet). Shield→GND via net-tie/cap.

- [ ] **Step 3: Place BQ25798 + reference network** (`U2`): VBUS in, single
  power inductor, external ACFET/BATFET, VBUS/SYS/BAT sense R + caps, REGN cap,
  bootstrap caps per TI SLUSD90. VBAT→`VBAT`, SYS→`SYS`, SDA/SCL→`I2C1_SDA`/
  `I2C1_SCL`, /QON→`PWR_QON`, TS→NTC or fixed divider.

- [ ] **Step 4: Reverse-polarity P-FET** (`Q1`) in `VBAT` battery path; gate net
  + optional gate zener. Battery enters via `J_BATT`/`J_BAL` (placed on Task 8's
  connector sheet — here just expose `VBAT`,`GND` hierarchical labels).

- [ ] **Step 5: Ship-mode button** `SW_PWR` from `PWR_QON` to `GND`.

- [ ] **Step 6: 3.3 V buck** (`U3`, TPS62933-class): `SYS`→VIN, FB divider set
  for 3.3 V, inductor + in/out caps, EN tie. VOUT→`3V3`.

- [ ] **Step 7: PWR_FLAGs.** Add `PWR_FLAG` on `VBUS`, `SYS`, `VBAT`, `3V3`
  (and `GND`). Run `sch_check_power_flags`.

- [ ] **Step 8: VALIDATION GATE.** ERC (`kicad-cli sch erc … --severity-all`),
  export SVG of this sheet, **`Read` it and describe** the power tree. Fix or
  justify every violation.

- [ ] **Step 9: Commit** `git commit -m "main pcb: power sheet (charger, 3V3, protection)"`.

---

## Task 3: MCU sheet (`main_02_mcu`)

**Files:** Create `main_02_mcu.kicad_sch`; Modify root.

**Interfaces:**
- Consumes: `3V3`, `GND`, `USB_DP`, `USB_DM`.
- Produces: every ESP32 signal net from spec §6 by exact name (`I2C0_SDA/SCL`,
  `I2C1_SDA/SCL`, `TOUCH_INT`, `DISP_DC/RST/BL`, `SPI2_SCK/MOSI/MISO`,
  `DISP_CS`, `SD_CS`, `I2S_BCLK/LRCLK/DIN`, `MOD_INT0..7`, `MUX_RST`,
  `UART0_TX/RX`, `JOY_LX/LY/RX/RY/LSW/RSW`).

- [ ] **Step 1: Place `U1`** (ESP32-S3-WROOM-1U). Tie 3V3/GND, EPAD→GND.

- [ ] **Step 2: EN reset network** — 10k pull-up to 3V3 + 1µF to GND + `SW_RST`
  to GND. **BOOT** — `SW_BOOT` on GPIO0 to GND (internal PU; add 100nF).

- [ ] **Step 3: USB** — GPIO19→`USB_DM`, GPIO20→`USB_DP`. `J_ANT` U.FL note
  (antenna is external; no PCB net besides the module's RF pin/keepout).

- [ ] **Step 4: Assign every GPIO** per spec §6 using short stub + label. Verify
  each label name matches the frozen map (esp. GPIO15=`MOD_INT7`, GPIO46=`DISP_BL`,
  GPIO45=`SD_CS`, GPIO3=`MUX_RST`). Mark truly-unused module pins with `No-Connect`.

- [ ] **Step 5: Decoupling** — 100nF per 3V3 pin + 10µF bulk at the module.

- [ ] **Step 6: VALIDATION GATE** — ERC + SVG render + **Read**. Confirm no
  net name typos vs. §6 (a typo here = silent disconnect downstream).

- [ ] **Step 7: Commit** `git commit -m "main pcb: MCU sheet (ESP32-S3, pinmap)"`.

---

## Task 4: Display + touch sheet (`main_03_display_touch`)

**Files:** Create `main_03_display_touch.kicad_sch`; Modify root.

**Interfaces:**
- Consumes: `3V3`,`GND`,`SPI2_SCK/MOSI/MISO`,`DISP_CS/DC/RST/BL`,`TOUCH_INT`,
  `I2C1_SDA/SCL`.

- [ ] **Step 1: Place `J_DISP`** (~1×14 pin header, `Connector_Generic:Conn_01x14`).
- [ ] **Step 2: Map pins** to: 3V3, GND, SCK(`SPI2_SCK`), MOSI(`SPI2_MOSI`),
  MISO(`SPI2_MISO`), CS(`DISP_CS`), DC(`DISP_DC`), RST(`DISP_RST`), BL(`DISP_BL`),
  T_SDA(`I2C1_SDA`), T_SCL(`I2C1_SCL`), T_INT(`TOUCH_INT`), T_RST(`DISP_RST` shared).
- [ ] **Step 3: Backlight** — if header BL pin drives an LED anode directly, add
  series R / N-FET low-side per module; else BL is a logic enable (label only).
- [ ] **Step 4: External pull-down on `DISP_BL`** (GPIO46 strap safety, spec §6 note).
- [ ] **Step 5: VALIDATION GATE** — ERC + render + **Read**.
- [ ] **Step 6: Commit** `git commit -m "main pcb: display+touch header sheet"`.

---

## Task 5: I²C sheet (`main_04_i2c`)

**Files:** Create `main_04_i2c.kicad_sch`; Modify root.

**Interfaces:**
- Consumes: `3V3`,`GND`,`I2C0_SDA/SCL`,`I2C1_SDA/SCL`,`MUX_RST`,`MOD_INT0..7`.

- [ ] **Step 1: Place TCA9548A** (`U4`, 0x70, A0/A1/A2→GND). Upstream SDA/SCL→
  `I2C0_SDA`/`I2C0_SCL`; /RESET→`MUX_RST` with 10k pull-up to 3V3 (spec §4).
- [ ] **Step 2: Trunk pull-ups** — 4.7k on `I2C0_SDA`/`I2C0_SCL` to 3V3.
- [ ] **Step 3: 8 module sockets** `J_MOD0..7` (`Connector_Generic:Conn_01x05`,
  JST-XH). Each: pin1=3V3, pin2=GND, pin3=SDn (mux channel), pin4=SCLn,
  pin5=`MOD_INTn`. Each channel gets its **own 4.7k SDA/SCL pull-ups** to 3V3.
- [ ] **Step 4: 8 INT pull-ups** — 10k on each `MOD_INT0..7` to 3V3.
- [ ] **Step 5: Internal bus** — expose `I2C1_SDA/SCL` with 4.7k pull-ups to
  3V3 (single pair; charger/RTC live here, touch on display sheet).
- [ ] **Step 6: VALIDATION GATE** — ERC + render + **Read**. Verify 8 distinct
  channel nets (no accidental merge) and INT/pin5 mapping matches nav3.
- [ ] **Step 7: Commit** `git commit -m "main pcb: I2C mux + 8 module sockets sheet"`.

---

## Task 6: Audio sheet (`main_05_audio`)

**Files:** Create `main_05_audio.kicad_sch`; Modify root.

**Interfaces:** Consumes `3V3`,`GND`,`I2S_BCLK/LRCLK/DIN`.

- [ ] **Step 1: Place MAX98357A** (`U5`): BCLK/LRCLK/DIN from I²S nets, GAIN/
  SD_MODE strap resistor (set mono-mix + gain per datasheet), 100nF + 10µF caps.
- [ ] **Step 2: Speaker out** → `J_SPK` (`Connector:Conn_01x02`, JST-PH 2-pin).
- [ ] **Step 3: VALIDATION GATE** — ERC + render + **Read**.
- [ ] **Step 4: Commit** `git commit -m "main pcb: I2S audio amp sheet"`.

---

## Task 7: Joystick I/O sheet (`main_06_joystick_io`)

**Files:** Create `main_06_joystick_io.kicad_sch`; Modify root.

**Interfaces:** Consumes `3V3`,`GND`,`JOY_LX/LY/RX/RY/LSW/RSW`.

- [ ] **Step 1: Place `J_JOYL`,`J_JOYR`** (`Connector_Generic:Conn_01x05`, JST-PH):
  pins 3V3, GND, X, Y, SW. Left→`JOY_LX/LY/LSW`, Right→`JOY_RX/RY/RSW`.
- [ ] **Step 2: Optional RC filter** on each wiper (series 1k + 10nF to GND) —
  place footprints, values per spec §7.4.
- [ ] **Step 3: VALIDATION GATE** — ERC + render + **Read**.
- [ ] **Step 4: Commit** `git commit -m "main pcb: joystick ADC header sheet"`.

---

## Task 8: Connectors/misc sheet (`main_07_connectors_misc`)

**Files:** Create `main_07_connectors_misc.kicad_sch`; Modify root.

**Interfaces:** Consumes `3V3`,`GND`,`VBAT`,`SPI2_SCK/MOSI/MISO`,`SD_CS`,
`UART0_TX/RX`.

- [ ] **Step 1: Battery** — `J_BATT` (XT30, `Connector:Conn_01x02` or XT30
  footprint) → `VBAT`,`GND`; `J_BAL` (`Connector:Conn_01x03`, JST-XH) 2S balance
  → cell-mid sense to BQ25798.
- [ ] **Step 2: microSD** — `J_SD` push-push socket: SCK/MOSI/MISO shared SPI2,
  CS→`SD_CS`, 3V3, GND. Optional 100nF + series resistors.
- [ ] **Step 3: Debug header** — `J_DBG` (`Connector:Conn_01x04`): 3V3,
  `UART0_TX`, `UART0_RX`, GND.
- [ ] **Step 4: VALIDATION GATE** — ERC + render + **Read**.
- [ ] **Step 5: Commit** `git commit -m "main pcb: battery/SD/debug connectors sheet"`.

---

## Task 9: Root wiring + full-project ERC + design-rule check

**Files:** Modify `main.kicad_sch` (root).

- [ ] **Step 1: Verify hierarchy** with `sch_list_sheets` — all 7 sheets present,
  each with correct hierarchical pins for the nets it consumes/produces.
- [ ] **Step 2: Annotate** the whole design (`sch_annotate`) — unique refs, none
  starting with `#`.
- [ ] **Step 3: Cross-sheet net audit.** For each shared net (`3V3`,`GND`,
  `I2C0_*`,`I2C1_*`, all `MOD_INT*`, SPI2, I²S, joystick, USB) confirm it appears
  on every intended sheet via `sch_get_net_names`/`sch_trace_net`. A net that
  exists on only one sheet = a missing hierarchical label.
- [ ] **Step 4: Full ERC** — `kicad-cli sch erc main.kicad_sch --severity-all
  --exit-code-violations`. Drive to zero or justify each.
- [ ] **Step 5: Design-rule check** — `schematic_design_rule_check` for missing
  decoupling, absent I²C pull-ups, missing bulk caps (catches what ERC won't).
- [ ] **Step 6: Cosmetic pass** — `sch_cosmetic_score`; fix overlapping refs.
- [ ] **Step 7: VALIDATION GATE** — full multi-page PDF export
  (`kicad-cli sch export pdf`), **`Read` every page** and describe. Commit
  `git commit -m "main pcb: root hierarchy, clean full-project ERC"`.

---

## Task 10: Footprint assignment + handoff artifacts

**Files:** Create `fab/main-bom.csv`, `HANDOFF.md`; Modify symbols (footprint fields).

- [ ] **Step 1: Assign footprints** to every symbol (`lib_assign_footprint`) per
  BOM.md. Verify pad count + pin mapping for U1–U5 against datasheets.
- [ ] **Step 2: `validate_footprints_vs_schematic`** (or re-ERC) — every symbol
  has a footprint, none missing.
- [ ] **Step 3: Export BOM** — `kicad-cli sch export bom main.kicad_sch -o
  fab/main-bom.csv`.
- [ ] **Step 4: Write `HANDOFF.md`** — the net + footprint paper-tape the user
  needs for layout: board outline target (~80×60, case constraints from spec
  §11), placement notes (connectors on edges, decoupling by ICs), and the
  "user owns layout from here" statement (brief §9.5).
- [ ] **Step 5: FINAL GATE** — ERC clean, BOM exported, full render re-read.
  Commit `git commit -m "main pcb: footprints assigned, BOM + handoff exported"`.

---

## Self-review notes (author)

- **Spec coverage:** §1 decisions→Tasks 1–8; §3 power→T2; §4 I²C isolation→T5+T3;
  §5 MCU→T3; §6 pinmap→T3 (frozen names enforced); §7 peripherals→T4–T8;
  §8 connectors→T2/T4/T5/T6/T7/T8; §9 BOM→T1; §10 sheet structure→T2–T9;
  §11 workflow/handoff→T10; §12 open items→resolved inline in T1/T2.
- **No PCB layout tasks** — correct per scope (handoff after T10).
- **Net-name consistency:** every ESP32 net in T3 is copied verbatim from spec §6;
  T4–T8 consume those exact names. GPIO15=`MOD_INT7`, GPIO46=`DISP_BL` reflect the
  strapping fix.
- **Risk:** Task 0 gates the whole plan on `sch_*` tool availability (IPC was
  reported unavailable in the design session). If the live tools don't work, the
  fallback is GUI capture from the spec — do NOT hand-edit S-expressions.
