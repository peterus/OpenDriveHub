# KiCad environment — snapshot of 2026-08-07

This file records one machine at one moment. Every version number, path and
tool name below was true on the date in the heading and may not be true now.
**Re-verify before relying on any of it.** The skill that points here contains
no version-dependent claims precisely so that this file can rot without taking
the guidance with it.

Fastest way to check whether this file is still current:

    kicad_get_version()

If the reported CLI version and IPC version differ from each other, or from the
versions below, stop and re-establish the facts before doing schematic work.

## Two KiCad installations, side by side

| | Version | Source | Invoked as |
|---|---|---|---|
| System | 9.0.8 | Debian `trixie-backports` package | `/usr/bin/kicad-cli` |
| Flatpak | 10.0.5 | `org.kicad.KiCad` from Flathub | `flatpak run org.kicad.KiCad` |

KiCad 9 is kept as an escape hatch. KiCad 10 is the working version and the one
the MCP server talks to.

Debian has no official KiCad apt repository — the PPA on kicad.org is
Ubuntu-only, and KiCad's own documentation names Flatpak as the recommended
route for every distribution outside Ubuntu and Fedora. KiCad 10 exists in
Debian `sid`/`forky` only; pulling it into `trixie` would drag unstable
libraries across.

### The CLI wrapper

`kicad-cli` inside the Flatpak is not on `$PATH`. A wrapper bridges it:

    #!/bin/sh
    exec flatpak run --command=kicad-cli org.kicad.KiCad "$@"

Installed at `~/.local/bin/kicad-cli-10`, and the MCP server is pointed at it
via `KICAD_MCP_KICAD_CLI`. Without this the server reaches KiCad 10 over IPC
while running every export, ERC and DRC through the 9.0.8 CLI.

The wrapper governs the shell as well as the server — it is not something the
MCP configuration alone takes care of. A bare `kicad-cli` typed at a shell
prompt reaches `/usr/bin/kicad-cli`, the 9.0.8 escape hatch, regardless of
what the server is doing. When validating work done through the server
(edits made via the `sch_*`/`pcb_*` MCP tools), run `kicad-cli-10` instead —
`kicad_get_version()` cannot catch this mismatch because it reports the
server's CLI, not the shell's.

### The sandbox boundary

The Flatpak declares `filesystems=home;/media;/run/media`. **Anything under
`/tmp` is invisible to KiCad 10**, including the agent scratchpad. Put working
files under `$HOME`.

## IPC

Needs no setup. `api.enable_server` is already `true` in a fresh KiCad 10.0.5
configuration, and the KiCad Python API resolves the Flatpak socket at
`~/.var/app/org.kicad.KiCad/cache/tmp/kicad/api.sock` on its own, negotiating
the access token on the first reply.

The MCP server's connection-failure message directs you to
`Preferences → Scripting → Enable IPC API Server`. On KiCad 10 there is nothing
to change there. If IPC is unreachable, the likelier causes are that KiCad is
not running, or that no document is open.

**Tool availability is decided when the MCP server starts.** IPC-gated tools —
`sch_add_power_symbol`, `sch_route_wire_between_pins`, `sch_create_sheet`,
`sch_add_pin_labels`, `sch_auto_place_symbols`, the `sch_*_plan` transaction
family and the cosmetic fixers — are absent from the session unless IPC was
reachable at that moment. Starting KiCad afterwards does not make them appear.

## MCP configuration

See `.mcp.json.example` in the repository root. Two independent axes:

| Variable | Controls | Effect of getting it wrong |
|---|---|---|
| `KICAD_MCP_PROFILE` | which tools exist | `agent_full` = 391 tools, `build` = 24 and no `sch_*` writers |
| `KICAD_MCP_OPERATING_MODE` | whether they may write | default `readonly` blocks every schematic edit |

Changing the profile when you meant to change the mode silently removes the
entire schematic toolset.

Both variables are read when the server process starts. Changing either one
means restarting the MCP client — the running session will not pick up the
new value, and the tool list it already computed does not change.

## Verified versus assumed

Invoked and confirmed working on 2026-08-07, KiCad 10.0.5 + kicad-mcp-pro
3.30.1:

- `kicad_create_new_project`
- `sch_add_component`
- `sch_get_pin_positions` — resolves all 16 pins of `Interface_Expansion:PCF8574AT`
- `sch_add_wire`
- `sch_add_label`

Everything else in the 391-tool surface is known only from reading the package
source. Confirm a tool is callable before building a procedure on it.

## The two installations bite the symbol cache

Measured 2026-08-07 while building `toggle3`.

