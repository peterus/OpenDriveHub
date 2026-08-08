# Main PCB — Session Handoff (2026-08-08)

Handoff so the schematic work can continue on another machine (one with the
**newest KiCad + a live IPC connection**, which unlocks the hierarchical
schematic tools that are unavailable in the current headless environment).

---

## TL;DR — where we are

- **Branch:** `main-pcb-schematic` (pushed to origin). Based on `main`.
- **Done:** brainstormed design → wrote spec → wrote plan → scaffolded a fresh
  **KiCad 10** project → locked the BOM. (Tasks 0 + 1 of the plan.)
- **Stopped at:** the start of schematic capture (plan Task 2), on a **toolchain
  blocker**: this headless environment has **no KiCad IPC connection**, and the
  hierarchical-schematic tools (`sch_create_sheet`, `sch_add_global_label`,
  `sch_add_hierarchical_label`, `sch_add_power_symbol`) are **IPC-gated =
  unavailable here**. A real multi-sheet hierarchy cannot be built headless.
- **Next action on the new PC:** with a live KiCad IPC connection, those tools
  light up → build the schematic hierarchically per the plan (the preferred
  "Option C"). See "Resume" below.

---

## The decision that was pending

The design must use **multiple hierarchical sheets** (user requirement, design
is large). Three options were on the table:

- **A — flat AI draft now, split into sheets in GUI later** (headless-compatible).
- **B — user builds the schematic in the GUI directly** (spec/BOM/pinmap already done).
- **C — enable KiCad IPC, then build the full hierarchy automatically.** ← the new
  PC (newest KiCad, IPC available) makes this the right path.

**On the new machine, do C.** First verify IPC is actually connected (see Resume
step 3); if it is, proceed with the plan's Tasks 2–10 as written (hierarchical).
If for some reason IPC is still unavailable, fall back to A.

---

## Skills to load first (in this order)

1. **`kicad-hardware`** (project skill) — the prime directive (never "done"
   without clean ERC + a *looked-at* render), the no-hand-editing-S-expressions
   rule, the validation gate, library policy. NON-NEGOTIABLE for this work.
2. **`kicad-schematic-design`** (project skill) — the 3-phase schematic workflow
   with approval + ERC gates.
3. **superpowers `executing-plans`** (or `subagent-driven-development`) — to
   execute the written plan task-by-task.
4. The **KiCad MCP built-in workflow** `project_design_workflow` — phase/gate
   state machine (Planner→Builder→Verifier→Fixer→Release). We drive Tasks 2–4
   (schematic phases) with it; **placement/routing stays with the user** (brief
   §9.5 — scripted layout is a dead end).

The generic `superpowers:brainstorming` and `writing-plans` are already **done**
(spec + plan exist) — no need to re-run them.

## Key files

| File | What |
|------|------|
| `docs/superpowers/specs/2026-08-08-transmitter-main-pcb-design.md` | **Design spec** — architecture, power tree, dual-I²C isolation, frozen GPIO pin map (§6), BOM, sheet structure. Source of truth. |
| `docs/superpowers/plans/2026-08-08-transmitter-main-pcb-schematic.md` | **Implementation plan** — Tasks 0–10, KiCad validation cycle, per-sheet component/net lists. |
| `hardware/transmitter/pcb/main/BOM.md` | **Locked BOM** — every symbol/footprint verified in KiCad-10 std libs (zero custom symbols), Mouser MPNs. |
| `hardware/transmitter/pcb/main/main.kicad_{pro,sch,pcb}` | Fresh KiCad-10 project (empty schematic, ERC-clean). |
| `hardware/transmitter/PCB_DESIGN_BRIEF.md` | Original brief (mechanical envelope, sub-PCBs, workflow split §9.5). |

## Frozen decisions (do not re-litigate)

- **MCU:** ESP32-S3-WROOM-1**U**-**N16R2** (external U.FL antenna; **Quad**-PSRAM —
  an R8/Octal variant steals GPIO35–37 and breaks the pin budget).
- **Charger:** BQ25798 (buck-boost 2S, I²C ADC telemetry) on the internal I²C bus.
- **Rails:** single **3.3 V** (TPS62933 buck from charger SYS). No 5 V rail.
- **Audio:** MAX98357A (I²S, 3.3 V, mono).
- **Display/touch:** ST7796 SPI + FT6236 touch (2.54 mm header harness).
- **Dual I²C isolation:** **I²C0** = module bus (TCA9548A mux only → 8 module
  sockets); **I²C1** = internal bus (touch + charger + RTC). A wedged module can
  only kill I²C0; firmware recovers via MUX_RST.
