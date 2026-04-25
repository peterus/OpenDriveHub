// Assembly check: render top shell + all front-panel components + sub-PCBs
// together to visually verify every cutout aligns with its component.
//
// Use this view as a regression check when adjusting layout positions or
// case dimensions. If a component clips into the shell wall (no cutout
// where it should be), it's immediately visible.

include <BOSL2/std.scad>
include <parameters.scad>

use <shell_top.scad>
use <../parts/layout_front.scad>

// SHELL_XRAY: when true, render the shell as a translucent background (lets
// you see PCBs and below-panel component bodies for cutout verification).
// When false, render solid — only above-panel features (lever balls,
// encoder shafts, illuminated caps, etc.) remain visible.
SHELL_XRAY = false;

module assembly_check() {
    if (SHELL_XRAY)
        %color(COLOR_PRINTED) shell_top();
    else
        color(COLOR_PRINTED) shell_top();

    // All vitamins and sub-PCBs at their layout positions; suppress the
    // mock translucent panel because the real shell already provides it.
    transmitter_layout_front(show_mock_panel=false);
}

// =============================================================================
// Standalone preview
// =============================================================================
SHOW_STANDALONE = true;

if (SHOW_STANDALONE) assembly_check();
