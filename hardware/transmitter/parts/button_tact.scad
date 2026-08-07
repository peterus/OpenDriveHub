// Small non-illuminated tactile pushbutton — the kind used for menu nav.
//
// Models a standard 6x6mm square tact switch with a slightly raised plastic
// cap. 4-pin through-hole or SMD; we show the through-hole variant.
//
// IMPORTANT: Exact heights vary — some caps are flush, some are 3mm proud.
// Adjust BTN_T_CAP.z for your specific switch.
//
// Conventions:
//   - Origin: center of attachable bounding volume.
//   - +Z is toward the user (cap).
//   - Anchor "panel" sits at the panel-mating face.

include <BOSL2/std.scad>
include <parameters.scad>
include <utils.scad>

// ----- Tact button nominal dimensions (verify) -----
BTN_T_BODY      = [6, 6, 3.5];    // switch body below panel [x, y, z]
BTN_T_CAP       = [6, 6, 2.5];    // cap above panel
BTN_T_PIN_PITCH = 4.5;            // pin-to-pin spacing (across body, both rows)
BTN_T_PIN_L     = 3.0;            // pin length below body
BTN_T_PANEL_CUT = 6.8;            // round panel cutout Ø (or use square 6.5x6.5)

module button_tact(anchor=CENTER, spin=0, orient=UP) {
    total_z = BTN_T_BODY.z + BTN_T_PIN_L + BTN_T_CAP.z;
    size    = [BTN_T_BODY.x, BTN_T_BODY.y, total_z];
    anchors = [
        named_anchor("panel",     [0, 0, -size.z/2 + BTN_T_BODY.z + BTN_T_PIN_L], UP),
        named_anchor("terminals", [0, 0, -size.z/2], DOWN),
    ];
    attachable(anchor, spin, orient, size=size, anchors=anchors) {
        down(size.z/2 - BTN_T_BODY.z - BTN_T_PIN_L)
            _btn_tact_geometry();
        children();
    }
}

module _btn_tact_geometry() {
    // Body
    color(COLOR_PLASTIC)
        down(BTN_T_BODY.z)
            cuboid(BTN_T_BODY, anchor=BOTTOM);

    // 4 pins
    color(COLOR_BRASS)
        down(BTN_T_BODY.z + BTN_T_PIN_L/2)
            for (sx = [-1, 1], sy = [-1, 1])
                translate([sx * BTN_T_PIN_PITCH/2, sy * BTN_T_PIN_PITCH/2, 0])
                    cuboid([0.6, 0.6, BTN_T_PIN_L]);

    // Cap (plastic, slightly rounded top)
    color("#e0e0e0")
        cuboid(BTN_T_CAP, anchor=BOTTOM, rounding=0.3, edges="Z");
}

module button_tact_panel_cutout(panel_thick=2) {
    translate([0, 0, -0.1])
        cyl(d=BTN_T_PANEL_CUT, l=panel_thick + 0.2, anchor=BOTTOM);
}

SHOW_STANDALONE = true;
SHOW_SECTION    = false;

if (SHOW_STANDALONE) {
    if (SHOW_SECTION) back_half(s=200) button_tact();
    else button_tact();
}
