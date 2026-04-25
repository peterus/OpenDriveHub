// USB-C female breakout PCB — generic AliExpress 17mm-pitch board.
// Source ref:
//   https://de.aliexpress.com/item/1005003311189170.html  (17mm hole pitch)
//
// IMPORTANT: only the 17mm hole pitch is confirmed. The remaining
// dimensions are typical for this class of board — verify with a caliper
// against the actual unit before printing the final case.
//
// Conventions:
//   - Origin: PCB top face (where the receptacle sits), centred along the
//     PCB outline.
//   - +X: along the long PCB axis, USB-C plug entry direction (the cable
//     plugs in from +X).
//   - +Y: along the short PCB axis (across the long axis).
//   - +Z: out of the PCB top face (where the receptacle sits).
//   - Anchor "panel": cable-entry face of the receptacle, oriented +X.

include <BOSL2/std.scad>
include <parameters.scad>
include <utils.scad>

// ----- PCB outline -----
USBC_PCB         = [22, 12, 1.6];   // [length, width, thickness]
USBC_HOLE_PITCH  = 17;              // confirmed from product listing
USBC_HOLE_D      = 2.2;             // M2 through-hole with light clearance
USBC_HOLE_INSET  = (USBC_PCB.x - USBC_HOLE_PITCH) / 2;   // = 2.5 mm

// ----- USB-C receptacle ------
USBC_CONN_W      = 9.0;             // body width (along Y)
USBC_CONN_H      = 3.3;             // body height (above PCB top)
USBC_CONN_LEN    = 7.5;             // body length along X
USBC_CONN_PROTR  = 1.5;             // mm the receptacle pokes past the +X PCB edge
USBC_OPENING_W   = 9.0;             // USB-C female cavity W
USBC_OPENING_H   = 3.3;             // USB-C female cavity H

// Total bounding box for attachable() — includes the protruding receptacle.
USBC_TOTAL_LEN   = USBC_PCB.x + USBC_CONN_PROTR;
USBC_BODY_TOP_Z  = USBC_PCB.z/2 + USBC_CONN_H;

// =============================================================================
module usb_c_breakout(anchor=CENTER, spin=0, orient=UP) {
    size = [USBC_TOTAL_LEN, USBC_PCB.y, USBC_PCB.z + USBC_CONN_H];
    // "panel" anchor: front face of the receptacle (cable entry side).
    anchors = [
        named_anchor("panel",
                     [USBC_TOTAL_LEN/2 - size.x/2 + USBC_TOTAL_LEN/2,
                      0,
                      0],
                     RIGHT),
    ];

    attachable(anchor, spin, orient, size=size, anchors=anchors) {
        // Centre of the assembled bounding box; shift PCB so the receptacle
        // protrusion sits at the +X edge of the bounding box.
        translate([-USBC_CONN_PROTR/2, 0, -USBC_CONN_H/2]) {
            difference() {
                color(COLOR_PCB)
                    cuboid(USBC_PCB, anchor=CENTER);
                // Mounting holes through the PCB
                for (sx = [-1, 1])
                    translate([sx * USBC_HOLE_PITCH/2, 0, 0])
                        cyl(d=USBC_HOLE_D, l=USBC_PCB.z + 0.2);
            }
            // USB-C receptacle body: sits on the PCB top, protrudes past +X.
            color("#a8a8a8")
                translate([USBC_PCB.x/2 - USBC_CONN_LEN/2 + USBC_CONN_PROTR,
                           0,
                           USBC_PCB.z/2 + USBC_CONN_H/2])
                    cuboid([USBC_CONN_LEN, USBC_CONN_W, USBC_CONN_H]);
        }
        children();
    }
}

// =============================================================================
// Panel cutout: USB-C opening only. Position relative to the breakout's
// "panel" anchor — pass panel_thick = wall thickness to be cut.
// Includes a small +Z offset to align with the receptacle opening above PCB.
// =============================================================================
module usb_c_breakout_panel_cutout(panel_thick=WALL_T, fit=FIT_FREE) {
    // The opening sits above the PCB top by the receptacle body height.
    // Centre of the opening along Z = USBC_CONN_H/2 above PCB top (= 0).
    // For panel cutting we extrude through the wall along the cable axis.
    translate([0, 0, USBC_PCB.z/2 + USBC_CONN_H/2 - USBC_CONN_H/2])
        cuboid([panel_thick + 0.4,
                USBC_OPENING_W + 2*fit,
                USBC_OPENING_H + 2*fit],
               anchor=CENTER);
}

// =============================================================================
// Standalone preview
// =============================================================================
SHOW_STANDALONE = true;

if (SHOW_STANDALONE) usb_c_breakout();
