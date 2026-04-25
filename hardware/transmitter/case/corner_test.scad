// Corner test piece — small chunk of the top shell for printing to verify
// the corner geometry (taper + vertical-corner round + top-edge round).
// Extracts the +X+Y corner over a CORNER_TEST_SIZE square, oriented for
// FDM printing with the back-rim flat on the bed and the panel face up.
//
// Render with F6 (CGAL) for an accurate view of the rounded top edge.

include <BOSL2/std.scad>
include <parameters.scad>

CORNER_TEST_SIZE = 55;   // edge length of the cube cutting out the corner

module corner_shell() {
    // Same outer + inner-cavity construction as shell_top, but no panel cutouts
    // (cutouts are validated separately; this test piece is just the shape).
    difference() {
        diff()
        translate([0, 0, PANEL_T])
            prismoid(
                size1=[CASE_W - 2*TAPER_X, CASE_H - 2*TAPER_Y],
                size2=[CASE_W,              CASE_H            ],
                h=TOP_DEPTH,
                rounding=CORNER_R,
                anchor=TOP
            )
            {
                edge_profile([TOP])
                    mask2d_roundover(r=TOP_EDGE_R);
            };

        translate([0, 0, 0.1])
            prismoid(
                size1=[CASE_W - 2*TAPER_X - 2*WALL_T,
                       CASE_H - 2*TAPER_Y - 2*WALL_T],
                size2=[CASE_W - 2*WALL_T, CASE_H - 2*WALL_T],
                h=TOP_DEPTH - PANEL_T + 0.2,
                rounding=max(CORNER_R - WALL_T, 0.5),
                anchor=TOP
            );
    }
}

// Print orientation: panel face DOWN on the bed (largest flat side for
// best adhesion and zero supports). Mirror Z then translate so the
// panel face sits at z=0.
translate([0, 0, PANEL_T])
    mirror([0, 0, 1])
        intersection() {
            corner_shell();
            translate([CASE_W/2 - CORNER_TEST_SIZE,
                       CASE_H/2 - CORNER_TEST_SIZE,
                       -TOP_DEPTH + PANEL_T - 0.5])
                cuboid([CORNER_TEST_SIZE + 1,
                        CORNER_TEST_SIZE + 1,
                        TOP_DEPTH + 1],
                       anchor=BOTTOM+LEFT+FRONT);
        }
