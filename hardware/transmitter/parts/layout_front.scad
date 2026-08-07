// Front-panel layout: all vitamins placed at their panel positions.
//
// This is the design-brief for the case shell. Positions are sketch-quality —
// tune the constants below when proportions feel off, then iterate by
// re-rendering. The case-shell module will read these same constants.
//
// Conventions:
//   - Panel occupies z ∈ [-PANEL_T, 0]; z=0 is the user-facing surface.
//   - Every vitamin is placed with anchor="panel" at (x, y, 0):
//     body extends into -Z (inside the case), user-side features into +Z.
//   - X = horizontal, +X right. Y = vertical, +Y up. Origin = panel center.

include <BOSL2/std.scad>
include <parameters.scad>
include <utils.scad>

use <joystick.scad>
use <toggle_switch.scad>
use <encoder.scad>
use <button_illuminated.scad>
use <button_tact.scad>
use <display.scad>
use <pcb_subpanel.scad>
use <pcb_main.scad>

// =============================================================================
// Panel geometry
// =============================================================================
PANEL_W = 280;
PANEL_H = 130;
PANEL_T = 2;

// =============================================================================
// Vitamin positions (x, y) on the panel face
// =============================================================================

// --- Display (centered horizontally, slightly above vertical center) ---
DISPLAY_POS = [0, 12];

// --- Joysticks (flanking the display, aligned with its vertical center) ---
JOYSTICK_X  = 85;
JOYSTICK_Y  = DISPLAY_POS.y;
JOYSTICK_LEFT_POS  = [-JOYSTICK_X, JOYSTICK_Y];
JOYSTICK_RIGHT_POS = [ JOYSTICK_X, JOYSTICK_Y];

// --- Toggle switches: pushed to the top corners so the index finger reaches
//     them without crossing over the joystick. Cluster is OUTSIDE the joystick.
TOGGLE_Y           = 45;
TOGGLE_SPACING_X   = 18;
TOGGLE_CLUSTER_OFF = 102;    // just outside joystick edge (joystick edge at 104)
TOGGLE_LEFT_X  = [ -TOGGLE_CLUSTER_OFF - TOGGLE_SPACING_X,
                   -TOGGLE_CLUSTER_OFF,
                   -TOGGLE_CLUSTER_OFF + TOGGLE_SPACING_X ];
TOGGLE_RIGHT_X = [  TOGGLE_CLUSTER_OFF - TOGGLE_SPACING_X,
                    TOGGLE_CLUSTER_OFF,
                    TOGGLE_CLUSTER_OFF + TOGGLE_SPACING_X ];

// --- Illuminated buttons: 3 each side, centered horizontally under each
//     joystick (visually balanced under the stick rather than along the edge).
ILLUM_BTN_Y        = -32;
ILLUM_BTN_SPACING  = 18;
ILLUM_BTN_CLUSTER  = JOYSTICK_X;   // each 3-row centered under its joystick
ILLUM_BTN_LEFT_X   = [ -ILLUM_BTN_CLUSTER - ILLUM_BTN_SPACING,
                       -ILLUM_BTN_CLUSTER,
                       -ILLUM_BTN_CLUSTER + ILLUM_BTN_SPACING ];
ILLUM_BTN_RIGHT_X  = [  ILLUM_BTN_CLUSTER - ILLUM_BTN_SPACING,
                        ILLUM_BTN_CLUSTER,
                        ILLUM_BTN_CLUSTER + ILLUM_BTN_SPACING ];

// --- Rotary encoders: pushed outward to make room for 3 horizontal nav
//     tacts in the center.
ENCODER_X = 30;
ENCODER_Y = -42;
ENCODER_LEFT_POS  = [-ENCODER_X, ENCODER_Y];
ENCODER_RIGHT_POS = [ ENCODER_X, ENCODER_Y];

// --- Tact nav buttons: 3 in a horizontal row between the encoders.
//     Mapping: LEFT = up/left, MID = enter/back/cancel, RIGHT = down/right.
NAV_BTN_Y          = ENCODER_Y;
NAV_BTN_SPACING    = 10;
NAV_BTN_LEFT_POS   = [-NAV_BTN_SPACING, NAV_BTN_Y];
NAV_BTN_MID_POS    = [ 0,               NAV_BTN_Y];
NAV_BTN_RIGHT_POS  = [ NAV_BTN_SPACING, NAV_BTN_Y];

// =============================================================================
// PCB depths (Z behind panel face). Each sub-PCB sits at its dominant
// component's body-bottom level. Main PCB sits behind the display so the
// display module's pin header plugs directly into it.
// =============================================================================
SUBPCB_Z_TOGGLE  = -13;     // toggle body bottom
SUBPCB_Z_ILLUM   = -10;     // illuminated-button body bottom
SUBPCB_Z_ENCODER = -6.5;    // encoder body bottom
SUBPCB_Z_NAV     = -3.5;    // tact body bottom (nav PCB sits in front of encoder PCB)
MAIN_PCB_Z       = -19.15;  // main PCB center, header top meets display pin tips at z=-14.1

