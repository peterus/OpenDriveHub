// Bottom shell — back half of the transmitter case.
//
// Pendant to shell_top.scad: same construction (tapered prismoid with
// rounded vertical corners + roundover on the outer panel edges), but
// inverted so it mates with the top shell's back rim.
//
// First iteration is just the basic shape. Battery compartment, USB-C
// cutout, and speaker grille come in later passes.
//
// Local coordinates:
//   z = 0                       mating face (joins top-shell back rim at world z=PANEL_T-TOP_DEPTH)
//   z = -BOTTOM_DEPTH            back panel exterior
//   z = -BOTTOM_DEPTH + PANEL_T  back panel interior
//
// Render with F6 (CGAL) — uses BOSL2 diff() / edge_profile (see SKILL.md).

include <BOSL2/std.scad>
include <parameters.scad>

// 2D rectangular path centred at origin (same helper as shell_top).
function rect_path(w, d) = [
    [-w/2, -d/2],
    [ w/2, -d/2],
    [ w/2,  d/2],
    [-w/2,  d/2]
];

// Bottom-shell taper goes INVERSE to the top-shell's: the back panel is
// BIGGER than the mating face, so the assembled case bulges out at both
// ends and is narrowest at the middle (mating rim) — gives a "barrel"
// silhouette like the reference photo, instead of a continuous wedge.
BOT_TAPER_X = TAPER_X;
BOT_TAPER_Y = TAPER_Y;

// Mating-face dimensions (must equal top-shell's back-rim dimensions).
BOT_FRONT_W = CASE_W - 2*TAPER_X;
BOT_FRONT_H = CASE_H - 2*TAPER_Y;

// Back-panel dimensions (bigger than mating — inverse taper).
BOT_BACK_W = BOT_FRONT_W + 2*BOT_TAPER_X;
BOT_BACK_H = BOT_FRONT_H + 2*BOT_TAPER_Y;

// =============================================================================
module bottom_shell() {
    difference() {
        // Outer: tapered prismoid; back panel (smaller) at z=-BOTTOM_DEPTH,
        // mating face (bigger) at z=0. The four edges where the back panel
        // meets the side walls get a roundover via diff() / edge_profile.
        diff()
        translate([0, 0, -BOTTOM_DEPTH])
            prismoid(
                size1=[BOT_BACK_W,  BOT_BACK_H ],
                size2=[BOT_FRONT_W, BOT_FRONT_H],
                h=BOTTOM_DEPTH,
                rounding=CORNER_R,
                anchor=BOTTOM
            )
            {
                edge_profile([BOT])
                    mask2d_roundover(r=TOP_EDGE_R);
            };

        // Inner cavity — open at the mating face (z=0), closed at the back
        // panel (z=-BOTTOM_DEPTH+PANEL_T). Same taper as outer, smaller by
        // wall thickness on every side.
        translate([0, 0, -BOTTOM_DEPTH + PANEL_T])
            prismoid(
                size1=[BOT_BACK_W  - 2*WALL_T, BOT_BACK_H  - 2*WALL_T],
                size2=[BOT_FRONT_W - 2*WALL_T, BOT_FRONT_H - 2*WALL_T],
                h=BOTTOM_DEPTH - PANEL_T + 0.2,
                rounding=max(CORNER_R - WALL_T, 0.5),
                anchor=BOTTOM
            );
    }
}

// =============================================================================
// Standalone preview
// =============================================================================
SHOW_STANDALONE = true;

if (SHOW_STANDALONE) {
    color(COLOR_PRINTED) bottom_shell();
}
