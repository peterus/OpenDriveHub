// USB Type-C receptacle, mid-mount SMD 16-pin (standard hobby part).
//
// Generic USB-C receptacle model — dimensions match most horizontal SMD
// receptacles used on DIY PCBs (e.g. GCT USB4105 family, generic clones).
// Only the body + mouth opening are modeled; PCB pads are abstracted.
//
// Conventions:
//   - Origin: center of the connector body.
//   - +X is cable-insertion direction (the user pushes the plug in +X;
//     the opening is on the +X face).
//   - +Z is up (thin axis of the connector).
//   - Anchor "panel" sits at the cable-entry face (+X), useful to pin against
//     the inside of a case wall.
//   - Anchor "pcb_bottom" sits at the SMD mounting plane (part underside).

include <BOSL2/std.scad>
include <parameters.scad>
include <utils.scad>

// ----- USB-C receptacle nominal dimensions -----
USBC_BODY       = [7.3, 8.94, 3.22];  // body [x=depth-in-connector, y=width, z=thickness]
USBC_MOUTH      = [2.6, 8.5, 2.0];    // cable-opening recess depth/width/height
USBC_MOUTH_Z    = 0;                   // mouth Z center (on body axis)
USBC_SHELL      = COLOR_METAL;        // metal shield
USBC_INSULATOR  = "#f0f0f0";           // white plastic tongue inside
USBC_TONGUE     = [5.5, 6.8, 0.7];     // visible plastic tongue inside the mouth
USBC_PANEL_CUT  = [9.0, 3.4];          // panel cutout W×H (Y×Z when connector is oriented along X)

module usb_c_receptacle(anchor=CENTER, spin=0, orient=UP) {
    size = USBC_BODY;
    anchors = [
        named_anchor("panel",      [size.x/2, 0, 0], RIGHT),
        named_anchor("pcb_bottom", [0, 0, -size.z/2], DOWN),
    ];
    attachable(anchor, spin, orient, size=size, anchors=anchors) {
        _usb_c_geometry();
        children();
    }
}

module _usb_c_geometry() {
    // Metal shell with the cable-entry mouth milled out
    color(USBC_SHELL)
        difference() {
            cuboid(USBC_BODY, rounding=0.5, edges="X");
            // Mouth recess on +X face
            translate([USBC_BODY.x/2 - USBC_MOUTH.x/2 + 0.01, 0, USBC_MOUTH_Z])
                cuboid([USBC_MOUTH.x + 0.02, USBC_MOUTH.y, USBC_MOUTH.z],
                       rounding=0.8, edges="X");
        }

    // Plastic tongue inside the mouth
    color(USBC_INSULATOR)
        translate([USBC_BODY.x/2 - USBC_TONGUE.x/2, 0, 0])
            cuboid(USBC_TONGUE);
}

// -----------------------------------------------------------------------------
// Panel cutout helper. Origin is the outside face of the panel; the cable
// goes in +X direction through the cutout.
// Oriented with Y = cutout width, Z = cutout height (matching USBC_PANEL_CUT).
// -----------------------------------------------------------------------------
module usb_c_panel_cutout(panel_thick=2) {
    // Cutout extrudes along X (cable direction)
    translate([-0.1, 0, 0])
        cuboid([panel_thick + 0.2, USBC_PANEL_CUT.x, USBC_PANEL_CUT.y],
               anchor=LEFT, rounding=0.4, edges="X");
}

// -----------------------------------------------------------------------------
// Standalone preview.
// -----------------------------------------------------------------------------
SHOW_STANDALONE = true;
SHOW_SECTION    = false;

if (SHOW_STANDALONE) {
    if (SHOW_SECTION) back_half(s=50) usb_c_receptacle();
    else usb_c_receptacle();
}
