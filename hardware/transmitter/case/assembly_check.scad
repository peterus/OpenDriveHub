// Assembly check: render top shell + all front-panel components + sub-PCBs
// together to visually verify every cutout aligns with its component.
//
// Use this view as a regression check when adjusting layout positions or
// case dimensions. If a component clips into the shell wall (no cutout
// where it should be), it's immediately visible.

include <BOSL2/std.scad>
include <parameters.scad>

use <shell_top.scad>
use <bottom_shell.scad>
use <battery_cover.scad>
use <../parts/layout_front.scad>

// SHELL_XRAY: when true, render the shell as a translucent background (lets
// you see PCBs and below-panel component bodies for cutout verification).
// When false, render solid — only above-panel features (lever balls,
// encoder shafts, illuminated caps, etc.) remain visible.
SHELL_XRAY = false;

module assembly_check() {
    // Top shell at world Z (panel-mating face at z=0, panel face at z=PANEL_T).
    if (SHELL_XRAY)
        %color(COLOR_PRINTED) shell_top();
    else
        color(COLOR_PRINTED) shell_top();

    // Bottom shell, translated so its mating face joins the top-shell back rim.
    translate([0, 0, PANEL_T - TOP_DEPTH]) {
        if (SHELL_XRAY)
            %color(COLOR_PRINTED) bottom_shell();
        else
            color(COLOR_PRINTED) bottom_shell();
    }

    // Battery cover sitting in its recess on the bottom-shell back panel.
    // World Z of the back-panel exterior:
    //   PANEL_T - TOP_DEPTH (= top-shell back rim) + (-BOTTOM_DEPTH)
    //   = -(TOP_DEPTH + BOTTOM_DEPTH - PANEL_T)
    translate([BATT_POS_X, BATT_POS_Y,
               PANEL_T - TOP_DEPTH - BOTTOM_DEPTH
                   + BATT_COVER_RECESS - BATT_COVER_T/2])
        battery_cover();

    // All vitamins and sub-PCBs at their layout positions; suppress the
    // mock translucent panel because the real shell already provides it.
    transmitter_layout_front(show_mock_panel=false);
}

// =============================================================================
// Standalone preview
// =============================================================================
SHOW_STANDALONE = true;

if (SHOW_STANDALONE) assembly_check();
