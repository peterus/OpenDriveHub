---
name: openscad-hardware
description: Use when editing .scad files or designing 3D-printable hardware in hardware/. Mandates parametric design with BOSL2 anchors, NopSCADlib standard parts, FDM-aware tolerances, and a visual render-check after every geometric change before declaring work done.
---

# openscad-hardware

You (the LLM) cannot see 3D shapes directly. This skill closes the gap with three things: libraries that eliminate spatial-math errors, a mandatory render-and-look workflow, and a reference corpus of FDM realities.

## The prime directive

**Never declare a .scad change "done" without having actually looked at a render.** Use the openscad-mcp `render_perspectives` tool (7 standard views), or fall back to direct CLI if MCP is unavailable. Then `Read` the PNGs — you are multimodal, you *can* see them. Describe what you see back to the user in your report. If the renders show nothing, clipping, or floating geometry, that's not done.

## When triggered

- Any edit to a `*.scad` file
- Any work under `hardware/`
- User asks to model a physical part, enclosure, bracket, or mount

## The render-check gate (non-negotiable sequence)

After every geometric change, run this exact sequence before reporting progress:

1. **`validate_scad`** — catches syntax errors and deprecated calls in a few ms.
2. **`analyze_model`** — exports STL via CGAL internally, so success here means the geometry is manifold. **This is your CGAL gate** — if it succeeds, preview-only CSG problems (non-manifold, zero-thickness, bad `difference`) are ruled out. Also: *always* cross-check the returned bounding-box dimensions against the numbers you expect from `parameters.scad`. Mismatches >0.1mm mean overlapping or gapped solids (see `pitfalls.md` #10).
3. **`render_perspectives`** with `output_format="file_path"` — 7 views, iso + 6 orthogonal.
4. **`Read` the PNGs** — you are multimodal; look at each one. Describe what you see back to the user. "It looks correct" without a Read call is a lie.

If any step fails, fix before proceeding. Declaring done without this gate is the #1 bug source.

## Workflow

1. **Check `parameters.scad`** in the relevant directory. If it doesn't exist, create it and put every numeric constant there — nothing hardcoded in part files.
2. **Use BOSL2 for all composition**. `include <BOSL2/std.scad>` (not `use` — BOSL2's anchor constants `TOP`/`BOTTOM`/`LEFT`/... require `include`). Prefer `attach(TOP) cuboid(...)` over `translate([...])` — any time you find yourself writing a `translate` with more than one non-zero offset, stop and ask whether `attach()` / `position()` expresses it better. See `references/libraries.md`.
3. **Use NopSCADlib for standard parts**: screws, heat-set inserts, boards, displays, connectors. Do not re-model what nophead already modeled correctly.
4. **Use `utils.scad` for colors and helpers** — `COLOR_METAL`, `COLOR_PCB`, etc. for consistent vitamin rendering; `print_bed_check()` in each printed part; `explode_shift()` in assemblies.
5. **Internal geometry** — when a part has pockets, bosses, or ribs, render a section view. Easiest: BOSL2's `back_half()`, `left_half()`, or `bottom_half()` wrapped around your part module — see `references/libraries.md`.
6. **Iterating on a part** — when changing a part that already renders, use MCP `compare_renders` to produce a side-by-side before/after. Essential when the user says "move it 5mm" and you need to verify the delta actually landed.
7. Run the render-check gate (above).

## Library policy

- **BOSL2** (`~/.local/share/OpenSCAD/libraries/BOSL2`): anchors, attachments, transforms, shapes, threading. Default choice for all geometric composition.
- **NopSCADlib** (`~/.local/share/OpenSCAD/libraries/NopSCADlib`): real-world parts. Has Raspberry Pi boards, ESP32 dev boards, common displays (SSD1306, TFTs), fasteners, inserts, connectors. Always prefer over hand-rolled.
- **MCAD** (bundled): rarely needed when BOSL2 is available.

## Red-flag patterns (stop and rewrite)

- `translate([a, b, c]) rotate([...]) translate([d, e, f])` — chain is a spatial-reasoning trap. Use BOSL2 anchors.
- Magic numbers in geometry expressions. Hoist to `parameters.scad` with a named constant.
- Copy-pasted geometry blocks for repetition. Use `for()` or `xcopies()`/`ycopies()` (BOSL2).
- `cube()` instead of `cuboid()` — BOSL2's `cuboid()` supports anchors, chamfers, rounding in one call.
- Missing tolerance on any hole/slot/fit. FDM needs slack. See `references/tolerances.md`.
- Non-manifold-prone operations: `difference()` with touching faces (add 0.01 overlap), zero-thickness walls.

## Rendering commands

**Via MCP (preferred)** — `render_perspectives` returns 7 PNGs in one call; `render_single` for custom camera; `validate_scad` for syntax; `analyze_model` returns bounding box + triangle count (catches dimensional bugs that visual inspection misses — always sanity-check dimensions against what you expect).

MCP gotchas:
- Pass `output_format="file_path"` explicitly. The default `"auto"` embeds PNGs as base64 which blows the token budget with >1 view.
- The MCP caches rendered PNGs by scad-file hash + parameters. Source-code changes DO invalidate it — but if a render looks stale after an edit, call `mcp__openscad__clear_cache` to force a full rebuild. Symptom: file size stays small / unchanged despite geometry changes.

**Direct CLI fallback:**
```bash
openscad -o preview.png --imgsize=1200,900 --viewall --autocenter \
  --colorscheme=Tomorrow --camera=0,0,0,60,0,30,0 part.scad
```
Camera tuple is `tx,ty,tz,rx,ry,rz,dist`. `--viewall --autocenter` with `dist=0` auto-frames.

## Directory convention

```
hardware/transmitter/parts/         one .scad per physical component, each with an anchor
  parameters.scad                   global constants (printer, tolerances, bed size)
  joystick.scad                     e.g. D400X-R4 model
  ...
  assembly.scad                     imports parts/*, positions them, added last
```

Each part file should:
- Import `parameters.scad`
- Define *one* main module named after the part
- Expose BOSL2 anchors for its mounting face (so assembly.scad can `attach()` it)
- Render a default call at file bottom so `openscad part.scad` produces a preview standalone

## References (read when the topic comes up)

- `references/libraries.md` — BOSL2 + NopSCADlib cheatsheet, the functions you will actually use
- `references/tolerances.md` — FDM tolerances, screw/insert hole sizes, wall thickness
- `references/pitfalls.md` — mistakes to not repeat

## MCP setup note

The openscad-mcp server requires Claude Code to be restarted after `.mcp.json` is added. The first run will pull the package via `uv run` (takes ~30s). If MCP tools aren't available, use the direct CLI fallback above — same rendering, just more lines.
