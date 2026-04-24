# Pitfalls — mistakes an LLM makes in OpenSCAD, and how to avoid them

## 1. Declaring work done without rendering

Failure: edited `.scad`, reported "done", user discovers parts floating/clipping.
Root cause: you can't see geometry from code. You *must* render + Read the PNG.
Avoid: after any geometric change, `render_perspectives` → Read → describe what you actually see. "I rendered and it looks correct" without a render call is a lie.

## 2. Chained translate+rotate drift

Failure: `translate([x+w/2, -d/2, z+h]) rotate([0,90,0]) translate([-h/2, 0, 0]) child();` works until you change a dimension, then everything slides.
Avoid: BOSL2 `attach()` / `position()`. Anchors are relative to the parent and survive dimension changes.

## 3. Magic numbers

Failure: `translate([19.4, 0, 0])` with no named constant. Six months later nobody knows what 19.4 referred to.
Avoid: every number is either 0, a small integer count (e.g. `$fn`), or a named constant in `parameters.scad`. Derive related numbers, don't duplicate.

## 4. Treating preview as truth

Failure: F5 preview looks fine, F6 CGAL render fails with "CGAL error in CGAL_Nef_polyhedron3()" or produces empty output.
Root cause: preview is OpenCSG (fast, non-exact). Non-manifold, coincident faces, or zero-thickness walls pass preview and fail render.
Avoid: `validate_scad` or a full CGAL render before calling anything done. Add 0.01mm overlap on every `difference()` where faces touch.

## 5. Modeling standard parts by hand

Failure: hand-coded an M3 screw hole with guessed dimensions; it's too tight.
Avoid: use NopSCADlib `screw_hole_radius(M3_cap_screw)` or `insert_hole_radius(M3_insert)`. These are tested against real parts.

## 6. Ignoring print orientation

Failure: designed a bracket where the thin neck must be printed along Z → snaps under load.
Avoid: note the intended print orientation in a comment at the top of each part. Stress should travel within XY layers, not across them.

## 7. No section view for internal geometry

Failure: modeled an enclosure with internal bosses/ribs. External renders look fine. User prints it and discovers the bosses don't align with the PCB because you couldn't see inside.
Avoid: when internal geometry matters, add a `cut_through` parameter that does `difference() { part(); translate([0,0,-0.1]) cuboid([1000,1000,1000], anchor=BOTTOM+LEFT); }` and render that variant too.

## 8. Forgetting `$fn`

Failure: holes are 6-sided, curves are blocky.
Avoid: set `$fn=64;` (or higher for large features) at the top of `parameters.scad`. For small features use `$fn=32` to keep render time down; bump for finals.

## 9. Panel cutouts that don't account for boot/shroud

Failure: modeled a "25mm hole" for a joystick's round panel cutout. The boot is 25mm but the lever needs clearance for its deflection angle — actual cutout may need to be 26-27mm.
Avoid: check datasheet for *panel cutout* dimension, not just part OD. Those are different numbers.

## 10. Overlapping solids that preview hides

Failure: two solids share a volume (e.g. pot body sitting *inside* the base plate instead of under it). Renders look plausible because one solid hides the overlap; CGAL unions them so the bounding box is smaller than the sum of parts.
Avoid: cross-check `analyze_model` bounding-box height against the sum of your nominal Z heights. If they disagree by more than ~0.1mm (polygon faceting), there's an overlap or gap somewhere.

## 11. Library path forgotten between sessions

Failure: next session, `use <BOSL2/std.scad>` fails because libs were cloned into a non-default dir or removed.
Avoid: libraries live at `~/.local/share/OpenSCAD/libraries/`. If `openscad --info` doesn't list your library path, something's off. Don't bundle BOSL2/NopSCADlib into the repo — reference them.
