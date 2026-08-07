---
name: kicad-hardware
description: Use when editing .kicad_sch / .kicad_pcb / .kicad_pro files or designing PCBs in hardware/*/pcb/.
---

# kicad-hardware

You (the LLM) cannot see schematics or boards directly. This skill closes the gap with three things: a strict no-hand-editing rule for KiCad files, a mandatory validate-and-look workflow after every change, and a library hierarchy that keeps you out of footprint-hell.

## The prime directive

**Never declare a KiCad change "done" without (a) clean ERC, (b) clean DRC if a PCB exists, and (c) actually looking at a render.** Use `kicad-cli sch export svg` for schematics and `kicad-cli pcb render` for boards (or the equivalent kicad-mcp-pro tools). Then `Read` the SVG/PNG — you are multimodal, you *can* see them. Describe what you see back to the user. "It looks correct" without a Read call is a lie.

ERC/DRC warnings are not optional decoration. Either fix them or explicitly tell the user which warning was suppressed and why.

## Division of labour

Settled empirically on the nav3 board, after scripted layout produced nothing
usable:

- **You (the agent) do**: schematic capture as a *starting draft*, project
  scaffolding, design rules, the BOM draft, validation, STEP export and the
  OpenSCAD case-fit check.
- **The user does**: schematic review and rework — your draft is a starting
  point, not a proposal to defend — and **the entire PCB layout**: placement,
  routing, pours, silkscreen, keep-outs, in the GUI.

Hand off when ERC is clean and a net-and-footprint list exists. From that point
the board file belongs to the user. Do not place or route footprints unless
asked directly.

## When triggered

- Any edit to `*.kicad_sch`, `*.kicad_pcb`, `*.kicad_pro`, `*.kicad_sym`, `*.kicad_mod`, `sym-lib-table`, `fp-lib-table`
- Any work under `hardware/*/pcb/`
- User asks to design a circuit, schematic, footprint, board, or to add components to an existing PCB

## Preflight

Before the first edit in a session, call `kicad_get_version()` and read three
things off it:

1. **CLI version against IPC version.** If they differ, exports and live edits
   are addressing different KiCad releases and results will not agree. Fix that
   before continuing.
2. **Whether IPC is connected.** It gates roughly a third of the schematic
   tools, and the gate is applied when the MCP server starts — not when you
   first call a tool.
3. **Whether a document is open**, if you intend to use anything live.

Then check that the specific tools your plan depends on are actually callable.
A tool being documented, or present in the server's own capability registry, is
not evidence that this session can invoke it — the registry over-reports.

Environment specifics live in `references/kicad-environment.md`. That file is a
dated snapshot; trust `kicad_get_version()` over it.

## File-edit policy (the hand-editing trap)

**Do not hand-edit `.kicad_sch` or `.kicad_pcb` with Edit/Write.** They are S-expression files where every component, wire, and pad has a UUID, and net connectivity is implicit in coordinates and label proximity. A misplaced paren, a stale UUID, or a 0.01 mm coordinate drift silently breaks netlists, ERC, or the file itself.

Use, in order of preference:

1. **kicad-mcp-pro tools** — the `sch_*` family for schematic work, `pcb_*` for
   boards. Confirm the specific tool is callable first; see Preflight.
2. **kicad-cli** — for ERC, DRC, exports and renders.
3. **Hand editing only**: project metadata in `.kicad_pro` (JSON, safe),
   `sym-lib-table` / `fp-lib-table` (S-expressions, but trivial structure),
   comments and text in title blocks.

If the editing tools are unavailable, say so and stop — do not hand-edit
S-expressions as a workaround.

## The validation gate (non-negotiable sequence)

After every meaningful change, run this exact sequence before reporting progress:

1. **`kicad-cli sch erc <file>.kicad_sch --output erc.json --format json --severity-all`** — schematic electrical rules. Read the JSON. Zero violations or each one explained and dismissed.
2. **`kicad-cli pcb drc <file>.kicad_pcb --output drc.json --format json --severity-all`** — design rules check. Same standard. Skip only if no PCB exists yet.
3. **`kicad-cli sch export svg <file>.kicad_sch -o sch.svg`** — schematic image. `Read` it. Describe what you see.
4. **`kicad-cli pcb render <file>.kicad_pcb -o top.png --side top --quality high`** plus a bottom or rotated view. `Read` the PNG. Describe what you see.

Failing any step → fix before continuing. Skipping any step → not done.

## Workflow (greenfield PCB)

1. **Read the design brief.** `hardware/transmitter/PCB_DESIGN_BRIEF.md` is
   canonical for the transmitter. Cross-check pin counts, voltages and the
   mechanical envelope.
2. **Lock the BOM first.** Every part with a manufacturer part number, and for
   each one a decision: standard KiCad library, SnapEDA or similar,
   manufacturer-provided, or hand-rolled as a last resort. Record it in a
   `BOM.md` next to the project. No wires before this exists.
3. **Scaffold the project**, then set the design intent before schematic work.
4. **Place before connecting.** Ask `sch_find_free_placement` for coordinates
   rather than guessing them, and add power symbols before the circuits that
   depend on them.
5. **Connect by name, not by geometry.** Prefer a short pin stub plus a
   same-named label over drawn wires between pins. Wires that cross unrelated
   pins get merged by KiCad on geometry, which produces silent shorts that no
   amount of visual inspection reliably catches. Where a tool offers a routed
   alternative it usually says so in its own documentation — believe the
   warning. Add junctions afterwards.
6. **Validate in this order**: power flags, ERC, then the schematic design-rule
   check, which reports missing decoupling, absent I²C pull-ups and crystal
   load capacitors that ERC will not.
7. **Then make it readable.** Cosmetic quality is measurable — score it, fix,
   re-measure. Overlapping reference designators are the usual first offender.
8. **Footprint assignment** — every symbol gets a footprint. Verify pad-count and pin-mapping for ICs against the datasheet, not just the symbol's pin numbers. Wrong footprint = dead board.

**Ownership changes here.** ERC is clean and a net-and-footprint list exists —
the hand-off condition from Division of labour. Steps 9-12 below describe
what happens to the board next, not what you do next: they belong to the
user, in the GUI. Read them for context; do not execute them unless asked
directly.

9. **PCB outline** (user) — set board edge first. For OpenDriveHub sub-PCBs the outline is constrained by the case cutouts in `hardware/transmitter/parts/layout_front.scad` — measure there, do not guess.
10. **Place** (user) — connectors and mechanically-constrained parts first (where they have to be), then ICs, then passives. Decoupling caps next to their IC pins, not "somewhere on the rail".
11. **Route** (user) — power and ground first (or pour ground), then high-speed signals, then the rest. For I²C-only sub-PCBs (nav3, encoder1, etc.) routing is trivial; for the main board it is the bulk of the work.
12. **DRC + render gate** (user runs layout to this point; you re-enter for validation) — see above. Then export gerbers + drill + position file + STEP for the case-fit check.

## Workflow (modify existing PCB)

1. Render the *current* state first (`pcb render`). Look at it. Establish baseline.
2. Make the change via MCP tools.
3. Render again. Compare to baseline. Confirm only the intended thing moved.
4. ERC + DRC.

## Library policy (in this order)

1. **KiCad's bundled standard libraries.** Covers passives, common ICs (74xx, CD4xxx, common MCUs, common interfaces), connectors, switches, the mechanical library. **Always check here first.**
2. **SnapEDA / Ultra Librarian / Component Search Engine** — for parts not in standard libs (IP5389, FT6236, ST7796 FPC connectors, etc.). Download as KiCad-format symbol+footprint+3D, drop into a project-local library, never into the system libs.
3. **Manufacturer-provided KiCad libraries** — some chip vendors (Espressif, ST, TI sometimes) ship official KiCad libs. Prefer these over SnapEDA when available.
4. **Hand-rolled symbol/footprint** — only if the part is genuinely not on the internet. Cross-check pad coordinates against the datasheet *millimeter by millimeter* before trusting it.

Project-local libs live next to the `.kicad_pro` file: `<board>-symbols.kicad_sym`, `<board>.pretty/` (footprint dir), `<board>.3dshapes/`. Register them in the project-local `sym-lib-table` / `fp-lib-table`, not the global ones, so the project stays self-contained.

## Red-flag patterns (stop and fix)

- A schematic with **no power flags** (`PWR_FLAG`) on a power net → ERC will scream "input power port not driven". Add flags at the source (regulator output, battery, USB).
- **Floating inputs** on logic ICs → tie unused inputs high or low, do not leave dangling.
- **Missing decoupling caps** on every IC VCC pin → 100 nF per pin, plus a bulk cap (1–10 µF) per IC.
- **No bulk cap near the regulator** → switching/LDO regulators need input + output caps per datasheet.
- **I²C bus with no pull-ups** → 4.7 kΩ to VCC on SDA + SCL, exactly once per bus (not per device).
- **Pads under no copper pour** when a ground pour is expected → run "fill all zones" before DRC.
- **Track widths chosen by default** without thinking → power tracks need width per current, signal tracks need width per impedance/length.
- **Hand-edited S-expressions** → see file-edit policy. Do not.

## Render and validation commands

**Schematic image:**
```bash
kicad-cli sch export svg board.kicad_sch -o board.svg --no-background-color
# or PDF for multi-page hierarchical:
kicad-cli sch export pdf board.kicad_sch -o board.pdf
```

**PCB 3D renders (look from multiple angles):**
```bash
kicad-cli pcb render board.kicad_pcb -o top.png    --side top    --quality high --width 1600 --height 1200
kicad-cli pcb render board.kicad_pcb -o bottom.png --side bottom --quality high --width 1600 --height 1200
kicad-cli pcb render board.kicad_pcb -o iso.png    --rotate '-30,0,30' --perspective --quality high
```

**PCB 2D layer plots (when 3D obscures routing):**
```bash
kicad-cli pcb export svg board.kicad_pcb -o front.svg --layers F.Cu,F.Mask,F.Silkscreen,Edge.Cuts --page-size-mode 2
kicad-cli pcb export svg board.kicad_pcb -o back.svg  --layers B.Cu,B.Mask,B.Silkscreen,Edge.Cuts --page-size-mode 2 --mirror
```

**ERC / DRC as JSON:**
```bash
kicad-cli sch erc  board.kicad_sch --output erc.json --format json --severity-all --exit-code-violations
kicad-cli pcb drc  board.kicad_pcb --output drc.json --format json --severity-all --exit-code-violations
```
The `--exit-code-violations` flag makes the command exit non-zero if violations exist, so the shell return code alone tells you if it passed.

**Manufacturing exports** (only after gate is clean):
```bash
kicad-cli pcb export gerbers  board.kicad_pcb -o fab/
kicad-cli pcb export drill    board.kicad_pcb -o fab/
kicad-cli pcb export pos      board.kicad_pcb -o fab/board-pos.csv --format csv
kicad-cli sch export bom      board.kicad_sch -o fab/board-bom.csv
```

The STEP export is **not** a fab output — it belongs to the case-fit loop below and goes somewhere else.

## Directory convention

```
hardware/transmitter/pcb/
  nav3/
    nav3.kicad_pro
    nav3.kicad_sch
    nav3.kicad_pcb
    nav3_local.kicad_sym         # project-local symbols (only if needed)
    nav3.pretty/                 # project-local footprints (only if needed)
    nav3.3dshapes/               # project-local STEP/WRL (only if needed)
    sym-lib-table                # project-local lib registration
    fp-lib-table
    BOM.md                       # part list with MPNs and lib sources
    fab/                         # gerbers, drill, pos, bom — generated, .gitignored
  encoder1/
  toggle3/
  illum3/
  main/
