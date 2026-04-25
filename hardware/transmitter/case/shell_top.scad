// Top shell — the front of the transmitter case.
//
// Hosts the front panel face with all component cutouts, plus the side
// walls extending back toward the case-mid plane. Hollow inside.
//
// Z convention (from case/parameters.scad):
//   z = 0       panel-mating (inner) face
//   z = PANEL_T panel outer face (user-facing)
//   z < 0       into the case interior

include <BOSL2/std.scad>
include <parameters.scad>

use <../parts/joystick.scad>
use <../parts/toggle_switch.scad>
use <../parts/encoder.scad>
use <../parts/button_illuminated.scad>
use <../parts/button_tact.scad>
use <../parts/display.scad>

// Rectangular 2D path centered at origin — input for rounded_prism.
function rect_path(w, d) = [
    [-w/2, -d/2],
    [ w/2, -d/2],
    [ w/2,  d/2],
    [-w/2,  d/2]
];

// Top↔bottom mounting boss with M3 heat-set insert at its base. Stands on
// the mating face and extends up into the top shell. The insert pocket
// opens DOWNWARD so the screw enters from the bottom shell side. Two
// stiffener fins point at +X and +Y in module-local coords; the placement
// loop rotates them to align with each case corner's walls.
module case_screw_boss_top() {
    h = CASE_BOSS_HEIGHT_TOP;
    union() {
        difference() {
            cyl(d=BOSS_OD, l=h, anchor=BOTTOM);
            // Insert pocket opens at the bottom face (mating side).
            translate([0, 0, -0.1])
                cyl(d=INSERT_M3_POCKET_D,
                    l=INSERT_M3_POCKET_H + 0.1,
                    anchor=BOTTOM);
        }
        for (rot = [0, 90])
            rotate([0, 0, rot])
                translate([BOSS_OD/2, 0, 0])
                    cuboid([CASE_BOSS_FIN_L, CASE_BOSS_FIN_T, h],
                           anchor=BOTTOM+LEFT);
    }
}

// =============================================================================
module shell_top() {
    // Outer = tapered prismoid (rounded vertical corners) with a roundover
    // applied to its 4 top-face edges via BOSL2 diff() + edge_profile().
    //
    // IMPORTANT: this uses tagged geometry. It renders correctly in F6
    // (CGAL render) and via openscad-mcp / openscad-cli. In F5 (OpenCSG
    // preview) the mask geometry shows as a translucent overlay rather
    // than being subtracted. Always use F6 to inspect the real result.

    difference() {
        union() {
            // Hollow outer shell — the cavity must be cut BEFORE the bosses
            // are unioned in, otherwise it would eat them.
            difference() {
                // Outer: prismoid where the BIG end is at the bottom (mating
                // rim) and the SMALL end is at the top (front panel). The
                // case bulges out at the middle (mating) and tapers inward
                // toward the panel face — gives the "barrel" silhouette.
                diff()
                translate([0, 0, PANEL_T])
                    prismoid(
                        size1=[CASE_W,              CASE_H            ],   // mating rim, BIG
                        size2=[CASE_W - 2*TAPER_X, CASE_H - 2*TAPER_Y],   // front panel, SMALL
                        h=TOP_DEPTH,
                        rounding=CORNER_R,
                        anchor=TOP
                    )
                    {
                        edge_profile([TOP])
                            mask2d_roundover(r=TOP_EDGE_R);
                    };

                // Inner cavity — same taper, smaller by wall thickness.
                translate([0, 0, 0.1])
                    prismoid(
                        size1=[CASE_W - 2*WALL_T, CASE_H - 2*WALL_T],
                        size2=[CASE_W - 2*TAPER_X - 2*WALL_T,
                               CASE_H - 2*TAPER_Y - 2*WALL_T],
                        h=TOP_DEPTH - PANEL_T + 0.2,
                        rounding=max(CORNER_R - WALL_T, 0.5),
                        anchor=TOP
                    );
            }

            // 4 corner bosses standing on the mating face. They merge into
            // the side wall near the panel face for stability. The rotation
            // aims the boss-internal +X/+Y fins toward the corresponding
            // case-corner walls.
            for (sx = [-1, 1], sy = [-1, 1])
                translate([sx*CASE_BOSS_OFFSET_X,
                           sy*CASE_BOSS_OFFSET_Y,
                           -(TOP_DEPTH - PANEL_T)])
                    rotate([0, 0, atan2(sy, sx) - 45])
                        case_screw_boss_top();
        }

        // ----- Panel cutouts -----

        // Display: window + 4 bolt holes
        translate([DISPLAY_POS.x, DISPLAY_POS.y, 0])
            display_4in_ips_panel_cutout(panel_thick=PANEL_T);

        // Joysticks: round boot opening + 4 M3 clearance per stick
        translate([-JOYSTICK_X, JOYSTICK_Y, 0])
            jh_d400x_r4_panel_cutout(panel_thick=PANEL_T);
        translate([ JOYSTICK_X, JOYSTICK_Y, 0])
            jh_d400x_r4_panel_cutout(panel_thick=PANEL_T);

        // Toggle switches (6)
        for (x = TOGGLE_LEFT_X)
            translate([x, TOGGLE_Y, 0])
                toggle_longbat_panel_cutout(panel_thick=PANEL_T);
        for (x = TOGGLE_RIGHT_X)
            translate([x, TOGGLE_Y, 0])
                toggle_longbat_panel_cutout(panel_thick=PANEL_T);

        // Illuminated buttons (6)
        for (x = ILLUM_BTN_LEFT_X)
            translate([x, ILLUM_BTN_Y, 0])
                button_illuminated_panel_cutout(panel_thick=PANEL_T);
        for (x = ILLUM_BTN_RIGHT_X)
            translate([x, ILLUM_BTN_Y, 0])
                button_illuminated_panel_cutout(panel_thick=PANEL_T);

        // Rotary encoders (2)
        translate([-ENCODER_X, ENCODER_Y, 0])
            ec11_encoder_panel_cutout(panel_thick=PANEL_T);
        translate([ ENCODER_X, ENCODER_Y, 0])
            ec11_encoder_panel_cutout(panel_thick=PANEL_T);

        // Nav tact buttons (3)
        for (x = [-NAV_BTN_SPACING, 0, NAV_BTN_SPACING])
            translate([x, NAV_BTN_Y, 0])
                button_tact_panel_cutout(panel_thick=PANEL_T);
    }
}

// =============================================================================
// Standalone preview
// =============================================================================
SHOW_STANDALONE = true;

if (SHOW_STANDALONE) {
    color(COLOR_PRINTED) shell_top();
}
