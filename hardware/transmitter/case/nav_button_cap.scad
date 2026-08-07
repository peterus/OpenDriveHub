// Printed button cap for the nav-row tact switches.
//
// Sits on top of the 6×6 mm built-in plastic cap of a standard tact
// switch (parts/button_tact.scad). The cap pokes through the panel
// hole and provides a finger-pressable surface ~2 mm above the panel
// exterior. Held in place by a press-fit socket on its underside —
// no glue, no captive geometry.
//
// Print orientation: stand on the socket (top face up). FDM-friendly,
// no overhangs.
//
// Coordinate convention:
//   z = 0     bottom of cap (socket opening = panel-mating face)
//   z > 0     up toward the user

include <BOSL2/std.scad>
include <parameters.scad>

// ----- Cap dimensions -----
// Shaft fits through the round panel cutout (BTN_T_PANEL_CUT = 6.8mm)
// with FIT_FREE clearance. Total height puts the top face 2mm proud
// of the panel exterior.
NBC_SHAFT_OD     = 6.8 - 2*FIT_FREE;  // ≈ 6.1
NBC_HEIGHT       = 5.0;                // socket bottom → top face
NBC_TOP_R        = 0.5;                // top-edge rounding for finger feel

// Socket envelopes the 6×6 tact cap with a press fit so the printed
// cap doesn't fall off in normal use.
NBC_SOCKET_W     = 6.0 - 2*FIT_PRESS;  // ≈ 6.16, slight interference
NBC_SOCKET_DEPTH = 2.5;                // = BTN_T_CAP.z

module nav_button_cap() {
    color(COLOR_PRINTED)
        difference() {
            cyl(d=NBC_SHAFT_OD, l=NBC_HEIGHT,
                rounding2=NBC_TOP_R, anchor=BOTTOM);
            // Press-fit socket cut from the bottom face.
            translate([0, 0, -0.1])
                cuboid([NBC_SOCKET_W, NBC_SOCKET_W,
                        NBC_SOCKET_DEPTH + 0.1],
                       anchor=BOTTOM);
        }
}

// =============================================================================
// Standalone preview
// =============================================================================
SHOW_STANDALONE = true;

if (SHOW_STANDALONE) nav_button_cap();
