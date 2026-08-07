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

## Known defects (measured on KiCad 10.0.5 + kicad-mcp-pro 3.30.1)

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