```

One KiCad project per physical PCB. The `fab/` directory is regeneratable — add it to `.gitignore`.

## PCB → OpenSCAD case-fit loop

Every board destined for the existing case must be verified against it geometrically, not by eyeballing dimensions. Once DRC is clean:

```bash
cd hardware/transmitter/pcb/<board>
kicad-cli pcb export step <board>.kicad_pcb -o <board>.step
# Promote to the canonical tracked location — the copy under pcb/ is gitignored:
cp <board>.step ../../parts/<board>_actual.step
cd ../../parts
STEP_IN=<board>_actual.step STL_OUT=<board>_actual.stl freecadcmd step_to_stl.py
```

Then import the STL in `parts/pcb_subpanel.scad` and render `case/assembly_check.scad` to confirm the panel-side components line up with the cutouts and clear the corner bosses.

Use `step_to_stl.py`, not a plain STEP→STL converter. It fuses the solids KiCad emits as separate bodies first; without that, OpenSCAD's CGAL backend (F6) treats their shared faces as non-manifold and silently drops most of the geometry.

Two alignment traps live in this loop — KiCad's screen-Y is inverted relative to STEP/STL Y, and a board's component centroid is generally not its PCB centroid. Both are documented with their fixes in `PCB_DESIGN_BRIEF.md` §9.7. Read that before hand-tuning any offset.

The case is the source of truth for outer dimensions; the PCB has to comply.
