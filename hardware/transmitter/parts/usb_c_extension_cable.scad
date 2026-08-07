// USB-C extension cable: female panel-mount on one end with 2 mounting
// screws, USB-C male plug on the other end, flexible cable in between.
//
// Source ref:
//   https://de.aliexpress.com/item/1005005682013267.html  (17mm screw pitch)
//
// IMPORTANT: only the 17mm screw pitch is confirmed. Other dimensions
// are typical for this style of panel-mount USB-C cable — verify against
// the actual unit before printing the final case.
//
// Convention (panel-mount female end):
//   - Origin: centre of the housing flange's outer face (the user-visible
//     side, where the cable plugs in).
//   - +X: along the screw-pitch axis.
//   - +Y: into the case interior (away from the user).
//   - +Z: vertical, perpendicular to the screw axis.
//   - Anchor "flange": outer face of the housing flange, oriented -Y
//     (toward the user).

include <BOSL2/std.scad>
include <parameters.scad>
include <utils.scad>

// ----- Female panel-mount housing -----
USBC_HOUSING_W      = 22;       // along screw-pitch axis (X)
USBC_HOUSING_H      = 9;        // vertical (Z)
USBC_HOUSING_DEPTH  = 5;        // thickness into case (Y)
USBC_SCREW_PITCH    = 17;       // confirmed from product listing

// USB-C female opening (recessed into the flange face)
USBC_OPENING_W      = 9.0;      // along X
USBC_OPENING_H      = 3.3;      // along Z
USBC_OPENING_DEPTH  = 4;        // recess depth into the housing

// Cable
USBC_CABLE_OD       = 4;
USBC_CABLE_LEN      = 200;      // a typical short extension; visualisation only

// =============================================================================
// Female panel-mount housing — for visualisation in the case assembly.
module usb_c_extension_panel(anchor=CENTER, spin=0, orient=UP) {
    size = [USBC_HOUSING_W, USBC_HOUSING_DEPTH, USBC_HOUSING_H];
    anchors = [
        named_anchor("flange", [0, -USBC_HOUSING_DEPTH/2, 0], FRONT),
    ];

    attachable(anchor, spin, orient, size=size, anchors=anchors) {
        union() {
            difference() {
                // Black plastic housing
                color(COLOR_PLASTIC)
                    cuboid([USBC_HOUSING_W, USBC_HOUSING_DEPTH, USBC_HOUSING_H],
                           rounding=0.5, edges="Y");
                // Female opening, recessed into the flange face (-Y side)
                translate([0, -USBC_HOUSING_DEPTH/2 + USBC_OPENING_DEPTH/2 - 0.05, 0])
                    cuboid([USBC_OPENING_W, USBC_OPENING_DEPTH + 0.1, USBC_OPENING_H],
                           rounding=0.5, edges="Y");
                // 2 mounting screw holes through the flange (M2)
                for (sx = [-1, 1])
                    translate([sx*USBC_SCREW_PITCH/2, 0, 0])
                        rotate([90, 0, 0])
                            cyl(d=2.4, l=USBC_HOUSING_DEPTH + 0.2, anchor=CENTER);
            }
            // Cable stub exiting the back (+Y)
            color(COLOR_PLASTIC)
                translate([0, USBC_HOUSING_DEPTH/2, 0])
                    rotate([-90, 0, 0])
                        cyl(d=USBC_CABLE_OD, l=15, anchor=BOTTOM);
        }
        children();
    }
}

// =============================================================================
// Standalone preview
// =============================================================================
SHOW_STANDALONE = true;

if (SHOW_STANDALONE) usb_c_extension_panel();
