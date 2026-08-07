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

## Known defects

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
