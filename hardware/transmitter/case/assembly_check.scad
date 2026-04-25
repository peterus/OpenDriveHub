// Assembly check: render top shell + all front-panel components + sub-PCBs
// together to visually verify every cutout aligns with its component.
//
// Use this view as a regression check when adjusting layout positions or
// case dimensions. If a component clips into the shell wall (no cutout
// where it should be), it's immediately visible.

include <BOSL2/std.scad>
include <parameters.scad>

use <top_shell.scad>
use <bottom_shell.scad>
use <battery_cover.scad>
use <battery_lid.scad>
use <../parts/battery.scad>
use <../parts/usb_c_breakout.scad>
use <../parts/layout_front.scad>

// SHELL_XRAY: when true, render the shell as a translucent background (lets
// you see PCBs and below-panel component bodies for cutout verification).
// When false, render solid — only above-panel features (lever balls,
// encoder shafts, illuminated caps, etc.) remain visible.
SHELL_XRAY = false;

// SECTION: cuts the assembly through the centre so the interior is
// visible. "x" removes the +X half (look at the cut from +X), "y"
// removes the +Y half (look at the cut from +Y), "none" leaves the
// assembly intact. Set via -D SECTION='"x"' on the command line.
SECTION = "none";

module assembly_full() {
    // Top shell at world Z (panel-mating face at z=0, panel face at z=PANEL_T).
    if (SHELL_XRAY)
        %color(COLOR_PRINTED) top_shell();
    else
        color(COLOR_PRINTED) top_shell();

    // Bottom shell, translated so its mating face joins the top-shell back rim.
    translate([0, 0, PANEL_T - TOP_DEPTH]) {
        if (SHELL_XRAY)
            %color(COLOR_PRINTED) bottom_shell();
        else
            color(COLOR_PRINTED) bottom_shell();
    }

    // Battery cover seated against the recess shoulder. Its outer face sits
    // BATT_COVER_Z_GAP below the panel exterior — the visible step is the
    // print-tolerance gap, not a geometry mismatch.
    translate([BATT_POS_X, BATT_POS_Y,
               PANEL_T - TOP_DEPTH - BOTTOM_DEPTH
                   + BATT_COVER_RECESS + BATT_COVER_Z_GAP
                   - BATT_COVER_T/2])
        battery_cover();

    // Battery interior lid — sits at the top of the corner bosses.
    // Boss top in world Z = PANEL_T - TOP_DEPTH - BOTTOM_DEPTH + PANEL_T + BATT_BOSS_HEIGHT
    translate([BATT_POS_X, BATT_POS_Y,
               PANEL_T - TOP_DEPTH - BOTTOM_DEPTH + PANEL_T
                   + BATT_BOSS_HEIGHT + BATT_LID_T/2])
        battery_lid();

    // LiPo cell sitting on the back-panel-interior, centred in its slot.
    // Wires hidden — they're flexible and routed wherever needed.
    translate([BATT_POS_X, BATT_POS_Y,
               PANEL_T - TOP_DEPTH - BOTTOM_DEPTH + PANEL_T])
        lipo_2s_2000mah(anchor=BOTTOM, show_wires=false);

    // USB-C breakout PCB — sits on its standoffs in the bottom shell with
    // the receptacle aligned with the -Y wall cutout. Long PCB axis runs
    // along case Y (rotated -90° from the breakout's local +X = cable axis).
    translate([USBC_POS_X, USBC_PCB_CENTER_Y,
               PANEL_T - TOP_DEPTH - BOTTOM_DEPTH + PANEL_T
                   + USBC_STANDOFF_H + 1.6/2])
        rotate([0, 0, -90])
            usb_c_breakout(anchor=BOTTOM);

    // All vitamins and sub-PCBs at their layout positions; suppress the
    // mock translucent panel because the real shell already provides it.
    transmitter_layout_front(show_mock_panel=false);
}

module assembly_check() {
    if (SECTION == "none") {
        assembly_full();
    } else {
        // Big cuboid covers one half of the case; subtracting it slices the
        // assembly through the centre so the interior is exposed.
        difference() {
            assembly_full();
            if (SECTION == "x")
                translate([0, 0, 0]) cuboid([400, 400, 400], anchor=LEFT);
            else if (SECTION == "y")
                translate([0, 0, 0]) cuboid([400, 400, 400], anchor=FRONT);
        }
    }
}

// =============================================================================
// Standalone preview
// =============================================================================
SHOW_STANDALONE = true;

if (SHOW_STANDALONE) assembly_check();