- **8 module sockets**, each with a **direct /INT** GPIO. Touch is OFF the mux.
- **Debug:** USB-C (USB-Serial-JTAG) + physical UART0 header on GPIO43/44.
- **GPIO pin map is FROZEN** — spec §6. Includes the strapping fix: GPIO15=MOD_INT7,
  GPIO46=DISP_BL (ext pull-down), GPIO45=SD_CS, GPIO3=MUX_RST, GPIO0=BOOT btn.
- **Display+SD share SPI2** (SCK/MOSI/MISO, separate CS). microSD is on the shared bus.
- **All BOM parts are KiCad-10 standard-library** — no project-local sym/fp libs.

## Environment / toolchain facts

- KiCad MCP Pro server **v3.30.1**, `kicad-cli-10` **10.0.5**.
- **`sch_*` tools work file-based/headless for**: add_component, add_symbol,
  add_wire, add_label (local only), add_no_connect, build_circuit, annotate,
  check_power_flags, all get_*/render/ERC. (Verified: add + readback works.)
- **IPC-gated (need live KiCad, were unavailable headless):** create_sheet,
  add_global_label, add_hierarchical_label, add_power_symbol, add_bus,
  delete/move symbol, route_wire_between_pins, add_missing_junctions, variants.
- `kicad_create_new_project(path, name)` creates `path/name/name.kicad_*`
  (pass `path` = the **parent** dir, i.e. `.../pcb`, name=`main`).
- **Do NOT reuse nav3** (KiCad 9). encoder1 branch is KiCad 10 if a starter is needed.
- Design intent already stored in the project via `project_set_design_intent`
  (7 required sheets, critical nets, JLCPCB).
- `project_import_design_spec` needs structured YAML/JSON and sandboxes to the
  project dir — our prose spec doesn't import cleanly; intent was set directly.

## Git state

- Branch `main-pcb-schematic`, commits (newest first):
  `e4f196a` BOM lock · `d27c950` scaffold KiCad10 · `efc6bc4` plan→KiCad10 ·
  `7f7eb93` plan · `c3c337b` spec.
- `fab/`, `*.kicad_prl`, `.kicad-mcp/` are gitignored (regeneratable/per-user).

## Resume (on the new PC)

1. `git fetch && git checkout main-pcb-schematic` (pull latest).
2. Load the skills listed above (kicad-hardware, kicad-schematic-design,
   executing-plans).
3. **Check IPC:** call `kicad_get_version()`. If `IPC connection: connected`
   (needs KiCad GUI running with the IPC/API server enabled + the MCP pointed at
   it) → the hierarchical tools are available → do **Option C**. If still
   `unavailable` → do **Option A** (flat build, split in GUI).
4. `kicad_set_project(project_dir=.../hardware/transmitter/pcb/main)`.
5. Execute the plan from **Task 2** (Power sheet) onward. Validation gate after
   every sheet: `kicad-cli sch erc` clean + render + **actually Read the image**.
6. Hand off to the user for PCB layout after Task 10 (ERC clean + net/footprint list).

---

## Resume prompt (paste into the new session)

> Wir bauen das **main-PCB-Schematic** des OpenDriveHub-Transmitters weiter.
> Repo ist ausgecheckt auf Branch **`main-pcb-schematic`**.
>
> 1. Lies zuerst `docs/superpowers/2026-08-08-main-pcb-session-handoff.md` — das
>    ist der komplette Stand.
> 2. Lade die Skills **kicad-hardware** und **kicad-schematic-design**, dazu
>    **superpowers:executing-plans**.
> 3. Der Design-Spec ist `docs/superpowers/specs/2026-08-08-transmitter-main-pcb-design.md`
>    (Pinmap §6 ist eingefroren), der Plan ist
>    `docs/superpowers/plans/2026-08-08-transmitter-main-pcb-schematic.md`,
>    die BOM ist `hardware/transmitter/pcb/main/BOM.md`. Tasks 0+1 sind fertig.
> 4. Prüfe mit `kicad_get_version()`, ob jetzt **IPC connected** ist. Wenn ja:
>    baue das Schematic **hierarchisch** (mehrere Sheets) per Plan Task 2→10 —
>    das war vorher headless blockiert. Wenn IPC weiter unavailable: bau flach
>    und ich splitte in der GUI.
> 5. Setz das Projekt aktiv (`kicad_set_project` auf
>    `hardware/transmitter/pcb/main`) und fang bei **Task 2 (Power-Sheet)** an.
>    Nach jedem Sheet: ERC sauber + Render + **Bild wirklich anschauen**, dann commit.
> 6. PCB-Layout mache ich (User) in der GUI — du übergibst nach Task 10 mit
>    sauberer ERC + Netz/Footprint-Liste (Brief §9.5).
