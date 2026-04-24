// JH-D400X-R4 — generic 4-potentiometer thumb joystick (Chinese clone, ~15€).
//
// IMPORTANT: these dimensions are based on typical listings of this clone part.
// Verify against the physical unit with a caliper before printing a mating
// panel or enclosure. The part varies between vendors by ~1–2mm in every axis.
//
// Conventions:
//   - Origin: center of the mounting base plate, on its top face (the face
//     that contacts the enclosure panel from below).
//   - +Z points up along the lever shaft.
//   - The body extends into -Z (below the panel), the handle extends into +Z.
//   - Anchor "panel" is at the top of the base plate (the mating face).

include <BOSL2/std.scad>
include <parameters.scad>

// ----- JH-D400X-R4 nominal dimensions (verify) -----
JS_BASE          = [38, 38];     // base plate footprint (mm)
JS_BASE_THICK    = 2.0;          // base plate thickness
JS_MOUNT_PCD     = [30, 30];     // mounting hole pattern (corner-to-corner)
JS_MOUNT_HOLE_D  = 3.2;          // M3 clearance
JS_BOOT_D        = 24;           // round boot above base plate
JS_BOOT_H        = 9;            // boot height above base
JS_SHAFT_D       = 4;            // metal shaft between boot and knob
JS_SHAFT_L       = 22;           // shaft length from top of boot to bottom of knob
JS_KNOB_D        = 16;           // ball knob diameter
JS_KNOB_H        = 16;           // knob height (≈spherical)
JS_BODY_D        = 30;           // cylindrical pot body below base
JS_BODY_H        = 28;           // body depth below base plate
JS_PIN_BLOCK     = [20, 8, 6];   // rough pin-header block (x: along base, y: depth, z: below body)
JS_MAX_TILT_DEG  = 25;           // lever deflection; panel cutout must clear this

// Derived
JS_ABOVE_PANEL   = JS_BASE_THICK + JS_BOOT_H + JS_SHAFT_L + JS_KNOB_H;
JS_BELOW_PANEL   = JS_BODY_H + JS_PIN_BLOCK.z;
JS_PANEL_CUTOUT_D = JS_BOOT_D + 2;  // clearance around boot for lever travel

// -----------------------------------------------------------------------------
// Main module: the joystick as-it-exists (for visualization + collision check).
// Not printed — this represents the physical vitamin.
// anchor="panel" puts the top of the base plate at the origin.
// -----------------------------------------------------------------------------
module jh_d400x_r4(anchor=CENTER, spin=0, orient=UP) {
    // Attachable outer envelope: base footprint × (below + above panel)
    size = [JS_BASE.x, JS_BASE.y, JS_BELOW_PANEL + JS_BASE_THICK + JS_BOOT_H + JS_SHAFT_L + JS_KNOB_H];
    anchors = [
        named_anchor("panel",       [0, 0, -size.z/2 + JS_BELOW_PANEL + JS_BASE_THICK], UP),
        named_anchor("panel_below", [0, 0, -size.z/2 + JS_BELOW_PANEL],                 UP),
        named_anchor("mount_fl",    [-JS_MOUNT_PCD.x/2, -JS_MOUNT_PCD.y/2,
                                      -size.z/2 + JS_BELOW_PANEL + JS_BASE_THICK], UP),
        named_anchor("mount_fr",    [ JS_MOUNT_PCD.x/2, -JS_MOUNT_PCD.y/2,
                                      -size.z/2 + JS_BELOW_PANEL + JS_BASE_THICK], UP),
        named_anchor("mount_bl",    [-JS_MOUNT_PCD.x/2,  JS_MOUNT_PCD.y/2,
                                      -size.z/2 + JS_BELOW_PANEL + JS_BASE_THICK], UP),
        named_anchor("mount_br",    [ JS_MOUNT_PCD.x/2,  JS_MOUNT_PCD.y/2,
                                      -size.z/2 + JS_BELOW_PANEL + JS_BASE_THICK], UP),
    ];

    attachable(anchor, spin, orient, size=size, anchors=anchors) {
        // Shift so base-plate top is at local origin, then build up/down.
        down(size.z/2 - JS_BELOW_PANEL - JS_BASE_THICK)
            _jh_d400x_r4_geometry();
        children();
    }
}

module _jh_d400x_r4_geometry() {
    // Pot body (below panel, attached to underside of base plate)
    color("#303030")
        down(JS_BASE_THICK + JS_BODY_H)
            cyl(d=JS_BODY_D, l=JS_BODY_H, anchor=BOTTOM);

    // Pin header block (below body)
    color("#c0c060")
        down(JS_BASE_THICK + JS_BODY_H + JS_PIN_BLOCK.z)
            cuboid(JS_PIN_BLOCK, anchor=BOTTOM);

    // Base plate (panel mating face on top)
    color("#202020")
        cuboid([JS_BASE.x, JS_BASE.y, JS_BASE_THICK], anchor=TOP) {
            // Mounting holes rendered as darker pits for visualization
            for (sx = [-1, 1], sy = [-1, 1])
                attach(BOTTOM)
                    translate([sx*JS_MOUNT_PCD.x/2, sy*JS_MOUNT_PCD.y/2, 0])
                        color("#808080") cyl(d=JS_MOUNT_HOLE_D, l=JS_BASE_THICK + 0.2, anchor=CENTER);
        }

    // Boot (rubber shroud above base)
    color("#404040")
        up(JS_BOOT_H/2)
            cyl(d1=JS_BOOT_D, d2=JS_BOOT_D*0.7, l=JS_BOOT_H, anchor=CENTER);

    // Metal shaft
    color("#c0c0c0")
        up(JS_BOOT_H + JS_SHAFT_L/2)
            cyl(d=JS_SHAFT_D, l=JS_SHAFT_L, anchor=CENTER);

    // Ball knob
    color("#101010")
        up(JS_BOOT_H + JS_SHAFT_L + JS_KNOB_H/2)
            sphere(d=JS_KNOB_D);
}

// -----------------------------------------------------------------------------
// Panel cutout helper: difference this from your enclosure panel.
// Positions: center of the joystick's boot opening, at panel top face.
// Produces: round boot hole + 4 M3 clearance holes at the mount pattern.
// Pass depth = panel thickness + small overshoot (default works for 2mm panel).
// -----------------------------------------------------------------------------
module jh_d400x_r4_panel_cutout(panel_thick=2, bolt_clear=true) {
    // Round hole for boot + lever clearance
    translate([0, 0, -0.1])
        cyl(d=JS_PANEL_CUTOUT_D, l=panel_thick + 0.2, anchor=BOTTOM);

    // Bolt-pattern clearance holes
    if (bolt_clear)
        for (sx = [-1, 1], sy = [-1, 1])
            translate([sx*JS_MOUNT_PCD.x/2, sy*JS_MOUNT_PCD.y/2, -0.1])
                cyl(d=JS_MOUNT_HOLE_D + 2*FIT_FREE, l=panel_thick + 0.2, anchor=BOTTOM);
}

// -----------------------------------------------------------------------------
// Standalone preview when this file is opened directly.
// -----------------------------------------------------------------------------
jh_d400x_r4();

// Visualize a mock panel with the cutout applied:
%difference() {
    up(-0.1) cuboid([JS_BASE.x + 20, JS_BASE.y + 20, 2], anchor=BOTTOM);
    up(-0.1) jh_d400x_r4_panel_cutout(panel_thick=2);
}
