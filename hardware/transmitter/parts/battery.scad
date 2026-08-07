// 2S LiPo pouch battery, 2000mAh nominal.
//
// IMPORTANT: pouch dimensions vary by ±2mm between manufacturers. Verify
// against your actual cell with a caliper before finalizing the battery
// compartment. Wire exit orientation also varies (center vs corner).
//
// Conventions:
//   - Origin: center of the pouch body.
//   - +X is the direction the wires exit the sealed edge.
//   - Anchor "wire_exit" sits on the sealed edge where wires emerge.

include <BOSL2/std.scad>
include <parameters.scad>
include <utils.scad>

// BATT_BODY now lives in parts/parameters.scad (shared with case/bottom_shell.scad).
BATT_EDGE_SEAL   = 4;              // sealed edge depth at wire end
BATT_CORNER_R    = 2;              // pouch corner rounding (Z-axis edges)
BATT_WIRE_D      = 1.8;            // OD of 18AWG silicone wire w/ insulation
BATT_WIRE_LEN    = 80;             // typical lead length
BATT_WIRE_SEP    = 6;              // red-to-black center spacing
BATT_BAL_PLUG    = [8, 4, 5.5];    // JST-XH 2S balance plug (3-pin, ~2.5mm pitch)
BATT_BAL_OFFSET  = 10;             // Y-offset of balance plug from power-wire centerline

// Estimated mass for BOM / weight budget
BATT_MASS_G = 32;

// -----------------------------------------------------------------------------
// Main module: the LiPo cell as a visualization vitamin.
// -----------------------------------------------------------------------------
module lipo_2s_2000mah(anchor=CENTER, spin=0, orient=UP, show_wires=true) {
    size = BATT_BODY;
    anchors = [
        named_anchor("wire_exit", [size.x/2, 0, 0], RIGHT),
        named_anchor("label_face", [0, 0, size.z/2], UP),
    ];

    attachable(anchor, spin, orient, size=size, anchors=anchors) {
        _lipo_2s_geometry(show_wires=show_wires);
        children();
    }
}

module _lipo_2s_geometry(show_wires=true) {
    // Main pouch body — silver-metallic foil
    color("#b0b0a8")
        cuboid(BATT_BODY, rounding=BATT_CORNER_R, edges="Z")
            attach(RIGHT, LEFT)
                color("#a8a8a0")
                    cuboid([BATT_EDGE_SEAL, BATT_BODY.y*0.92, BATT_EDGE_SEAL*0.7]);

    if (show_wires) _lipo_2s_wires();
}

module _lipo_2s_wires() {
    // Wire bundle originates just past the sealed edge
    wire_start_x = BATT_BODY.x/2 + BATT_EDGE_SEAL;

    // Red wire (+)
    color("#c02020")
        translate([wire_start_x, BATT_WIRE_SEP/2, 0])
            cyl(d=BATT_WIRE_D, l=BATT_WIRE_LEN, orient=RIGHT, anchor=LEFT);

    // Black wire (-)
    color("#202020")
        translate([wire_start_x, -BATT_WIRE_SEP/2, 0])
            cyl(d=BATT_WIRE_D, l=BATT_WIRE_LEN, orient=RIGHT, anchor=LEFT);

    // Balance connector (JST-XH 2S) at end of wire bundle, offset in +Y
    color(COLOR_PLASTIC)
        translate([wire_start_x + BATT_WIRE_LEN - BATT_BAL_PLUG.x/2, BATT_BAL_OFFSET, 0])
            cuboid(BATT_BAL_PLUG);

    // Balance wires (thin, gray) from seal to plug
    for (offset_y = [0, BATT_WIRE_SEP * 0.4, BATT_WIRE_SEP * 0.8])
        color("#808080")
            translate([wire_start_x + (BATT_WIRE_LEN - BATT_BAL_PLUG.x)/2,
                       BATT_BAL_OFFSET - BATT_WIRE_SEP * 0.4 + offset_y, 0])
                cyl(d=0.9, l=BATT_WIRE_LEN - BATT_BAL_PLUG.x, orient=RIGHT, anchor=CENTER);
}

// -----------------------------------------------------------------------------
// Standalone preview — override via MCP variables or OpenSCAD -D flag.
// -----------------------------------------------------------------------------
SHOW_STANDALONE = true;
SHOW_SECTION    = false;

if (SHOW_STANDALONE) {
    if (SHOW_SECTION) back_half(s=300) lipo_2s_2000mah();
    else lipo_2s_2000mah();
}
