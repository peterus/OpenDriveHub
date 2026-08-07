// EC11 rotary encoder with integrated push-button.
//
// 5-pin through-hole: 3 encoder (A, B, C/common) + 2 SPST momentary push.
// Pressing the shaft down closes the integrated switch.
//
// IMPORTANT: Body/bushing dimensions vary between manufacturers. These
// numbers fit the most common EC11 family (Alps, Bourns, generic clones).
// Verify against your specific encoder with a caliper.
//
// Conventions:
//   - Origin: center of attachable bounding volume.
//   - +Z points along the shaft (user-facing).
//   - Anchor "panel" sits on the panel-mating face (body top).
//   - Anchor "terminals" sits at the tip of the solder pins (bottom).
//   - D-flat on shaft is on the +X side (for set-screw knob alignment).

include <BOSL2/std.scad>
include <parameters.scad>
include <utils.scad>

// ----- EC11+switch nominal dimensions (verify) -----
EC_BODY        = [12.5, 13.4, 6.5];   // body below panel [x, y, z]
EC_BUSH_D      = 7;                    // M7×0.75 threaded bushing OD
EC_BUSH_H      = 7;                    // bushing above body (clears panel + nut)
EC_NUT_AF      = 10;                   // hex panel nut across-flats
EC_NUT_H       = 1.5;
EC_SHAFT_D     = 6;                    // shaft OD
EC_SHAFT_L     = 20;                   // bushing-top to shaft-tip
EC_SHAFT_FLAT  = 0.5;                  // D-cut depth (0 = round shaft)
EC_PIN_ROW_Y   = 2.5;                  // gap between encoder-row and switch-row
EC_PIN_PITCH   = 2.5;                  // spacing between pins in a row
EC_PIN_L       = 3.5;                  // pin length below body
EC_PANEL_CUT   = 7.2;                  // panel cutout Ø (bushing + clearance)

module ec11_encoder(anchor=CENTER, spin=0, orient=UP) {
    total_z = EC_BODY.z + EC_PIN_L + EC_BUSH_H + EC_SHAFT_L;
    nut_od  = EC_NUT_AF / cos(30);
    size    = [max(EC_BODY.x, nut_od), max(EC_BODY.y, nut_od), total_z];
    anchors = [
        named_anchor("panel",     [0, 0, -size.z/2 + EC_BODY.z + EC_PIN_L], UP),
        named_anchor("terminals", [0, 0, -size.z/2], DOWN),
    ];
    attachable(anchor, spin, orient, size=size, anchors=anchors) {
        down(size.z/2 - EC_BODY.z - EC_PIN_L)
            _ec11_geometry();
        children();
    }
}

module _ec11_geometry() {
    // Body (below panel)
    color(COLOR_PLASTIC)
        down(EC_BODY.z)
            cuboid(EC_BODY, anchor=BOTTOM);

    // Pin array: encoder row (3 pins) + switch row (2 pins)
    color(COLOR_BRASS)
        down(EC_BODY.z + EC_PIN_L/2) {
            back(EC_PIN_ROW_Y/2)
                for (x = [-EC_PIN_PITCH, 0, EC_PIN_PITCH])
                    translate([x, 0, 0])
                        cuboid([0.6, 0.6, EC_PIN_L]);
            fwd(EC_PIN_ROW_Y/2)
                for (x = [-EC_PIN_PITCH/2, EC_PIN_PITCH/2])
                    translate([x, 0, 0])
                        cuboid([0.6, 0.6, EC_PIN_L]);
        }

    // Threaded bushing
    color(COLOR_METAL)
        cyl(d=EC_BUSH_D, l=EC_BUSH_H, anchor=BOTTOM);

    // Hex panel nut (shown sitting on body-top for clarity)
    color(COLOR_METAL)
        up(EC_NUT_H/2)
            zrot(30)
                cyl(d=EC_NUT_AF / cos(30), l=EC_NUT_H, $fn=6, anchor=CENTER);

    // Shaft with optional D-flat on +X side
    up(EC_BUSH_H)
        color(COLOR_METAL)
            difference() {
                cyl(d=EC_SHAFT_D, l=EC_SHAFT_L, anchor=BOTTOM);
                if (EC_SHAFT_FLAT > 0)
                    translate([EC_SHAFT_D/2 - EC_SHAFT_FLAT, 0, EC_SHAFT_L/2])
                        cuboid([EC_SHAFT_D, EC_SHAFT_D + 2, EC_SHAFT_L + 0.2], anchor=LEFT);
            }
}

// -----------------------------------------------------------------------------
// Panel cutout helper.
// -----------------------------------------------------------------------------
module ec11_encoder_panel_cutout(panel_thick=2) {
    translate([0, 0, -0.1])
        cyl(d=EC_PANEL_CUT, l=panel_thick + 0.2, anchor=BOTTOM);
}

// -----------------------------------------------------------------------------
// Standalone preview.
// -----------------------------------------------------------------------------
SHOW_STANDALONE = true;
SHOW_SECTION    = false;

if (SHOW_STANDALONE) {
    if (SHOW_SECTION) back_half(s=200) ec11_encoder();
    else ec11_encoder();
}