The MCP server runs **outside** the Flatpak, so it reads symbols from the
Debian package at `/usr/share/kicad/symbols` — **KiCad 9**. `kicad-cli-10` runs
inside the Flatpak and validates against
`~/.local/share/flatpak/runtime/org.kicad.KiCad.Library.Symbols/.../symbols` —
**KiCad 10**. Every symbol the server places therefore lands in the schematic as
a KiCad 9 definition and ERC reports `lib_symbol_mismatch` against it.

`Switch:SW_SPDT` diffed between the two: KiCad 10 adds `in_pos_files`,
`duplicate_pin_numbers_are_jumpers`, `show_name` and `do_not_autoplace`, moves
`hide` from inside `effects` up to the property, changes the empty Datasheet
from `"~"` to `""`, and orders the pins differently. **Pin numbers, names and
coordinates are identical** — the netlist is unaffected, so this is a metadata
warning, not a wiring risk.

Fix is GUI-side: *Tools → Update Symbols from Library*, which rewrites the
cached definition from KiCad 10. Confirmed on `toggle3` — after the user ran it,
the cached `SW_SPDT` carried the KiCad 10 markers (`show_name`,
`do_not_autoplace`, `in_pos_files`, `duplicate_pin_numbers_are_jumpers`) and ERC
went from 3 warnings to 0.

Expect this on every symbol the server places from a system library, on every
board.

**Beware of attributing the fix to a tool call.** In this session the ERC
warnings vanished between two agent tool calls and the cache turned out to
already match KiCad 10 — which looked like a server-side write path silently
re-serialising `lib_symbols`. It was not: the user had run the GUI fix in
parallel. When a warning disappears without a matching action, establish who
changed the file before writing down a mechanism.

## Editing a schematic that is open in the GUI

KiCad does **not** reload schematics from disk, and F5 in Eeschema is redraw,
not reload. The MCP server's file-backed writes therefore stay invisible in a
window that already has the file open, and a `Ctrl+S` from that window
overwrites every server-side edit made since it was opened.

Working sequence: make the server-side edits, then close the document
**discarding changes**, then reopen. Confirm which document KiCad actually has
open before starting — `~<name>.kicad_sch.lck` next to the project names it.

## Tool defects seen while authoring toggle3 (2026-08-07)

- **`sch_get_connectivity_graph` mis-groups nets.** Seen merging `+3V3`, `GND`
  and `PWR_FLAG` into a single group, and separately reporting GND-tied address
  pins as `~unnamed`. Both were wrong. Use
  `kicad-cli sch export netlist` and read the `(nets ...)` block — that is
  KiCad's own connectivity engine and it was correct both times.
- **`sch_add_power_symbol` writes hash references** (`#PWRb9ee`) instead of the
  sequential `#PWR0101` form. `kicad-cli` then reports "schematic has annotation
  errors" on every export. **`sch_annotate` does not fix them** — it reported
  "Annotated 12 symbol(s)" and changed nothing. Fix in the GUI: *Tools →
  Annotate Schematic* with *Keep existing annotations*.
- **`sch_add_label` defaults to a 1.52 mm font** while KiCad's own default sheet
  text is 1.27 mm, so added labels look oversized next to existing ones.
  `sch_normalize_text_sizes(apply=true)` pulls them onto the sheet's dominant
  size and is connectivity-neutral.
- **`sch_render_png` is unavailable** — it needs CairoSVG and Pillow, which this
  install lacks. Render with `kicad-cli-10 sch export pdf` piped through
  `pdftoppm -png -r 150 -cropbox`; `rsvg-convert`, `inkscape` and ImageMagick
  are all absent, `pdftoppm` is present.
- **`sch_cosmetic_score` misses reference-text-over-symbol collisions.** It
  scored a sheet whose designators sat on top of their switch bodies without
  flagging it; the defect was obvious in the render. Look at the image — the
  score is a supplement, not a substitute. `sch_autoplace_fields` fixes it.
- **All 48 `pcb_write` tools are unavailable** unless a PCB document is open in
  KiCad; live PCB writes need KiCad 10 + an open board. The `routing` family
  additionally needs `KICAD_MCP_OPERATING_MODE=experimental`, and the
  manufacturing/release family needs `manufacturing`. `project_get_next_action`
  will happily recommend `manufacturing_quality_gate()`, which is itself
  unavailable in `write` mode.

## PCB layout through the MCP server is not possible — a write kills the IPC link

Measured 2026-08-08 on kicad-mcp-pro 3.30.1 + KiCad 10.0.5. Confirmed on a
deliberately clean stack after two inconclusive attempts:

