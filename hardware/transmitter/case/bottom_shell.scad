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

// Bottom-shell tapers INWARD going from mating face to back panel —
// matching the top-shell which now also tapers inward going from mating
// to front panel. The mating rim is the WIDEST point of the assembled
// case; both ends taper down to a smaller panel face. This gives the
// "barrel" / belly silhouette where the middle bulges outward.
BOT_TAPER_X = TAPER_X;
BOT_TAPER_Y = TAPER_Y;

// Mating-face dimensions (must equal top-shell's back-rim dimensions = CASE_W/H).
BOT_FRONT_W = CASE_W;
BOT_FRONT_H = CASE_H;

// Back-panel dimensions (smaller — taper inward).
BOT_BACK_W = BOT_FRONT_W - 2*BOT_TAPER_X;
BOT_BACK_H = BOT_FRONT_H - 2*BOT_TAPER_Y;

// Corner boss with two M3 insert pockets (bottom for cover screw,
// top for battery-lid screw). Drawn relative to its bottom face.
module batt_corner_boss() {
    difference() {
        cyl(d=BATT_BOSS_OD, l=BATT_BOSS_HEIGHT, anchor=BOTTOM);
        // Bottom insert pocket (opens DOWN for cover screw from outside)
        translate([0, 0, -0.1])
            cyl(d=INSERT_M3_POCKET_D, l=INSERT_M3_POCKET_H + 0.1, anchor=BOTTOM);
        // Top insert pocket (opens UP for lid screw from case interior)
        translate([0, 0, BATT_BOSS_HEIGHT + 0.1])
            cyl(d=INSERT_M3_POCKET_D, l=INSERT_M3_POCKET_H + 0.1, anchor=TOP);
    }
}

// =============================================================================
module bottom_shell() {
    difference() {
        union() {
            // Hollow shell: outer tapered prismoid minus inner cavity.
            difference() {
                // Outer with rounded back panel
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

                // Inner cavity
                translate([0, 0, -BOTTOM_DEPTH + PANEL_T])
                    prismoid(
                        size1=[BOT_BACK_W  - 2*WALL_T, BOT_BACK_H  - 2*WALL_T],
                        size2=[BOT_FRONT_W - 2*WALL_T, BOT_FRONT_H - 2*WALL_T],
                        h=BOTTOM_DEPTH - PANEL_T + 0.2,
                        rounding=max(CORNER_R - WALL_T, 0.5),
                        anchor=BOTTOM
                    );
            }

            // 4 corner bosses on the back-panel-interior face.
            for (sx = [-1, 1], sy = [-1, 1])
                translate([BATT_POS_X + sx*BATT_BOSS_OFFSET_X,
                           BATT_POS_Y + sy*BATT_BOSS_OFFSET_Y,
                           -BOTTOM_DEPTH + PANEL_T])
                    batt_corner_boss();
        }

        // ----- Subtractions on the back panel -----

        // Cover recess (exterior side of back panel; cover sits flush).
        translate([BATT_POS_X, BATT_POS_Y, -BOTTOM_DEPTH - 0.1])
            cuboid([BATT_COVER_W, BATT_COVER_H, BATT_COVER_RECESS + 0.1],
                   rounding=BATT_COVER_R, edges="Z", anchor=BOTTOM);

        // Through-hole for battery insertion.
        translate([BATT_POS_X, BATT_POS_Y, -BOTTOM_DEPTH - 0.1])
            cuboid([BATT_OPENING_W, BATT_OPENING_H, PANEL_T + 0.2],
                   rounding=BATT_COVER_R, edges="Z", anchor=BOTTOM);

        // 4 cover-screw clearance holes through the back panel.
        for (sx = [-1, 1], sy = [-1, 1])
            translate([BATT_POS_X + sx*BATT_BOSS_OFFSET_X,
                       BATT_POS_Y + sy*BATT_BOSS_OFFSET_Y,
                       -BOTTOM_DEPTH - 0.1])
                cyl(d=COVER_SCREW_CLEAR, l=PANEL_T + 0.2, anchor=BOTTOM);
    }
}

// =============================================================================
// Standalone preview
// =============================================================================
SHOW_STANDALONE = true;

if (SHOW_STANDALONE) {
    color(COLOR_PRINTED) bottom_shell();
}
