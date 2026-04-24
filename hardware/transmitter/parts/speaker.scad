// Small mylar speaker, ~28mm diameter, 8Ω 0.5-1W.
//
// Typical DIY-grade speaker for small enclosures: thin flange rim, paper/plastic
// cone, rear magnet, two solder tabs. Sized to match the common Ø28mm × 5mm
// "mini speaker" widely sold for Arduino/ESP32 projects.
//
// Conventions:
//   - Origin: center of the front face (sound-emitting side).
//   - +Z = sound exits toward user.
//   - Anchor "front_rim" on the front flange (panel side).
//   - Anchor "back" at the magnet rear (case-interior side).

include <BOSL2/std.scad>
include <parameters.scad>
include <utils.scad>

// ----- Ø28mm mylar speaker nominal dimensions -----
SPK_OD        = 28;         // outer diameter of front flange
SPK_FLANGE_H  = 1.5;        // flange thickness
SPK_CONE_D    = 25;         // cone outer diameter (visible area)
SPK_CONE_H    = 3.0;        // cone depth below flange top
SPK_MAGNET_D  = 16;         // rear magnet diameter
SPK_MAGNET_H  = 5.0;        // rear magnet height (below flange back)
SPK_TOTAL_H   = SPK_FLANGE_H + SPK_MAGNET_H;  // overall depth
SPK_GRILLE_D  = 24;         // suggested case grille diameter (covers cone area)

module speaker_28mm(anchor=CENTER, spin=0, orient=UP) {
    size = [SPK_OD, SPK_OD, SPK_TOTAL_H];
    anchors = [
        named_anchor("front_rim", [0, 0,  size.z/2], UP),
        named_anchor("back",      [0, 0, -size.z/2], DOWN),
    ];
    attachable(anchor, spin, orient, size=size, anchors=anchors) {
        // Geometry is drawn from magnet (-SPK_MAGNET_H) up to flange-top (+SPK_FLANGE_H);
        // its center sits at (SPK_FLANGE_H - SPK_MAGNET_H)/2 in its own frame.
        // Shift by the negative of that to center on the attachable origin.
        up(-(SPK_FLANGE_H - SPK_MAGNET_H) / 2)
            _speaker_geometry();
        children();
    }
}

module _speaker_geometry() {
    // Front flange (rim, visible from panel side)
    color(COLOR_PLASTIC)
        difference() {
            cyl(d=SPK_OD, l=SPK_FLANGE_H, anchor=BOTTOM);
            // Cone recess (the paper cone sits inside)
            translate([0, 0, -0.01])
                cyl(d=SPK_CONE_D, l=SPK_FLANGE_H + 0.02, anchor=BOTTOM);
        }

    // Paper/mylar cone (shallow dome inside the flange)
    color("#d0b090")
        translate([0, 0, SPK_FLANGE_H - SPK_CONE_H])
            cyl(d1=SPK_MAGNET_D, d2=SPK_CONE_D, l=SPK_CONE_H, anchor=BOTTOM);

    // Rear magnet (steel cylinder)
    color(COLOR_METAL)
        down(SPK_MAGNET_H)
            cyl(d=SPK_MAGNET_D, l=SPK_MAGNET_H, anchor=BOTTOM);

    // Two solder tabs on the flange back edge (simplified)
    for (sx = [-1, 1])
        color(COLOR_BRASS)
            translate([sx * SPK_OD/2.5, 0, -0.5])
                cuboid([3, 2, 1], anchor=TOP);
}

// -----------------------------------------------------------------------------
// Grille-cutout helper: array of small sound holes in a circular pattern.
// Call where you want the grille in your case, with the +Z axis pointing
// through the case wall toward the user.
// -----------------------------------------------------------------------------
module speaker_28mm_grille(panel_thick=2, hole_d=1.5, pattern_d=SPK_GRILLE_D) {
    // Concentric rings of holes
    rings = [
        [0, 1],   // center: 1 hole
        [5, 6],
        [9, 10],
        [pattern_d/2 - 1, 14],
    ];
    for (ring = rings) {
        r = ring[0];
        n = ring[1];
        if (r == 0) {
            translate([0, 0, -0.1])
                cyl(d=hole_d, l=panel_thick + 0.2, anchor=BOTTOM);
        } else {
            for (i = [0 : n - 1])
                rotate([0, 0, 360/n * i])
                    translate([r, 0, -0.1])
                        cyl(d=hole_d, l=panel_thick + 0.2, anchor=BOTTOM);
        }
    }
}

// -----------------------------------------------------------------------------
// Standalone preview.
// -----------------------------------------------------------------------------
SHOW_STANDALONE = true;
SHOW_SECTION    = false;

if (SHOW_STANDALONE) {
    if (SHOW_SECTION) back_half(s=100) speaker_28mm();
    else speaker_28mm();
}