- KiCad 10 freshly restarted, schematic and board both saved and open
- MCP client started afterwards, so the write tools registered
- `kicad_get_version` reporting `IPC version: 10.0.5`, `Open PCB documents: 1`
- **a single `pcb_move_footprint` as the first action of the session**

Result: `CLI_TIMEOUT: Error receiving reply from KiCad: Timed out`, the board
unchanged, and the connection dead. The server's own diagnostics then read
*"The KiCad IPC connection dropped (KiCad may have closed or restarted) and did
not recover."* Nothing had closed or restarted — the write did it.

**Do not attempt PCB layout through this server.** Placement, routing, zones and
board outline all have to happen in the KiCad GUI. This matches the skill's
division of labour, though for a different reason than the 2026 note recorded.

### Why this took three sessions to pin down

After the connection dies, reads keep working — they **silently fall back to
parsing the `.kicad_pcb` file**. `pcb_get_footprints` still returns correct,
plausible data, labelled `file-backed fallback` in a diagnostics block that is
easy to skim past. So the board "reads fine" while every write vanishes.

Earlier attempts saw the same underlying failure wearing a different mask:
mutations returned cheerful success strings (`Deleted 50 item(s).`,
`Moved footprint 'H101' to (104.0, 104.0) mm with verified rotation`) against an
unchanged board. The `with verified rotation` wording is outright false.

Two rules follow:

1. **A PCB write tool's return string is not evidence.** Read the board back.
2. **Check the read's `Source:` field.** `live-gui` means the editor;
   `file-backed fallback` means the IPC link is gone and you are looking at
   disk, not at what the tool claims to have changed.

Read-only work is unaffected and remains the agent's useful contribution:
`run_drc`, `pcb_visual_qa`, `pcb_score_placement`, `pcb_critique_placement`,
`validate_footprints_vs_schematic`, renders, and the STEP → STL case-fit loop.

### Schematic writes are fine — the split is architectural, not general

Tested straight afterwards, with the IPC link already dead from the PCB write:
`sch_set_title_block_info` wrote a field, reported `roundtrip: validated` with
before/after hashes, and the change was confirmed present in the file. Reverting
it worked the same way. Element counts and the exported netlist were unchanged
throughout.

So this is **not** "IPC writes are broken". It follows the server's own
per-category fallback policy, visible in `kicad_get_server_info`:

| Category | Policy |
|---|---|
| `schematic` | IPC when required, otherwise a **transactional file writer** with structural fingerprint loss detection |
| `pcb_write` | **Fail closed** when an IPC-required mutation has no live backend |

Schematic mutations have a file-backed path and survive a dead IPC link. PCB
mutations do not — and since the first PCB write kills the link, they never
succeed. **Schematic capture through the agent is fine; PCB layout is not.**

### A KiCad 10 save re-serialises the whole schematic

Unrelated to correctness, but alarming at first sight: after the user saved the
schematic in KiCad 10, `git diff` showed 267 insertions and 402 deletions
including apparently-removed `(wire ...)` blocks. Nothing was lost — elements
are just re-ordered in the file. Counts of `wire`, `label`, `junction`,
`no_connect` and `symbol` were identical before and after, and so was the
exported netlist.

Check counts and the netlist before reacting to a large `.kicad_sch` diff.

| Call | Returned | Board afterwards |
|---|---|---|
| `pcb_delete_items` (50 UUIDs) | `Deleted 50 item(s).` | unchanged — still 33 tracks, 12 footprints, 1 zone, 4 shapes |
| `pcb_delete_items` (1 UUID) | `Deleted 1 item(s).` | unchanged |
| `pcb_move_footprint("H101", 104, 104)` | `Moved footprint 'H101' to (104.0, 104.0) mm **with verified rotation** 0.000 degrees` | `H101` still at (103.00, 103.00) |

The `pcb_move_footprint` message is the dangerous one: it claims to have
*verified* the result. It has not. **Never treat a PCB write tool's return
string as evidence — always read the board back** with `pcb_get_footprints` /
`pcb_get_board_summary`.

Reads are fine throughout. `pcb_get_board_summary` reports `Source: live-gui`
and every `pcb_get_*` returns correct live data. Only writes are affected.

Also ruled out along the way: a dangling `pcb_begin_commit` transaction, a
missing PCB document, missing IPC, missing tool registration, and a stale KiCad
instance with accumulated API connections. None of them was the cause; the final
test had all of them excluded by construction.

