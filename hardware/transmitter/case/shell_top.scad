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

// =============================================================================
module shell_top() {
    // NOTE: We tried rounded_prism for the outer (would give Top-Edge-Rundung
    // combined with the taper) but it interacts badly with the inner-cavity
    // boolean — the panel slab gets eaten. For now we use prismoid (taper +
    // rounded vertical corners + sharp top-face edges) which renders cleanly.
    // The rounded top edge can come back via offset_sweep / minkowski later.

    difference() {
        // Outer shell — tapered prismoid, front face wider than back face.
        translate([0, 0, PANEL_T])
            prismoid(
                size1=[CASE_W - 2*TAPER_X, CASE_H - 2*TAPER_Y],
                size2=[CASE_W,              CASE_H            ],
                h=TOP_DEPTH,
                rounding=CORNER_R,
                anchor=TOP
            );

        // Inner cavity — same taper, smaller by wall thickness. Anchor TOP
        // so the inner top face sits right at the panel inner face (z=0.1),
        // leaving a PANEL_T panel slab.
        translate([0, 0, 0.1])
            prismoid(
                size1=[CASE_W - 2*TAPER_X - 2*WALL_T,
                       CASE_H - 2*TAPER_Y - 2*WALL_T],
                size2=[CASE_W - 2*WALL_T, CASE_H - 2*WALL_T],
                h=TOP_DEPTH - PANEL_T + 0.2,
                rounding=max(CORNER_R - WALL_T, 0.5),
                anchor=TOP
            );

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
