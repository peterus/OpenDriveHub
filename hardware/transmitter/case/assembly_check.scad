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

module assembly_check() {
    // Top shell (translucent so components remain visible through the panel)
    %color(COLOR_PRINTED) shell_top();

    // All vitamins and sub-PCBs at their layout positions
    transmitter_layout_front();
}

// =============================================================================
// Standalone preview
// =============================================================================
SHOW_STANDALONE = true;

if (SHOW_STANDALONE) assembly_check();