The dead connection never heals on its own. A plain Python client still opens
the socket fine and `kicad_set_project()` does not re-establish it — only
restarting the MCP server does, and the next write kills it again.

### Opening a project in KiCad 9 downgrades `.kicad_pro`

`net_settings.meta.version` drops **5 → 4** and the per-netclass
`tuning_profile` fields disappear. Observed on `encoder1.kicad_pro` and
`test.kicad_pro`.

This was first blamed on the MCP server, wrongly. **KiCad 9 wrote it** — the
project had been opened briefly in the system KiCad 9.0.8 before the user
switched to the Flatpak KiCad 10. KiCad 10 writes schema 5, KiCad 9 writes 4, so
the file flips whenever the wrong version opens it.

Both installations are on this machine and the 9 one is the default on `$PATH`,
so this is easy to trigger by accident. `git diff` the `.kicad_pro` before
committing and revert it if the only change is this downgrade.

### The transaction family is broken too

`pcb_begin_commit` succeeds, but both ways of ending the transaction crash:

    pcb_push_commit  -> Board.push_commit() missing 1 required positional argument: 'commit'
    pcb_drop_commit  -> Board.drop_commit() missing 1 required positional argument: 'commit'

So a transaction group, once opened, can be neither applied nor discarded. Do
not call `pcb_begin_commit`.

## PCB write tools need a board open *before* the server starts

Measured 2026-08-08 while trying to lay out `encoder1`.

The 48 `pcb_write` tools — `pcb_set_board_outline`, `pcb_sync_from_schematic`,
`pcb_place_component`, `pcb_move_footprint`, `pcb_add_track`,
`pcb_add_copper_zone`, `pcb_refill_zones`, `pcb_delete_items`, `pcb_save` and the
rest — are registered only if a PCB document is open in KiCad **at the moment the
MCP server process starts**. This is the same start-time gate the IPC section
above describes for `sch_*`, and it behaves the same way: opening the board
afterwards does not make the tools appear.

What *does* start working immediately is live PCB **reading**.
`pcb_get_board_summary` reports `Source: live-gui` and `pcb_get_footprints`
returns the open board's placement as soon as KiCad has it open. That asymmetry
is misleading — live reads working is not evidence that writes will.

To lay out a board: open both the `.kicad_pcb` and the `.kicad_sch` in KiCad
first, then start the MCP client.

## `kicad_get_tools_in_category()` is not evidence of callability

Same session. With no PCB open at server start,
`kicad_get_tools_in_category("pcb_write")` still listed all 48 tool names with
maturity flags and `Active operating mode: write`, implying they were ready.
None of them existed in the session. The call reads the server's **static
registry**, not what this session can invoke.

The authoritative check is whether the tool is present in the session's own tool
list — on a deferred-tool harness, whether `ToolSearch` can resolve the name. If
it returns "no matching deferred tools found", the tool cannot be called no
matter what the registry says.

This is the concrete case behind the Preflight rule in the skill: confirm the
specific tools your plan depends on are callable *before* planning around them.

## Known defects (measured on KiCad 10.0.5 + kicad-mcp-pro 3.30.1)

**`pcb_get_origin` always fails.** It raises
`Board.get_origin() missing 1 required positional argument: 'origin_type'` — the
server calls the KiCad API without the required argument and exposes no
parameter to supply it. There is no workaround through the tool; read the
origin from the `.kicad_pcb` or the GUI instead. (Measured 2026-08-08 on a live
board that every other `pcb_get_*` read fine.)


**`sch_build_circuit` orphans child sheets.** It rebuilds one file — the active
schematic — from scratch with a fresh root UUID and a flat
`(sheet_instances (path "/"))`, and takes no `sheet` parameter. Pointed at a
root sheet it deletes the `(sheet ...)` references; the child files survive on
disk but are no longer part of the design. It also calls
`transactional_write(..., allow_node_loss=True)`, disabling the loss detection
the server advertises elsewhere. Use it per child sheet, never on a root sheet.

**The server stamps a KiCad 10 format version.** Written schematics carry
`(version 20250316)` and `generator "kicad-mcp-pro"`. KiCad 9.0.8 loaded such a
file without complaint through `kicad-cli`; this was not checked against the
KiCad 9 GUI.

**KiCad 10 reports library drift on KiCad 9 boards.** `nav3` gives 0 ERC
violations under 9.0.8 and 35 warnings under 10.0.5 — 23 `lib_symbol_issues`,
12 `footprint_link_issues`, no electrical errors. The cached symbol definitions
simply predate KiCad 10's libraries. The footprint count was measured in an
isolated copy without an `fp-lib-table` and may be inflated.