// =============================================================================
// Layout module
// =============================================================================
module transmitter_layout_front(show_mock_panel=true) {
    // Mock panel — translucent so we can see vitamins penetrating it.
    // Suppress when the real case shell is also being rendered.
    if (show_mock_panel)
        %color(COLOR_GHOST)
            down(PANEL_T)
                cuboid([PANEL_W, PANEL_H, PANEL_T], anchor=BOTTOM);

    // Display
    translate([DISPLAY_POS.x, DISPLAY_POS.y, 0])
        display_4in_ips(anchor="panel");

    // Joysticks
    translate([JOYSTICK_LEFT_POS.x,  JOYSTICK_LEFT_POS.y,  0])
        jh_d400x_r4(anchor="panel");
    translate([JOYSTICK_RIGHT_POS.x, JOYSTICK_RIGHT_POS.y, 0])
        jh_d400x_r4(anchor="panel");

    // Toggle switches (6)
    for (x = TOGGLE_LEFT_X)
        translate([x, TOGGLE_Y, 0]) toggle_longbat(anchor="panel");
    for (x = TOGGLE_RIGHT_X)
        translate([x, TOGGLE_Y, 0]) toggle_longbat(anchor="panel");

    // Illuminated buttons (6)
    for (x = ILLUM_BTN_LEFT_X)
        translate([x, ILLUM_BTN_Y, 0]) button_illuminated(anchor="panel");
    for (x = ILLUM_BTN_RIGHT_X)
        translate([x, ILLUM_BTN_Y, 0]) button_illuminated(anchor="panel");

    // Rotary encoders
    translate([ENCODER_LEFT_POS.x,  ENCODER_LEFT_POS.y,  0])
        ec11_encoder(anchor="panel");
    translate([ENCODER_RIGHT_POS.x, ENCODER_RIGHT_POS.y, 0])
        ec11_encoder(anchor="panel");

    // Tact nav buttons (3, horizontal row)
    translate([NAV_BTN_LEFT_POS.x,  NAV_BTN_LEFT_POS.y,  0])
        button_tact(anchor="panel");
    translate([NAV_BTN_MID_POS.x,   NAV_BTN_MID_POS.y,   0])
        button_tact(anchor="panel");
    translate([NAV_BTN_RIGHT_POS.x, NAV_BTN_RIGHT_POS.y, 0])
        button_tact(anchor="panel");

    // ----- Sub-panel PCBs at their respective depths -----
    if (SHOW_PCBS) {
        // 2x toggle PCBs (left/right, centered on each toggle cluster)
        translate([-TOGGLE_CLUSTER_OFF, TOGGLE_Y, SUBPCB_Z_TOGGLE])
            subpanel_pcb_toggle3();
        translate([ TOGGLE_CLUSTER_OFF, TOGGLE_Y, SUBPCB_Z_TOGGLE])
            subpanel_pcb_toggle3();

        // 2x illuminated PCBs (left/right, centered on each button cluster)
        translate([-ILLUM_BTN_CLUSTER, ILLUM_BTN_Y, SUBPCB_Z_ILLUM])
            subpanel_pcb_illum3();
        translate([ ILLUM_BTN_CLUSTER, ILLUM_BTN_Y, SUBPCB_Z_ILLUM])
            subpanel_pcb_illum3();

        // 2x encoder PCBs (one per encoder) + 1x nav PCB. Splitting the
        // encoders keeps the central nav-PCB area clear in XY.
        translate([ENCODER_LEFT_POS.x,  ENCODER_LEFT_POS.y,  SUBPCB_Z_ENCODER])
            subpanel_pcb_encoder1();
        translate([ENCODER_RIGHT_POS.x, ENCODER_RIGHT_POS.y, SUBPCB_Z_ENCODER])
            subpanel_pcb_encoder1();
        // The actual KiCad layout places the switch row 5mm "north" of the PCB
        // centroid (PCB-local Y=-5 in KiCad screen coords, which becomes +5
        // after the STEP export negates Y). To align the switch caps with the
        // panel cutouts at NAV_BTN_Y, place the PCB centroid 5mm "south".
        translate([0, NAV_BTN_Y - 5, SUBPCB_Z_NAV])
            subpanel_pcb_nav3();

        // Main PCB behind the display (display plugs directly into its header)
        translate([DISPLAY_POS.x, DISPLAY_POS.y, MAIN_PCB_Z])
            pcb_main();
    }
}

// =============================================================================
// Standalone preview
// =============================================================================
SHOW_STANDALONE = true;
SHOW_PCBS       = true;   // include sub-panel + main PCBs in preview

if (SHOW_STANDALONE) {
    transmitter_layout_front();
}
