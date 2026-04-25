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

// 4 walls forming a battery slot between the corner bosses. The +Y wall
// (toward the main PCB) has a notch for the LiPo power + balance wires.
// Drawn relative to BATT_POS, sitting on the back-panel-interior.
module batt_slot_walls() {
    // Inner-face offsets: half the battery body + a small clearance.
    inner_x = BATT_BODY.x/2 + BATT_WALL_INSET;   // 41
    inner_y = BATT_BODY.y/2 + BATT_WALL_INSET;   // 18.5
    // Wall span along their long dimension reaches between the bosses.
    span_x = 2 * BATT_BOSS_OFFSET_X;             // 90
    span_y = 2 * BATT_BOSS_OFFSET_Y;             // 44

    difference() {
        union() {
            // Two X-walls (left and right of battery, run along Y).
            for (sx = [-1, 1])
                translate([sx*(inner_x + BATT_WALL_T/2), 0, 0])
                    cuboid([BATT_WALL_T, span_y, BATT_BOSS_HEIGHT],
                           anchor=BOTTOM);
            // Two Y-walls (top and bottom of battery, run along X).
            for (sy = [-1, 1])
                translate([0, sy*(inner_y + BATT_WALL_T/2), 0])
                    cuboid([span_x, BATT_WALL_T, BATT_BOSS_HEIGHT],
                           anchor=BOTTOM);
        }
        // Wire-exit notch in the +Y wall (top of wall, leaves bottom intact
        // so the LiPo body still sits captive).
        translate([0, inner_y + BATT_WALL_T/2,
                   BATT_BOSS_HEIGHT - BATT_WIRE_NOTCH_H])
            cuboid([BATT_WIRE_NOTCH_W,
                    BATT_WALL_T + 0.4,
                    BATT_WIRE_NOTCH_H + 0.1],
                   anchor=BOTTOM);
    }
}

// Top↔bottom mounting boss in the bottom shell. Sits on the back-panel
// interior and reaches up to the mating face. Hollow tube — clearance for
// the M3 screw all the way through to the back panel exterior.
module case_screw_boss_bottom() {
    difference() {
        cyl(d=BOSS_OD, l=CASE_BOSS_HEIGHT_BOT, anchor=BOTTOM);
        translate([0, 0, -0.1])
            cyl(d=COVER_SCREW_CLEAR,
                l=CASE_BOSS_HEIGHT_BOT + 0.2,
                anchor=BOTTOM);
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

            // Slot walls between the bosses with a wire-exit notch.
            translate([BATT_POS_X, BATT_POS_Y, -BOTTOM_DEPTH + PANEL_T])
                batt_slot_walls();

            // 4 case-corner bosses with M3 screw clearance for fastening
            // the bottom shell to the top shell.
            for (sx = [-1, 1], sy = [-1, 1])
                translate([sx*CASE_BOSS_OFFSET_X,
                           sy*CASE_BOSS_OFFSET_Y,
                           -BOTTOM_DEPTH + PANEL_T])
                    case_screw_boss_bottom();
        }

        // ----- Subtractions on the back panel -----

        // Cover recess (exterior side of back panel). Recess is enlarged by
        // BATT_COVER_FIT (XY) and BATT_COVER_Z_GAP (Z) so the cover slides in
        // with print tolerance and a small visible gap for inspection.
        translate([BATT_POS_X, BATT_POS_Y, -BOTTOM_DEPTH - 0.1])
            cuboid([BATT_COVER_W + 2*BATT_COVER_FIT,
                    BATT_COVER_H + 2*BATT_COVER_FIT,
                    BATT_COVER_RECESS + BATT_COVER_Z_GAP + 0.1],
                   rounding=BATT_COVER_R + BATT_COVER_FIT, edges="Z",
                   anchor=BOTTOM);

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

        // 4 case-corner screw clearance holes through the back panel
        // (continuation of the case_screw_boss_bottom through-tubes).
        for (sx = [-1, 1], sy = [-1, 1])
            translate([sx*CASE_BOSS_OFFSET_X,
                       sy*CASE_BOSS_OFFSET_Y,
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
