# BOSL2 + NopSCADlib Cheatsheet

Only what you'll actually use. Full docs at `~/.local/share/OpenSCAD/libraries/BOSL2/` and NopSCADlib's inline docs.

## BOSL2

Import: `include <BOSL2/std.scad>` — not `use`. BOSL2's direction constants (`TOP`, `BOTTOM`, `LEFT`, etc.) are defined as values, which `use` does not import. For gears/threads/etc., the specific modules can be `use`d (`use <BOSL2/threading.scad>`).

### Anchor constants (directions)

`TOP BOTTOM LEFT RIGHT FRONT BACK CENTER` — combinable: `TOP+LEFT+FRONT` = top-left-front corner. Internal: `UP DOWN`. Also `FWD=FRONT`, `BOT=BOTTOM`.

### Shapes with built-in anchors

Use these **instead of** `cube()`, `cylinder()`, `sphere()`:

- `cuboid([w,d,h], anchor=..., rounding=..., chamfer=..., edges=...)` — box with rounded/chamfered edges and anchors
- `cyl(l=..., d=..., anchor=..., rounding1=..., chamfer2=...)` — cylinder with per-end chamfer/round
- `sphere(d=..., anchor=...)`
- `prismoid([w1,d1], [w2,d2], h=...)` — tapered box
- `rect_tube(size=..., wall=..., h=...)` — hollow rectangular tube
- `tube(od=..., id=..., h=...)` — hollow cylinder

### Attachment — THE reason to use BOSL2

```openscad
cuboid([40,40,10]) {
    // child attached to top face of parent
    attach(TOP) cyl(d=8, l=15);
    // child positioned at a corner, oriented along parent's face
    attach(TOP+RIGHT, BOTTOM) cuboid([5,10,20]);
}
```

`attach(parent_anchor, child_anchor=BOTTOM, overlap=0)` — child's child_anchor is placed at parent's parent_anchor. No manual translate+rotate. Eliminates 90% of positioning bugs.

`position(anchor)` — places child at anchor without reorienting. Use when you want child upright but moved to e.g. `TOP+RIGHT` corner.

### Distribution (arrays)

- `xcopies(spacing=..., n=...)` — distribute children along X
- `ycopies(...)`, `zcopies(...)`
- `grid_copies(n=[nx,ny], spacing=[sx,sy])` — 2D grid
- `mirror_copy(v=[1,0,0])` — keep original + mirror

### Transforms (when anchors don't fit)

- `up(z)`, `down(z)`, `left(x)`, `right(x)`, `fwd(y)`, `back(y)` — readable single-axis translate
- `zrot(a)`, `xrot(a)`, `yrot(a)` — single-axis rotate
- `xflip()`, `yflip()`, `zflip()` — mirror

### Hole helpers

- `screw_hole(spec="M3", length=10, thread=false)` — clearance hole for M-spec screw
- `diff() cuboid(...) { tag("remove") attach(TOP) cyl(...); }` — tagged difference for cleaner subtraction

### Defining your own anchored parts

```openscad
module my_bracket(anchor=CENTER, spin=0, orient=UP) {
    size = [40, 20, 10];
    anchors = [
        named_anchor("mount_a", [15, 0, 5], UP),
        named_anchor("mount_b", [-15, 0, 5], UP),
    ];
    attachable(anchor, spin, orient, size=size, anchors=anchors) {
        cuboid(size);
        children();
    }
}
// usage
my_bracket() attach("mount_a") cyl(d=3, l=8);
```

## NopSCADlib

Import the specific file you need, not everything. Examples:

```openscad
include <NopSCADlib/core.scad>;
use <NopSCADlib/vitamins/screw.scad>;
use <NopSCADlib/vitamins/insert.scad>;
use <NopSCADlib/vitamins/pcb.scad>;
```

### Fasteners

- `screw(type, length)` — `type` is e.g. `M3_cap_screw`, `M3_pan_screw`, `M3_cs_cap_screw` (countersunk), `M3_hex_screw`
- `screw_hole_radius(type)` — clearance radius
- `insert(type)` — heat-set insert, e.g. `M3_insert`. Use `insert_hole_radius(type)` for the pocket
- `nut(type)` — e.g. `M3_nut`

### Ready-made PCBs

`vitamins/pcbs.scad` lists many. `pcb(<type>)` renders the PCB. Common ones include ESP32 variants, Pi boards, Arduino boards. Search the file for what you need before modeling by hand.

### Displays

`vitamins/displays.scad`, `vitamins/oleds.scad` — SSD1306 OLEDs, common TFTs.

## Rule of thumb

Before modeling a standard part (screw, insert, common board, display, connector, fan, PSU): `grep` NopSCADlib first. Don't reinvent.
